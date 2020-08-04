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

import logging
import logging.handlers
import os


class SectionLevelFilter(object):
    def __init__(self, levels):
        self._levels = levels

    def filter(self, logRecord):
        return logRecord.levelno in self._levels


class OutOfMouduleFilter(object):
    def __init__(self, out_names):
        self._out_names = out_names

    def filter(self, logRecord):
        return logRecord.name not in self._out_names


class OutOfMouduleAndSectionLevelFilter(object):
    def __init__(self, out_names, levels):
        self._out_names = out_names
        self._levels = levels

    def filter(self, logRecord):
        if logRecord.name in self._out_names:
            return False
        return logRecord.levelno in self._levels


class StreamHandler(logging.StreamHandler):
    def __init__(self, *args, **kwargs):
        super(StreamHandler, self).__init__(*args, **kwargs)
        self.addFilter(OutOfMouduleFilter(["pipeline.profiler"]))


log_dir = "PipelineServingLogs"
if not os.path.exists(log_dir):
    os.makedirs(log_dir)

# root logger
_LOGGER = logging.getLogger()
_LOGGER.setLevel(logging.DEBUG)

formatter = logging.Formatter(
    "%(levelname)s %(asctime)s [%(filename)s:%(lineno)d] %(message)s")
# info and warn
file_info = logging.handlers.RotatingFileHandler(
    os.path.join(log_dir, "INFO.log"))
file_info.addFilter(OutOfMouduleFilter(["pipeline.profiler"]))
file_info.addFilter(SectionLevelFilter([logging.INFO, logging.WARNING]))
file_info.setFormatter(formatter)

# err and critical
file_err = logging.handlers.RotatingFileHandler(
    os.path.join(log_dir, "ERROR.log"))
file_err.addFilter(OutOfMouduleFilter(["pipeline.profiler"]))
file_err.setLevel(logging.ERROR)
file_err.setFormatter(formatter)

_LOGGER.addHandler(file_info)
_LOGGER.addHandler(file_err)

# tracer logger
_TRACER = logging.getLogger("pipeline.profiler")
_TRACER.setLevel(logging.INFO)
_TRACER.addFilter(logging.Filter("pipeline.profiler"))

# tracer
tracer_formatter = logging.Formatter("%(asctime)s %(message)s")
file_trace = logging.handlers.RotatingFileHandler(
    os.path.join(log_dir, "TRACE.log"))
file_trace.setFormatter(tracer_formatter)
_TRACER.addHandler(file_trace)
