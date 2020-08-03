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

import os
import sys
import logging
if sys.version_info.major == 2:
    import Queue
elif sys.version_info.major == 3:
    import queue as Queue
else:
    raise Exception("Error Python version")
from time import time as _time
import time
import threading
import multiprocessing

_LOGGER = logging.getLogger()


class PerformanceTracer(object):
    def __init__(self, interval_s=1):
        self._data_buffer = multiprocessing.Manager().Queue()
        self._interval_s = interval_s
        self._proc = None
        self._channels = []
        self._trace_filename = os.path.join("PipelineServingLogs", "INDEX.log")

    def data_buffer(self):
        return self._data_buffer

    def start(self):
        self._proc = multiprocessing.Process(
            target=self._trace_func, args=(self._channels, ))
        self._proc.daemon = True
        self._proc.start()

    def set_channels(self, channels):
        self._channels = channels

    def _trace_func(self, channels):
        trace_file = open(self._trace_filename, "a")
        actions = ["prep", "midp", "postp"]
        tag_dict = {}
        while True:
            op_cost = {}
            trace_file.write("==========================")

            # op
            while not self._data_buffer.empty():
                name, action, stage, timestamp = self._data_buffer.get()
                tag = "{}_{}".format(name, action)
                if tag in tag_dict:
                    assert stage == 1
                    start_timestamp = tag_dict.pop(tag)
                    cost = timestamp - start_timestamp
                    if name not in op_cost:
                        op_cost[name] = {}
                    if action not in op_cost[name]:
                        op_cost[name][action] = []
                    op_cost[name][action].append(cost)
                else:
                    assert stage == 0
                    tag_dict[tag] = timestamp

            for name in op_cost:
                tot_cost, cal_cost = 0.0, 0.0
                for action, costs in op_cost[name].items():
                    op_cost[name][action] = sum(costs) / (1e3 * len(costs))
                    tot_cost += op_cost[name][action]

                msg = ", ".join([
                    "{}[{} ms]".format(action, cost)
                    for action, cost in op_cost[name].items()
                ])

                for action in actions:
                    if action in op_cost[name]:
                        cal_cost += op_cost[name][action]

                trace_file.write("Op({}) {}".format(name, msg))
                if name != "DAG":
                    trace_file.write("Op({}) idle[{}]".format(
                        name, 1 - 1.0 * cal_cost / tot_cost))

            # channel
            for channel in channels:
                trace_file.write("Channel({}) size[{}]".format(channel.name,
                                                               channel.size()))
            time.sleep(self._interval_s)


class UnsafeTimeProfiler(object):
    """ thread unsafe profiler """

    def __init__(self):
        self.pid = os.getpid()
        self.print_head = 'PROFILE\tpid:{}\t'.format(self.pid)
        self.time_record = [self.print_head]
        self._enable = False

    def enable(self, enable):
        self._enable = enable

    def record(self, name):
        if self._enable is False:
            return
        timestamp = int(round(_time() * 1000000))
        self.time_record.append('{}:{} '.format(name, timestamp))
        return timestamp

    def print_profile(self):
        if self._enable is False:
            return
        sys.stderr.write(self.gen_profile_str())

    def gen_profile_str(self):
        if self._enable is False:
            return
        self.time_record.append('\n')
        profile_str = ''.join(self.time_record)
        self.time_record = [self.print_head]
        return profile_str


class TimeProfiler(object):
    def __init__(self):
        self._pid = os.getpid()
        self._print_head = 'PROFILE\tpid:{}\t'.format(self._pid)
        self._time_record = Queue.Queue()
        self._enable = False
        self._lock = threading.Lock()

    def enable(self, enable):
        self._enable = enable

    def record(self, name_with_tag):
        if self._enable is False:
            return
        timestamp = int(round(_time() * 1000000))
        name_with_tag = name_with_tag.split("_")
        tag = name_with_tag[-1]
        name = '_'.join(name_with_tag[:-1])
        with self._lock:
            self._time_record.put((name, tag, timestamp))
        return timestamp

    def print_profile(self):
        if self._enable is False:
            return
        sys.stderr.write(self.gen_profile_str())

    def gen_profile_str(self):
        if self._enable is False:
            return
        print_str = self._print_head
        tmp = {}
        with self._lock:
            while not self._time_record.empty():
                name, tag, timestamp = self._time_record.get()
                if name in tmp:
                    ptag, ptimestamp = tmp.pop(name)
                    print_str += "{}_{}:{} ".format(name, ptag, ptimestamp)
                    print_str += "{}_{}:{} ".format(name, tag, timestamp)
                else:
                    tmp[name] = (tag, timestamp)
            print_str = "\n{}\n".format(print_str)
            for name, item in tmp.items():
                tag, timestamp = item
                self._time_record.put((name, tag, timestamp))
            return print_str
