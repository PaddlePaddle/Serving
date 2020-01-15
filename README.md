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
import paddle_serving as serving

logging.basicConfig(format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger("fluid")
logger.setLevel(logging.INFO)

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
    save_dirname = "cnn_model"

    for i in range(epochs):
        exe.train_from_dataset(program=fluid.default_main_program(),
                               dataset=dataset, debug=False)
        logger.info("TRAIN --> pass: {}".format(i))
        fluid.io.save_inference_model("%s/epoch%d.model" % (save_dirname, i),
                                      [data.name, label.name], [acc], exe)
        serving.save_model("%s/epoch%d.model" % (save_dirname, i), "client_config{}".format(i),
                           {"words": data, "label": label},
                           {"acc": acc, "cost": avg_cost, "prediction": prediction})
```

#### 本地服务启动
TBA

#### 客户端预测
``` python
from paddle_serving import Client
import sys

client = Client()
client.load_client_config(sys.argv[1])
client.connect(["127.0.0.1:9292"])

for line in sys.stdin:
    group = line.strip().split()
    words = [int(x) for x in group[1:int(group[0])]]
    label = [int(group[-1])]
    feed = {"words": words, "label": label}
    fetch = ["acc", "cost", "prediction"]
    fetch_map = client.predict(feed=feed, fetch=fetch)
    print("{} {}".format(fetch_map["prediction"][1], label[0]))

```


#### 完成操作截屏
TBA

### 文档

[设计文档](doc/DESIGN.md)

[从零开始写一个预测服务](doc/CREATING.md)

[编译安装](doc/INSTALL.md)

[FAQ](doc/FAQ.md)

