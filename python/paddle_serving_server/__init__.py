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

from . import dag
from . import monitor
from . import rpc_service
from . import serve
from . import web_service
from . import version

from dag import *
from monitor import *
from rpc_service import *
from serve import *
from web_service import *
from version import *

SERVER_VERSION = "0.0.0"
__version__ = SERVER_VERSION