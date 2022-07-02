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
import logging.config
import os


class SectionLevelFilter(object):
    def __init__(self, levels):
        self._levels = levels

    def filter(self, logRecord):
        return logRecord.levelno in self._levels

log_dir = "PipelineServingLogs"
if 'SERVING_LOG_PATH' in os.environ:
    serving_log_path = os.environ['SERVING_LOG_PATH']
    log_dir = os.path.join(serving_log_path, log_dir)

if not os.path.exists(log_dir):
    os.makedirs(log_dir)

logger_config = {
    "version": 1,
    "formatters": {
        "normal_fmt": {
            "format":
            "%(levelname)s %(asctime)s [%(filename)s:%(lineno)d] %(message)s",
        },
        "tracer_fmt": {
            "format": "%(asctime)s %(message)s",
        },
    },
    "handlers": {
        "f_pipeline.log": {
            "class": "logging.handlers.RotatingFileHandler",
            "level": "INFO",
            "formatter": "normal_fmt",
            "filename": os.path.join(log_dir, "pipeline.log"),
            "maxBytes": 512000000,
            "backupCount": 20,
        },
        "f_pipeline.log.wf": {
            "class": "logging.handlers.RotatingFileHandler",
            "level": "WARNING",
            "formatter": "normal_fmt",
            "filename": os.path.join(log_dir, "pipeline.log.wf"),
            "maxBytes": 512000000,
            "backupCount": 10,
        },
        "f_tracer.log": {
            "class": "logging.handlers.RotatingFileHandler",
            "level": "INFO",
            "formatter": "tracer_fmt",
            "filename": os.path.join(log_dir, "pipeline.tracer"),
            "maxBytes": 512000000,
            "backupCount": 5,
        },
    },
    "loggers": {
        # propagate = True
        ".".join(__name__.split(".")[:-1] + ["profiler"]): {
            "level": "INFO",
            "handlers": ["f_tracer.log"],
        },
    },
    "root": {
        "level": "DEBUG",
        "handlers": ["f_pipeline.log", "f_pipeline.log.wf"],
    },
}

logging.config.dictConfig(logger_config)
