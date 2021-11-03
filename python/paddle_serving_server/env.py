# coding:utf-8
# Copyright (c) 2020  PaddlePaddle Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License"
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
'''
This module is used to store environmental variables in PaddleServing.


SERVING_HOME              -->  the root directory for storing Paddleserving related data. Default to the current directory of starting PaddleServing . Users can
                               change the default value through the SERVING_HOME environment variable.
CONF_HOME                 -->  Store the default configuration files.

'''

import os
import sys

def _get_user_home():
    return os.path.expanduser(os.getcwd())


def _get_serving_home():
    if 'SERVING_HOME' in os.environ:
        home_path = os.environ['SERVING_HOME']
        if os.path.exists(home_path):
            if os.path.isdir(home_path):
                return home_path
            else:
                raise RuntimeError('The environment variable SERVING_HOME {} is not a directory.'.format(home_path))
        else:
            return home_path
    return os.path.join(_get_user_home())


def _get_sub_home(directory):
    home = os.path.join(_get_serving_home(), directory)
    if not os.path.exists(home):
        os.makedirs(home)
    return home

SERVING_HOME = _get_serving_home()
CONF_HOME = _get_sub_home("")
