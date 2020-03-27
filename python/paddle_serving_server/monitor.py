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
"""
Usage:
    Start monitor with one line command
    Example:
        python -m paddle_serving_server.monitor
"""
import os
import time
import argparse


class Monitor(object):
    '''
    Monitor base class. It is used to monitor the remote model, pull and update the local model.
    '''

    def __init__(self, interval):
        self._remote_path = None
        self._remote_model_name = None
        self._remote_donefile_name = None
        self._local_path = None
        self._local_model_name = None
        self._local_donefile_name = None
        self._interval = interval
        self._remote_donefile_timestamp = None
        self._local_tmp_dir = None

    def set_remote_path(self, remote_path):
        self._remote_path = remote_path

    def set_remote_model_name(self, model_name):
        self._remote_model_name = model_name

    def set_remote_donefile_name(self, donefile_name):
        self._remote_donefile_name = donefile_name

    def set_local_path(self, local_path):
        self._local_path = local_path

    def set_local_model_name(self, model_name):
        self._local_model_name = model_name

    def set_local_donefile_name(self, donefile_name):
        self._local_donefile_name = donefile_name

    def set_local_tmp_dir(self, tmp_dir):
        self._local_tmp_dir = tmp_dir

    def _check_params(self):
        if self._remote_path is None:
            raise Exception('remote_path not set.')
        if self._remote_model_name is None:
            raise Exception('remote_model_name not set.')
        if self._remote_donefile_name is None:
            raise Exception('remote_donefile_name not set.')
        if self._local_model_name is None:
            raise Exception('local_model_name not set.')
        if self._local_path is None:
            raise Exception('local_path not set.')
        if self._local_donefile_name is None:
            raise Exception('local_donefile_name not set.')
        if self._local_tmp_dir is None:
            raise Exception('local_tmp_dir not set.')

    def run(self):
        '''
        Monitor the remote model by polling and update the local model.
        '''
        self._check_params()
        if not os.path.exists(self._local_tmp_dir):
            os.makedirs(self._local_tmp_dir)
        while True:
            [flag, timestamp] = self._exist_remote_donefile()
            if flag:
                if self._remote_donefile_timestamp is None or \
                        timestamp != self._remote_donefile_timestamp:
                    self._remote_donefile_timestamp = timestamp
                    self._pull_remote_model()
                    print('[INFO] pull remote model')
                    self._update_local_model()
                    print('[INFO] update model')
                    self._update_local_donefile()
                    print('[INFO] update local donefile')
            else:
                print('[INFO] no donefile.')
            print('[INFO] sleep {}s'.format(self._interval))
            time.sleep(self._interval)

    def _exist_remote_donefile(self):
        raise Exception('This function must be inherited.')

    def _pull_remote_model(self):
        raise Exception('This function must be inherited.')

    def _update_local_model(self):
        raise Exception('This function must be inherited.')

    def _update_local_donefile(self):
        raise Exception('This function must be inherited.')


class FTPMonitor(Monitor):
    def __init__(self, ftp_ip, ftp_port, username="", password="", interval=10):
        import ftplib
        super(FTPMonitor, self).__init__(interval)
        self._ftp_ip = ftp_ip
        self._ftp_port = ftp_port
        self._ftp = ftplib.FTP()
        self._connect(ftp_ip, ftp_port, username, password)

    def _connect(self, ftp_ip, ftp_port, username, password):
        self._ftp.connect(ftp_ip, ftp_port)
        self._ftp.login(username, password)

    def _exist_remote_donefile(self):
        import ftplib
        try:
            donefile_path = '{}/{}'.format(self._remote_path,
                                           self._remote_donefile_name)
            timestamp = self._ftp.voidcmd('MDTM {}'.format(donefile_path))[
                4:].strip()
            return [True, timestamp]
        except ftplib.error_perm:
            return [False, None]

    def _pull_remote_model(self):
        cmd = 'wget -nH -r -P {} ftp://{}:{}/{}/{} &> /dev/null'.format(
            self._local_tmp_dir, self._ftp_ip, self._ftp_port,
            self._remote_path, self._remote_model_name)
        if os.system(cmd) != 0:
            raise Exception('pull remote model failed.')

    def _update_local_model(self):
        cmd = 'cp -r {}/{}/* {}/{}'.format(
            self._local_tmp_dir, self._remote_model_name, self._local_path,
            self._local_model_name)
        if os.system(cmd) != 0:
            raise Exception('update local model failed.')

    def _update_local_donefile(self):
        cmd = 'touch {}/{}/{}'.format(self._local_path, self._local_model_name,
                                      self._local_donefile_name)
        if os.system(cmd) != 0:
            raise Exception('update local donefile failed.')


def parse_args():
    parser = argparse.ArgumentParser(description="Monitor")
    parser.add_argument(
        "--type", type=str, required=True, help="Type of remote server")
    parser.add_argument(
        "--remote_path", type=str, required=True, help="Remote path")
    parser.add_argument(
        "--remote_model_name",
        type=str,
        required=True,
        help="Remote model name")
    parser.add_argument(
        "--remote_donefile_name",
        type=str,
        required=True,
        help="Remote donefile name")
    parser.add_argument(
        "--local_path", type=str, required=True, help="Local path")
    parser.add_argument(
        "--local_model_name", type=str, required=True, help="Local model name")
    parser.add_argument(
        "--local_donefile_name",
        type=str,
        required=True,
        help="Local donfile name(fluid_time_file in model file)")
    parser.add_argument(
        "--local_tmp_dir", type=str, default='tmp', help="Local tmp dir")
    parser.add_argument(
        "--interval", type=int, default=10, help="Time interval")
    parser.add_argument("--ftp_ip", type=str, help="Ip the ftp")
    parser.add_argument("--ftp_port", type=int, help="Port the ftp")
    return parser.parse_args()


def start_ftp_monitor():
    args = parse_args()
    obj = FTPMonitor(args.ftp_ip, args.ftp_port, interval=args.interval)
    obj.set_remote_path(args.remote_path)
    obj.set_remote_model_name(args.remote_model_name)
    obj.set_remote_donefile_name(args.remote_donefile_name)
    obj.set_local_path(args.local_path)
    obj.set_local_model_name(args.local_model_name)
    obj.set_local_donefile_name(args.local_donefile_name)
    obj.set_local_tmp_dir(args.local_tmp_dir)
    obj.run()


if __name__ == "__main__":
    args = parse_args()
    if args.type == 'ftp':
        start_ftp_monitor()
    else:
        raise Exception('unsupport type.')
