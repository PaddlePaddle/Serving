#   Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# pylint: disable=doc-string-missing
from time import time as _time
import threading
import multiprocessing
from paddle_serving_client import MultiLangClient, Client
from concurrent import futures
import logging
import func_timeout
import os
import sys
import numpy as np
from numpy import *

from .proto import pipeline_service_pb2
from .channel import (ThreadChannel, ProcessChannel, ChannelDataEcode,
                      ChannelData, ChannelDataType, ChannelStopError,
                      ChannelTimeoutError)
from .util import NameGenerator
from .profiler import UnsafeTimeProfiler as TimeProfiler

_LOGGER = logging.getLogger()
_op_name_gen = NameGenerator("Op")


class Op(object):
    def __init__(self,
                 name=None,
                 input_ops=[],
                 server_endpoints=[],
                 fetch_list=[],
                 client_config=None,
                 concurrency=1,
                 timeout=-1,
                 retry=1,
                 batch_size=1,
                 auto_batching_timeout=None):
        if name is None:
            name = _op_name_gen.next()
        self.name = name  # to identify the type of OP, it must be globally unique
        self.concurrency = concurrency  # amount of concurrency
        self.set_input_ops(input_ops)

        self._server_endpoints = server_endpoints
        self.with_serving = False
        if len(self._server_endpoints) != 0:
            self.with_serving = True
        self._client_config = client_config
        self._fetch_names = fetch_list

        self._timeout = timeout
        self._retry = max(1, retry)
        self._input = None
        self._outputs = []

        self._batch_size = batch_size
        self._auto_batching_timeout = auto_batching_timeout
        if self._auto_batching_timeout is not None:
            if self._auto_batching_timeout <= 0 or self._batch_size == 1:
                self._auto_batching_timeout = None
            else:
                self._auto_batching_timeout = self._auto_batching_timeout / 1000.0

        self._server_use_profile = False

        # only for multithread
        self._for_init_op_lock = threading.Lock()
        self._for_close_op_lock = threading.Lock()
        self._succ_init_op = False
        self._succ_close_op = False

    def use_default_auto_batching_config(self):
        if self._batch_size != 1:
            _LOGGER.warn("Op({}) reset batch_size=1 (original: {})"
                         .format(self.name, self._batch_size))
            self._batch_size = 1
        if self._auto_batching_timeout != None:
            _LOGGER.warn(
                "Op({}) reset auto_batching_timeout=None (original: {})"
                .format(self.name, self._auto_batching_timeout))
            self._auto_batching_timeout = None

    def use_profiler(self, use_profile):
        self._server_use_profile = use_profile

    def init_client(self, client_type, client_config, server_endpoints,
                    fetch_names):
        if self.with_serving == False:
            _LOGGER.info("Op({}) no client".format(self.name))
            return None
        _LOGGER.info("Op({}) service endpoints: {}".format(self.name,
                                                           server_endpoints))
        _LOGGER.debug("Op({}) fetch_names: {}".format(self.name, fetch_names))
        if client_type == 'brpc':
            _LOGGER.debug("Op({}) client_config: {}".format(self.name,
                                                            client_config))
            client = Client()
            client.load_client_config(client_config)
        elif client_type == 'grpc':
            client = MultiLangClient()
        else:
            raise ValueError("unknow client type: {}".format(client_type))
        client.connect(server_endpoints)
        self._fetch_names = fetch_names
        return client

    def get_input_ops(self):
        return self._input_ops

    def set_input_ops(self, ops):
        if not isinstance(ops, list):
            ops = [] if ops is None else [ops]
        self._input_ops = []
        for op in ops:
            if not isinstance(op, Op):
                raise TypeError(
                    self._log('input op must be Op type, not {}'.format(
                        type(op))))
            self._input_ops.append(op)

    def add_input_channel(self, channel):
        if not isinstance(channel, (ThreadChannel, ProcessChannel)):
            raise TypeError(
                self._log('input channel must be Channel type, not {}'.format(
                    type(channel))))
        channel.add_consumer(self.name)
        self._input = channel

    def clean_input_channel(self):
        self._input = None

    def _get_input_channel(self):
        return self._input

    def add_output_channel(self, channel):
        if not isinstance(channel, (ThreadChannel, ProcessChannel)):
            raise TypeError(
                self._log('output channel must be Channel type, not {}'.format(
                    type(channel))))
        channel.add_producer(self.name)
        self._outputs.append(channel)

    def clean_output_channels(self):
        self._outputs = []

    def _get_output_channels(self):
        return self._outputs

    def preprocess(self, input_dicts):
        # multiple previous Op
        if len(input_dicts) != 1:
            raise NotImplementedError(
                'this Op has multiple previous inputs. Please override this func.'
            )

        (_, input_dict), = input_dicts.items()
        return input_dict

    def process(self, feed_batch):
        err, err_info = ChannelData.check_batch_npdata(feed_batch)
        if err != 0:
            raise NotImplementedError(
                "{} Please override preprocess func.".format(err_info))
        call_result = self.client.predict(
            feed=feed_batch, fetch=self._fetch_names)
        if isinstance(self.client, MultiLangClient):
            if call_result is None or call_result["serving_status_code"] != 0:
                return None
            call_result.pop("serving_status_code")
        return call_result

    def postprocess(self, input_dict, fetch_dict):
        return fetch_dict

    def _parse_channeldata(self, channeldata_dict):
        data_id, error_channeldata = None, None
        client_need_profile, profile_set = False, set()
        parsed_data = {}

        key = list(channeldata_dict.keys())[0]
        data_id = channeldata_dict[key].id
        client_need_profile = channeldata_dict[key].client_need_profile

        for name, data in channeldata_dict.items():
            if data.ecode != ChannelDataEcode.OK.value:
                error_channeldata = data
                break
            parsed_data[name] = data.parse()
            if client_need_profile:
                profile_set |= data.profile_data_set
        return (data_id, error_channeldata, parsed_data, client_need_profile,
                profile_set)

    def _push_to_output_channels(self,
                                 data,
                                 channels,
                                 name=None,
                                 profile_str=None,
                                 client_need_profile=False,
                                 profile_set=None):
        if name is None:
            name = self.name

        # add profile into channeldata
        if client_need_profile and profile_set is not None:
            if profile_str is not None:
                profile_set.add(profile_str)
            data.add_profile(profile_set)

        for channel in channels:
            channel.push(data, name)

    def start_with_process(self, client_type):
        proces = []
        for concurrency_idx in range(self.concurrency):
            p = multiprocessing.Process(
                target=self._run,
                args=(concurrency_idx, self._get_input_channel(),
                      self._get_output_channels(), client_type, False))
            p.start()
            proces.append(p)
        return proces

    def start_with_thread(self, client_type):
        threads = []
        for concurrency_idx in range(self.concurrency):
            t = threading.Thread(
                target=self._run,
                args=(concurrency_idx, self._get_input_channel(),
                      self._get_output_channels(), client_type, True))
            # When a process exits, it attempts to terminate
            # all of its daemonic child processes.
            t.daemon = True
            t.start()
            threads.append(t)
        return threads

    def init_op(self):
        pass

    def _run_preprocess(self, parsed_data_dict, log_func):
        _LOGGER.debug(log_func("try to run preprocess"))
        preped_data_dict = {}
        err_channeldata_dict = {}
        for data_id, parsed_data in parsed_data_dict.items():
            preped_data, error_channeldata = None, None
            try:
                preped_data = self.preprocess(parsed_data)
            except NotImplementedError as e:
                # preprocess function not implemented
                error_info = log_func("preprocess data[{}] failed: {}".format(
                    data_id, e))
                error_channeldata = ChannelData(
                    ecode=ChannelDataEcode.NOT_IMPLEMENTED.value,
                    error_info=error_info,
                    data_id=data_id)
            except TypeError as e:
                # Error type in channeldata.datatype
                error_info = log_func("preprocess data[{}] failed: {}".format(
                    data_id, e))
                _LOGGER.error(error_info)
                error_channeldata = ChannelData(
                    ecode=ChannelDataEcode.TYPE_ERROR.value,
                    error_info=error_info,
                    data_id=data_id)
            except Exception as e:
                error_info = log_func("preprocess data[{}] failed: {}".format(
                    data_id, e))
                _LOGGER.error(error_info)
                error_channeldata = ChannelData(
                    ecode=ChannelDataEcode.UNKNOW.value,
                    error_info=error_info,
                    data_id=data_id)
            if error_channeldata is not None:
                err_channeldata_dict[data_id] = error_channeldata
            else:
                preped_data_dict[data_id] = preped_data
        _LOGGER.debug(log_func("succ run preprocess"))
        return preped_data_dict, err_channeldata_dict

    def _run_process(self, preped_data_dict, log_func):
        _LOGGER.debug(log_func("try to run process"))
        midped_data_dict = {}
        err_channeldata_dict = {}
        if self.with_serving:
            data_ids = preped_data_dict.keys()
            feed_batch = [preped_data_dict[data_id] for data_id in data_ids]
            midped_batch = None
            ecode = ChannelDataEcode.OK.value
            if self._timeout <= 0:
                try:
                    midped_batch = self.process(feed_batch)
                except Exception as e:
                    ecode = ChannelDataEcode.UNKNOW.value
                    error_info = log_func("process batch failed: {}".format(e))
                    _LOGGER.error(error_info)
            else:
                for i in range(self._retry):
                    try:
                        midped_batch = func_timeout.func_timeout(
                            self._timeout, self.process, args=(feed_batch, ))
                    except func_timeout.FunctionTimedOut as e:
                        if i + 1 >= self._retry:
                            ecode = ChannelDataEcode.TIMEOUT.value
                            error_info = log_func(e)
                            _LOGGER.error(error_info)
                        else:
                            _LOGGER.warn(
                                log_func("PaddleService timeout, retry({}/{})"
                                         .format(i + 1, self._retry)))
                    except Exception as e:
                        ecode = ChannelDataEcode.UNKNOW.value
                        error_info = log_func("process batch failed: {}".format(
                            e))
                        _LOGGER.error(error_info)
                        break
                    else:
                        break
            if ecode != ChannelDataEcode.OK.value:
                for data_id in data_ids:
                    err_channeldata_dict[data_id] = ChannelData(
                        ecode=ecode, error_info=error_info, data_id=data_id)
            elif midped_batch is None:
                # op client return None
                error_info = log_func(
                    "predict failed. pls check the server side.")
                _LOGGER.error(error_info)
                for data_id in data_ids:
                    err_channeldata_dict[data_id] = ChannelData(
                        ecode=ChannelDataEcode.CLIENT_ERROR.value,
                        error_info=error_info,
                        data_id=data_id)
            else:
                # transform np format to dict format
                for idx, data_id in enumerate(data_ids):
                    midped_data_dict[data_id] = {
                        k: v[idx]
                        for k, v in midped_batch.items()
                    }
        else:
            midped_data_dict = preped_data_dict
        _LOGGER.debug(log_func("succ run process"))
        return midped_data_dict, err_channeldata_dict

    def _run_postprocess(self, parsed_data_dict, midped_data_dict, log_func):
        _LOGGER.debug(log_func("try to run postprocess"))
        postped_data_dict = {}
        err_channeldata_dict = {}
        for data_id, midped_data in midped_data_dict.items():
            postped_data, err_channeldata = None, None
            try:
                postped_data = self.postprocess(parsed_data_dict[data_id],
                                                midped_data)
            except Exception as e:
                error_info = log_func("postprocess data[{}] failed: {}"
                                      .format(data_id, e))
                _LOGGER.error(error_info)
                err_channeldata = ChannelData(
                    ecode=ChannelDataEcode.UNKNOW.value,
                    error_info=error_info,
                    data_id=data_id)
            if err_channeldata is not None:
                err_channeldata_dict[data_id] = err_channeldata
                continue
            else:
                if not isinstance(postped_data, dict):
                    error_info = log_func("output of postprocess funticon must be " \
                        "dict type, but get {}".format(type(postped_data)))
                    _LOGGER.error(error_info)
                    err_channeldata = ChannelData(
                        ecode=ChannelDataEcode.UNKNOW.value,
                        error_info=error_info,
                        data_id=data_id)
                    err_channeldata_dict[data_id] = err_channeldata
                    continue

                output_data = None
                err, _ = ChannelData.check_npdata(postped_data)
                if err == 0:
                    output_data = ChannelData(
                        ChannelDataType.CHANNEL_NPDATA.value,
                        npdata=postped_data,
                        data_id=data_id)
                else:
                    output_data = ChannelData(
                        ChannelDataType.DICT.value,
                        dictdata=postped_data,
                        data_id=data_id)
                postped_data_dict[data_id] = output_data
        _LOGGER.debug(log_func("succ run postprocess"))
        return postped_data_dict, err_channeldata_dict

    def _auto_batching_generator(self, input_channel, op_name, batch_size,
                                 timeout, log_func):
        while True:
            batch = []
            _LOGGER.debug(
                log_func("Auto-batching expect size: {}; timeout(s): {}".format(
                    batch_size, timeout)))
            while len(batch) == 0:
                endtime = None
                if timeout is not None:
                    endtime = _time() + timeout
                for idx in range(batch_size):
                    try:
                        channeldata_dict = None
                        if timeout is not None:
                            remaining = endtime - _time()
                            if remaining <= 0.0:
                                _LOGGER.debug(log_func("Auto-batching timeout"))
                                break
                            channeldata_dict = input_channel.front(op_name,
                                                                   timeout)
                        else:
                            channeldata_dict = input_channel.front(op_name)
                        batch.append(channeldata_dict)
                    except ChannelTimeoutError:
                        _LOGGER.debug(log_func("Auto-batching timeout"))
                        break
            _LOGGER.debug(
                log_func("Auto-batching actual size: {}".format(len(batch))))
            yield batch

    def _parse_channeldata_batch(self, batch, output_channels):
        parsed_data_dict = {}
        need_profile_dict = {}
        profile_dict = {}
        for channeldata_dict in batch:
            (data_id, error_channeldata, parsed_data,
                    client_need_profile, profile_set) = \
                            self._parse_channeldata(channeldata_dict)
            if error_channeldata is None:
                parsed_data_dict[data_id] = parsed_data
                need_profile_dict[data_id] = client_need_profile
                profile_dict[data_id] = profile_set
            else:
                # error data in predecessor Op
                # (error_channeldata with profile info)
                self._push_to_output_channels(error_channeldata,
                                              output_channels)

        return parsed_data_dict, need_profile_dict, profile_dict

    def _run(self, concurrency_idx, input_channel, output_channels, client_type,
             is_thread_op):
        def get_log_func(op_info_prefix):
            def log_func(info_str):
                return "{} {}".format(op_info_prefix, info_str)

            return log_func

        op_info_prefix = "[{}|{}]".format(self.name, concurrency_idx)
        log = get_log_func(op_info_prefix)
        tid = threading.current_thread().ident

        # init op
        profiler = None
        try:
            profiler = self._initialize(is_thread_op, client_type,
                                        concurrency_idx)
        except Exception as e:
            _LOGGER.error(log("init op failed: {}".format(e)))
            os._exit(-1)
        _LOGGER.info(log("succ init"))

        batch_generator = self._auto_batching_generator(
            input_channel=input_channel,
            op_name=self.name,
            batch_size=self._batch_size,
            timeout=self._auto_batching_timeout,
            log_func=log)

        while True:
            try:
                channeldata_dict_batch = next(batch_generator)
            except ChannelStopError:
                _LOGGER.debug(log("channel stop."))
                self._finalize(is_thread_op)
                break

            # parse channeldata batch
            try:
                parsed_data_dict, need_profile_dict, profile_dict \
                        = self._parse_channeldata_batch(
                                channeldata_dict_batch, output_channels)
            except ChannelStopError:
                _LOGGER.debug(log("channel stop."))
                self._finalize(is_thread_op)
                break
            if len(parsed_data_dict) == 0:
                # data in the whole batch is all error data
                continue

            # preprecess
            profiler.record("prep#{}_0".format(op_info_prefix))
            preped_data_dict, err_channeldata_dict \
                    = self._run_preprocess(parsed_data_dict, log)
            profiler.record("prep#{}_1".format(op_info_prefix))
            try:
                for data_id, err_channeldata in err_channeldata_dict.items():
                    self._push_to_output_channels(
                        data=err_channeldata,
                        channels=output_channels,
                        client_need_profile=need_profile_dict[data_id],
                        profile_set=profile_dict[data_id])
            except ChannelStopError:
                _LOGGER.debug(log("channel stop."))
                self._finalize(is_thread_op)
                break
            if len(parsed_data_dict) == 0:
                continue

            # process
            profiler.record("midp#{}_0".format(op_info_prefix))
            midped_data_dict, err_channeldata_dict \
                    = self._run_process(preped_data_dict, log)
            profiler.record("midp#{}_1".format(op_info_prefix))
            try:
                for data_id, err_channeldata in err_channeldata_dict.items():
                    self._push_to_output_channels(
                        data=err_channeldata,
                        channels=output_channels,
                        client_need_profile=need_profile_dict[data_id],
                        profile_set=profile_dict[data_id])
            except ChannelStopError:
                _LOGGER.debug(log("channel stop."))
                self._finalize(is_thread_op)
                break
            if len(midped_data_dict) == 0:
                continue

            # postprocess
            profiler.record("postp#{}_0".format(op_info_prefix))
            postped_data_dict, err_channeldata_dict \
                    = self._run_postprocess(
                            parsed_data_dict, midped_data_dict, log)
            profiler.record("postp#{}_1".format(op_info_prefix))
            try:
                for data_id, err_channeldata in err_channeldata_dict.items():
                    self._push_to_output_channels(
                        data=error_channeldata,
                        channels=output_channels,
                        client_need_profile=need_profile_dict[data_id],
                        profile_set=profile_dict[data_id])
            except ChannelStopError:
                _LOGGER.debug(log("channel stop."))
                self._finalize(is_thread_op)
                break
            if len(postped_data_dict) == 0:
                continue

            # push data to channel (if run succ)
            try:
                profile_str = profiler.gen_profile_str()
                for data_id, postped_data in postped_data_dict.items():
                    if self._server_use_profile:
                        sys.stderr.write(profile_str)
                    self._push_to_output_channels(
                        data=postped_data,
                        channels=output_channels,
                        profile_str=profile_str,
                        client_need_profile=need_profile_dict[data_id],
                        profile_set=profile_dict[data_id])
            except ChannelStopError:
                _LOGGER.debug(log("channel stop."))
                self._finalize(is_thread_op)
                break

    def _initialize(self, is_thread_op, client_type, concurrency_idx):
        if is_thread_op:
            with self._for_init_op_lock:
                if not self._succ_init_op:
                    # for the threaded version of Op, each thread cannot get its concurrency_idx
                    self.concurrency_idx = None
                    # init client
                    self.client = self.init_client(
                        client_type, self._client_config,
                        self._server_endpoints, self._fetch_names)
                    # user defined
                    self.init_op()
                    self._succ_init_op = True
                    self._succ_close_op = False
        else:
            self.concurrency_idx = concurrency_idx
            # init client
            self.client = self.init_client(client_type, self._client_config,
                                           self._server_endpoints,
                                           self._fetch_names)
            # user defined
            self.init_op()

        # use a separate TimeProfiler per thread or process
        profiler = TimeProfiler()
        profiler.enable(True)
        return profiler

    def _finalize(self, is_thread_op):
        if is_thread_op:
            with self._for_close_op_lock:
                if not self._succ_close_op:
                    self._profiler = None
                    self.client = None
                    self._succ_init_op = False
                    self._succ_close_op = True

    def _log(self, info):
        return "{} {}".format(self.name, info)


class RequestOp(Op):
    """ RequestOp do not run preprocess, process, postprocess. """

    def __init__(self):
        # PipelineService.name = "@G"
        super(RequestOp, self).__init__(name="@G", input_ops=[])
        # init op
        try:
            self.init_op()
        except Exception as e:
            _LOGGER.error("Op(Request) init op failed: {}".format(e))
            os._exit(-1)

    def unpack_request_package(self, request):
        dictdata = {}
        for idx, key in enumerate(request.key):
            data = request.value[idx]
            try:
                data = eval(data)
            except Exception as e:
                pass
            dictdata[key] = data
        return dictdata


class ResponseOp(Op):
    """ ResponseOp do not run preprocess, process, postprocess. """

    def __init__(self, input_ops):
        super(ResponseOp, self).__init__(name="@R", input_ops=input_ops)
        # init op
        try:
            self.init_op()
        except Exception as e:
            _LOGGER.error("Op(ResponseOp) init op failed: {}".format(e))
            os._exit(-1)

    def pack_response_package(self, channeldata):
        resp = pipeline_service_pb2.Response()
        resp.ecode = channeldata.ecode
        if resp.ecode == ChannelDataEcode.OK.value:
            if channeldata.datatype == ChannelDataType.CHANNEL_NPDATA.value:
                feed = channeldata.parse()
                # ndarray to string:
                # https://stackoverflow.com/questions/30167538/convert-a-numpy-ndarray-to-stringor-bytes-and-convert-it-back-to-numpy-ndarray
                np.set_printoptions(threshold=np.nan)
                for name, var in feed.items():
                    resp.value.append(var.__repr__())
                    resp.key.append(name)
            elif channeldata.datatype == ChannelDataType.DICT.value:
                feed = channeldata.parse()
                for name, var in feed.items():
                    if not isinstance(var, str):
                        resp.ecode = ChannelDataEcode.TYPE_ERROR.value
                        resp.error_info = self._log(
                            "fetch var type must be str({}).".format(
                                type(var)))
                        break
                    resp.value.append(var)
                    resp.key.append(name)
            else:
                resp.ecode = ChannelDataEcode.TYPE_ERROR.value
                resp.error_info = self._log(
                    "Error type({}) in datatype.".format(channeldata.datatype))
                _LOGGER.error(resp.error_info)
        else:
            resp.error_info = channeldata.error_info
        return resp


class VirtualOp(Op):
    ''' For connecting two channels. '''

    def __init__(self, name, concurrency=1):
        super(VirtualOp, self).__init__(
            name=name, input_ops=None, concurrency=concurrency)
        self._virtual_pred_ops = []

    def add_virtual_pred_op(self, op):
        self._virtual_pred_ops.append(op)

    def _actual_pred_op_names(self, op):
        if not isinstance(op, VirtualOp):
            return [op.name]
        names = []
        for x in op._virtual_pred_ops:
            names.extend(self._actual_pred_op_names(x))
        return names

    def add_output_channel(self, channel):
        if not isinstance(channel, (ThreadChannel, ProcessChannel)):
            raise TypeError(
                self._log('output channel must be Channel type, not {}'.format(
                    type(channel))))
        for op in self._virtual_pred_ops:
            for op_name in self._actual_pred_op_names(op):
                channel.add_producer(op_name)
        self._outputs.append(channel)

    def _run(self, concurrency_idx, input_channel, output_channels, client_type,
             is_thread_op):
        def get_log_func(op_info_prefix):
            def log_func(info_str):
                return "{} {}".format(op_info_prefix, info_str)

            return log_func

        op_info_prefix = "[{}|{}]".format(self.name, concurrency_idx)
        log = get_log_func(op_info_prefix)
        tid = threading.current_thread().ident

        while True:
            try:
                channeldata_dict = input_channel.front(self.name)
            except ChannelStopError:
                _LOGGER.debug(log("Channel stop."))
                break

            try:
                for name, data in channeldata_dict.items():
                    self._push_to_output_channels(
                        data, channels=output_channels, name=name)
            except ChannelStopError:
                _LOGGER.debug(log("Channel stop."))
                break
