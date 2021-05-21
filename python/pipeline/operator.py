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
import time
import threading
import multiprocessing
from paddle_serving_client import MultiLangClient, Client
from concurrent import futures
import logging
import func_timeout
import os
import sys
import collections
import numpy as np
import json
from numpy import *
if sys.version_info.major == 2:
    import Queue
elif sys.version_info.major == 3:
    import queue as Queue
else:
    raise Exception("Error Python version")

from .proto import pipeline_service_pb2
from .channel import (ThreadChannel, ProcessChannel, ChannelDataErrcode,
                      ChannelData, ChannelDataType, ChannelStopError,
                      ChannelTimeoutError, ProductErrCode)
from .util import NameGenerator
from .profiler import UnsafeTimeProfiler as TimeProfiler
from . import local_service_handler

_LOGGER = logging.getLogger(__name__)
_op_name_gen = NameGenerator("Op")


class Op(object):
    def __init__(self,
                 name=None,
                 input_ops=[],
                 server_endpoints=None,
                 fetch_list=None,
                 client_config=None,
                 client_type=None,
                 concurrency=None,
                 timeout=None,
                 retry=0,
                 batch_size=None,
                 auto_batching_timeout=None,
                 local_service_handler=None):
        # In __init__, all the parameters are just saved and Op is not initialized
        if name is None:
            name = _op_name_gen.next()
        self.name = name  # to identify the type of OP, it must be globally unique
        self.concurrency = concurrency  # amount of concurrency
        self.set_input_ops(input_ops)

        self._local_service_handler = local_service_handler
        self._server_endpoints = server_endpoints
        self._fetch_names = fetch_list
        self._client_config = client_config
        self.client_type = client_type
        self._timeout = timeout
        self._retry = max(1, retry)
        self._batch_size = batch_size
        self._auto_batching_timeout = auto_batching_timeout

        self._input = None
        self._outputs = []

        self._server_use_profile = False
        self._tracer = None

        # only for thread op
        self._for_init_op_lock = threading.Lock()
        self._for_close_op_lock = threading.Lock()
        self._succ_init_op = False
        self._succ_close_op = False

    def init_from_dict(self, conf):
        """
        Initializing one Op from config.yaml. If server_endpoints exist,
        which is remote RPC mode, otherwise it is local RPC mode. There
        are three types of predictios in local RPC mode, brpc, grpc and
        local_predictor.

        Args:
            conf: config.yaml

        Returns:
            None
        """
        # init op
        if self.concurrency is None:
            self.concurrency = conf["concurrency"]
        if self._retry is None:
            self._retry = conf["retry"]
        if self._fetch_names is None:
            self._fetch_names = conf.get("fetch_list")
        if self._client_config is None:
            self._client_config = conf.get("client_config")

        if self._timeout is None:
            self._timeout = conf["timeout"]
        if self._timeout > 0:
            self._timeout = self._timeout / 1000.0
        else:
            self._timeout = -1

        if self._batch_size is None:
            self._batch_size = conf["batch_size"]
        if self._auto_batching_timeout is None:
            self._auto_batching_timeout = conf["auto_batching_timeout"]
        if self._auto_batching_timeout <= 0 or self._batch_size == 1:
            _LOGGER.debug(
                self._log(
                    "Because auto_batching_timeout <= 0 or batch_size == 1,"
                    " set auto_batching_timeout to None."))
            self._auto_batching_timeout = None
        else:
            self._auto_batching_timeout = self._auto_batching_timeout / 1000.0

        self.model_config = None
        self.workdir = None
        self.thread_num = self.concurrency
        self.device_type = -1
        self.devices = ""
        self.mem_optim = False
        self.ir_optim = False
        self.precision = "fp32"
        if self._server_endpoints is None:
            server_endpoints = conf.get("server_endpoints", [])
            if len(server_endpoints) != 0:
                # remote service
                self.with_serving = True
                self._server_endpoints = server_endpoints
                self.client_type = conf["client_type"]
            else:
                if self._local_service_handler is None:
                    local_service_conf = conf.get("local_service_conf")
                    _LOGGER.info("local_service_conf: {}".format(
                        local_service_conf))
                    self.model_config = local_service_conf.get("model_config")
                    self.client_type = local_service_conf.get("client_type")
                    self.workdir = local_service_conf.get("workdir")
                    self.thread_num = local_service_conf.get("thread_num")
                    self.device_type = local_service_conf.get("device_type")
                    self.devices = local_service_conf.get("devices")
                    self.mem_optim = local_service_conf.get("mem_optim")
                    self.ir_optim = local_service_conf.get("ir_optim")
                    self._fetch_names = local_service_conf.get("fetch_list")
                    self.precision = local_service_conf.get("precision")
                    if self.model_config is None:
                        self.with_serving = False
                    else:
                        # local rpc service
                        self.with_serving = True
                        if self.client_type == "brpc" or self.client_type == "grpc":
                            service_handler = local_service_handler.LocalServiceHandler(
                                model_config=self.model_config,
                                client_type=self.client_type,
                                workdir=self.workdir,
                                thread_num=self.thread_num,
                                device_type=self.device_type,
                                devices=self.devices,
                                mem_optim=self.mem_optim,
                                ir_optim=self.ir_optim,
                                precision=self.precision)
                            service_handler.prepare_server()  # get fetch_list
                            serivce_ports = service_handler.get_port_list()
                            self._server_endpoints = [
                                "127.0.0.1:{}".format(p) for p in serivce_ports
                            ]
                            if self._client_config is None:
                                self._client_config = service_handler.get_client_config(
                                )
                            if self._fetch_names is None:
                                self._fetch_names = service_handler.get_fetch_list(
                                )
                        elif self.client_type == "local_predictor":
                            service_handler = local_service_handler.LocalServiceHandler(
                                model_config=self.model_config,
                                client_type=self.client_type,
                                workdir=self.workdir,
                                thread_num=self.thread_num,
                                device_type=self.device_type,
                                devices=self.devices,
                                fetch_names=self._fetch_names,
                                mem_optim=self.mem_optim,
                                ir_optim=self.ir_optim,
                                precision=self.precision)
                            if self._client_config is None:
                                self._client_config = service_handler.get_client_config(
                                )
                        self._local_service_handler = service_handler
                else:
                    self.with_serving = True
                    self._local_service_handler.prepare_server(
                    )  # get fetch_list
                    serivce_ports = self._local_service_handler.get_port_list()
                    self._server_endpoints = [
                        "127.0.0.1:{}".format(p) for p in serivce_ports
                    ]
                    if self._client_config is None:
                        self._client_config = self._local_service_handler.get_client_config(
                        )
                    if self._fetch_names is None:
                        self._fetch_names = self._local_service_handler.get_fetch_list(
                        )
        else:
            self.with_serving = True

        if not isinstance(self, RequestOp) and not isinstance(self, ResponseOp):
            _LOGGER.info(
                self._log("\n\tinput_ops: {},"
                          "\n\tserver_endpoints: {}"
                          "\n\tfetch_list: {}"
                          "\n\tclient_config: {}"
                          "\n\tconcurrency: {},"
                          "\n\ttimeout(s): {},"
                          "\n\tretry: {},"
                          "\n\tbatch_size: {},"
                          "\n\tauto_batching_timeout(s): {}".format(
                              ", ".join([op.name for op in self._input_ops
                                         ]), self._server_endpoints,
                              self._fetch_names, self._client_config,
                              self.concurrency, self._timeout, self._retry,
                              self._batch_size, self._auto_batching_timeout)))

    def launch_local_rpc_service(self):
        """
        Launching multiple local rpc servers.

        Args:
            None

        Returns:
            None
        """
        if self._local_service_handler is None:
            _LOGGER.warning(
                self._log("Failed to launch local rpc"
                          " service: local_service_handler is None."))
            return
        port = self._local_service_handler.get_port_list()
        #if self._local_service_handler.client_type == "local_predictor":
        #    _LOGGER.info("Op({}) use local predictor.")
        #    return
        self._local_service_handler.start_server()
        _LOGGER.info("Op({}) use local rpc service at port: {}"
                     .format(self.name, port))

    def use_default_auto_batching_config(self):
        """
        Set the auto batching config default.

        Args:
            None

        Returns:
            None
        """
        if self._batch_size != 1:
            _LOGGER.warning("Op({}) reset batch_size=1 (original: {})"
                            .format(self.name, self._batch_size))
            self._batch_size = 1
        if self._auto_batching_timeout != None:
            _LOGGER.warning(
                "Op({}) reset auto_batching_timeout=None (original: {})"
                .format(self.name, self._auto_batching_timeout))
            self._auto_batching_timeout = None

    def use_profiler(self, use_profile):
        self._server_use_profile = use_profile

    def set_tracer(self, tracer):
        self._tracer = tracer

    def init_client(self, client_config, server_endpoints):
        """
        Initialize the client object. There are three types of clients, brpc,
        grpc and local_predictor. In grpc or brpc mode, the client connects 
        endpoints.

        Args:
            client_config: client config info
            server_endpoints: server IP/Port list.

        Returns:
            client: client object.
        """
        if self.with_serving == False:
            _LOGGER.info("Op({}) has no client (and it also do not "
                         "run the process function)".format(self.name))
            return None
        if self.client_type == 'brpc':
            client = Client()
            client.load_client_config(client_config)
        elif self.client_type == 'grpc':
            client = MultiLangClient()
        elif self.client_type == 'local_predictor':
            if self.local_predictor is None:
                raise ValueError("local predictor not yet created")
            client = self.local_predictor
        else:
            raise ValueError("Failed to init client: unknow client "
                             "type {}".format(self.client_type))
        if self._fetch_names is None:
            self._fetch_names = client.fetch_names_
            _LOGGER.info("Op({}) has no fetch name set. So fetch all vars")
        if self.client_type != "local_predictor":
            client.connect(server_endpoints)
        return client

    def get_input_ops(self):
        return self._input_ops

    def set_input_ops(self, ops):
        """
        Set input ops.Each op have many input ops, but only one input
        channel.

        Args:
            ops: op list

        Returns:
            None.
        """
        if not isinstance(ops, list):
            ops = [] if ops is None else [ops]
        self._input_ops = []
        for op in ops:
            if not isinstance(op, Op):
                _LOGGER.critical(
                    self._log("Failed to set input_ops: input op "
                              "must be Op type, not {}".format(type(op))))
                os._exit(-1)
            self._input_ops.append(op)

    def add_input_channel(self, channel):
        """
        Adding one input channel to the Op. Each op have many front op,
        but, only one input channel.
        """
        if not isinstance(channel, (ThreadChannel, ProcessChannel)):
            _LOGGER.critical(
                self._log("Failed to set input_channel: input "
                          "channel must be Channel type, not {}".format(
                              type(channel))))
            os._exit(-1)
        channel.add_consumer(self.name)
        self._input = channel

    def clean_input_channel(self):
        self._input = None

    def _get_input_channel(self):
        return self._input

    def add_output_channel(self, channel):
        """
        Adding one output channel to the Op. Each op have many output channels,
        But only one front channel.

        Args:
            channel: an output channel object.

        Returns:
            None
        """
        if not isinstance(channel, (ThreadChannel, ProcessChannel)):
            _LOGGER.critical(
                self._log("Failed to add output_channel: output channel "
                          "must be Channel type, not {}".format(type(channel))))
            os._exit(-1)
        channel.add_producer(self.name)
        self._outputs.append(channel)

    def clean_output_channels(self):
        self._outputs = []

    def _get_output_channels(self):
        return self._outputs

    def preprocess(self, input_dicts, data_id=0, log_id=0):
        """
        In preprocess stage, assembling data for process stage. users can 
        override this function for model feed features.

        Args:
            input_dicts: input data to be preprocessed
            data_id: inner unique id, 0 default
            log_id: global unique id for RTT, 0 default

        Return:
            output_data: data for process stage
            is_skip_process: skip process stage or not, False default
            prod_errcode: None default, otherwise, product errores occured.
                          It is handled in the same way as exception. 
            prod_errinfo: "" default
        """
        # multiple previous Op
        if len(input_dicts) != 1:
            _LOGGER.critical(
                self._log(
                    "Failed to run preprocess: this Op has multiple previous "
                    "inputs. Please override this func."))
            os._exit(-1)

        (_, input_dict), = input_dicts.items()
        return input_dict, False, None, ""

    def process(self, feed_batch, typical_logid=0):
        """
        In process stage, send requests to the inference server or predict locally.
        users do not need to inherit this function
        Args:
            feed_batch: data to be fed to inference server
            typical_logid: mark batch predicts, usually the first logid in batch,
                0 default.

        Returns:
            call_result: predict result
        """
        err, err_info = ChannelData.check_batch_npdata(feed_batch)
        if err != 0:
            _LOGGER.critical(
                self._log("Failed to run process: {}. Please override "
                          "preprocess func.".format(err_info)))
            os._exit(-1)
        if self.client_type == "local_predictor":
            call_result = self.client.predict(
                feed=feed_batch[0],
                fetch=self._fetch_names,
                batch=True,
                log_id=typical_logid)
        else:
            call_result = self.client.predict(
                feed=feed_batch,
                fetch=self._fetch_names,
                batch=True,
                log_id=typical_logid)
        if isinstance(self.client, MultiLangClient):
            if call_result is None or call_result["serving_status_code"] != 0:
                return None
            call_result.pop("serving_status_code")
        return call_result

    def postprocess(self, input_data, fetch_data, log_id=0):
        """
        In postprocess stage, assemble data for next op or output.
        Args:
            input_data: data returned in preprocess stage, dict(for single predict) or list(for batch predict)
            fetch_data: data returned in process stage, dict(for single predict) or list(for batch predict)
            log_id: logid, 0 default

        Returns: 
            fetch_dict: fetch result must be dict type.
            prod_errcode: None default, otherwise, product errores occured.
                          It is handled in the same way as exception.
            prod_errinfo: "" default
        """
        fetch_dict = {}
        if isinstance(fetch_data, dict):
            fetch_dict = fetch_data
        return fetch_dict, None, ""

    def _parse_channeldata(self, channeldata_dict):
        """
        Parse one channeldata 
        Args:
            channeldata_dict : channel data to be parsed, dict type
        
        Return:
            data_id: created by dag._id_generator, unique
            error_channeldata: error channeldata
            parsed_data: get np/dict data from channeldata
            client_need_profile: need profile info
            profile_set: profile info
            log_id: logid for tracing a request 
        """
        data_id, error_channeldata = None, None
        client_need_profile, profile_set = False, set()
        parsed_data = {}

        key = list(channeldata_dict.keys())[0]
        data_id = channeldata_dict[key].id
        log_id = channeldata_dict[key].log_id
        client_need_profile = channeldata_dict[key].client_need_profile

        for name, data in channeldata_dict.items():
            if data.error_code != ChannelDataErrcode.OK.value:
                error_channeldata = data
                break
            parsed_data[name] = data.parse()
            if client_need_profile:
                profile_set |= data.profile_data_set
        return (data_id, error_channeldata, parsed_data, client_need_profile,
                profile_set, log_id)

    def _push_to_output_channels(self,
                                 data,
                                 channels,
                                 name=None,
                                 profile_str=None,
                                 client_need_profile=False,
                                 profile_set=None):
        """
        Push data to output channels, Do not run the later stage(preprocess,
        process, postprocess)
        Args:
            data: channeldata, to be pushed
            channels: output channels
            name: op name  
            profile_str: one profile message
            client_need_profile: False default
            profile_set: profile message collections

        Returns:
            None
        """
        if name is None:
            name = self.name

        # add profile into channeldata
        if client_need_profile and profile_set is not None:
            if profile_str is not None:
                profile_set.add(profile_str)
            data.add_profile(profile_set)

        for channel in channels:
            channel.push(data, name)

    def start_with_process(self):
        """
        Each OP creates a process to run the main loop, initializes the CUDA
        environment in each individual process.

        Args:
            None

        Returns:
            process array
        """
        trace_buffer = None
        if self._tracer is not None:
            trace_buffer = self._tracer.data_buffer()
        process = []
        for concurrency_idx in range(self.concurrency):
            p = multiprocessing.Process(
                target=self._run,
                args=(concurrency_idx, self._get_input_channel(),
                      self._get_output_channels(), False, trace_buffer,
                      self.model_config, self.workdir, self.thread_num,
                      self.device_type, self.devices, self.mem_optim,
                      self.ir_optim, self.precision))
            p.daemon = True
            p.start()
            process.append(p)
        return process

    def start_with_thread(self):
        """
        Each OP creates a thread to run the main loop, initializes the CUDA 
        environment in the main thread.

        Args:
            None
 
        Returns:
            thread array
        """
        trace_buffer = None
        if self._tracer is not None:
            trace_buffer = self._tracer.data_buffer()

        #Init cuda env in main thread
        if self.client_type == "local_predictor":
            _LOGGER.info("Init cuda env in main thread")
            self.local_predictor = self._local_service_handler.get_client(0)

        threads = []
        for concurrency_idx in range(self.concurrency):
            t = threading.Thread(
                target=self._run,
                args=(concurrency_idx, self._get_input_channel(),
                      self._get_output_channels(), True, trace_buffer,
                      self.model_config, self.workdir, self.thread_num,
                      self.device_type, self.devices, self.mem_optim,
                      self.ir_optim, self.precision))
            # When a process exits, it attempts to terminate
            # all of its daemonic child processes.
            t.daemon = True
            t.start()
            threads.append(t)
        return threads

    def init_op(self):
        pass

    def _run_preprocess(self, parsed_data_dict, op_info_prefix, logid_dict):
        """
        Run preprocess stage
        Args:
            parsed_data_dict: data to be pre-processed
            op_info_prefix: input op info
            logid_dict: logid dict

        Returns:
            preped_data_dict: data preprocessed, to be processed 
            err_channeldata_dict: when exceptions occurred, putting errors in it.
            skip_process_dict: skip process stage or not

        """
        _LOGGER.debug("{} Running preprocess".format(op_info_prefix))
        preped_data_dict = collections.OrderedDict()
        err_channeldata_dict = collections.OrderedDict()
        skip_process_dict = {}
        for data_id, parsed_data in parsed_data_dict.items():
            preped_data, error_channeldata = None, None
            is_skip_process = False
            prod_errcode, prod_errinfo = None, None
            log_id = logid_dict.get(data_id)
            try:
                preped_data, is_skip_process, prod_errcode, prod_errinfo = self.preprocess(
                    parsed_data, data_id, logid_dict.get(data_id))
                # Set skip_process_dict
                if is_skip_process is True:
                    skip_process_dict[data_id] = True
            except TypeError as e:
                # Error type in channeldata.datatype
                error_info = "(data_id={} log_id={}) {} Failed to preprocess: {}".format(
                    data_id, log_id, op_info_prefix, e)
                _LOGGER.error(error_info, exc_info=True)
                error_channeldata = ChannelData(
                    error_code=ChannelDataErrcode.TYPE_ERROR.value,
                    error_info=error_info,
                    data_id=data_id,
                    log_id=log_id)
            except Exception as e:
                error_info = "(data_id={} log_id={}) {} Failed to preprocess: {}".format(
                    data_id, log_id, op_info_prefix, e)
                _LOGGER.error(error_info, exc_info=True)
                error_channeldata = ChannelData(
                    error_code=ChannelDataErrcode.UNKNOW.value,
                    error_info=error_info,
                    data_id=data_id,
                    log_id=log_id)

            if prod_errcode is not None:
                # product errors occured
                error_channeldata = ChannelData(
                    error_code=ChannelDataErrcode.PRODUCT_ERROR.value,
                    error_info="",
                    prod_error_code=prod_errcode,
                    prod_error_info=prod_errinfo,
                    data_id=data_id,
                    log_id=log_id)

            if error_channeldata is not None:
                err_channeldata_dict[data_id] = error_channeldata
            else:
                preped_data_dict[data_id] = preped_data
        _LOGGER.debug("{} Succ preprocess".format(op_info_prefix))
        return preped_data_dict, err_channeldata_dict, skip_process_dict

    def _run_process(self, preped_data_dict, op_info_prefix, skip_process_dict,
                     logid_dict):
        """
        Run process stage
        Args:
            preped_data_dict: feed the data to be predicted by the model.  
            op_info_prefix: prefix op info
            skip_process_dict: skip process stage or not
            logid_dict: logid dict

        Returns:
            midped_data_dict: data midprocessed, to be post-processed 
            err_channeldata_dict: when exceptions occurred, putting errors in it 
        """
        _LOGGER.debug("{} Running process".format(op_info_prefix))
        midped_data_dict = collections.OrderedDict()
        err_channeldata_dict = collections.OrderedDict()
        is_skip_process = False
        data_ids = list(preped_data_dict.keys())

        # skip process stage
        if len(data_ids) == 1 and skip_process_dict.get(data_ids[0]) == True:
            is_skip_process = True
        if self.with_serving is False or is_skip_process is True:
            midped_data_dict = preped_data_dict
            _LOGGER.warning("(data_id={} log_id={}) OP={} skip process stage. " \
                "with_serving={}, is_skip_process={}".format(data_ids[0],
                logid_dict.get(data_ids[0]), self.name, self.with_serving,
                is_skip_process))
            return midped_data_dict, err_channeldata_dict

        # use typical_logid to mark batch data
        # data_ids is one self-increasing unique key. 
        typical_logid = data_ids[0]
        if len(data_ids) != 1:
            for data_id in data_ids:
                _LOGGER.info(
                    "(data_id={} logid={}) Auto-batching is On Op={}!!" \
                    "We selected logid={} (from batch: {}) as a " \
                    "representative for logging.".format(
                    data_id, logid_dict.get(data_id), self.name,
                    typical_logid, data_ids))

        one_input = preped_data_dict[data_ids[0]]
        feed_batch = []
        feed_dict = {}
        cur_offset = 0
        input_offset_dict = {}
        batch_input = False

        if isinstance(one_input, dict):
            # For dict type, data structure is dict.
            # Merge multiple dicts for data_ids into one dict.
            # feed_batch is the input param of predict func.
            # input_offset_dict is used for data restration[data_ids]
            if len(data_ids) == 1:
                feed_batch = [preped_data_dict[data_id] for data_id in data_ids]
            else:
                for data_id in data_ids:
                    for key, val in preped_data_dict[data_id].items():
                        has_val = feed_dict.get(key)
                        if has_val is None:
                            feed_dict[key] = val
                            continue
                        # merge 2 np.arrray
                        if isinstance(val, np.ndarray):
                            feed_dict[key] = np.append(
                                feed_dict[key], val, axis=0)
                feed_batch.append(feed_dict)

            for data_id in data_ids:
                start = cur_offset
                for key, val in preped_data_dict[data_id].items():
                    if isinstance(val, (list, np.ndarray)):
                        cur_offset += len(val)
                    else:
                        cur_offset += 1
                    break
                input_offset_dict[data_id] = [start, cur_offset]
        elif isinstance(one_input, list):
            # For list type, data structure of one_input is [dict, dict, ...]
            # Data structure of feed_batch is [dict1_1, dict1_2, dict2_1, ...]   
            # Data structure of input_offset_dict is { data_id : [start, end] }
            batch_input = True
            for data_id in data_ids:
                feed_batch.extend(preped_data_dict[data_id])
                data_size = len(preped_data_dict[data_id])
                start = cur_offset
                cur_offset = start + data_size
                input_offset_dict[data_id] = [start, cur_offset]
        else:
            _LOGGER.critical(
                "(data_id={} log_id={}){} Failed to process: expect input type is dict"
                " or list(batch input), but get {}".format(data_ids[
                    0], typical_logid, op_info_prefix, type(one_input)))
            for data_id in data_ids:
                error_code = ChannelDataErrcode.TYPE_ERROR.value
                error_info = "expect input type is dict or list, but get {}".format(
                    type(one_input))
                err_channeldata_dict[data_id] = ChannelData(
                    error_code=error_code,
                    error_info=error_info,
                    data_id=data_id,
                    log_id=logid_dict.get(data_id))
            return midped_data_dict, err_channeldata_dict

        midped_batch = None
        error_code = ChannelDataErrcode.OK.value
        if self._timeout <= 0:
            # No retry
            try:
                if batch_input is False:
                    midped_batch = self.process(feed_batch, typical_logid)
                else:
                    midped_batch = []
                    for idx in range(len(feed_batch)):
                        predict_res = self.process([feed_batch[idx]],
                                                   typical_logid)
                        midped_batch.append(predict_res)
            except Exception as e:
                error_code = ChannelDataErrcode.UNKNOW.value
                error_info = "(data_id={} log_id={}) {} Failed to process(batch: {}): {}".format(
                    data_ids[0], typical_logid, op_info_prefix, data_ids, e)
                _LOGGER.error(error_info, exc_info=True)
        else:
            # retry N times configed in yaml files.
            for i in range(self._retry):
                try:
                    # time out for each process
                    if batch_input is False:
                        midped_batch = func_timeout.func_timeout(
                            self._timeout,
                            self.process,
                            args=(feed_batch, typical_logid))
                    else:
                        midped_batch = []
                        for idx in range(len(feed_batch)):
                            predict_res = func_timeout.func_timeout(
                                self._timeout,
                                self.process,
                                args=([feed_batch[idx]], typical_logid))
                            midped_batch[idx].append(predict_res)

                except func_timeout.FunctionTimedOut as e:
                    if i + 1 >= self._retry:
                        error_code = ChannelDataErrcode.TIMEOUT.value
                        error_info = "(log_id={}) {} Failed to process(batch: {}): " \
                            "exceeded retry count.".format(typical_logid, op_info_prefix, data_ids)
                        _LOGGER.error(error_info)
                    else:
                        _LOGGER.warning(
                            "(log_id={}) {} Failed to process(batch: {}): timeout,"
                            " and retrying({}/{})...".format(
                                typical_logid, op_info_prefix, data_ids, i + 1,
                                self._retry))
                except Exception as e:
                    error_code = ChannelDataErrcode.UNKNOW.value
                    error_info = "(log_id={}) {} Failed to process(batch: {}): {}".format(
                        typical_logid, op_info_prefix, data_ids, e)
                    _LOGGER.error(error_info, exc_info=True)
                    break
                else:
                    break

        # 2 kinds of errors
        if error_code != ChannelDataErrcode.OK.value or midped_batch is None:
            error_info = "(log_id={}) {} failed to predict.".format(
                typical_logid, self.name)
            _LOGGER.error(error_info)
            for data_id in data_ids:
                err_channeldata_dict[data_id] = ChannelData(
                    error_code=ChannelDataErrcode.CLIENT_ERROR.value,
                    error_info=error_info,
                    data_id=data_id,
                    log_id=logid_dict.get(data_id))
            return midped_data_dict, err_channeldata_dict

        # Split batch infer result to each data_ids
        if batch_input is False:
            var_names = midped_batch.keys()
            lod_var_names = set()
            lod_offset_names = set()
            # midped_batch is dict type for single input 
            for name in var_names:
                lod_offset_name = "{}.lod".format(name)
                if lod_offset_name in var_names:
                    _LOGGER.debug("(log_id={}) {} {} is LodTensor".format(
                        typical_logid, op_info_prefix, name))
                    lod_var_names.add(name)
                    lod_offset_names.add(lod_offset_name)

            for idx, data_id in enumerate(data_ids):
                midped_data_dict[data_id] = {}

            for name, value in midped_batch.items():
                if name in lod_offset_names:
                    continue
                if name in lod_var_names:
                    # lodtensor
                    lod_offset_name = "{}.lod".format(name)
                    lod_offset = midped_batch[lod_offset_name]
                    for idx, data_id in enumerate(data_ids):
                        data_offset_left = input_offset_dict[data_id][0]
                        data_offset_right = input_offset_dict[data_id][1]
                        lod_offset_left = lod_offset[data_offset_left]
                        lod_offset_right = lod_offset[data_offset_right]
                        midped_data_dict[data_id][name] = value[
                            lod_offset_left:lod_offset_right]
                        midped_data_dict[data_id][lod_offset_name] = \
                            lod_offset[data_offset_left:data_offset_right + 1] - lod_offset[data_offset_left]
                else:
                    # normal tensor
                    for idx, data_id in enumerate(data_ids):
                        start = input_offset_dict[data_id][0]
                        end = input_offset_dict[data_id][1]
                        midped_data_dict[data_id][name] = value[start:end]
        else:
            # midped_batch is list type for batch input
            for idx, data_id in enumerate(data_ids):
                start = input_offset_dict[data_id][0]
                end = input_offset_dict[data_id][1]
                midped_data_dict[data_id] = midped_batch[start:end]
        return midped_data_dict, err_channeldata_dict

    def _run_postprocess(self, parsed_data_dict, midped_data_dict,
                         op_info_prefix, logid_dict):
        """
        Run postprocess stage.
        Args:
            parsed_data_dict: data returned in preprocess stage 
            midped_data_dict: data returned in process stage
            op_info_prefix: prefix op info
            logid_dict: logid dict

        Returns:
            postped_data_dict: data postprocessed 
            err_channeldata_dict: when exceptions occurred, putting errors in it
 
        """
        _LOGGER.debug("{} Running postprocess".format(op_info_prefix))
        postped_data_dict = collections.OrderedDict()
        err_channeldata_dict = collections.OrderedDict()
        for data_id, midped_data in midped_data_dict.items():
            log_id = logid_dict.get(data_id)
            postped_data, err_channeldata = None, None
            prod_errcode, prod_errinfo = None, None
            try:
                postped_data, prod_errcode, prod_errinfo = self.postprocess(
                    parsed_data_dict[data_id], midped_data,
                    logid_dict.get(data_id))
            except Exception as e:
                error_info = "(data_id={} log_id={}) {} Failed to postprocess: {}".format(
                    data_id, log_id, op_info_prefix, e)
                _LOGGER.error(error_info, exc_info=True)
                err_channeldata = ChannelData(
                    error_code=ChannelDataErrcode.UNKNOW.value,
                    error_info=error_info,
                    data_id=data_id,
                    log_id=log_id)

            if prod_errcode is not None:
                # product errors occured
                err_channeldata = ChannelData(
                    error_code=ChannelDataErrcode.PRODUCT_ERROR.value,
                    error_info="",
                    prod_error_code=prod_errcode,
                    prod_error_info=prod_errinfo,
                    data_id=data_id,
                    log_id=log_id)

            if err_channeldata is not None:
                err_channeldata_dict[data_id] = err_channeldata
                continue
            else:
                if not isinstance(postped_data, dict):
                    error_info = "(log_id={} log_id={}) {} Failed to postprocess: " \
                            "output of postprocess funticon must be " \
                            "dict type, but get {}".format(
                                data_id, log_id, op_info_prefix,
                                type(postped_data))
                    _LOGGER.error(error_info)
                    err_channeldata = ChannelData(
                        error_code=ChannelDataErrcode.UNKNOW.value,
                        error_info=error_info,
                        data_id=data_id,
                        log_id=log_id)
                    err_channeldata_dict[data_id] = err_channeldata
                    continue

                output_data = None
                err, _ = ChannelData.check_npdata(postped_data)
                if err == 0:
                    output_data = ChannelData(
                        ChannelDataType.CHANNEL_NPDATA.value,
                        npdata=postped_data,
                        data_id=data_id,
                        log_id=log_id)
                else:
                    output_data = ChannelData(
                        ChannelDataType.DICT.value,
                        dictdata=postped_data,
                        data_id=data_id,
                        log_id=log_id)
                postped_data_dict[data_id] = output_data
        _LOGGER.debug("{} Succ postprocess".format(op_info_prefix))
        return postped_data_dict, err_channeldata_dict

    def _auto_batching_generator(self, input_channel, op_name, batch_size,
                                 timeout, op_info_prefix):
        """
        Merge batch_size requests for one prediction.Taking one piece of data 
        from the input channel each time until equals batch_size, or the waiting 
        time exceeds auto_batching_timeout.

        Args:
            input_channel: the input channel of Op
            op_name: op name
            batch_size: batch size, Less than worker_num
            timeout: batch timeout, seconds, If timeout is None, and the quantity 
                taken from the front is less than batch_size, blocking occured.
            op_info_prefix: op link info.

        Returns:
            None
        """
        while True:
            batch = []
            while len(batch) == 0:
                endtime = None
                if timeout is not None:
                    endtime = _time() + timeout
                for idx in range(batch_size):
                    try:
                        channeldata_dict = None
                        front_start_time = int(round(_time() * 1000000))
                        if timeout is not None:
                            remaining = endtime - _time()
                            if remaining <= 0.0:
                                _LOGGER.debug("{} Failed to generate batch: "
                                              "timeout".format(op_info_prefix))
                                break
                            channeldata_dict = input_channel.front(op_name,
                                                                   timeout)
                        else:
                            channeldata_dict = input_channel.front(op_name)
                        batch.append(channeldata_dict)
                        _LOGGER.debug(
                            "_auto_batching_generator get {} channeldata from op:{} input channel. time={}".
                            format(idx, op_name, front_start_time))
                    except ChannelTimeoutError:
                        _LOGGER.debug("{} Failed to generate batch: "
                                      "timeout".format(op_info_prefix))
                        break
            _LOGGER.debug("{} Got actual batch_size: {}".format(op_info_prefix,
                                                                len(batch)))
            yield batch

    def _parse_channeldata_batch(self, batch, output_channels):
        """
        Parse channeldatas batch
        Args:
            batch: auto-batching batch datas
            output_channels: output channels 

        Returns:
            parsed_data_dict: parsed from channeldata in batch
            need_profile_dict: need profile dict in batch 
            profile_dict: profile info dict in batch
            logid_dict: trace each request in batch
        """
        parsed_data_dict = collections.OrderedDict()
        need_profile_dict = {}
        profile_dict = {}
        logid_dict = {}
        for channeldata_dict in batch:
            (data_id, error_channeldata, parsed_data,
                    client_need_profile, profile_set, log_id) = \
                            self._parse_channeldata(channeldata_dict)
            if error_channeldata is None:
                parsed_data_dict[data_id] = parsed_data
                need_profile_dict[data_id] = client_need_profile
                profile_dict[data_id] = profile_set
                logid_dict[data_id] = log_id
            else:
                # error data in predecessor Op
                # (error_channeldata with profile info)
                self._push_to_output_channels(error_channeldata,
                                              output_channels)

        return parsed_data_dict, need_profile_dict, profile_dict, logid_dict

    def _run(self, concurrency_idx, input_channel, output_channels,
             is_thread_op, trace_buffer, model_config, workdir, thread_num,
             device_type, devices, mem_optim, ir_optim, precision):
        """
        _run() is the entry function of OP process / thread model.When client 
        type is local_predictor in process mode, the CUDA environment needs to 
        be initialized by LocalServiceHandler[child process], otherwise, Cuda
        error(3), initialization error is occured. Preprocess, process and 
        postprocess are executed in the main loop. The preprocess and postprocess
        function is usually rewrited by users. Trace data is recorded by trace_que.

        Args:
            concurrency_idx: thread/process index
            input_channel: input channel, take the data to be processed
            output_channels: output channel, store processed data
            is_thread_op: False, It's process op; True, It's thread op
            trace_buffer: store trace infomations
            model_config: model config path
            workdir: work directory
            thread_num: number of threads, concurrent quantity
            device_type: support multiple devices
            devices: gpu id list[gpu], "" default[cpu]
            mem_optim: use memory/graphics memory optimization, True default.
            ir_optim: use calculation chart optimization, False default.
            precision: inference precision, e.g. "fp32", "fp16", "int8" 

        Returns:
            None
        """
        op_info_prefix = "[{}|{}]".format(self.name, concurrency_idx)

        # init ops
        profiler = None
        try:
            if is_thread_op == False and self.client_type == "local_predictor":
                self.service_handler = local_service_handler.LocalServiceHandler(
                    model_config=model_config,
                    client_type="local_predictor",
                    workdir=workdir,
                    thread_num=thread_num,
                    device_type=device_type,
                    devices=devices,
                    mem_optim=mem_optim,
                    ir_optim=ir_optim,
                    precision=precision)

                _LOGGER.info("Init cuda env in process {}".format(
                    concurrency_idx))
                self.local_predictor = self.service_handler.get_client(
                    concurrency_idx)
            # check all ops initialized successfully.
            profiler = self._initialize(is_thread_op, concurrency_idx)

        except Exception as e:
            _LOGGER.critical(
                "{} failed to init op: {}".format(op_info_prefix, e),
                exc_info=True)
            os._exit(-1)
        _LOGGER.info("{} Succ init".format(op_info_prefix))

        batch_generator = self._auto_batching_generator(
            input_channel=input_channel,
            op_name=self.name,
            batch_size=self._batch_size,
            timeout=self._auto_batching_timeout,
            op_info_prefix=op_info_prefix)

        start, end = None, None
        trace_que = collections.deque()
        while True:
            start = int(round(_time() * 1000000))
            try:
                channeldata_dict_batch = next(batch_generator)
            except ChannelStopError:
                _LOGGER.debug("{} Stop.".format(op_info_prefix))
                self._finalize(is_thread_op)
                break
            end = int(round(_time() * 1000000))
            in_time = end - start

            # parse channeldata batch
            try:
                parsed_data_dict, need_profile_dict, profile_dict, logid_dict\
                        = self._parse_channeldata_batch(
                                channeldata_dict_batch, output_channels)
            except ChannelStopError:
                _LOGGER.debug("{} Stop.".format(op_info_prefix))
                self._finalize(is_thread_op)
                break
            if len(parsed_data_dict) == 0:
                # data in the whole batch is all error data
                continue

            # print
            front_cost = int(round(_time() * 1000000)) - start
            for data_id, parsed_data in parsed_data_dict.items():
                _LOGGER.debug(
                    "(data_id={}) POP INPUT CHANNEL! op:{}, cost:{} ms".format(
                        data_id, self.name, front_cost / 1000.0))

            # preprecess
            start = profiler.record("prep#{}_0".format(op_info_prefix))
            preped_data_dict, err_channeldata_dict, skip_process_dict \
                    = self._run_preprocess(parsed_data_dict, op_info_prefix, logid_dict)
            end = profiler.record("prep#{}_1".format(op_info_prefix))
            prep_time = end - start
            try:
                # put error requests into output channel, skip process and postprocess stage
                for data_id, err_channeldata in err_channeldata_dict.items():
                    self._push_to_output_channels(
                        data=err_channeldata,
                        channels=output_channels,
                        client_need_profile=need_profile_dict[data_id],
                        profile_set=profile_dict[data_id])
            except ChannelStopError:
                _LOGGER.debug("{} Stop.".format(op_info_prefix))
                self._finalize(is_thread_op)
                break
            if len(preped_data_dict) == 0:
                continue

            # process
            start = profiler.record("midp#{}_0".format(op_info_prefix))
            midped_data_dict, err_channeldata_dict \
                    = self._run_process(preped_data_dict, op_info_prefix, skip_process_dict, logid_dict)
            end = profiler.record("midp#{}_1".format(op_info_prefix))
            midp_time = end - start
            try:
                for data_id, err_channeldata in err_channeldata_dict.items():
                    self._push_to_output_channels(
                        data=err_channeldata,
                        channels=output_channels,
                        client_need_profile=need_profile_dict[data_id],
                        profile_set=profile_dict[data_id])
            except ChannelStopError:
                _LOGGER.debug("{} Stop.".format(op_info_prefix))
                self._finalize(is_thread_op)
                break
            if len(midped_data_dict) == 0:
                continue

            # postprocess
            start = profiler.record("postp#{}_0".format(op_info_prefix))
            postped_data_dict, err_channeldata_dict \
                    = self._run_postprocess(parsed_data_dict, midped_data_dict, op_info_prefix, logid_dict)
            end = profiler.record("postp#{}_1".format(op_info_prefix))
            postp_time = end - start
            after_postp_time = _time()
            try:
                for data_id, err_channeldata in err_channeldata_dict.items():
                    self._push_to_output_channels(
                        data=err_channeldata,
                        channels=output_channels,
                        client_need_profile=need_profile_dict[data_id],
                        profile_set=profile_dict[data_id])
            except ChannelStopError:
                _LOGGER.debug("{} Stop.".format(op_info_prefix))
                self._finalize(is_thread_op)
                break
            if len(postped_data_dict) == 0:
                continue
            # push data to channel (if run succ)
            start = int(round(_time() * 1000000))
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
                    after_outchannel_time = _time()
                    _LOGGER.debug(
                        "(data_id={}) PUSH OUTPUT CHANNEL! op:{} push cost:{} ms".
                        format(data_id, self.name, (after_outchannel_time -
                                                    after_postp_time) * 1000))
                    _LOGGER.debug(
                        "(data_id={}) PUSH OUTPUT CHANNEL! op:{} push data:{}".
                        format(data_id, self.name, postped_data.get_all_data()))
            except ChannelStopError:
                _LOGGER.debug("{} Stop.".format(op_info_prefix))
                self._finalize(is_thread_op)
                break
            end = int(round(_time() * 1000000))
            out_time = end - start
            after_outchannel_time = int(round(_time() * 1000000))
            if trace_buffer is not None:
                trace_que.append({
                    "name": self.name,
                    "actions": {
                        "in": in_time,
                        "prep": prep_time,
                        "midp": midp_time,
                        "postp": postp_time,
                        "out": out_time,
                    }
                })
                while trace_que:
                    info = trace_que[0]
                    try:
                        trace_buffer.put_nowait(info)
                        trace_que.popleft()
                    except Queue.Full:
                        break

    def _initialize(self, is_thread_op, concurrency_idx):
        """
        Initialize one OP object in the target function of a thread or porcess.
        Initialize the client object with _client_config and _server_endpoints.
        Create a TimeProfiler per thread or process for recording profiler info.

        Args:
            is_thread_op: True, one op runs in one thread; False, one op runs
                in one process.
            concurrency_idx: process id, Thread mode does not use this param.

        Returns:
            TimeProfiler
        """
        if is_thread_op:
            with self._for_init_op_lock:
                if not self._succ_init_op:
                    # for the threaded version of Op, each thread cannot get its concurrency_idx
                    self.concurrency_idx = None
                    # init client
                    self.client = self.init_client(self._client_config,
                                                   self._server_endpoints)
                    # user defined
                    self.init_op()
                    self._succ_init_op = True
                    self._succ_close_op = False
        else:
            self.concurrency_idx = concurrency_idx
            # init client
            self.client = self.init_client(self._client_config,
                                           self._server_endpoints)
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
    """
    RequestOp is a special Op, for unpacking one request package. If the
    request needs one special unpackaging method, you need to inherit class
    RequestOp and rewrite function unpack_request_package.Notice!!! Class
    RequestOp does not run preprocess, process, postprocess.
    """

    def __init__(self):
        """
        Initialize the RequestOp
        """
        # PipelineService.name = "@DAGExecutor"
        super(RequestOp, self).__init__(name="@DAGExecutor", input_ops=[])
        # init op
        try:
            self.init_op()
        except Exception as e:
            _LOGGER.critical("Op(Request) Failed to init: {}".format(e))
            os._exit(-1)

    def unpack_request_package(self, request):
        """
        Unpack request package by gateway.proto
        Args:
            request: HTTP body, JSON format

        Returns:
            dict_data: json fields in HTTP body
            log_id: log_id
            prod_errcode: None or ProductErrCode.SUCC.value default, otherwise,
                          product errores occured.It is handled in the same way
                          as exception.
            prod_errinfo: "" default 
        """
        dict_data = {}
        log_id = None
        if request is None:
            _LOGGER.critical("request is None")
            raise ValueError("request is None")

        for idx, key in enumerate(request.key):
            dict_data[key] = request.value[idx]
        log_id = request.logid
        _LOGGER.info("RequestOp unpack one request. log_id:{}, clientip:{} \
            name:{}, method:{}".format(log_id, request.clientip, request.name,
                                       request.method))

        return dict_data, log_id, None, ""


class ResponseOp(Op):
    """ 
    ResponseOp is a special Op, for packing one response package. If the channeldata 
    needs a special packaging method, you need to inherit class ReponseOp and rewrite
    pack_response_package function. Notice!!! Class ResponseOp does not run preprocess,
    process, postprocess.
    """

    def __init__(self, input_ops):
        """
        Initialize the ResponseOp
        """
        super(ResponseOp, self).__init__(
            name="@DAGExecutor", input_ops=input_ops)
        # init op
        try:
            self.init_op()
        except Exception as e:
            _LOGGER.critical("Op(ResponseOp) Failed to init: {}".format(
                e, exc_info=True))
            os._exit(-1)

    def pack_response_package(self, channeldata):
        """
        Getting channeldata from the last channel, packting the response 
        package serialized by protobuf.  

        Args:
            channeldata: Type ChannelData

        Returns:
            resp: pipeline_service_pb2.Response()
        """
        resp = pipeline_service_pb2.Response()
        error_code = channeldata.error_code
        error_info = ""
        if error_code == ChannelDataErrcode.OK.value:
            # Framework level errors
            if channeldata.datatype == ChannelDataType.CHANNEL_NPDATA.value:
                feed = channeldata.parse()
                # ndarray to string:
                # https://stackoverflow.com/questions/30167538/convert-a-numpy-ndarray-to-stringor-bytes-and-convert-it-back-to-numpy-ndarray
                np.set_printoptions(threshold=sys.maxsize)
                for name, var in feed.items():
                    resp.value.append(var.__repr__())
                    resp.key.append(name)
            elif channeldata.datatype == ChannelDataType.DICT.value:
                feed = channeldata.parse()
                for name, var in feed.items():
                    if not isinstance(var, str):
                        error_code = ChannelDataErrcode.TYPE_ERROR.value
                        error_info = self._log(
                            "fetch var type must be str({}).".format(
                                type(var)))
                        _LOGGER.error("(logid={}) Failed to pack RPC "
                                      "response package: {}".format(
                                          channeldata.id, resp.err_msg))
                        break
                    resp.value.append(var)
                    resp.key.append(name)
            else:
                error_code = ChannelDataErrcode.TYPE_ERROR.value
                error_info = self._log("error type({}) in datatype.".format(
                    channeldata.datatype))
                _LOGGER.error("(logid={}) Failed to pack RPC response"
                              " package: {}".format(channeldata.id, error_info))
        else:
            # Product level errors
            error_info = channeldata.error_info
            if error_code == ChannelDataErrcode.PRODUCT_ERROR.value:
                #rewrite error_code when product errors occured
                error_code = channeldata.prod_error_code
                error_info = channeldata.prod_error_info

        # pack results
        if error_code is None:
            error_code = 0
        resp.err_no = error_code
        resp.err_msg = error_info

        return resp


class VirtualOp(Op):
    """ 
    To connect 2 ops across levels in dag view, we create virtual ops
    between non-virtual ops, and transfer data only. For examples, 
    the pred ops of F are D & E.In the process of building DAG, we will
    create channels layer by layer according to dag views.Op F is not 
    in the next layer view of [B, E], so we will create a virtual OP 
    'V1' whose pred OP is E. And so on, we create two virtual op 'V2'
    and 'V3', Finally, we find the non-virtual op F. we create 4 channels
    among E, V1, V2, V3 and F, the producer of V1, V2, V3 and F is E.
    
        DAG: [A -> B -> C -> D -> F]
               \-> E ----------/

        DAG view: [[A], [B, E], [C], [D], [F]]
        BUILD DAG: [A -> B -> C -> D -> E -> F]
                     \-> E -> V1-> V2-> V3/
    """

    def __init__(self, name, concurrency=1):
        super(VirtualOp, self).__init__(
            name=name, input_ops=None, concurrency=concurrency)
        self._virtual_pred_ops = []

    def add_virtual_pred_op(self, op):
        """
        Add the front op of current vritual op.
        
        Args:
            op: one op object, may be a virtual op or not.

        Returns:
            None
        """
        self._virtual_pred_ops.append(op)

    def _actual_pred_op_names(self, op):
        """
        Recursively find the front op which is a non-virtual op.
   
        Args:
            op: one op object
            
        Returns:
            names: the name of non-virtual pred ops.
        """
        # can use disjoint-set, but it's not necessary
        if not isinstance(op, VirtualOp):
            return [op.name]
        names = []
        for x in op._virtual_pred_ops:
            names.extend(self._actual_pred_op_names(x))
        return names

    def add_output_channel(self, channel):
        """
        Adding the output channel of non-virtual pred ops.

        Args:
            channel: one channel.
          
        Returns:
            None.
        """
        if not isinstance(channel, (ThreadChannel, ProcessChannel)):
            _LOGGER.critical(
                self._log("Failed to add output_channel: output_channel"
                          " must be Channel type, not {}".format(
                              type(channel))))
            os._exit(-1)
        for op in self._virtual_pred_ops:
            for op_name in self._actual_pred_op_names(op):
                channel.add_producer(op_name)
        self._outputs.append(channel)

    def _run(self, concurrency_idx, input_channel, output_channels, client_type,
             is_thread_op):
        """
        The target function _run() only transfers data between OPs in one thread
        or process.

        Args:
            concurrency_idx: process id, not avaliable in thread mode.
            input_channel: input channel
            output_channels: output channels
            client_type: no use
            is_thread_op: True, thread mode; False, process mode

        Returns:
            None
        """
        op_info_prefix = "[{}|{}]".format(self.name, concurrency_idx)
        log = get_log_func(op_info_prefix)
        tid = threading.current_thread().ident

        batch_generator = self._auto_batching_generator(
            input_channel=input_channel,
            op_name=self.name,
            batch_size=1,
            timeout=None,
            log_func=log)

        while True:
            try:
                channeldata_dict_batch = next(batch_generator)
            except ChannelStopError:
                _LOGGER.debug("{} Stop.".format(op_info_prefix))
                self._finalize(is_thread_op)
                break

            try:
                for channeldata_dict in channeldata_dict_batch:
                    for name, data in channeldata_dict.items():
                        self._push_to_output_channels(
                            data, channels=output_channels, name=name)
            except ChannelStopError:
                _LOGGER.debug("{} Stop.".format(op_info_prefix))
                self._finalize(is_thread_op)
                break
