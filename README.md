# Paddle Serving
An easy-to-use Machine Learning Model Inference Service Deployment Tool

[![Release](https://img.shields.io/badge/Release-0.0.3-yellowgreen)](Release)
[![Issues](https://img.shields.io/github/issues/PaddlePaddle/Serving)](Issues)
[![License](https://img.shields.io/github/license/PaddlePaddle/Serving)](LICENSE)

[中文](./doc/README_CN.md)

Paddle Serving is the online inference service framework of [Paddle](https://github.com/PaddlePaddle/Paddle) that can help developers easily deploy a deep learning model service on server side and send request from mobile devices, edge devices as well as data centers. Currently, Paddle Serving supports the deep learning models produced by Paddle althought it can be very easy to support other deep learning framework's model inference. Paddle Serving is designed oriented from industrial practice. For example, multiple models management for online service, double buffers model loading, models online A/B testing are supported. Highly concurrent [Baidu-rpc](https://github.com/apache/incubator-brpc) is used as the underlying communication library which is also from industry practice. Paddle Serving provides user-friendly API that can integrate with Paddle training code seamlessly, and users can finish model training and model serving in an end-to-end fasion.



## Quick Start

Paddle Serving supports light-weighted Python API for model inference and can be integrated with trainining process seemlessly. Here is a Boston House Pricing example for users to do quick start.

### Installation

```shell
pip install paddle-serving-client
pip install paddle-serving-server
```


### Training Scripts

``` python
import paddle

train_reader = paddle.batch(paddle.reader.shuffle(
    paddle.dataset.uci_housing.train(), buf_size=500), batch_size=16)

x = paddle.fluid.data(name='x', shape=[None, 13], dtype='float32')
y = paddle.fluid.data(name='y', shape=[None, 1], dtype='float32')

y_predict = paddle.fluid.layers.fc(input=x, size=1, act=None)
cost = paddle.fluid.layers.square_error_cost(input=y_predict, label=y)
avg_loss = paddle.fluid.layers.mean(cost)
sgd_optimizer = paddle.fluid.optimizer.SGD(learning_rate=0.01)
sgd_optimizer.minimize(avg_loss)

place = paddle.fluid.CPUPlace()
feeder = paddle.fluid.DataFeeder(place=place, feed_list=[x, y])
exe = paddle.fluid.Executor(place)
exe.run(paddle.fluid.default_startup_program())

import paddle_serving_client.io as serving_io

for pass_id in range(30):
    for data_train in train_reader():
        avg_loss_value, = exe.run(
            paddle.fluid.default_main_program(),
            feed=feeder.feed(data_train),
            fetch_list=[avg_loss])

serving_io.save_model(
    "serving_server_model", "serving_client_conf",
    {"x": x}, {"y": y_predict}, paddle.fluid.default_main_program())
```



### Server Side Scripts

```
import sys
from paddle_serving.serving_server import OpMaker
from paddle_serving.serving_server import OpSeqMaker
from paddle_serving.serving_server import Server

op_maker = OpMaker()
read_op = op_maker.create('general_reader')
infer_op = op_maker.create('general_infer')
response_op = op_maker.create('general_response')

op_seq_maker = OpSeqMaker()
op_seq_maker.add_op(read_op)
op_seq_maker.add_op(infer_op)
op_seq_maker.add_op(response_op)

server = Server()
server.set_op_sequence(op_seq_maker.get_op_sequence())
server.load_model_config(sys.argv[1])
server.prepare_server(workdir="work_dir1", port=9393, device="cpu")
```

### Start Server

```
python test_server.py serving_server_model
```

### Client Side Scripts

```
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

[Design Doc(Chinese)](doc/DESIGN.md)

[FAQ(Chinese)](doc/FAQ.md)

### Advanced features and development

[Compile from source code(Chinese)](doc/COMPILE.md)

## Contribution

If you want to contribute code to Paddle Serving, please reference [Contribution Guidelines](doc/CONTRIBUTE.md)
