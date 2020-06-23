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
import multiprocessing.queues
import sys
if sys.version_info.major == 2:
    import Queue
elif sys.version_info.major == 3:
    import queue as Queue
else:
    raise Exception("Error Python version")
import os
from paddle_serving_client import MultiLangClient, Client
from concurrent import futures
import numpy as np
import grpc
from ..proto import general_python_service_pb2 as pyservice_pb2
from ..proto import pyserving_channel_pb2 as channel_pb2
from ..proto import general_python_service_pb2_grpc
import logging
import random
import time
import func_timeout
import enum
import collections
import copy

from .operator import Op, VirtualOp
from .channel import ThreadChannel, ProcessChannel, ChannelData, ChannelDataEcode, ChannelDataType
from .profiler import TimeProfiler

_profiler = TimeProfiler()


class GeneralPythonService(
        general_python_service_pb2_grpc.GeneralPythonServiceServicer):
    def __init__(self, in_channel, out_channel, retry=2):
        super(GeneralPythonService, self).__init__()
        self.name = "#G"
        self.set_in_channel(in_channel)
        self.set_out_channel(out_channel)
        logging.debug(self._log(in_channel.debug()))
        logging.debug(self._log(out_channel.debug()))
        #TODO: 
        #  multi-lock for different clients
        #  diffenert lock for server and client
        self._id_lock = threading.Lock()
        self._cv = threading.Condition()
        self._globel_resp_dict = {}
        self._id_counter = 0
        self._retry = retry
        self._recive_func = threading.Thread(
            target=GeneralPythonService._recive_out_channel_func, args=(self, ))
        self._recive_func.start()

    def _log(self, info_str):
        return "[{}] {}".format(self.name, info_str)

    def set_in_channel(self, in_channel):
        if not isinstance(in_channel, (ThreadChannel, ProcessChannel)):
            raise TypeError(
                self._log('in_channel must be Channel type, but get {}'.format(
                    type(in_channel))))
        in_channel.add_producer(self.name)
        self._in_channel = in_channel

    def set_out_channel(self, out_channel):
        if not isinstance(out_channel, (ThreadChannel, ProcessChannel)):
            raise TypeError(
                self._log('out_channel must be Channel type, but get {}'.format(
                    type(out_channel))))
        out_channel.add_consumer(self.name)
        self._out_channel = out_channel

    def _recive_out_channel_func(self):
        while True:
            channeldata = self._out_channel.front(self.name)
            if not isinstance(channeldata, ChannelData):
                raise TypeError(
                    self._log('data must be ChannelData type, but get {}'.
                              format(type(channeldata))))
            with self._cv:
                data_id = channeldata.id
                self._globel_resp_dict[data_id] = channeldata
                self._cv.notify_all()

    def _get_next_id(self):
        with self._id_lock:
            self._id_counter += 1
            return self._id_counter - 1

    def _get_data_in_globel_resp_dict(self, data_id):
        resp = None
        with self._cv:
            while data_id not in self._globel_resp_dict:
                self._cv.wait()
            resp = self._globel_resp_dict.pop(data_id)
            self._cv.notify_all()
        return resp

    def _pack_data_for_infer(self, request):
        logging.debug(self._log('start inferce'))
        data_id = self._get_next_id()
        npdata = {}
        try:
            for idx, name in enumerate(request.feed_var_names):
                logging.debug(
                    self._log('name: {}'.format(request.feed_var_names[idx])))
                logging.debug(
                    self._log('data: {}'.format(request.feed_insts[idx])))
                npdata[name] = np.frombuffer(
                    request.feed_insts[idx], dtype=request.type[idx])
                npdata[name].shape = np.frombuffer(
                    request.shape[idx], dtype="int32")
        except Exception as e:
            return ChannelData(
                ecode=ChannelDataEcode.RPC_PACKAGE_ERROR.value,
                error_info="rpc package error",
                data_id=data_id), data_id
        else:
            return ChannelData(
                datatype=ChannelDataType.CHANNEL_NPDATA.value,
                npdata=npdata,
                data_id=data_id), data_id

    def _pack_data_for_resp(self, channeldata):
        logging.debug(self._log('get channeldata'))
        resp = pyservice_pb2.Response()
        resp.ecode = channeldata.ecode
        if resp.ecode == ChannelDataEcode.OK.value:
            if channeldata.datatype == ChannelDataType.CHANNEL_PBDATA.value:
                for inst in channeldata.pbdata.insts:
                    resp.fetch_insts.append(inst.data)
                    resp.fetch_var_names.append(inst.name)
                    resp.shape.append(inst.shape)
                    resp.type.append(inst.type)
            elif channeldata.datatype in (ChannelDataType.CHANNEL_FUTURE.value,
                                          ChannelDataType.CHANNEL_NPDATA.value):
                feed = channeldata.parse()
                for name, var in feed.items():
                    resp.fetch_insts.append(var.tobytes())
                    resp.fetch_var_names.append(name)
                    resp.shape.append(
                        np.array(
                            var.shape, dtype="int32").tobytes())
                    resp.type.append(str(var.dtype))
            else:
                raise TypeError(
                    self._log("Error type({}) in datatype.".format(
                        channeldata.datatype)))
        else:
            resp.error_info = channeldata.error_info
        return resp

    def inference(self, request, context):
        _profiler.record("{}-prepack_0".format(self.name))
        data, data_id = self._pack_data_for_infer(request)
        _profiler.record("{}-prepack_1".format(self.name))

        resp_channeldata = None
        for i in range(self._retry):
            logging.debug(self._log('push data'))
            _profiler.record("{}-push_0".format(self.name))
            self._in_channel.push(data, self.name)
            _profiler.record("{}-push_1".format(self.name))

            logging.debug(self._log('wait for infer'))
            _profiler.record("{}-fetch_0".format(self.name))
            resp_channeldata = self._get_data_in_globel_resp_dict(data_id)
            _profiler.record("{}-fetch_1".format(self.name))

            if resp_channeldata.ecode == ChannelDataEcode.OK.value:
                break
            if i + 1 < self._retry:
                logging.warn("retry({}): {}".format(
                    i + 1, resp_channeldata.error_info))

        _profiler.record("{}-postpack_0".format(self.name))
        resp = self._pack_data_for_resp(resp_channeldata)
        _profiler.record("{}-postpack_1".format(self.name))
        _profiler.print_profile()
        return resp


class PipelineServer(object):
    def __init__(self,
                 use_multithread=True,
                 client_type='brpc',
                 use_future=False,
                 retry=2,
                 profile=False):
        self._channels = []
        self._user_ops = []
        self._actual_ops = []
        self._port = None
        self._worker_num = None
        self._in_channel = None
        self._out_channel = None
        self._retry = retry
        self._use_multithread = use_multithread
        self._client_type = client_type
        self._use_future = use_future
        if not self._use_multithread:
            self._manager = multiprocessing.Manager()
            if profile:
                raise Exception(
                    "profile cannot be used in multiprocess version temporarily")
            if self._use_future:
                raise Exception("cannot use future in multiprocess")
        if self._client_type == 'brpc' and self._use_future:
            logging.warn("brpc impl cannot use future")
        _profiler.enable(profile)

    def add_channel(self, channel):
        self._channels.append(channel)

    def add_op(self, op):
        self._user_ops.append(op)

    def add_ops(self, ops):
        self._user_ops.extend(ops)

    def gen_desc(self):
        logging.info('here will generate desc for PAAS')
        pass

    def _topo_sort(self):
        indeg_num = {}
        que_idx = 0  # scroll queue 
        ques = [Queue.Queue() for _ in range(2)]
        for op in self._user_ops:
            if len(op.get_input_ops()) == 0:
                op.name = "#G"  # update read_op.name
                break
        outdegs = {op.name: [] for op in self._user_ops}
        zero_indeg_num, zero_outdeg_num = 0, 0
        for idx, op in enumerate(self._user_ops):
            # check the name of op is globally unique
            if op.name in indeg_num:
                raise Exception("the name of Op must be unique")
            indeg_num[op.name] = len(op.get_input_ops())
            if indeg_num[op.name] == 0:
                ques[que_idx].put(op)
                zero_indeg_num += 1
            for pred_op in op.get_input_ops():
                outdegs[pred_op.name].append(op)
        if zero_indeg_num != 1:
            raise Exception("DAG contains multiple input Ops")
        for _, succ_list in outdegs.items():
            if len(succ_list) == 0:
                zero_outdeg_num += 1
        if zero_outdeg_num != 1:
            raise Exception("DAG contains multiple output Ops")

        # topo sort to get dag_views
        dag_views = []
        sorted_op_num = 0
        while True:
            que = ques[que_idx]
            next_que = ques[(que_idx + 1) % 2]
            dag_view = []
            while que.qsize() != 0:
                op = que.get()
                dag_view.append(op)
                sorted_op_num += 1
                for succ_op in outdegs[op.name]:
                    indeg_num[succ_op.name] -= 1
                    if indeg_num[succ_op.name] == 0:
                        next_que.put(succ_op)
            dag_views.append(dag_view)
            if next_que.qsize() == 0:
                break
            que_idx = (que_idx + 1) % 2
        if sorted_op_num < len(self._user_ops):
            raise Exception("not legal DAG")

        # create channels and virtual ops
        def name_generator(prefix):
            def number_generator():
                idx = 0
                while True:
                    yield "{}{}".format(prefix, idx)
                    idx += 1

            return number_generator()

        def gen_channel(name_gen):
            channel = None
            if self._use_multithread:
                if sys.version_info.major == 2:
                    channel = ThreadChannel(name=name_gen.next())
                elif sys.version_info.major == 3:
                    channel = ThreadChannel(name=name_gen.__next__())
                else:
                    raise Exception("Error Python version")
            else:
                if sys.version_info.major == 2:
                    channel = ProcessChannel(
                        self._manager, name=name_gen.next())
                elif sys.version_info.major == 3:
                    channel = ProcessChannel(
                        self._manager, name=name_gen.__next__())
                else:
                    raise Exception("Error Python version")
            return channel

        def gen_virtual_op(name_gen):
            virtual_op = None
            if sys.version_info.major == 2:
                virtual_op = VirtualOp(name=name_gen.next())
            elif sys.version_info.major == 3:
                virtual_op = VirtualOp(name=op_name_gen.__next__())
            else:
                raise Exception("Error Python version")
            return virtual_op

        virtual_op_name_gen = name_generator("vir")
        channel_name_gen = name_generator("chl")
        virtual_ops = []
        channels = []
        input_channel = None
        actual_view = None
        for v_idx, view in enumerate(dag_views):
            if v_idx + 1 >= len(dag_views):
                break
            next_view = dag_views[v_idx + 1]
            if actual_view is None:
                actual_view = view
            actual_next_view = []
            pred_op_of_next_view_op = {}
            for op in actual_view:
                # find actual succ op in next view and create virtual op
                for succ_op in outdegs[op.name]:
                    if succ_op in next_view:
                        if succ_op not in actual_next_view:
                            actual_next_view.append(succ_op)
                        if succ_op.name not in pred_op_of_next_view_op:
                            pred_op_of_next_view_op[succ_op.name] = []
                        pred_op_of_next_view_op[succ_op.name].append(op)
                    else:
                        # create virtual op
                        virtual_op = gen_virtual_op(virtual_op_name_gen)
                        virtual_ops.append(virtual_op)
                        outdegs[virtual_op.name] = [succ_op]
                        actual_next_view.append(virtual_op)
                        pred_op_of_next_view_op[virtual_op.name] = [op]
                        virtual_op.add_virtual_pred_op(op)
            actual_view = actual_next_view
            # create channel
            processed_op = set()
            for o_idx, op in enumerate(actual_next_view):
                if op.name in processed_op:
                    continue
                channel = gen_channel(channel_name_gen)
                channels.append(channel)
                logging.debug("{} => {}".format(channel.name, op.name))
                op.add_input_channel(channel)
                pred_ops = pred_op_of_next_view_op[op.name]
                if v_idx == 0:
                    input_channel = channel
                else:
                    # if pred_op is virtual op, it will use ancestors as producers to channel
                    for pred_op in pred_ops:
                        logging.debug("{} => {}".format(pred_op.name,
                                                        channel.name))
                        pred_op.add_output_channel(channel)
                processed_op.add(op.name)
                # find same input op to combine channel
                for other_op in actual_next_view[o_idx + 1:]:
                    if other_op.name in processed_op:
                        continue
                    other_pred_ops = pred_op_of_next_view_op[other_op.name]
                    if len(other_pred_ops) != len(pred_ops):
                        continue
                    same_flag = True
                    for pred_op in pred_ops:
                        if pred_op not in other_pred_ops:
                            same_flag = False
                            break
                    if same_flag:
                        logging.debug("{} => {}".format(channel.name,
                                                        other_op.name))
                        other_op.add_input_channel(channel)
                        processed_op.add(other_op.name)
        output_channel = gen_channel(channel_name_gen)
        channels.append(output_channel)
        last_op = dag_views[-1][0]
        last_op.add_output_channel(output_channel)

        self._actual_ops = virtual_ops
        for op in self._user_ops:
            if len(op.get_input_ops()) == 0:
                # pass read op
                continue
            self._actual_ops.append(op)
        self._channels = channels
        for c in channels:
            logging.debug(c.debug())
        return input_channel, output_channel

    def prepare_server(self, port, worker_num):
        self._port = port
        self._worker_num = worker_num

        input_channel, output_channel = self._topo_sort()
        self._in_channel = input_channel
        self._out_channel = output_channel
        for op in self._actual_ops:
            if op.with_serving:
                self.prepare_serving(op)
        self.gen_desc()

    def _run_ops(self):
        threads_or_proces = []
        for op in self._actual_ops:
            op.init_profiler(_profiler)
            if self._use_multithread:
                threads_or_proces.extend(
                    op.start_with_thread(self._client_type, self._use_future))
            else:
                threads_or_proces.extend(
                    op.start_with_process(self._client_type, self._use_future))
        return threads_or_proces

    def _stop_ops(self):
        for op in self._actual_ops:
            op.stop()

    def run_server(self):
        op_threads_or_proces = self._run_ops()
        server = grpc.server(
            futures.ThreadPoolExecutor(max_workers=self._worker_num))
        general_python_service_pb2_grpc.add_GeneralPythonServiceServicer_to_server(
            GeneralPythonService(self._in_channel, self._out_channel,
                                 self._retry), server)
        server.add_insecure_port('[::]:{}'.format(self._port))
        server.start()
        server.wait_for_termination()
        self._stop_ops()  # TODO
        for x in op_threads_or_proces:
            x.join()

    def prepare_serving(self, op):
        model_path = op._server_model
        port = op._server_port
        device = op._device

        if self._client_type == "grpc":
            if device == "cpu":
                cmd = "(Use grpc impl) python -m paddle_serving_server.serve" \
                      " --model {} --thread 4 --port {} --use_multilang &>/dev/null &".format(model_path, port)
            else:
                cmd = "(Use grpc impl) python -m paddle_serving_server_gpu.serve" \
                      " --model {} --thread 4 --port {} --use_multilang &>/dev/null &".format(model_path, port)
        elif self._client_type == "brpc":
            if device == "cpu":
                cmd = "(Use brpc impl) python -m paddle_serving_server.serve" \
                      " --model {} --thread 4 --port {} &>/dev/null &".format(model_path, port)
            else:
                cmd = "(Use brpc impl) python -m paddle_serving_server_gpu.serve" \
                      " --model {} --thread 4 --port {} &>/dev/null &".format(model_path, port)
        else:
            raise Exception("unknow client type: {}".format(self._client_type))
        # run a server (not in PyServing)
        logging.info("run a server (not in PyServing): {}".format(cmd))
