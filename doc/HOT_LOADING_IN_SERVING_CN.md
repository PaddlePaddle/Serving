# Paddle Serving中的模型热加载

## 背景

在实际的工业场景下，通常是定期不间断产出模型，服务端需要在服务不中断的情况下按时更新迭代模型。

这里用本地搭建FTP的形式，模拟监控远程模型，拉取更新本地模型，来展示Paddle Serving的模型热加载功能。

## 示例

示例目录结构，示例中用`local_path`来模拟本地，用`remote_path`来模拟远程：

```shell
.
├── local_path
└── remote_path
```

### 远程部分

进入`remote_path`文件夹：

```shell
cd remote_path
```

#### 生产远程模型

运行下面的Python代码生产模型。

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

for pass_id in range(30):
    for data_train in train_reader():
        avg_loss_value, = exe.run(fluid.default_main_program(),
                                  feed=feeder.feed(data_train),
                                  fetch_list=[avg_loss])
    time.sleep(60) # Simulate the production model every other period of time
    serving_io.save_model("uci_housing_model", "uci_housing_client",
                          {"x": x}, {"price": y_predict},
                          fluid.default_main_program())
    os.system('touch donefile')
    print('save {}'.format(pass_id))
```

上面的代码会每隔 60 秒在当前目录下产出 Boston 房价预测模型`uci_housing_model`，并在每次产出后更新时间戳文件`donefile`：

```shell
.
├── donefile           # timestamp file 
├── local_train.py
├── uci_housing_client
└── uci_housing_model  # output model
```

#### 启动FTP服务

这里使用`pyftpdlib`开启FTP服务，执行下面的命令（您可能需要使用`pip install pyftpdlib `来安装相关的库）：

```shell
python -m pyftpdlib -p 8080
```



### 本地部分

进入`local_path`文件夹：

```shell
cd local_path
```

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

用下面的命令来执行监控程序，通过轮询方式监控远程地址的时间戳文件`donefile`，当时间戳变更则认为远程模型已经更新，将远程模型拉取到本地临时路径下（默认为`./tmp`），更新本地模型以及Paddle Serving的时间戳文件`fluid_time_file`：

```shell
python -m paddle_serving_server.monitor --type='ftp' --ftp_ip='127.0.0.1' --ftp_port='8080' --remote_path='/' --remote_model_name='uci_housing_model' --remote_donefile_name='donefile' --local_path='./' --local_model_name='uci_housing_model' --local_donefile_name='fluid_time_file' --local_tmp_dir='tmp'
```

上面的代码会监控远程路径`ftp://127.0.0.1:8080/`下的`donefile`文件来判断远程模型是否更新，若已经更新则将远程模型`ftp://127.0.0.1:8080/uci_housing_model`拉取到本地`./tmp`路径下，之后更新本地路径的模型`./uci_housing_model`，并更新Paddle Serving的时间戳文件`./uci_housing_model/fluid_time_file`。

#### 查看Server日志

通过下面命令查看Server的运行日志：

```shell
tail -f log/serving.INFO
```

日志中显示模型已经被热加载：

```shell
W0327 19:00:38.498729  5559 infer.h:509] td_core[0x7f20e8068f10] clone model from pd_core[0x7f20e8005f90] succ, cur_idx[1].
W0327 19:00:38.498737  5559 infer.h:489] Succ load clone model, path[uci_housing_model]
W0327 19:00:38.498744  5559 infer.h:656] Succ reload version engine: 18446744073709551615
I0327 19:00:38.498752  5559 manager.h:131] Finish reload 1 workflow(s)
I0327 19:00:48.498860  5559 server.cpp:150] Begin reload framework...
W0327 19:00:48.498947  5559 infer.h:656] Succ reload version engine: 18446744073709551615
I0327 19:00:48.498970  5559 manager.h:131] Finish reload 1 workflow(s)
I0327 19:00:58.499076  5559 server.cpp:150] Begin reload framework...
W0327 19:00:58.499167  5559 infer.h:656] Succ reload version engine: 18446744073709551615
I0327 19:00:58.499181  5559 manager.h:131] Finish reload 1 workflow(s)
I0327 19:01:08.499277  5559 server.cpp:150] Begin reload framework...
W0327 19:01:08.499366  5559 infer.h:656] Succ reload version engine: 18446744073709551615
I0327 19:01:08.499379  5559 manager.h:131] Finish reload 1 workflow(s)
I0327 19:01:18.499492  5559 server.cpp:150] Begin reload framework...
W0327 19:01:18.499637  5559 infer.h:656] Succ reload version engine: 18446744073709551615
I0327 19:01:18.499655  5559 manager.h:131] Finish reload 1 workflow(s)
I0327 19:01:28.499745  5559 server.cpp:150] Begin reload framework...
W0327 19:01:28.499814  5559 infer.h:250] begin reload model[uci_housing_model].
I0327 19:01:28.500083  5559 infer.h:66] InferEngineCreationParams: model_path = uci_housing_model, enable_memory_optimization = 0, static_optimization = 0, force_update_static_cache = 0
I0327 19:01:28.500160  5559 analysis_predictor.cc:84] Profiler is deactivated, and no profiling report will be generated.
W0327 19:01:28.500176  5559 init.cc:159] AVX is available, Please re-compile on local machine
I0327 19:01:28.500628  5559 analysis_predictor.cc:833] MODEL VERSION: 0.0.0
I0327 19:01:28.500653  5559 analysis_predictor.cc:835] PREDICTOR VERSION: 1.7.1
I0327 19:01:28.502399  5559 graph_pattern_detector.cc:101] ---  detected 1 subgraphs
I0327 19:01:28.504007  5559 analysis_predictor.cc:462] ======= optimize end =======
W0327 19:01:28.504101  5559 infer.h:472] Succ load common model[0x7f20e806b8b0], path[uci_housing_model].
I0327 19:01:28.504154  5559 analysis_predictor.cc:84] Profiler is deactivated, and no profiling report will be generated.
W0327 19:01:28.504194  5559 infer.h:509] td_core[0x7f20e80b9680] clone model from pd_core[0x7f20e806b8b0] succ, cur_idx[0].
I0327 19:01:28.504287  5559 analysis_predictor.cc:84] Profiler is deactivated, and no profiling report will be generated.
W0327 19:01:28.504330  5559 infer.h:509] td_core[0x7f20e80bf1e0] clone model from pd_core[0x7f20e806b8b0] succ, cur_idx[0].
I0327 19:01:28.504365  5559 analysis_predictor.cc:84] Profiler is deactivated, and no profiling report will be generated.
W0327 19:01:28.504403  5559 infer.h:509] td_core[0x7f20e80af2a0] clone model from pd_core[0x7f20e806b8b0] succ, cur_idx[0].
I0327 19:01:28.504436  5559 analysis_predictor.cc:84] Profiler is deactivated, and no profiling report will be generated.
W0327 19:01:28.504483  5559 infer.h:509] td_core[0x7f20e8004a00] clone model from pd_core[0x7f20e806b8b0] succ, cur_idx[0].
I0327 19:01:28.504516  5559 analysis_predictor.cc:84] Profiler is deactivated, and no profiling report will be generated.
W0327 19:01:28.504551  5559 infer.h:509] td_core[0x7f20e80a8960] clone model from pd_core[0x7f20e806b8b0] succ, cur_idx[0].
I0327 19:01:28.504580  5559 analysis_predictor.cc:84] Profiler is deactivated, and no profiling report will be generated.
W0327 19:01:28.504611  5559 infer.h:509] td_core[0x7f20e80a4bd0] clone model from pd_core[0x7f20e806b8b0] succ, cur_idx[0].
I0327 19:01:28.504639  5559 analysis_predictor.cc:84] Profiler is deactivated, and no profiling report will be generated.
W0327 19:01:28.504669  5559 infer.h:509] td_core[0x7f20e80b8f20] clone model from pd_core[0x7f20e806b8b0] succ, cur_idx[0].
I0327 19:01:28.504699  5559 analysis_predictor.cc:84] Profiler is deactivated, and no profiling report will be generated.
W0327 19:01:28.504730  5559 infer.h:509] td_core[0x7f20e80a4ab0] clone model from pd_core[0x7f20e806b8b0] succ, cur_idx[0].
I0327 19:01:28.504760  5559 analysis_predictor.cc:84] Profiler is deactivated, and no profiling report will be generated.
W0327 19:01:28.504796  5559 infer.h:509] td_core[0x7f20e807ee40] clone model from pd_core[0x7f20e806b8b0] succ, cur_idx[0].
I0327 19:01:28.504827  5559 analysis_predictor.cc:84] Profiler is deactivated, and no profiling report will be generated.
W0327 19:01:28.504904  5559 infer.h:509] td_core[0x7f20e8085900] clone model from pd_core[0x7f20e806b8b0] succ, cur_idx[0].
I0327 19:01:28.505043  5559 analysis_predictor.cc:84] Profiler is deactivated, and no profiling report will be generated.
W0327 19:01:28.505097  5559 infer.h:509] td_core[0x7f20e8088500] clone model from pd_core[0x7f20e806b8b0] succ, cur_idx[0].
W0327 19:01:28.505110  5559 infer.h:489] Succ load clone model, path[uci_housing_model]
W0327 19:01:28.505120  5559 infer.h:656] Succ reload version engine: 18446744073709551615
I0327 19:01:28.505131  5559 manager.h:131] Finish reload 1 workflow(s)
I0327 19:01:38.505468  5559 server.cpp:150] Begin reload framework...
W0327 19:01:38.505568  5559 infer.h:656] Succ reload version engine: 18446744073709551615
I0327 19:01:38.505584  5559 manager.h:131] Finish reload 1 workflow(s)
```
