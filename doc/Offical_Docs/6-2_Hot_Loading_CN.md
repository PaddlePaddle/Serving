# Paddle Serving 中的模型热加载

## 背景

在实际的工业场景下，通常是远端定期不间断产出模型，线上服务端需要在服务不中断的情况下拉取新模型对旧模型进行更新迭代。

## Server Monitor

Paddle Serving 提供了一个自动监控脚本，远端地址更新模型后会拉取新模型更新本地模型，同时更新本地模型文件夹中的时间戳文件 `fluid_time_stamp` 实现热加载。

目前支持下面几种类型的远端监控 Monitor：

| Monitor类型 |                             描述                             |                           特殊选项                           |
| :---------: | :----------------------------------------------------------: | :----------------------------------------------------------: |
|   general   | 远端无认证，可以通过 `wget` 直接访问下载文件（如无需认证的FTP，BOS等） |                 `general_host` 通用远端host                  |
|  hdfs/afs(HadoopMonitor)   |        远端为 HDFS 或 AFS，通过 Hadoop-Client 执行相关命令        | `hadoop_bin` Hadoop 二进制的路径 <br/>`fs_name` Hadoop fs_name，默认为空<br/>`fs_ugi` Hadoop fs_ugi，默认为空 |
|     ftp     | 远端为 FTP，通过 `ftplib` 进行相关访问（使用该 Monitor，您可能需要执行 `pip install ftplib` 下载 `ftplib`） | `ftp_host` FTP host<br>`ftp_port` FTP port<br>`ftp_username` FTP username，默认为空<br>`ftp_password` FTP password，默认为空 |

|    Monitor通用选项     |                             描述                             |         默认值         |
| :--------------------: | :----------------------------------------------------------: | :--------------------: |
|         `type`         |                       指定 Monitor 类型                        |           无           |
|     `remote_path`      |                      指定远端的基础路径                      |           无           |
|  `remote_model_name`   |                   指定远端需要拉取的模型名                   |           无           |
| `remote_donefile_name` |           指定远端标志模型更新完毕的 donefile 文件名           |           无           |
|      `local_path`      |                       指定本地工作路径                       |           无           |
|   `local_model_name`   |                        指定本地模型名                        |           无           |
| `local_timestamp_file` | 指定本地用于热加载的时间戳文件，该文件被认为在 `local_path/local_model_name` 下。 |   `fluid_time_file`    |
|    `local_tmp_path`    |    指定本地存放临时文件的文件夹路径，若不存在则自动创建。    | `_serving_monitor_tmp` |
|       `interval`       |                 指定轮询间隔时间，单位为秒。                 |          `10`          |
|  `unpacked_filename`   | Monitor 支持 tarfile 打包的远程模型。如果远程模型是打包格式，则需要设置该选项来告知 Monitor 解压后的文件名。 |         `None`         |
|        `debug`         |       如果添加 `--debug` 选项，则输出更详细的中间信息。        |    默认不添加该选项    |

下面通过 HadoopMonitor 示例来展示 Paddle Serving 的模型热加载功能。

## HadoopMonitor 示例

示例中在 `product_path` 中生产模型上传至 hdfs，在 `server_path` 中模拟服务端模型热加载：

```shell
.
├── product_path
└── server_path
```

**一.生产模型**

在 `product_path` 下运行下面的 Python 代码生产模型（运行前需要修改 hadoop 相关的参数），每隔 60 秒会产出 Boston 房价预测模型的打包文件 `uci_housing.tar.gz` 并上传至 hdfs 的`/`路径下，上传完毕后更新时间戳文件 `donefile` 并上传至 hdfs 的`/`路径下。

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

hdfs 上的文件如下列所示：

```bash
# hadoop fs -ls /
Found 2 items
-rw-r--r--   1 root supergroup          0 2020-04-02 02:54 /donefile
-rw-r--r--   1 root supergroup       2101 2020-04-02 02:54 /uci_housing.tar.gz
```

**二.服务端加载模型**

进入 `server_path` 文件夹。

1. 用初始模型启动 Server 端

这里使用预训练的 Boston 房价预测模型作为初始模型：

```shell
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/uci_housing.tar.gz
tar -xzf uci_housing.tar.gz
```

启动 Server 端：

```shell
python -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9292
```

2. 执行监控程序

用下面的命令来执行 HDFS 监控程序：

```shell
python -m paddle_serving_server.monitor \
	--type='hdfs' --hadoop_bin='/hadoop-3.1.2/bin/hadoop' \
	--remote_path='/' --remote_model_name='uci_housing.tar.gz' \
	--remote_donefile_name='donefile' --local_path='.' \
	--local_model_name='uci_housing_model' --local_timestamp_file='fluid_time_file' \
	--local_tmp_path='_tmp' --unpacked_filename='uci_housing_model' --debug
```

上面代码通过轮询方式监控远程 HDFS 地址`/`的时间戳文件`/donefile`，当时间戳变更则认为远程模型已经更新，将远程打包模型`/uci_housing.tar.gz`拉取到本地临时路径`./_tmp/uci_housing.tar.gz`下，解包出模型文件`./_tmp/uci_housing_model`后，更新本地模型`./uci_housing_model`以及Paddle Serving的时间戳文件`./uci_housing_model/fluid_time_file`。

预计输出如下：

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

3. 查看 Server 日志

通过下面命令查看 Server 的运行日志：

```shell
tail -f log/serving.INFO
```

日志中显示模型已经被热加载：

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
