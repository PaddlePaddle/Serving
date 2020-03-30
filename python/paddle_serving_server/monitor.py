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
import commands
import datetime


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
        self._local_tmp_path = None

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

    def set_local_tmp_path(self, tmp_path):
        self._local_tmp_path = tmp_path

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
        if self._local_tmp_path is None:
            raise Exception('local_tmp_path not set.')

    def run(self):
        '''
        Monitor the remote model by polling and update the local model.
        '''
        self._check_params()
        if not os.path.exists(self._local_tmp_path):
            os.makedirs(self._local_tmp_path)
        while True:
            [flag, timestamp] = self._exist_remote_file(
                self._remote_path, self._remote_donefile_name)
            if flag:
                if self._remote_donefile_timestamp is None or \
                        timestamp != self._remote_donefile_timestamp:
                    self._remote_donefile_timestamp = timestamp
                    self._pull_remote_dir(self._remote_path,
                                          self._remote_model_name,
                                          self._local_tmp_path)
                    print('{} [INFO] pull remote model'.format(
                        datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')))
                    self._update_local_model(
                        self._local_tmp_path, self._remote_model_name,
                        self._local_path, self._local_model_name)
                    print('{} [INFO] update model'.format(datetime.datetime.now(
                    ).strftime('%Y-%m-%d %H:%M:%S')))
                    self._update_local_donefile(self._local_path,
                                                self._local_model_name,
                                                self._local_donefile_name)
                    print('{} [INFO] update local donefile'.format(
                        datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')))
            else:
                print('{} [INFO] no donefile.'.format(datetime.datetime.now(
                ).strftime('%Y-%m-%d %H:%M:%S')))
            print('{} [INFO] sleep {}s'.format(datetime.datetime.now().strftime(
                '%Y-%m-%d %H:%M:%S'), self._interval))
            time.sleep(self._interval)

    def _exist_remote_file(self, path, filename):
        raise Exception('This function must be inherited.')

    def _pull_remote_dir(self, remote_path, dirname, local_tmp_path):
        raise Exception('This function must be inherited.')

    def _update_local_model(self, local_tmp_path, remote_model_name, local_path,
                            local_model_name):
        tmp_model_path = os.path.join(local_tmp_path, remote_model_name)
        local_model_path = os.path.join(local_path, local_model_name)
        cmd = 'cp -r {}/* {}'.format(tmp_model_path, local_model_path)
        if os.system(cmd) != 0:
            raise Exception('update local model failed.')

    def _update_local_donefile(self, local_path, local_model_name,
                               local_donefile_name):
        donefile_path = os.path.join(local_path, local_model_name,
                                     local_donefile_name)
        cmd = 'touch {}'.format(donefile_path)
        if os.system(cmd) != 0:
            raise Exception('update local donefile failed.')


class HDFSMonitor(Monitor):
    ''' HDFS Monitor. '''

    def __init__(self, bin_path, interval=10):
        super(HDFSMonitor, self).__init__(interval)
        self._hdfs_bin_path = bin_path

    def _exist_remote_file(self, path, filename):
        remote_filepath = os.path.join(path, filename)
        cmd = '{} dfs -stat "%Y" {}'.format(self._hdfs_bin_path,
                                            remote_filepath)
        [status, timestamp] = commands.getstatusoutput(cmd)
        if status == 0:
            return [True, timestamp]
        else:
            return [False, None]

    def _pull_remote_dir(self, remote_path, dirname, local_tmp_path):
        remote_dirpath = os.path.join(remote_path, dirname)
        cmd = '{} dfs -get -f {} {}'.format(self._hdfs_bin_path, remote_dirpath,
                                            local_tmp_path)
        if os.system(cmd) != 0:
            raise Exception('pull remote dir failed.')


class FTPMonitor(Monitor):
    ''' FTP Monitor. '''

    def __init__(self, ftp_ip, ftp_port, username="", password="", interval=10):
        super(FTPMonitor, self).__init__(interval)
        import ftplib
        self._ftp_ip = ftp_ip
        self._ftp_port = ftp_port
        self._ftp = ftplib.FTP()
        self._connect(ftp_ip, ftp_port, username, password)

    def _connect(self, ftp_ip, ftp_port, username, password):
        self._ftp.connect(ftp_ip, ftp_port)
        self._ftp.login(username, password)

    def _exist_remote_file(self, path, filename):
        import ftplib
        try:
            filepath = os.path.join(path, filename)
            timestamp = self._ftp.voidcmd('MDTM {}'.format(filepath))[4:].strip(
            )
            return [True, timestamp]
        except ftplib.error_perm:
            return [False, None]

    def _pull_remote_dir(self, remote_path, dirname, local_tmp_path):
        filepath = os.path.join(remote_path, dirname)
        cmd = 'wget -nH -r -P {} ftp://{}:{}/{} &> /dev/null'.format(
            local_tmp_path, self._ftp_ip, self._ftp_port, filepath)
        if os.system(cmd) != 0:
            raise Exception('pull remote dir failed.')


def parse_args():
    ''' parse args. '''
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
        "--local_tmp_path", type=str, default='tmp', help="Local tmp path")
    parser.add_argument(
        "--interval", type=int, default=10, help="Time interval")
    parser.add_argument("--ftp_ip", type=str, help="Ip the ftp")
    parser.add_argument("--ftp_port", type=int, help="Port the ftp")
    parser.add_argument(
        "--hdfs_bin", type=str, default='hdfs', help="Hdfs binary file path")
    return parser.parse_args()


def get_monitor(mtype):
    if mtype == 'ftp':
        return FTPMonitor(args.ftp_ip, args.ftp_port, interval=args.interval)
    elif mtype == 'hdfs':
        return HDFSMonitor(args.hdfs_bin, interval=args.interval)
    else:
        raise Exception('unsupport type.')


def start_monitor(monitor, args):
    monitor.set_remote_path(args.remote_path)
    monitor.set_remote_model_name(args.remote_model_name)
    monitor.set_remote_donefile_name(args.remote_donefile_name)
    monitor.set_local_path(args.local_path)
    monitor.set_local_model_name(args.local_model_name)
    monitor.set_local_donefile_name(args.local_donefile_name)
    monitor.set_local_tmp_path(args.local_tmp_path)
    monitor.run()


if __name__ == "__main__":
    args = parse_args()
    monitor = get_monitor(args.type)
    start_monitor(monitor, args)
