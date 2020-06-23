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


class ChannelDataEcode(enum.Enum):
    OK = 0
    TIMEOUT = 1
    NOT_IMPLEMENTED = 2
    TYPE_ERROR = 3
    RPC_PACKAGE_ERROR = 4
    UNKNOW = 5


class ChannelDataType(enum.Enum):
    DICT = 0
    CHANNEL_NPDATA = 1
    ERROR = 2


class ChannelData(object):
    pass


class ThreadChannel(Queue.Queue):
    pass


class ProcessChannel(multiprocessing.queues.Queue):
    pass
