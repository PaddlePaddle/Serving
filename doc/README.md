# Paddle Serving

([简体中文](./README_CN.md)|English)

Paddle Serving is PaddlePaddle's online estimation service framework, which can help developers easily implement remote prediction services that call deep learning models from mobile and server ends. At present, Paddle Serving is mainly based on models that support PaddlePaddle training. It can be used in conjunction with the Paddle training framework to quickly deploy inference services. Paddle Serving is designed around common industrial-level deep learning model deployment scenarios. Some common functions include multi-model management, model hot loading, [Baidu-rpc](https://github.com/apache/incubator-brpc)-based high-concurrency low-latency response capabilities, and online model A/B tests. The API that cooperates with the Paddle training framework can enable users to seamlessly transition between training and remote deployment, improving the landing efficiency of deep learning models.

------------

## Quick Start

Paddle Serving's current develop version supports lightweight Python API for fast predictions, and training with Paddle can get through. We take the most classic Boston house price prediction as an example to fully explain the process of model training on a single machine and model deployment using Paddle Serving.

#### Install

It is highly recommended that you build Paddle Serving inside Docker, please read [How to run PaddleServing in Docker](doc/RUN_IN_DOCKER.md)

```
pip install paddle-serving-client
pip install paddle-serving-server
```

#### Training Script
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

#### Server Side Code
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

#### Launch Server End
``` shell
python test_server.py serving_server_model
```

#### Client Prediction
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

### Document

[Design Doc](DESIGN.md)

[FAQ](FAQ.md)

### Senior Developer Guildlines

[Compile Tutorial](INSTALL.md)

## Contribution
If you want to make contributions to Paddle Serving Please refer to [CONRTIBUTE](CONTRIBUTE.md)
