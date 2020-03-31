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
        self._local_timestamp_file = None
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

    def set_local_timestamp_file(self, timestamp_file):
        self._local_timestamp_file = timestamp_file

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
        if self._local_timestamp_file is None:
            raise Exception('local_timestamp_file not set.')
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
                self._remote_path, self._remote_donefile_name,
                self._local_tmp_path)
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
                                                self._local_timestamp_file)
                    print('{} [INFO] update local donefile'.format(
                        datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')))
            else:
                print('{} [INFO] no donefile.'.format(datetime.datetime.now(
                ).strftime('%Y-%m-%d %H:%M:%S')))
            print('{} [INFO] sleep {}s'.format(datetime.datetime.now().strftime(
                '%Y-%m-%d %H:%M:%S'), self._interval))
            time.sleep(self._interval)

    def _exist_remote_file(self, path, filename, local_tmp_path):
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
                               local_timestamp_file):
        donefile_path = os.path.join(local_path, local_model_name,
                                     local_timestamp_file)
        cmd = 'touch {}'.format(donefile_path)
        if os.system(cmd) != 0:
            raise Exception('update local donefile failed.')


class HDFSMonitor(Monitor):
    ''' HDFS Monitor. '''

    def __init__(self, bin_path, interval=10):
        super(HDFSMonitor, self).__init__(interval)
        self._hdfs_bin_path = bin_path

    def _exist_remote_file(self, path, filename, local_tmp_path):
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

    def __init__(self, host, port, username="", password="", interval=10):
        super(FTPMonitor, self).__init__(interval)
        import ftplib
        self._ftp = ftplib.FTP()
        self._ftp.connect(host, port)
        self._ftp.login(username, password)
        self._ftp_url = 'ftp://{}:{}/'.format(host, port)

    def _exist_remote_file(self, path, filename, local_tmp_path):
        import ftplib
        try:
            filepath = os.path.join(path, filename)
            timestamp = self._ftp.voidcmd('MDTM {}'.format(filepath))[4:].strip(
            )
            return [True, timestamp]
        except ftplib.error_perm:
            return [False, None]

    def _download_remote_files(remote_path,
                               remote_dirname,
                               local_tmp_path,
                               overwrite=True):
        try:
            remote_dirpath = os.path.join(remote_path, remote_dirname)
            self._ftp.cwd(remote_dirpath)
            os.mkdir(os.path.join(local_tmp_path, remote_dirname))
        except OSError:
            # folder already exists at the local_tmp_path
            pass
        except ftplib.error_perm:
            raise Exception('remote_path({}) not exist.'.format(remote_path))

        filelist = [x for x in self_ftp.mlsd()]
        for file in filelist:
            if file[1]['type'] == 'file':
                fullpath = os.path.join(local_tmp_path, remote_dirname, file[0])
                if not overwrite and os.path.isfile(fullpath):
                    continue
                else:
                    with open(fullpath, 'wb') as f:
                        self._ftp.retrbinary('RETR ' + file[0], f.write)
            elif file[1]['type'] == 'dir':
                self._download_remote_files(
                    os.path.join(remote_path, remote_dirname), file[0],
                    os.path.join(local_tmp_path, remote_dirname), overwrite)
            else:
                print('Unknown type: ' + file[1]['type'])

    def _pull_remote_dir(self, remote_path, dirname, local_tmp_path):
        self._exist_remote_file(
            remote_path, dirname, local_tmp_path, overwrite=True)


class GeneralMonitor(Monitor):
    ''' General Monitor. '''

    def __init__(self, host, interval=10):
        super(GeneralMonitor, self).__init__(interval)
        self._host = host

    def _get_local_file_timestamp(self, filename):
        return os.path.getmtime(filename)

    def _exist_remote_file(self, path, filename, local_tmp_path):
        remote_filepath = os.path.join(path, filename)
        url = '{}/{}'.format(self._host, remote_filepath)
        cmd = 'wget -N -P {} {}'.format(local_tmp_path, url)
        if os.system(cmd) != 0:
            return [False, None]
        else:
            timestamp = self._get_local_file_timestamp(
                os.path.join(local_tmp_path, filename))
            return [True, timestamp]

    def _pull_remote_dir(self, remote_path, dirname, local_tmp_path):
        remote_dirpath = os.path.join(remote_path, dirname)
        url = '{}/{}'.format(self._host, remote_dirpath)
        cmd = 'wget -nH -r -P {} {} &> /dev/null'.format(local_tmp_path, url)
        if os.system(cmd) != 0:
            raise Exception('pull remote dir failed.')


def parse_args():
    ''' parse args. '''
    parser = argparse.ArgumentParser(description="Monitor")
    parser.add_argument(
        "--type", type=str, default='general', help="Type of remote server")
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
        "--local_timestamp_file",
        type=str,
        default='fluid_time_file',
        help="Local timestamp file name(fluid_time_file in model file)")
    parser.add_argument(
        "--local_tmp_path",
        type=str,
        default='_serving_monitor_tmp',
        help="Local tmp path")
    parser.add_argument(
        "--interval", type=int, default=10, help="Time interval")
    parser.add_argument(
        "--general_host", type=str, help="Host of general remote server")
    parser.add_argument("--hdfs_bin", type=str, help="Hdfs binary file path")
    parser.add_argument("--ftp_host", type=str, help="Host of ftp")
    parser.add_argument("--ftp_port", type=int, help="Port of ftp")
    parser.add_argument(
        "--ftp_username", type=str, default='', help="Username of ftp")
    parser.add_argument(
        "--ftp_password", type=str, default='', help="Password of ftp")
    return parser.parse_args()


def get_monitor(mtype):
    ''' get monitor. '''
    if mtype == 'ftp':
        return FTPMonitor(
            args.ftp_host,
            args.ftp_port,
            username=args.ftp_username,
            password=args.ftp_password,
            interval=args.interval)
    elif mtype == 'hdfs':
        return HDFSMonitor(args.hdfs_bin, interval=args.interval)
    elif mtype == 'general':
        return GeneralMonitor(args.general_host, interval=args.interval)
    else:
        raise Exception('unsupport type.')


def start_monitor(monitor, args):
    monitor.set_remote_path(args.remote_path)
    monitor.set_remote_model_name(args.remote_model_name)
    monitor.set_remote_donefile_name(args.remote_donefile_name)
    monitor.set_local_path(args.local_path)
    monitor.set_local_model_name(args.local_model_name)
    monitor.set_local_timestamp_file(args.local_timestamp_file)
    monitor.set_local_tmp_path(args.local_tmp_path)
    monitor.run()


if __name__ == "__main__":
    args = parse_args()
    monitor = get_monitor(args.type)
    start_monitor(monitor, args)
