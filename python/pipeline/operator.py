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

import threading
import multiprocessing
from paddle_serving_client import MultiLangClient, Client
from concurrent import futures
import logging
import func_timeout

from .channel import ThreadChannel, ProcessChannel, ChannelDataEcode, ChannelData, ChannelDataType
from .util import NameGenerator

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
                 retry=1):
        if name is None:
            name = _op_name_gen.next()
        self._is_run = False
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
        self._profiler = None

    def init_profiler(self, profiler):
        self._profiler = profiler

    def _profiler_record(self, string):
        if self._profiler is None:
            return
        self._profiler.record(string)

    def init_client(self, client_type, client_config, server_endpoints,
                    fetch_names):
        if self.with_serving == False:
            logging.debug("{} no client".format(self.name))
            return
        logging.debug("{} client_config: {}".format(self.name, client_config))
        logging.debug("{} fetch_names: {}".format(self.name, fetch_names))
        if client_type == 'brpc':
            self._client = Client()
        elif client_type == 'grpc':
            self._client = MultiLangClient()
        else:
            raise ValueError("unknow client type: {}".format(client_type))
        self._client.load_client_config(client_config)
        self._client.connect(server_endpoints)
        self._fetch_names = fetch_names

    def _get_input_channel(self):
        return self._input

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

    def _get_output_channels(self):
        return self._outputs

    def add_output_channel(self, channel):
        if not isinstance(channel, (ThreadChannel, ProcessChannel)):
            raise TypeError(
                self._log('output channel must be Channel type, not {}'.format(
                    type(channel))))
        channel.add_producer(self.name)
        self._outputs.append(channel)

    def preprocess(self, channeldata):
        # multiple previous Op
        if isinstance(channeldata, dict):
            raise NotImplementedError(
                'this Op has multiple previous inputs. Please override this method'
            )

        if channeldata.datatype is not ChannelDataType.CHANNEL_NPDATA.value:
            raise NotImplementedError(
                'datatype in channeldata is not CHANNEL_NPDATA({}). '
                'Please override this method'.format(channeldata.datatype))

        # get numpy dict
        feed_data = channeldata.parse()
        return feed_data

    def process(self, feed_dict):
        if not isinstance(feed_dict, dict):
            raise Exception(
                self._log(
                    'feed_dict must be dict type(the output of preprocess()), but get {}'.
                    format(type(feed_dict))))
        logging.debug(self._log('feed_dict: {}'.format(feed_dict)))
        logging.debug(self._log('fetch: {}'.format(self._fetch_names)))
        if isinstance(self._client, MultiLangClient):
            call_result = self._client.predict(
                feed=feed_dict, fetch=self._fetch_names)
            logging.debug(self._log("get call_result"))
        else:
            call_result = self._client.predict(
                feed=feed_dict, fetch=self._fetch_names)
            logging.debug(self._log("get fetch_dict"))
        return call_result

    def postprocess(self, fetch_dict):
        return fetch_dict

    def stop(self):
        self._input.stop()
        for channel in self._outputs:
            channel.stop()
        self._is_run = False

    def _parse_channeldata(self, channeldata):
        data_id, error_channeldata = None, None
        if isinstance(channeldata, dict):
            parsed_data = {}
            key = list(channeldata.keys())[0]
            data_id = channeldata[key].id
            for _, data in channeldata.items():
                if data.ecode != ChannelDataEcode.OK.value:
                    error_channeldata = data
                    break
        else:
            data_id = channeldata.id
            if channeldata.ecode != ChannelDataEcode.OK.value:
                error_channeldata = channeldata
        return data_id, error_channeldata

    def _push_to_output_channels(self, data, channels, name=None):
        if name is None:
            name = self.name
        for channel in channels:
            channel.push(data, name)

    def start_with_process(self, client_type):
        proces = []
        for concurrency_idx in range(self.concurrency):
            p = multiprocessing.Process(
                target=self._run,
                args=(concurrency_idx, self._get_input_channel(),
                      self._get_output_channels(), client_type))
            p.start()
            proces.append(p)
        return proces

    def start_with_thread(self, client_type):
        threads = []
        for concurrency_idx in range(self.concurrency):
            t = threading.Thread(
                target=self._run,
                args=(concurrency_idx, self._get_input_channel(),
                      self._get_output_channels(), client_type))
            t.start()
            threads.append(t)
        return threads

    def load_user_resources(self):
        pass

    def _run(self, concurrency_idx, input_channel, output_channels,
             client_type):
        def get_log_func(op_info_prefix):
            def log_func(info_str):
                return "{} {}".format(op_info_prefix, info_str)

            return log_func

        op_info_prefix = "[{}|{}]".format(self.name, concurrency_idx)
        log = get_log_func(op_info_prefix)
        tid = threading.current_thread().ident

        # create client based on client_type
        self.init_client(client_type, self._client_config,
                         self._server_endpoints, self._fetch_names)

        # load user resources
        self.load_user_resources()

        self._is_run = True
        while self._is_run:
            self._profiler_record("{}-get#{}_0".format(op_info_prefix, tid))
            channeldata = input_channel.front(self.name)
            self._profiler_record("{}-get#{}_1".format(op_info_prefix, tid))
            logging.debug(log("input_data: {}".format(channeldata)))

            data_id, error_channeldata = self._parse_channeldata(channeldata)

            # error data in predecessor Op
            if error_channeldata is not None:
                self._push_to_output_channels(error_channeldata,
                                              output_channels)
                continue

            # preprecess
            try:
                self._profiler_record("{}-prep#{}_0".format(op_info_prefix,
                                                            tid))
                preped_data = self.preprocess(channeldata)
                self._profiler_record("{}-prep#{}_1".format(op_info_prefix,
                                                            tid))
            except NotImplementedError as e:
                # preprocess function not implemented
                error_info = log(e)
                logging.error(error_info)
                self._push_to_output_channels(
                    ChannelData(
                        ecode=ChannelDataEcode.NOT_IMPLEMENTED.value,
                        error_info=error_info,
                        data_id=data_id),
                    output_channels)
                continue
            except TypeError as e:
                # Error type in channeldata.datatype
                error_info = log(e)
                logging.error(error_info)
                self._push_to_output_channels(
                    ChannelData(
                        ecode=ChannelDataEcode.TYPE_ERROR.value,
                        error_info=error_info,
                        data_id=data_id),
                    output_channels)
                continue
            except Exception as e:
                error_info = log(e)
                logging.error(error_info)
                self._push_to_output_channels(
                    ChannelData(
                        ecode=ChannelDataEcode.UNKNOW.value,
                        error_info=error_info,
                        data_id=data_id),
                    output_channels)
                continue

            # process
            midped_data = None
            if self.with_serving:
                ecode = ChannelDataEcode.OK.value
                self._profiler_record("{}-midp#{}_0".format(op_info_prefix,
                                                            tid))
                if self._timeout <= 0:
                    try:
                        midped_data = self.process(preped_data)
                    except Exception as e:
                        ecode = ChannelDataEcode.UNKNOW.value
                        error_info = log(e)
                        logging.error(error_info)
                else:
                    for i in range(self._retry):
                        try:
                            midped_data = func_timeout.func_timeout(
                                self._timeout,
                                self.process,
                                args=(preped_data, ))
                        except func_timeout.FunctionTimedOut as e:
                            if i + 1 >= self._retry:
                                ecode = ChannelDataEcode.TIMEOUT.value
                                error_info = log(e)
                                logging.error(error_info)
                            else:
                                logging.warn(
                                    log("timeout, retry({})".format(i + 1)))
                        except Exception as e:
                            ecode = ChannelDataEcode.UNKNOW.value
                            error_info = log(e)
                            logging.error(error_info)
                            break
                        else:
                            break
                if ecode != ChannelDataEcode.OK.value:
                    self._push_to_output_channels(
                        ChannelData(
                            ecode=ecode, error_info=error_info,
                            data_id=data_id),
                        output_channels)
                    continue
                self._profiler_record("{}-midp#{}_1".format(op_info_prefix,
                                                            tid))
                # op client return None
                if midped_data is None:
                    self._push_to_output_channels(
                        ChannelData(
                            ecode=ChannelDataEcode.CLIENT_ERROR.value,
                            error_info=log(
                                "predict failed. pls check the server side."),
                            data_id=data_id),
                        output_channels)
                    continue
            else:
                midped_data = preped_data

            # postprocess
            output_data = None
            self._profiler_record("{}-postp#{}_0".format(op_info_prefix, tid))
            try:
                postped_data = self.postprocess(midped_data)
            except Exception as e:
                ecode = ChannelDataEcode.UNKNOW.value
                error_info = log(e)
                logging.error(error_info)
                self._push_to_output_channels(
                    ChannelData(
                        ecode=ecode, error_info=error_info, data_id=data_id),
                    output_channels)
                continue
            if not isinstance(postped_data, dict):
                ecode = ChannelDataEcode.TYPE_ERROR.value
                error_info = log("output of postprocess funticon must be " \
                        "dict type, but get {}".format(type(postped_data)))
                logging.error(error_info)
                self._push_to_output_channels(
                    ChannelData(
                        ecode=ecode, error_info=error_info, data_id=data_id),
                    output_channels)
                continue

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
            self._profiler_record("{}-postp#{}_1".format(op_info_prefix, tid))

            # push data to channel (if run succ)
            self._profiler_record("{}-push#{}_0".format(op_info_prefix, tid))
            self._push_to_output_channels(output_data, output_channels)
            self._profiler_record("{}-push#{}_1".format(op_info_prefix, tid))

    def _log(self, info):
        return "{} {}".format(self.name, info)


class ReadOp(Op):
    def __init__(self, concurrency=1):
        # PipelineService.name = "#G"
        super(ReadOp, self).__init__(
            name="#G", input_ops=[], concurrency=concurrency)


class VirtualOp(Op):
    ''' For connecting two channels. '''

    def __init__(self, name, concurrency=1):
        super(VirtualOp, self).__init__(
            name=name, input_ops=None, concurrency=concurrency)
        self._virtual_pred_ops = []

    def add_virtual_pred_op(self, op):
        self._virtual_pred_ops.append(op)

    def add_output_channel(self, channel):
        if not isinstance(channel, (ThreadChannel, ProcessChannel)):
            raise TypeError(
                self._log('output channel must be Channel type, not {}'.format(
                    type(channel))))
        for op in self._virtual_pred_ops:
            channel.add_producer(op.name)
        self._outputs.append(channel)

    def _run(self, concurrency_idx, input_channel, output_channels,
             client_type):
        def get_log_func(op_info_prefix):
            def log_func(info_str):
                return "{} {}".format(op_info_prefix, info_str)

            return log_func

        op_info_prefix = "[{}|{}]".format(self.name, concurrency_idx)
        log = get_log_func(op_info_prefix)
        tid = threading.current_thread().ident

        self._is_run = True
        while self._is_run:
            self._profiler_record("{}-get#{}_0".format(op_info_prefix, tid))
            channeldata = input_channel.front(self.name)
            self._profiler_record("{}-get#{}_1".format(op_info_prefix, tid))

            self._profiler_record("{}-push#{}_0".format(op_info_prefix, tid))
            if isinstance(channeldata, dict):
                for name, data in channeldata.items():
                    self._push_to_output_channels(
                        data, channels=output_channels, name=name)
            else:
                self._push_to_output_channels(
                    channeldata,
                    channels=output_channels,
                    name=self._virtual_pred_ops[0].name)
            self._profiler_record("{}-push#{}_1".format(op_info_prefix, tid))
