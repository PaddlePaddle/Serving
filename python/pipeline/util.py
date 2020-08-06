# Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
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

import sys
import logging
import threading
import multiprocessing

_LOGGER = logging.getLogger(__name__)


class NameGenerator(object):
    # use unsafe-id-generator
    def __init__(self, prefix):
        self._idx = -1
        self._prefix = prefix
        self._id_generator = UnsafeIdGenerator(1000000000000000000)

    def next(self):
        next_id = self._id_generator.next()
        return "{}{}".format(self._prefix, next_id)


class UnsafeIdGenerator(object):
    def __init__(self, max_id, base_counter=0, step=1):
        self._base_counter = base_counter
        self._counter = self._base_counter
        self._step = step
        self._max_id = max_id  # for reset

    def next(self):
        if self._counter >= self._max_id:
            self._counter = self._base_counter
            _LOGGER.info("Reset Id: {}".format(self._counter))
        next_id = self._counter
        self._counter += self._step
        return next_id


class ThreadIdGenerator(UnsafeIdGenerator):
    def __init__(self, max_id, base_counter=0, step=1, lock=None):
        # if you want to use your lock, you may need to use Reentrant-Lock
        self._lock = lock
        if self._lock is None:
            self._lock = threading.Lock()
        super(ThreadIdGenerator, self).__init__(max_id, base_counter, step)

    def next(self):
        next_id = None
        with self._lock:
            if self._counter >= self._max_id:
                self._counter = self._base_counter
                _LOGGER.info("Reset Id: {}".format(self._counter))
            next_id = self._counter
            self._counter += self._step
        return next_id


class ProcessIdGenerator(UnsafeIdGenerator):
    def __init__(self, max_id, base_counter=0, step=1, lock=None):
        # if you want to use your lock, you may need to use Reentrant-Lock
        self._lock = lock
        if self._lock is None:
            self._lock = multiprocessing.Lock()
        self._base_counter = base_counter
        self._counter = multiprocessing.Manager().Value('i', 0)
        self._step = step
        self._max_id = max_id

    def next(self):
        next_id = None
        with self._lock:
            if self._counter.value >= self._max_id:
                self._counter.value = self._base_counter
                _LOGGER.info("Reset Id: {}".format(self._counter.value))
            next_id = self._counter.value
            self._counter.value += self._step
        return next_id
