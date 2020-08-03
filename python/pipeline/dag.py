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
import sys
import copy
if sys.version_info.major == 2:
    import Queue
elif sys.version_info.major == 3:
    import queue as Queue
else:
    raise Exception("Error Python version")
import os
import logging

from .operator import Op, RequestOp, ResponseOp, VirtualOp
from .channel import (ThreadChannel, ProcessChannel, ChannelData,
                      ChannelDataEcode, ChannelDataType, ChannelStopError)
from .profiler import TimeProfiler
from .util import NameGenerator
from .proto import pipeline_service_pb2

_LOGGER = logging.getLogger()


class DAGExecutor(object):
    def __init__(self, response_op, dag_conf):
        self._retry = dag_conf["retry"]
        client_type = dag_conf["client_type"]
        self._server_use_profile = dag_conf["use_profile"]
        channel_size = dag_conf["channel_size"]
        self._is_thread_op = dag_conf["is_thread_op"]
        build_dag_each_worker = dag_conf["build_dag_each_worker"]

        self.name = "@G"
        self._profiler = TimeProfiler()
        self._profiler.enable(True)

        self._dag = DAG(self.name, response_op, self._server_use_profile,
                        self._is_thread_op, client_type, channel_size,
                        build_dag_each_worker)
        (in_channel, out_channel, pack_rpc_func,
         unpack_rpc_func) = self._dag.build()
        self._dag.start()

        self._set_in_channel(in_channel)
        self._set_out_channel(out_channel)
        self._pack_rpc_func = pack_rpc_func
        self._unpack_rpc_func = unpack_rpc_func

        self._id_lock = threading.Lock()
        self._id_counter = 0
        self._reset_max_id = 1000000000000000000
        self._cv_pool = {}
        self._cv_for_cv_pool = threading.Condition()
        self._fetch_buffer = {}
        self._recive_func = None

        self._client_profile_key = "pipeline.profile"
        self._client_profile_value = "1"

    def start(self):
        self._recive_func = threading.Thread(
            target=DAGExecutor._recive_out_channel_func, args=(self, ))
        self._recive_func.start()
        _LOGGER.debug("[DAG Executor] Start recive thread")

    def stop(self):
        self._dag.stop()
        self._dag.join()
        _LOGGER.info("[DAG Executor] Stop")

    def _get_next_data_id(self):
        data_id = None
        with self._id_lock:
            if self._id_counter >= self._reset_max_id:
                _LOGGER.info("[DAG Executor] Reset request id")
                self._id_counter -= self._reset_max_id
            data_id = self._id_counter
            self._id_counter += 1
        cond_v = threading.Condition()
        with self._cv_for_cv_pool:
            self._cv_pool[data_id] = cond_v
            self._fetch_buffer[data_id] = None
        return data_id, cond_v

    def _set_in_channel(self, in_channel):
        if not isinstance(in_channel, (ThreadChannel, ProcessChannel)):
            _LOGGER.critical("[DAG Executor] Failed to set in_channel: "
                             "in_channel must be Channel type, but get {}".
                             format(type(in_channel)))
            os._exit(-1)
        in_channel.add_producer(self.name)
        self._in_channel = in_channel

    def _set_out_channel(self, out_channel):
        if not isinstance(out_channel, (ThreadChannel, ProcessChannel)):
            _LOGGER.critical("[DAG Executor] Failed to set out_channel: "
                             "must be Channel type, but get {}".format(
                                 type(out_channel)))
            os._exit(-1)
        out_channel.add_consumer(self.name)
        self._out_channel = out_channel

    def _recive_out_channel_func(self):
        cv = None
        while True:
            try:
                channeldata_dict = self._out_channel.front(self.name)
            except ChannelStopError:
                _LOGGER.info("[DAG Executor] Stop.")
                with self._cv_for_cv_pool:
                    for data_id, cv in self._cv_pool.items():
                        closed_errror_data = ChannelData(
                            ecode=ChannelDataEcode.CLOSED_ERROR.value,
                            error_info="dag closed.",
                            data_id=data_id)
                        with cv:
                            self._fetch_buffer[data_id] = closed_errror_data
                            cv.notify_all()
                break

            if len(channeldata_dict) != 1:
                _LOGGER.critical(
                    "[DAG Executor] Failed to fetch result: out_channel "
                    "cannot have multiple input ops")
                os._exit(-1)
            (_, channeldata), = channeldata_dict.items()
            if not isinstance(channeldata, ChannelData):
                _LOGGER.critical(
                    '[DAG Executor] Failed to fetch result: data in out_channel" \
                    " must be ChannelData type, but get {}'
                    .format(type(channeldata)))
                os._exit(-1)

            data_id = channeldata.id
            _LOGGER.debug("(logid={}) [recive thread] Fetched data".format(
                data_id))
            with self._cv_for_cv_pool:
                cond_v = self._cv_pool[data_id]
            with cond_v:
                self._fetch_buffer[data_id] = channeldata
                cond_v.notify_all()

    def _get_channeldata_from_fetch_buffer(self, data_id, cond_v):
        ready_data = None

        with cond_v:
            with self._cv_for_cv_pool:
                if self._fetch_buffer[data_id] is not None:
                    # The requested data is already ready
                    ready_data = self._fetch_buffer[data_id]
                    self._cv_pool.pop(data_id)
                    self._fetch_buffer.pop(data_id)
            if ready_data is None:
                # Wait for data ready
                cond_v.wait()
                with self._cv_for_cv_pool:
                    ready_data = self._fetch_buffer[data_id]
                    self._cv_pool.pop(data_id)
                    self._fetch_buffer.pop(data_id)
        _LOGGER.debug("(logid={}) [resp thread] Got data".format(data_id))
        return ready_data

    def _pack_channeldata(self, rpc_request, data_id):
        dictdata = None
        try:
            dictdata = self._unpack_rpc_func(rpc_request)
        except Exception as e:
            _LOGGER.error(
                "(logid={}) Failed to parse RPC request package: {}"
                .format(data_id, e),
                exc_info=True)
            return ChannelData(
                ecode=ChannelDataEcode.RPC_PACKAGE_ERROR.value,
                error_info="rpc package error: {}".format(e),
                data_id=data_id)
        else:
            # because unpack_rpc_func is rewritten by user, we need
            # to look for client_profile_key field in rpc_request
            profile_value = None
            for idx, key in enumerate(rpc_request.key):
                if key == self._client_profile_key:
                    profile_value = rpc_request.value[idx]
                    break
            client_need_profile = (profile_value == self._client_profile_value)
            _LOGGER.debug("(logid={}) Need profile in client: {}".format(
                data_id, client_need_profile))
            return ChannelData(
                datatype=ChannelDataType.DICT.value,
                dictdata=dictdata,
                data_id=data_id,
                client_need_profile=client_need_profile)

    def call(self, rpc_request):
        data_id, cond_v = self._get_next_data_id()
        _LOGGER.info("(logid={}) Succ generate id".format(data_id))

        start_call, end_call = None, None
        if not self._is_thread_op:
            start_call = self._profiler.record("call_{}#DAG-{}_0".format(
                data_id, data_id))
        else:
            start_call = self._profiler.record("call_{}#DAG_0".format(data_id))

        _LOGGER.debug("(logid={}) Parsing RPC request package".format(data_id))
        self._profiler.record("prepack_{}#{}_0".format(data_id, self.name))
        req_channeldata = self._pack_channeldata(rpc_request, data_id)
        self._profiler.record("prepack_{}#{}_1".format(data_id, self.name))

        resp_channeldata = None
        for i in range(self._retry):
            _LOGGER.debug("(logid={}) Pushing data into Graph engine".format(
                data_id))
            try:
                self._in_channel.push(req_channeldata, self.name)
            except ChannelStopError:
                _LOGGER.debug("[DAG Executor] Stop")
                with self._cv_for_cv_pool:
                    self._cv_pool.pop(data_id)
                return self._pack_for_rpc_resp(
                    ChannelData(
                        ecode=ChannelDataEcode.CLOSED_ERROR.value,
                        error_info="dag closed.",
                        data_id=data_id))

            _LOGGER.debug("(logid={}) Wait for Graph engine...".format(data_id))
            resp_channeldata = self._get_channeldata_from_fetch_buffer(data_id,
                                                                       cond_v)

            if resp_channeldata.ecode == ChannelDataEcode.OK.value:
                _LOGGER.debug("(logid={}) Succ predict".format(data_id))
                break
            else:
                _LOGGER.error("(logid={}) Failed to predict: {}"
                              .format(data_id, resp_channeldata.error_info))
                if resp_channeldata.ecode != ChannelDataEcode.TIMEOUT.value:
                    break

            if i + 1 < self._retry:
                _LOGGER.warning("(logid={}) DAGExecutor retry({}/{})".format(
                    data_id, i + 1, self._retry))

        _LOGGER.debug("(logid={}) Packing RPC response package".format(data_id))
        self._profiler.record("postpack_{}#{}_0".format(data_id, self.name))
        rpc_resp = self._pack_for_rpc_resp(resp_channeldata)
        self._profiler.record("postpack_{}#{}_1".format(data_id, self.name))
        if not self._is_thread_op:
            end_call = self._profiler.record("call_{}#DAG-{}_1".format(data_id,
                                                                       data_id))
        else:
            end_call = self._profiler.record("call_{}#DAG_1".format(data_id))
        _LOGGER.log(level=1,
                    msg="(logid={}) call[{} ms]".format(
                        data_id, (end_call - start_call) / 1e3))

        profile_str = self._profiler.gen_profile_str()
        if self._server_use_profile:
            sys.stderr.write(profile_str)

        # add profile info into rpc_resp
        profile_value = ""
        if resp_channeldata.client_need_profile:
            profile_set = resp_channeldata.profile_data_set
            profile_set.add(profile_str)
            profile_value = "".join(list(profile_set))
        rpc_resp.key.append(self._client_profile_key)
        rpc_resp.value.append(profile_value)

        return rpc_resp

    def _pack_for_rpc_resp(self, channeldata):
        try:
            return self._pack_rpc_func(channeldata)
        except Exception as e:
            _LOGGER.error(
                "(logid={}) Failed to pack RPC response package: {}"
                .format(channeldata.id, e),
                exc_info=True)
            resp = pipeline_service_pb2.Response()
            resp.ecode = ChannelDataEcode.RPC_PACKAGE_ERROR.value
            resp.error_info = "rpc package error: {}".format(e)
            return resp


class DAG(object):
    def __init__(self, request_name, response_op, use_profile, is_thread_op,
                 client_type, channel_size, build_dag_each_worker):
        self._request_name = request_name
        self._response_op = response_op
        self._use_profile = use_profile
        self._is_thread_op = is_thread_op
        self._channel_size = channel_size
        self._client_type = client_type
        self._build_dag_each_worker = build_dag_each_worker
        if not self._is_thread_op:
            self._manager = multiprocessing.Manager()
        _LOGGER.info("[DAG] Succ init")

    def get_use_ops(self, response_op):
        unique_names = set()
        used_ops = set()
        succ_ops_of_use_op = {}  # {op_name: succ_ops}
        que = Queue.Queue()
        que.put(response_op)
        while que.qsize() != 0:
            op = que.get()
            for pred_op in op.get_input_ops():
                if pred_op.name not in succ_ops_of_use_op:
                    succ_ops_of_use_op[pred_op.name] = []
                if op != response_op:
                    succ_ops_of_use_op[pred_op.name].append(op)
                if pred_op not in used_ops:
                    que.put(pred_op)
                    used_ops.add(pred_op)
                    # check the name of op is globally unique
                    if pred_op.name in unique_names:
                        _LOGGER.critical("Failed to get used Ops: the"
                                         " name of Op must be unique: {}".
                                         format(pred_op.name))
                        os._exit(-1)
                    unique_names.add(pred_op.name)
        return used_ops, succ_ops_of_use_op

    def _gen_channel(self, name_gen):
        channel = None
        if self._is_thread_op:
            channel = ThreadChannel(
                name=name_gen.next(), maxsize=self._channel_size)
        else:
            channel = ProcessChannel(
                self._manager, name=name_gen.next(), maxsize=self._channel_size)
        _LOGGER.debug("[DAG] Generate channel: {}".format(channel.name))
        return channel

    def _gen_virtual_op(self, name_gen):
        vir_op = VirtualOp(name=name_gen.next())
        _LOGGER.debug("[DAG] Generate virtual_op: {}".format(vir_op.name))
        return vir_op

    def _topo_sort(self, used_ops, response_op, out_degree_ops):
        out_degree_num = {
            name: len(ops)
            for name, ops in out_degree_ops.items()
        }
        que_idx = 0  # scroll queue 
        ques = [Queue.Queue() for _ in range(2)]
        zero_indegree_num = 0
        for op in used_ops:
            if len(op.get_input_ops()) == 0:
                zero_indegree_num += 1
        if zero_indegree_num != 1:
            _LOGGER.critical("Failed to topo sort: DAG contains "
                             "multiple RequestOps")
            os._exit(-1)
        last_op = response_op.get_input_ops()[0]
        ques[que_idx].put(last_op)

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
                for pred_op in op.get_input_ops():
                    out_degree_num[pred_op.name] -= 1
                    if out_degree_num[pred_op.name] == 0:
                        next_que.put(pred_op)
            dag_views.append(dag_view)
            if next_que.qsize() == 0:
                break
            que_idx = (que_idx + 1) % 2
        if sorted_op_num < len(used_ops):
            _LOGGER.critical("Failed to topo sort: not legal DAG")
            os._exit(-1)

        return dag_views, last_op

    def _build_dag(self, response_op):
        if response_op is None:
            _LOGGER.critical("Failed to build DAG: ResponseOp"
                             " has not been set.")
            os._exit(-1)
        used_ops, out_degree_ops = self.get_use_ops(response_op)
        if not self._build_dag_each_worker:
            _LOGGER.info("================= USED OP =================")
            for op in used_ops:
                if op.name != self._request_name:
                    _LOGGER.info(op.name)
            _LOGGER.info("-------------------------------------------")
        if len(used_ops) <= 1:
            _LOGGER.critical(
                "Failed to build DAG: besides RequestOp and ResponseOp, "
                "there should be at least one Op in DAG.")
            os._exit(-1)
        if self._build_dag_each_worker:
            _LOGGER.info("Because `build_dag_each_worker` mode is used, "
                         "Auto-batching is set to the default config: "
                         "batch_size=1, auto_batching_timeout=None")
            for op in used_ops:
                op.use_default_auto_batching_config()

        dag_views, last_op = self._topo_sort(used_ops, response_op,
                                             out_degree_ops)
        dag_views = list(reversed(dag_views))
        if not self._build_dag_each_worker:
            _LOGGER.debug("================== DAG ====================")
            for idx, view in enumerate(dag_views):
                _LOGGER.debug("(VIEW {})".format(idx))
                for op in view:
                    _LOGGER.debug("  [{}]".format(op.name))
                    for out_op in out_degree_ops[op.name]:
                        _LOGGER.debug("    - {}".format(out_op.name))
            _LOGGER.debug("-------------------------------------------")

        # create channels and virtual ops
        virtual_op_name_gen = NameGenerator("vir")
        channel_name_gen = NameGenerator("chl")
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
                for succ_op in out_degree_ops[op.name]:
                    if succ_op in next_view:
                        if succ_op not in actual_next_view:
                            actual_next_view.append(succ_op)
                        if succ_op.name not in pred_op_of_next_view_op:
                            pred_op_of_next_view_op[succ_op.name] = []
                        pred_op_of_next_view_op[succ_op.name].append(op)
                    else:
                        # create virtual op
                        virtual_op = self._gen_virtual_op(virtual_op_name_gen)
                        virtual_ops.append(virtual_op)
                        out_degree_ops[virtual_op.name] = [succ_op]
                        actual_next_view.append(virtual_op)
                        pred_op_of_next_view_op[virtual_op.name] = [op]
                        virtual_op.add_virtual_pred_op(op)
            actual_view = actual_next_view
            # create channel
            processed_op = set()
            for o_idx, op in enumerate(actual_next_view):
                if op.name in processed_op:
                    continue
                channel = self._gen_channel(channel_name_gen)
                channels.append(channel)
                op.add_input_channel(channel)
                pred_ops = pred_op_of_next_view_op[op.name]
                if v_idx == 0:
                    input_channel = channel
                else:
                    # if pred_op is virtual op, it will use ancestors as producers to channel
                    for pred_op in pred_ops:
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
                        other_op.add_input_channel(channel)
                        processed_op.add(other_op.name)
        output_channel = self._gen_channel(channel_name_gen)
        channels.append(output_channel)
        last_op.add_output_channel(output_channel)

        pack_func, unpack_func = None, None
        pack_func = response_op.pack_response_package

        actual_ops = virtual_ops
        for op in used_ops:
            if len(op.get_input_ops()) == 0:
                unpack_func = op.unpack_request_package
                continue
            actual_ops.append(op)

        for c in channels:
            _LOGGER.debug("Channel({}):\n\t- producers: {}\n\t- consumers: {}"
                          .format(c.name, c.get_producers(), c.get_consumers()))

        return (actual_ops, channels, input_channel, output_channel, pack_func,
                unpack_func)

    def build(self):
        (actual_ops, channels, input_channel, output_channel, pack_func,
         unpack_func) = self._build_dag(self._response_op)
        _LOGGER.info("[DAG] Succ build DAG")

        self._actual_ops = actual_ops
        self._channels = channels
        self._input_channel = input_channel
        self._output_channel = output_channel
        self._pack_func = pack_func
        self._unpack_func = unpack_func

        return self._input_channel, self._output_channel, self._pack_func, self._unpack_func

    def start(self):
        self._threads_or_proces = []
        for op in self._actual_ops:
            op.use_profiler(self._use_profile)
            if self._is_thread_op:
                self._threads_or_proces.extend(
                    op.start_with_thread(self._client_type))
            else:
                self._threads_or_proces.extend(
                    op.start_with_process(self._client_type))
        _LOGGER.info("[DAG] start")

        # not join yet
        return self._threads_or_proces

    def join(self):
        for x in self._threads_or_proces:
            x.join()

    def stop(self):
        for chl in self._channels:
            chl.stop()
        for op in self._actual_ops:
            op.clean_input_channel()
            op.clean_output_channels()
