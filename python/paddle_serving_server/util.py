# coding:utf-8
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
import signal
import os
import time
import json
import platform
from paddle_serving_server.env import CONF_HOME


def pid_is_exist(pid: int):
    '''
    Try to kill process by PID.

    Args:
        pid(int): PID of process to be killed.

    Returns:
         True if PID will be killed.

    Examples:
    .. code-block:: python

        pid_is_exist(pid=8866)
    '''
    try:
        os.kill(pid, 0)
    except:
        return False
    else:
        return True

def kill_stop_process_by_pid(command : str, pid : int):
    '''
    using different signals to kill process group by PID .

    Args:
        command(str): stop->SIGINT, kill->SIGKILL
        pid(int): PID of process to be killed.

    Returns:
       None
    Examples:
    .. code-block:: python

       kill_stop_process_by_pid("stop", 9494)
    '''
    if not pid_is_exist(pid):
        print("Process [%s]  has been stopped."%pid)
        return
    if platform.system() == "Windows":
        os.kill(pid, signal.SIGINT)   
    else:
        try:
             if command == "stop":
                 os.killpg(pid, signal.SIGINT)
             elif command == "kill":
                 os.killpg(pid, signal.SIGKILL)
        except ProcessLookupError:
             if command == "stop":
                 os.kill(pid, signal.SIGINT)
             elif command == "kill":
                 os.kill(pid, signal.SIGKILL)

def dump_pid_file(portList, model):
    '''
    Write PID info to file.

    Args:
        portList(List): PiplineServing includes http_port and rpc_port
                        PaddleServing include one port
        model(str): 'Pipline' for PiplineServing
                    Specific model list for ServingModel 

    Returns:
       None
    Examples:
    .. code-block:: python

       dump_pid_file([9494, 10082], 'serve')
    '''
    pid = os.getpid()
    if platform.system() == "Windows":
        gid = pid
    else:
        gid = os.getpgid(pid)      
    pidInfoList = []
    filepath = os.path.join(CONF_HOME, "ProcessInfo.json")
    if os.path.exists(filepath):
        if os.path.getsize(filepath):
            with open(filepath, "r") as fp:
                pidInfoList = json.load(fp)
                # delete old pid data when new port number is same as old's
                for info in pidInfoList:
                    storedPort = list(info["port"])
                    interList = list(set(portList)&set(storedPort))
                    if interList:
                        pidInfoList.remove(info)

    with open(filepath, "w") as fp:
        info ={"pid": gid, "port" : portList, "model" : str(model), "start_time" : time.time()}
        pidInfoList.append(info)
        json.dump(pidInfoList, fp)

def load_pid_file(filepath: str):
    '''
    Read PID info from file.
    '''
    if not os.path.exists(filepath):
        raise ValueError(
            "ProcessInfo.json file is not exists, All processes of PaddleServing has been stopped.")
        return False
    
    if os.path.getsize(filepath):
        with open(filepath, "r") as fp:
            infoList = json.load(fp)
            return infoList
    else:
        os.remove(filepath)
        print("ProcessInfo.json file is empty, All processes of PaddleServing has been stopped.")
        return False

import yaml
from argparse import ArgumentParser,RawDescriptionHelpFormatter

class ArgsParser(ArgumentParser):
    def __init__(self):
        super(ArgsParser, self).__init__(
            formatter_class=RawDescriptionHelpFormatter)
        self.add_argument("-c", "--config", help="configuration file to use")
        self.add_argument(
            "-o", "--opt", nargs='+', help="set configuration options")

    def parse_args(self, argv=None):
        args = super(ArgsParser, self).parse_args(argv)
        assert args.config is not None, \
            "Please specify --config=configure_file_path."
        args.conf_dict = self._parse_opt(args.opt, args.config)
        return args

    def _parse_helper(self, v):
        if v.isnumeric():
            if "." in v:
                v = float(v)
            else:
                v = int(v)
        elif v == "True" or v == "False":
            v = (v == "True")
        return v

    def _parse_opt(self, opts, conf_path):
        f = open(conf_path)
        config = yaml.load(f, Loader=yaml.Loader)
        if not opts:
            return config
        for s in opts:
            s = s.strip()
            k, v = s.split('=')
            v = self._parse_helper(v)
            cur = config
            parent = cur
            for kk in k.split("."):
                if kk not in cur:
                     cur[kk] = {}
                     parent = cur
                     cur = cur[kk]
                else:
                     parent = cur
                     cur = cur[kk]
            parent[k.split(".")[-1]] = v
        return config
