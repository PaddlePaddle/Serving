# Paddle Serving

[中文](./README_CN.md)

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



### Server Side Scripts

```
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

[Develop a serving application with C++(Chinese)](doc/CREATING.md)

[Compile from source code(Chinese)](doc/INSTALL.md)

## Contribution

If you want to contribute code to Paddle Serving, please reference [Contribution Guidelines](
