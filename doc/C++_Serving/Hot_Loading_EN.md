# Hot Loading in Paddle Serving

([简体中文](./Hot_Loading_CN.md)|English)

## Background

In the industrial scenario, it is usually the remote periodic output model, and the online server needs to pull down the new model to update the old model without service interruption.

## Server Monitor

Paddle Serving provides an automatic monitoring script. After the remote address updates the model, the new model will be pulled to update the local model. At the same time, the `fluid_time_stamp` in the local model folder will be updated to realize model hot loading.

Currently, the following types of Monitors are supported:

| Monitor Type |                         Description                          |                       Specific options                       |
| :----------: | :----------------------------------------------------------: | :----------------------------------------------------------: |
|   general    | Without authentication, you can directly access the download file by `wget` (such as FTP and BOS which do not need authentication) |             `general_host` General remote host.              |
|     hdfs/afs(HadoopMonitor)     | The remote is HDFS or AFS, and relevant commands are executed through Hadoop-client | `hadoop_bin` Path of Hadoop binary file.<br/>`fs_name` Hadoop fs_name. Not used if set in Hadoop-client.<br/>`fs_ugi` Hadoop fs_ugi, Not used if set in Hadoop-client. |
|     ftp      | The remote is FTP, and relevant commands are executed through `ftplib`(Using this monitor, you need to install `ftplib` with command `pip install ftplib`) | `ftp_host` FTP remote host.<br>`ftp_port` FTP remote port.<br>`ftp_username` FTP username. Not used if anonymous access.<br>`ftp_password` FTP password. Not used if anonymous access. |

| Monitor Shared options |                         Description                          |               Default                |
| :--------------------: | :----------------------------------------------------------: | :----------------------------------: |
|         `type`         |                 Specify the type of monitor                  |                  /                   |
|     `remote_path`      |             Specify the base path for the remote             |                  /                   |
|  `remote_model_name`   |     Specify the model name to be pulled from the remote      |                  /                   |
| `remote_donefile_name` | Specify the donefile name that marks the completion of the remote model update |                  /                   |
|      `local_path`      |                   Specify local work path                    |                  /                   |
|   `local_model_name`   |                   Specify local model name                   |                  /                   |
| `local_timestamp_file` | Specify the timestamp file used locally for hot loading, The file is considered to be placed in the `local_path/local_model_name` folder. |          `fluid_time_file`           |
|    `local_tmp_path`    | Specify the path of the folder where temporary files are stored locally. If it does not exist, it will be created automatically. |        `_serving_monitor_tmp`        |
|       `interval`       |           Specify the polling interval in seconds.           |                 `10`                 |
|  `unpacked_filename`   | Monitor supports the `tarfile` packaged remote model file. If the remote model is in a packaged format, you need to set this option to tell monitor the name of the extracted file. |                `None`                |
|        `debug`         | If the `--debug` option is added, more detailed intermediate information will be output. | This option is not added by default. |

The following is an example of HadoopMonitor to show the model hot loading of Paddle Serving.

## HadoopMonitor example

In this example, the production model is uploaded to HDFS in `product_path` folder, and the server hot loads the model in `server_path` folder:

```shell
.
├── product_path
└── server_path
```

### Product model

Run the following Python code products model in `product_path` folder(You need to modify Hadoop related parameters before running). Every 60 seconds, the package file of Boston house price prediction model `uci_housing.tar.gz` will be generated and uploaded to the path of HDFS `/`. After uploading, the timestamp file `donefile` will be updated and uploaded to the path of HDFS `/`.

```python
import os
import sys
import time
import tarfile
import paddle
import paddle.fluid as fluid
import paddle_serving_client.io as serving_io

train_reader = paddle.batch(
    paddle.reader.shuffle(
        paddle.dataset.uci_housing.train(), buf_size=500),
    batch_size=16)

test_reader = paddle.batch(
    paddle.reader.shuffle(
        paddle.dataset.uci_housing.test(), buf_size=500),
    batch_size=16)

x = fluid.data(name='x', shape=[None, 13], dtype='float32')
y = fluid.data(name='y', shape=[None, 1], dtype='float32')

y_predict = fluid.layers.fc(input=x, size=1, act=None)
cost = fluid.layers.square_error_cost(input=y_predict, label=y)
avg_loss = fluid.layers.mean(cost)
sgd_optimizer = fluid.optimizer.SGD(learning_rate=0.01)
sgd_optimizer.minimize(avg_loss)

place = fluid.CPUPlace()
feeder = fluid.DataFeeder(place=place, feed_list=[x, y])
exe = fluid.Executor(place)
exe.run(fluid.default_startup_program())

def push_to_hdfs(local_file_path, remote_path):
    afs = 'afs://***.***.***.***:***' # User needs to change
    uci = '***,***' # User needs to change
    hadoop_bin = '/path/to/haddop/bin' # User needs to change
    prefix = '{} fs -Dfs.default.name={} -Dhadoop.job.ugi={}'.format(hadoop_bin, afs, uci)
    os.system('{} -rmr {}/{}'.format(
      prefix, remote_path, local_file_path))
    os.system('{} -put {} {}'.format(
      prefix, local_file_path, remote_path))

name = "uci_housing"
for pass_id in range(30):
    for data_train in train_reader():
        avg_loss_value, = exe.run(fluid.default_main_program(),
                                  feed=feeder.feed(data_train),
                                  fetch_list=[avg_loss])
    # Simulate the production model every other period of time
    time.sleep(60)
    model_name = "{}_model".format(name)
    client_name = "{}_client".format(name)
    serving_io.save_model(model_name, client_name,
                          {"x": x}, {"price": y_predict},
                          fluid.default_main_program())
    # Packing model
    tar_name = "{}.tar.gz".format(name)
    tar = tarfile.open(tar_name, 'w:gz')
    tar.add(model_name)
    tar.close()

    # Push packaged model file to hdfs
    push_to_hdfs(tar_name, '/')

    # Generate donefile
    donefile_name = 'donefile'
    os.system('touch {}'.format(donefile_name))

    # Push donefile to hdfs
    push_to_hdfs(donefile_name, '/')
```

The files on HDFS are as follows:

```bash
# hadoop fs -ls /
Found 2 items
-rw-r--r--   1 root supergroup          0 2020-04-02 02:54 /donefile
-rw-r--r--   1 root supergroup       2101 2020-04-02 02:54 /uci_housing.tar.gz
```

### Server loading model

Enter the `server_path` folder.

#### Start server with the initial model

Here, the trained Boston house price prediction model is used as the initial model:

```shell
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/uci_housing.tar.gz
tar -xzf uci_housing.tar.gz
```

Start Server：

```shell
python -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9292
```

#### Execute monitor

Use the following command to execute the HDFSMonitor:

```shell
python -m paddle_serving_server.monitor \
	--type='hdfs' --hadoop_bin='/hadoop-3.1.2/bin/hadoop' \
	--remote_path='/' --remote_model_name='uci_housing.tar.gz' \
	--remote_donefile_name='donefile' --local_path='.' \
	--local_model_name='uci_housing_model' --local_timestamp_file='fluid_time_file' \
	--local_tmp_path='_tmp' --unpacked_filename='uci_housing_model' --debug
```

The above code monitors the remote timestamp file `/donefile` of the remote HDFS address `/` every 10 seconds by polling. When the remote timestamp file changes, the remote model is considered to have been updated. Pull the remote packaging model `/uci_housing.tar.gz` to the local temporary path `./_tmp/uci_housing.tar.gz`. After unpacking to get the model file `./_tmp/uci_housing_model`, update the local model `./uci_housing_model` and the model timestamp file `./uci_housing_model/fluid_time_file` of Paddle Serving.

The expected output is as follows:

```shell
2020-04-02 10:12 INFO     [monitor.py:85] _hadoop_bin: /hadoop-3.1.2/bin/hadoop
2020-04-02 10:12 INFO     [monitor.py:85] _fs_name:
2020-04-02 10:12 INFO     [monitor.py:85] _fs_ugi:
2020-04-02 10:12 INFO     [monitor.py:209] AFS prefix cmd: /hadoop-3.1.2/bin/hadoop fs
2020-04-02 10:12 INFO     [monitor.py:85] _remote_path: /
2020-04-02 10:12 INFO     [monitor.py:85] _remote_model_name: uci_housing.tar.gz
2020-04-02 10:12 INFO     [monitor.py:85] _remote_donefile_name: donefile
2020-04-02 10:12 INFO     [monitor.py:85] _local_model_name: uci_housing_model
2020-04-02 10:12 INFO     [monitor.py:85] _local_path: .
2020-04-02 10:12 INFO     [monitor.py:85] _local_timestamp_file: fluid_time_file
2020-04-02 10:12 INFO     [monitor.py:85] _local_tmp_path: _tmp
2020-04-02 10:12 INFO     [monitor.py:85] _interval: 10
2020-04-02 10:12 DEBUG    [monitor.py:214] check cmd: /hadoop-3.1.2/bin/hadoop fs  -ls /donefile 2>/dev/null
2020-04-02 10:12 DEBUG    [monitor.py:216] resp: -rw-r--r--   1 root supergroup          0 2020-04-02 10:11 /donefile
2020-04-02 10:12 INFO     [monitor.py:138] doneilfe(donefile) changed.
2020-04-02 10:12 DEBUG    [monitor.py:233] pull cmd: /hadoop-3.1.2/bin/hadoop fs  -get /uci_housing.tar.gz _tmp/uci_housing.tar.gz 2>/dev/null
2020-04-02 10:12 INFO     [monitor.py:144] pull remote model(uci_housing.tar.gz).
2020-04-02 10:12 INFO     [monitor.py:98] unpack remote file(uci_housing.tar.gz).
2020-04-02 10:12 DEBUG    [monitor.py:108] remove packed file(uci_housing.tar.gz).
2020-04-02 10:12 INFO     [monitor.py:110] using unpacked filename: uci_housing_model.
2020-04-02 10:12 DEBUG    [monitor.py:175] update model cmd: cp -r _tmp/uci_housing_model/* ./uci_housing_model
2020-04-02 10:12 INFO     [monitor.py:152] update local model(uci_housing_model).
2020-04-02 10:12 DEBUG    [monitor.py:184] update timestamp cmd: touch ./uci_housing_model/fluid_time_file
2020-04-02 10:12 INFO     [monitor.py:157] update model timestamp(fluid_time_file).
2020-04-02 10:12 INFO     [monitor.py:161] sleep 10s.
2020-04-02 10:12 DEBUG    [monitor.py:214] check cmd: /hadoop-3.1.2/bin/hadoop fs  -ls /donefile 2>/dev/null
2020-04-02 10:12 DEBUG    [monitor.py:216] resp: -rw-r--r--   1 root supergroup          0 2020-04-02 10:11 /donefile
2020-04-02 10:12 INFO     [monitor.py:161] sleep 10s.
```



#### View server logs

View the running log of the server with the following command:

```shell
tail -f log/serving.INFO
```

The log shows that the model has been hot loaded:

```shell
I0330 09:38:40.087316  7361 server.cpp:150] Begin reload framework...
W0330 09:38:40.087399  7361 infer.h:656] Succ reload version engine: 18446744073709551615
I0330 09:38:40.087414  7361 manager.h:131] Finish reload 1 workflow(s)
I0330 09:38:50.087535  7361 server.cpp:150] Begin reload framework...
W0330 09:38:50.087641  7361 infer.h:250] begin reload model[uci_housing_model].
I0330 09:38:50.087972  7361 infer.h:66] InferEngineCreationParams: model_path = uci_housing_model, enable_memory_optimization = 0, static_optimization = 0, force_update_static_cache = 0
I0330 09:38:50.088027  7361 analysis_predictor.cc:88] Profiler is deactivated, and no profiling report will be generated.
I0330 09:38:50.088393  7361 analysis_predictor.cc:841] MODEL VERSION: 1.7.1
I0330 09:38:50.088413  7361 analysis_predictor.cc:843] PREDICTOR VERSION: 1.6.3
I0330 09:38:50.089519  7361 graph_pattern_detector.cc:96] ---  detected 1 subgraphs
I0330 09:38:50.090925  7361 analysis_predictor.cc:470] ======= optimize end =======
W0330 09:38:50.090986  7361 infer.h:472] Succ load common model[0x7fc83c06abd0], path[uci_housing_model].
I0330 09:38:50.091022  7361 analysis_predictor.cc:88] Profiler is deactivated, and no profiling report will be generated.
W0330 09:38:50.091050  7361 infer.h:509] td_core[0x7fc83c0ad770] clone model from pd_core[0x7fc83c06abd0] succ, cur_idx[0].
...
W0330 09:38:50.091784  7361 infer.h:489] Succ load clone model, path[uci_housing_model]
W0330 09:38:50.091794  7361 infer.h:656] Succ reload version engine: 18446744073709551615
I0330 09:38:50.091820  7361 manager.h:131] Finish reload 1 workflow(s)
I0330 09:39:00.091987  7361 server.cpp:150] Begin reload framework...
W0330 09:39:00.092161  7361 infer.h:656] Succ reload version engine: 18446744073709551615
I0330 09:39:00.092177  7361 manager.h:131] Finish reload 1 workflow(s)
```
