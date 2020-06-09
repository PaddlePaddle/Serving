## 十分钟构建Bert-As-Service

(简体中文|[English](./BERT_10_MINS.md))

Bert-As-Service的目标是给定一个句子，服务可以将句子表示成一个语义向量返回给用户。[Bert模型](https://arxiv.org/abs/1810.04805)是目前NLP领域的热门模型，在多种公开的NLP任务上都取得了很好的效果，使用Bert模型计算出的语义向量来做其他NLP模型的输入对提升模型的表现也有很大的帮助。Bert-As-Service可以让用户很方便地获取文本的语义向量表示并应用到自己的任务中。为了实现这个目标，我们通过四个步骤说明使用Paddle Serving在十分钟内就可以搭建一个这样的服务。示例中所有的代码和文件均可以在Paddle Serving的[示例](https://github.com/PaddlePaddle/Serving/tree/develop/python/examples/bert)中找到。

#### Step1：保存可服务模型

Paddle Serving支持基于Paddle进行训练的各种模型，并通过指定模型的输入和输出变量来保存可服务模型。为了方便，我们可以从paddlehub加载一个已经训练好的bert中文模型，并利用两行代码保存一个可部署的服务，服务端和客户端的配置分别放在`bert_seq20_model`和`bert_seq20_client`文件夹。

``` python
import paddlehub as hub
model_name = "bert_chinese_L-12_H-768_A-12"
module = hub.Module(model_name)
inputs, outputs, program = module.context(trainable=True, max_seq_len=20)
feed_keys = ["input_ids", "position_ids", "segment_ids", "input_mask"]
fetch_keys = ["pooled_output", "sequence_output"]
feed_dict = dict(zip(feed_keys, [inputs[x] for x in feed_keys]))
fetch_dict = dict(zip(fetch_keys, [outputs[x] for x in fetch_keys]))

import paddle_serving_client.io as serving_io
serving_io.save_model("bert_seq20_model", "bert_seq20_client", feed_dict, fetch_dict, program)
```

#### Step2：启动服务

``` shell
python -m paddle_serving_server_gpu.serve --model bert_seq20_model --port 9292 --gpu_ids 0
```

| 参数    | 含义                       |
| ------- | -------------------------- |
| model   | server端配置与模型文件路径 |
| thread  | server端线程数             |
| port    | server端端口号             |
| gpu_ids | GPU索引号                  |

#### Step3：客户端数据预处理逻辑

Paddle Serving内建了很多经典典型对应的数据预处理逻辑，对于中文Bert语义表示的计算，我们采用paddle_serving_app下的ChineseBertReader类进行数据预处理，开发者可以很容易获得一个原始的中文句子对应的多个模型输入字段。

安装paddle_serving_app

```shell
pip install paddle_serving_app
```

#### Step4：客户端访问

客户端脚本 bert_client.py内容如下

``` python
import sys
from paddle_serving_client import Client
from paddle_serving_app.reader import ChineseBertReader

reader = ChineseBertReader()
fetch = ["pooled_output"]
endpoint_list = ["127.0.0.1:9292"]
client = Client()
client.load_client_config("bert_seq20_client/serving_client_conf.prototxt")
client.connect(endpoint_list)

for line in sys.stdin:
    feed_dict = reader.process(line)
    result = client.predict(feed=feed_dict, fetch=fetch)
```

执行

```shell
cat data.txt | python bert_client.py
```

从data.txt文件中读取样例，并将结果打印到标准输出。

### 性能测试

我们基于V100对基于Padde Serving研发的Bert-As-Service的性能进行测试并与基于Tensorflow实现的Bert-As-Service进行对比，从用户配置的角度，采用相同的batch size和并发数进行压力测试，得到4块V100下的整体吞吐性能数据如下。

![4v100_bert_as_service_benchmark](4v100_bert_as_service_benchmark.png)
