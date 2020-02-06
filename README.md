# Paddle Serving
Paddle Serving是PaddlePaddle的在线预估服务框架，能够帮助开发者轻松实现从移动端、服务器端调用深度学习模型的远程预测服务。当前Paddle Serving以支持PaddlePaddle训练的模型为主，可以与Paddle训练框架联合使用，快速部署预估服务。

## 快速上手
Paddle Serving当前的develop版本支持轻量级Python API进行快速预测，我们假设远程已经部署的Paddle Serving的文本分类模型，您可以在自己的服务器快速安装客户端并进行快速预测。

#### 安装
```
pip install paddle-serving-client
pip install paddle-serving-server
```

#### 训练脚本
``` python
import os
import sys
import paddle
import logging
import paddle.fluid as fluid

def load_vocab(filename):
    vocab = {}
    with open(filename) as f:
        wid = 0
        for line in f:
            vocab[line.strip()] = wid
            wid += 1
    vocab["<unk>"] = len(vocab)
    return vocab

if __name__ == "__main__":
    vocab = load_vocab('imdb.vocab')
    dict_dim = len(vocab)

    data = fluid.layers.data(name="words", shape=[1], dtype="int64", lod_level=1)
    label = fluid.layers.data(name="label", shape=[1], dtype="int64")

    dataset = fluid.DatasetFactory().create_dataset()
    filelist = ["train_data/%s" % x for x in os.listdir("train_data")]
    dataset.set_use_var([data, label])
    pipe_command = "python imdb_reader.py"
    dataset.set_pipe_command(pipe_command)
    dataset.set_batch_size(4)
    dataset.set_filelist(filelist)
    dataset.set_thread(10)
    from nets import cnn_net
    avg_cost, acc, prediction = cnn_net(data, label, dict_dim)
    optimizer = fluid.optimizer.SGD(learning_rate=0.01)
    optimizer.minimize(avg_cost)

    exe = fluid.Executor(fluid.CPUPlace())
    exe.run(fluid.default_startup_program())
    epochs = 30

    import paddle_serving_client.io as serving_io

    for i in range(epochs):
        exe.train_from_dataset(program=fluid.default_main_program(),
                               dataset=dataset, debug=False)
        logger.info("TRAIN --> pass: {}".format(i))
        if i == 20:
            serving_io.save_model("serving_server_model",
                                  "serving_client_conf",
                                  {"words": data, "label": label},
                                  {"cost": avg_cost, "acc": acc,
                                   "prediction": prediction},
                                  fluid.default_main_program())
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
import sys

client = Client()
client.load_client_config(sys.argv[1])
client.connect(["127.0.0.1:9393"])

for line in sys.stdin:
    group = line.strip().split()
    words = [int(x) for x in group[1:int(group[0]) + 1]]
    label = [int(group[-1])]
    feed = {"words": words, "label": label}
    fetch = ["cost", "acc", "prediction"]
    fetch_map = client.predict(feed=feed, fetch=fetch)
    print("{} {}".format(fetch_map["prediction"][1], label[0]))

```

### 文档

[设计文档](doc/DESIGN.md)

[从零开始写一个预测服务](doc/CREATING.md)

[编译安装](doc/INSTALL.md)

[FAQ](doc/FAQ.md)

