# Paddle Serving中的模型热加载

## 背景

在实际的工业场景下，通常是远端定期不间断产出模型，线上服务端需要在服务不中断的情况下拉取新模型对旧模型进行更新迭代。

Paddle Serving目前支持下面几种类型的远端监控Monitor：

| Monitor类型 |                    描述                    |                           特殊选项                           |
| :---------: | :----------------------------------------: | :----------------------------------------------------------: |
|   General   | 远端无认证，可以通过`wget`直接访问下载文件 |                 `general_host` 通用远端host                  |
|    HDFS     |   远端为HDFS，通过HDFS二进制执行相关命令   |                 `hdfs_bin` HDFS二进制的路径                  |
|     FTP     |    远端为FTP，可以通过用户名、密码访问     | `ftp_host` FTP host<br>`ftp_port` FTP port<br>`ftp_username` FTP username，默认为空<br>`ftp_password` FTP password，默认为空 |
|     AFS     |  远端为AFS，通过Hadoop-client执行相关命令  | `hadoop_bin` Hadoop二进制的路径<br>`hadoop_host` AFS host，默认为空<br>`hadoop_ugi` AFS ugi，默认为空 |

|    Monitor通用选项     |                             描述                             |
| :--------------------: | :----------------------------------------------------------: |
|         `type`         |                       指定Monitor类型                        |
|     `remote_path`      |                      指定远端的基础路径                      |
|  `remote_model_name`   |                   指定远端需要拉取的模型名                   |
| `remote_donefile_name` |           指定远端标志模型更新完毕的donefile文件名           |
|      `local_path`      |                       指定本地工作路径                       |
|   `local_model_name`   |                        指定本地模型名                        |
| `local_timestamp_file` | 指定本地用于热加载的时间戳文件，该文件被认为在`local_path/local_model_name`下。默认为`fluid_time_file` |
|    `local_tmp_path`    | 指定本地存放临时文件的文件夹路径。默认为`_serving_monitor_tmp`，若不存在则自动创建 |
|       `interval`       |                       指定轮询间隔时间                       |

下面通过HDFSMonitor示例来展示Paddle Serving的模型热加载功能。

## HDFSMonitor示例

示例中在`product_path`中生产模型上传至hdfs，在`server_path`中模拟服务端模型热加载：

```shell
.
├── server_path
└── product_path
```

### 生产模型

在`product_path`下运行下面的Python代码生产模型，每隔 60 秒会产出 Boston 房价预测模型`uci_housing_model`并上传至hdfs的`/`路径下，上传完毕后更新时间戳文件`donefile`并上传至hdfs的`/`路径下。

```python
import os
import time
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
    hdfs_bin = 'hdfs'
    os.system('{} dfs -put -f {} {}'.format(
      hdfs_bin, local_file_path, remote_path))

for pass_id in range(30):
    for data_train in train_reader():
        avg_loss_value, = exe.run(fluid.default_main_program(),
                                  feed=feeder.feed(data_train),
                                  fetch_list=[avg_loss])
    time.sleep(60) # Simulate the production model every other period of time
    serving_io.save_model("uci_housing_model", "uci_housing_client",
                          {"x": x}, {"price": y_predict},
                          fluid.default_main_program())
    push_to_hdfs('uci_housing_model', '/')
    os.system('touch donefile')
    push_to_hdfs('donefile', '/')
```

hdfs上的文件如下列所示：

```bash
# hdfs dfs -ls /
Found 2 items
-rw-r--r--   1 root supergroup          0 2020-03-30 09:27 /donefile
drwxr-xr-x   - root supergroup          0 2020-03-30 09:27 /uci_housing_model
```

### 服务端加载模型

进入`server_path`文件夹。

#### 用初始模型启动Server端

这里使用预训练的 Boston 房价预测模型作为初始模型：

```shell
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/uci_housing.tar.gz
tar -xzf uci_housing.tar.gz
```

启动Server端：

```shell
python -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9292
```

#### 执行监控程序

用下面的命令来执行HDFS监控程序：

```shell
python -m paddle_serving_server.monitor \
					--type='hdfs' \
					--hdfs_bin='hdfs' \
					--remote_path='/' \
					--remote_model_name='uci_housing_model' \
					--remote_donefile_name='donefile' \
					--local_path='.' \
					--local_model_name='uci_housing_model' \
					--local_timestamp_file='fluid_time_file' \
					--local_tmp_path='_tmp'
```

上面代码通过轮询方式监控远程HDFS地址`/`的时间戳文件`/donefile`，当时间戳变更则认为远程模型已经更新，将远程模型`/uci_housing_model`拉取到本地临时路径`./_tmp/uci_housing_model`下，更新本地模型`./uci_housing_model`以及Paddle Serving的时间戳文件`./uci_housing_model/fluid_time_file`。

#### 查看Server日志

通过下面命令查看Server的运行日志：

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
