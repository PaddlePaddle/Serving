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
import json
import copy
import re
import logging

_LOGGER = logging.getLogger(__name__)


class Analyst(object):
    def __init__(self, profile_file):
        self._profile_file = profile_file
        self._trace = None

        self.ave_call = None
        self.ave_prepack = None
        self.ave_postpack = None
        self.op_analyst = None

        self.start_time = None
        self.end_time = None

    def _prase_line(self, pid_str, time_str, counter):
        pid = pid_str.split(":")[1]
        event_list = time_str.split(" ")
        trace_list = []
        for event in event_list:
            name, ts = event.split(":")
            name_list = name.split("_")
            ph = "B" if (name_list[-1] == "0") else "E"
            if len(name_list) == 2:
                name = name_list[0]
            else:
                name = "_".join(name_list[:-1])
            name_list = name.split("#")
            if len(name_list) > 1:
                tid = name_list[-1]
                name = "#".join(name_list[:-1])
            else:
                tid = 0
            event_dict = {}
            event_dict["name"] = name
            event_dict["tid"] = tid
            event_dict["pid"] = pid
            event_dict["ts"] = ts
            event_dict["ph"] = ph

            trace_list.append(event_dict)
        return trace_list

    def get_trace(self):
        if self._trace is not None:
            return self._trace
        all_list = []
        counter = 0
        with open(self._profile_file) as f:
            for line in f.readlines():
                line = line.strip().split("\t")
                if line[0] == "PROFILE" and len(line) >= 3:
                    trace_list = self._prase_line(line[1], line[2], counter)
                    counter += 1
                    for trace in trace_list:
                        all_list.append(trace)
        self._trace = all_list
        return self._trace

    def save_trace(self, trace_file):
        self.get_trace()
        trace = json.dumps(self._trace, indent=2, separators=(',', ':'))
        with open(trace_file, "w") as f:
            f.write(trace)

    def print_profile(self):
        self.get_profile()
        print("graph engine call: {}".format(self.ave_call))
        print("rpc prepack: {}".format(self.ave_prepack))
        print("rpc postpack: {}".format(self.ave_postpack))
        print("OP: {}".format(self.op_analyst))

    def get_op_analyst(self):
        self.get_profile()
        return self.op_analyst

    def get_profile(self):
        if self.ave_call is not None and \
                self.ave_prepack is not None and \
                self.ave_postpack is not None and \
                self.op_analyst is not None:
            return (self.ave_call, self.ave_prepack, self.ave_postpack,
                    self.op_analyst)
        trace = self.get_trace()
        time_dict = {}
        time_list_dict = {}
        start, end = None, None
        for event in trace:
            name = "{}#{}".format(event["name"], event["tid"])
            event_t = int(event["ts"])
            if name in time_dict:
                ts = event_t - time_dict.pop(name)
                ts = ts / 1e3  # ms
                if name not in time_list_dict:
                    time_list_dict[name] = []
                time_list_dict[name].append(ts)
            else:
                time_dict[name] = event_t
            if start is None:
                start = event_t
            elif start > event_t:
                start = event_t
            if end is None:
                end = event_t
            elif end < event_t:
                end = event_t
        self.start_time = start
        self.end_time = end

        op_analyst = OpAnalyst(start, end)
        # reduce prepack_n, postpack_n, call_n
        pat_prepack = re.compile(r"prepack_\d+#@G")
        prepack_time_list = []
        pat_postpack = re.compile(r"postpack_\d+#@G")
        postpack_time_list = []
        pat_call = re.compile(r"call_\d+#DAG")
        call_time_list = []
        for name in time_list_dict:
            if pat_prepack.match(name):
                prepack_time_list.extend(time_list_dict[name])
            elif pat_postpack.match(name):
                postpack_time_list.extend(time_list_dict[name])
            elif pat_call.match(name):
                call_time_list.extend(time_list_dict[name])
            else:
                op_analyst.add(name, time_list_dict[name])

        self.ave_call = sum(call_time_list) * 1.0 / len(call_time_list)
        self.ave_prepack = sum(prepack_time_list) * 1.0 / len(prepack_time_list)
        self.ave_postpack = sum(postpack_time_list) * 1.0 / len(
            postpack_time_list)
        self.op_analyst = op_analyst
        return (self.ave_call, self.ave_prepack, self.ave_postpack,
                self.op_analyst)


class OpAnalyst(object):
    def __init__(self, start_time, end_time):
        self.op_time_list_dict = {}
        self._qps = None
        self._close = False
        self.start_time = start_time
        self.end_time = end_time

    def add(self, name_str, ts_list):
        if self._close:
            _LOGGER.error("Failed to add item: OpAnalyst is closed.")
            return
        op_name, curr_idx, step = self._parse(name_str)
        if op_name not in self.op_time_list_dict:
            self.op_time_list_dict[op_name] = {}
        if curr_idx not in self.op_time_list_dict[op_name]:
            self.op_time_list_dict[op_name][curr_idx] = {}
        if step not in self.op_time_list_dict[op_name][curr_idx]:
            self.op_time_list_dict[op_name][curr_idx][step] = []
        self.op_time_list_dict[op_name][curr_idx][step].extend(ts_list)

    def _parse(self, name):
        step, name_str = name.split("#")
        name_str = name_str[1:-1]
        op_name, curr_idx = name_str.split("|")
        return op_name, curr_idx, step

    def _reduce_profile(self):
        """
        Calculating the average time-consuming of multiple concurrent OPs.
        """
        if self._close:
            return
        for op_name in self.op_time_list_dict:
            total_time = None
            for curr_idx in self.op_time_list_dict[op_name]:
                ave_dict = {}
                for step in self.op_time_list_dict[op_name][curr_idx]:
                    ave_dict[step] = sum(self.op_time_list_dict[op_name][
                        curr_idx][step]) * 1.0 / len(self.op_time_list_dict[
                            op_name][curr_idx][step])
                if total_time is None:
                    total_time = ave_dict
                else:
                    for step in ave_dict:
                        total_time[step] += ave_dict[step]
            for step in total_time:
                total_time[step] = total_time[step] * 1.0 / len(
                    self.op_time_list_dict[op_name])
            self.op_time_list_dict[op_name] = total_time
        self._close = True

    def _get_qps(self):
        """
        Calculating QPS for each step based on the time
        consumed in each step of OP.
        """
        if self._qps is not None:
            return self._qps
        self._reduce_profile()
        self._qps = {}
        for op_name, times in self.op_time_list_dict.items():
            self._qps[op_name] = {
                step: 1000.0 / ts
                for step, ts in times.items()
            }
        return self._qps

    def __str__(self):
        self._reduce_profile()
        return json.dumps(
            self.op_time_list_dict, indent=2, separators=(', ', ':'))

    def qps(self, op_name=None):
        """
        Get the average QPS of each step of each OP (in q/s)
        """
        self._get_qps()
        if op_name is None:
            return self._qps
        else:
            return self._qps[op_name]

    def times(self, op_name=None):
        """
        Get the average time of each step of each OP (in ms)
        """
        self._reduce_profile()
        if op_name is None:
            return self.op_time_list_dict
        else:
            return self.op_time_list_dict[op_name]

    def concurrency_analysis(self, op_config_yaml):
        """
        Through OP time consuming and op_config_yaml to
        calculate the theoretical QPS, as well as the
        number of concurrency required by each OPs.
        
        It should be noted that since multiple models
        will affect each other on one card, only the
        case that each model is on a different card can
        be calculated.

        The format of the yaml file is as follows:
        ```yaml
        <op_name>:
            <step(prep, midp or postp)>: <GPU id>
        ```

        For example:
        ```yaml
        cnn:
            midp: 0
        bow:
            midp: 1
        ```
        """
        import yaml
        with open(op_config_yaml) as f:
            op_config = yaml.load(f, yaml.FullLoader)

        # check that each model is deployed on a different card
        card_set = set()
        # and finding the most time consuming part (GPU)
        op_times = self.times()
        most_time = 0
        most_time_op_name = None
        for op in op_config:
            for step, cards in op_config[op].items():
                if isinstance(cards, int):
                    cards = [cards]
                elif isinstance(cards, str):
                    cards = [int(x) for x in cards.split(',')]
                else:
                    raise Exception("Error cards type.")
                for card in cards:
                    if card in card_set:
                        raise Exception(
                            "Analysis is failed because "
                            "different services interact when different"
                            " models are deployed on one card.")
                    else:
                        card_set.add(card)
                times_each_card = op_times[op][step] / len(cards)
                if most_time < times_each_card:
                    most_time = times_each_card
                    most_time_op_name = op

        # calculate base qps
        base_qps = 1.0 / most_time  # q/ms
        _LOGGER.info("Most Time Consuming (GPU): {} ms (op: {})"
                     .format(most_time, most_time_op_name))
        _LOGGER.info("Theoretically Expected QPS: {} q/s".format(base_qps *
                                                                 1000))

        # reduce op times
        op_times = {
            op_name: sum(list(step_times.values()))
            for op_name, step_times in op_times.items()
        }

        # calculate op concurrency
        op_concurrency = {
            op_name: round(base_qps * times, 3)
            for op_name, times in op_times.items()
        }
        return op_concurrency
