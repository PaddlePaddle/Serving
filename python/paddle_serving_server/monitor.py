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
import shutil
import tarfile


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
        self._unpacked_filename = None

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

    def set_unpacked_filename(self, unpacked_filename):
        self._unpacked_filename = unpacked_filename

    def _check_param_help(self, param_name, param_value):
        return "Please check the {}({}) parameter.".format(param_name,
                                                           param_value)

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

    def _decompress_model_file(self, local_tmp_path, model_name,
                               unpacked_filename):
        if unpacked_filename is None:
            return model_name
        tar_model_path = os.path.join(local_tmp_path, model_name)
        if not tarfile.is_tarfile(tar_model_path):
            raise Exception('not a tar packaged file type. {}'.format(
                self._check_param_help('remote_model_name', model_name)))
        try:
            tar = tarfile.open(tar_model_path)
            tar.extractall(local_tmp_path)
            tar.close()
        except:
            raise Exception(
                'Decompressing failed, maybe no disk space left. {}'.foemat(
                    self._check_param_help('local_tmp_path', local_tmp_path)))
        finally:
            os.remove(tar_model_path)
            if not os.path.exists(unpacked_filename):
                raise Exception('file not exist. {}'.format(
                    self._check_param_help('unpacked_filename',
                                           unpacked_filename)))
            return unpacked_filename

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
                    unpacked_filename = self._decompress_model_file(
                        self._local_tmp_path, self._remote_model_name,
                        self._unpacked_filename)
                    self._update_local_model(
                        self._local_tmp_path, unpacked_filename,
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


class AFSMonitor(Monitor):
    ''' AFS Monitor(by hadoop-client). '''

    def __init__(self,
                 hadoop_bin,
                 hadoop_host=None,
                 hadoop_ugi=None,
                 interval=10):
        super(AFSMonitor, self).__init__(interval)
        self._hadoop_bin = hadoop_bin
        self._hadoop_host = hadoop_host
        self._hadoop_ugi = hadoop_ugi
        self._cmd_prefix = '{} fs '.format(self._hadoop_bin)
        if not self._hadoop_host and not self._hadoop_ugi:
            self._cmd_prefix += '-D fs.default.name={} -D hadoop.job.ugi={} '.format(
                self._hadoop_host, self._hadoop_ugi)

    def _exist_remote_file(self, path, filename, local_tmp_path):
        remote_filepath = os.path.join(path, filename)
        cmd = '{} -ls {}'.format(self._cmd_prefix, remote_filepath)
        [status, output] = commands.getstatusoutput(cmd)
        if status == 0:
            [_, _, _, _, _, mdate, mtime, _] = output.split('\n')[-1]
            timestr = mdate + mtime
            return [True, timestr]
        else:
            return [False, None]

    def _pull_remote_dir(self, remote_path, dirname, local_tmp_path):
        # remove old file before pull remote dir
        local_dirpath = os.path.join(local_tmp_path, dirname)
        if os.path.exists(local_dirpath):
            shutil.rmtree(local_dirpath)
        remote_dirpath = os.path.join(remote_path, dirname)
        cmd = '{} -get {} {}'.format(self._cmd_prefix, remote_dirpath,
                                     local_dirpath)
        if os.system(cmd) != 0:
            raise Exception('pull remote dir failed. {}'.format(
                self._check_param_help('remote_model_name', dirname)))


class HDFSMonitor(Monitor):
    ''' HDFS Monitor. '''

    def __init__(self, bin_path, interval=10):
        super(HDFSMonitor, self).__init__(interval)
        self._hdfs_bin_path = bin_path
        self._prefix_cmd = '{} dfs '.format(self._hdfs_bin_path)

    def _exist_remote_file(self, path, filename, local_tmp_path):
        remote_filepath = os.path.join(path, filename)
        cmd = '{} -stat "%Y" {}'.format(self._prefix_cmd, remote_filepath)
        [status, timestamp] = commands.getstatusoutput(cmd)
        if status == 0:
            return [True, timestamp]
        else:
            return [False, None]

    def _pull_remote_dir(self, remote_path, dirname, local_tmp_path):
        remote_dirpath = os.path.join(remote_path, dirname)
        cmd = '{} -get -f {} {}'.format(self._prefix_cmd, remote_dirpath,
                                        local_tmp_path)
        if os.system(cmd) != 0:
            raise Exception('pull remote dir failed. {}'.format(
                self._check_param_help('remote_model_name', dirname)))


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
            self._ftp.cwd(path)
            timestamp = self._ftp.voidcmd('MDTM {}'.format(filename))[4:].strip(
            )
            return [True, timestamp]
        except ftplib.error_perm:
            return [False, None]

    def _download_remote_file(self,
                              remote_path,
                              remote_filename,
                              local_tmp_path,
                              overwrite=True):
        local_fullpath = os.path.join(local_tmp_path, remote_filename)
        if not overwrite and os.path.isfile(fullpath):
            return
        else:
            with open(local_fullpath, 'wb') as f:
                self._ftp.cwd(remote_path)
                self._ftp.retrbinary('RETR {}'.format(remote_filename), f.write)

    def _download_remote_files(self,
                               remote_path,
                               remote_dirname,
                               local_tmp_path,
                               overwrite=True):
        import ftplib
        remote_dirpath = os.path.join(remote_path, remote_dirname)
        # Check whether remote_dirpath is a file or a folder
        try:
            self._ftp.cwd(remote_dirpath)

            local_dirpath = os.path.join(local_tmp_path, remote_dirname)
            if not os.path.exists(local_dirpath):
                os.mkdir(local_dirpath)

            output = []
            self._ftp.dir(output.append)
            for line in output:
                [attr, _, _, _, _, _, _, _, name] = line.split()
                if attr[0] == 'd':
                    self._download_remote_files(
                        os.path.join(remote_path, remote_dirname), name,
                        os.path.join(local_tmp_path, remote_dirname), overwrite)
                else:
                    self._download_remote_file(remote_dirname, name,
                                               local_tmp_path, overwrite)
        except ftplib.error_perm:
            self._download_remote_file(remote_path, remote_dirname,
                                       local_tmp_path, overwrite)
            return

    def _pull_remote_dir(self, remote_path, dirname, local_tmp_path):
        self._download_remote_files(
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
        cmd = 'wget -N -P {} {} &>/dev/null'.format(local_tmp_path, url)
        if os.system(cmd) != 0:
            return [False, None]
        else:
            timestamp = self._get_local_file_timestamp(
                os.path.join(local_tmp_path, filename))
            return [True, timestamp]

    def _pull_remote_dir(self, remote_path, dirname, local_tmp_path):
        remote_dirpath = os.path.join(remote_path, dirname)
        url = '{}/{}'.format(self._host, remote_dirpath)
        cmd = 'wget -nH -r -P {} {} &>/dev/null'.format(local_tmp_path, url)
        if os.system(cmd) != 0:
            raise Exception('pull remote dir failed. {}'.format(
                self._check_param_help('remote_model_name', dirname)))


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
        "--unpacked_filename",
        type=str,
        default=None,
        help="If the model of the remote production is a packaged file, the unpacked file name should be set. Currently, only tar packaging format is supported."
    )
    parser.add_argument(
        "--interval", type=int, default=10, help="Time interval")
    # general monitor
    parser.add_argument(
        "--general_host", type=str, help="Host of general remote server")
    # hdfs monitor
    parser.add_argument("--hdfs_bin", type=str, help="Hdfs binary file path")
    # ftp monitor
    parser.add_argument("--ftp_host", type=str, help="Host of ftp")
    parser.add_argument("--ftp_port", type=int, help="Port of ftp")
    parser.add_argument(
        "--ftp_username", type=str, default='', help="Username of ftp")
    parser.add_argument(
        "--ftp_password", type=str, default='', help="Password of ftp")
    # afs monitor
    parser.add_argument(
        "--hadoop_bin", type=str, help="Hadoop_bin_path for afs")
    parser.add_argument(
        "--hadoop_host", type=str, default=None, help="Hadoop_host for afs")
    parser.add_argument(
        "--hadoop_ugi", type=str, default=None, help="Hadoop_ugi for afs")
    return parser.parse_args()


def get_monitor(mtype):
    """ generator monitor instance.

    Args:
        mtype: type of monitor

    Returns:
        monitor instance.
    """
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
    elif mtype == 'afs':
        return AFSMonitor(
            args.hadoop_bin,
            args.hadoop_host,
            args.hadoop_ugi,
            interval=args.interval)
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
    monitor.set_unpacked_filename(args.unpacked_filename)
    monitor.run()


if __name__ == "__main__":
    args = parse_args()
    monitor = get_monitor(args.type)
    start_monitor(monitor, args)
