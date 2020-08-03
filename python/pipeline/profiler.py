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
import threading

_LOGGER = logging.getLogger()


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
