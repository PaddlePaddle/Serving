# Paddle Serving

(简体中文|[English](./README.md))

Paddle Serving是PaddlePaddle的在线预估服务框架，能够帮助开发者轻松实现从移动端、服务器端调用深度学习模型的远程预测服务。当前Paddle Serving以支持PaddlePaddle训练的模型为主，可以与Paddle训练框架联合使用，快速部署预估服务。Paddle Serving围绕常见的工业级深度学习模型部署场景进行设计，一些常见的功能包括多模型管理、模型热加载、基于[Baidu-rpc](https://github.com/apache/incubator-brpc)的高并发低延迟响应能力、在线模型A/B实验等。与Paddle训练框架互相配合的API可以使用户在训练与远程部署之间无缝过度，提升深度学习模型的落地效率。

------------

## 快速上手指南

Paddle Serving当前的develop版本支持轻量级Python API进行快速预测，并且与Paddle的训练可以打通。我们以最经典的波士顿房价预测为示例，完整说明在单机进行模型训练以及使用Paddle Serving进行模型部署的过程。

#### 安装

强烈建议您在Docker内构建Paddle Serving，请查看[如何在Docker中运行PaddleServing](RUN_IN_DOCKER_CN.md)

```
pip install paddle-serving-client
pip install paddle-serving-server
```

#### 训练脚本
``` python
import sys
import paddle
import paddle.fluid as fluid

train_reader = paddle.batch(paddle.reader.shuffle(
    paddle.dataset.uci_housing.train(), buf_size=500), batch_size=16)

test_reader = paddle.batch(paddle.reader.shuffle(
    paddle.dataset.uci_housing.test(), buf_size=500), batch_size=16)

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

import paddle_serving_client.io as serving_io

for pass_id in range(30):
    for data_train in train_reader():
        avg_loss_value, = exe.run(
            fluid.default_main_program(),
            feed=feeder.feed(data_train),
            fetch_list=[avg_loss])

serving_io.save_model(
    "serving_server_model", "serving_client_conf",
    {"x": x}, {"y": y_predict}, fluid.default_main_program())
```

#### 服务器端代码
``` python
import sys
from paddle_serving.serving_server import OpMaker
from paddle_serving.serving_server import OpSeqMaker
from paddle_serving.serving_server import Server

op_maker = OpMaker()
read_op = op_maker.create('general_reader')
general_infer_op = op_maker.create('general_infer')

op_seq_maker = OpSeqMaker()
op_seq_maker.add_op(read_op)
op_seq_maker.add_op(general_infer_op)

server = Server()
server.set_op_sequence(op_seq_maker.get_op_sequence())
server.load_model_config(sys.argv[1])
server.prepare_server(workdir="work_dir1", port=9393, device="cpu")
server.run_server()
```

#### 服务器端启动
``` shell
python test_server.py serving_server_model
```

#### 客户端预测
``` python
from paddle_serving_client import Client
import paddle
import sys

client = Client()
client.load_client_config(sys.argv[1])
client.connect(["127.0.0.1:9292"])

test_reader = paddle.batch(paddle.reader.shuffle(
    paddle.dataset.uci_housing.test(), buf_size=500), batch_size=1)

for data in test_reader():
    fetch_map = client.predict(feed={"x": data[0][0]}, fetch=["y"])
    print("{} {}".format(fetch_map["y"][0], data[0][1][0]))

```

### 文档

[设计文档](DESIGN_CN.md)

[FAQ](FAQ.md)

### 资深开发者使用指南

[编译指南](COMPILE_CN.md)

## 贡献
如果你想要给Paddle Serving做贡献，请参考[贡献指南](CONTRIBUTE.md)
