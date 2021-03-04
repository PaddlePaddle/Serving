# 如何使用Paddle Serving做ABTEST

(简体中文|[English](./ABTEST_IN_PADDLE_SERVING.md))

该文档将会用一个基于IMDB数据集的文本分类任务的例子，介绍如何使用Paddle Serving搭建A/B Test框架，例中的Client端、Server端结构如下图所示。

<img src="abtest.png" style="zoom:33%;" />

需要注意的是：A/B Test只适用于RPC模式，不适用于WEB模式。

### 下载数据以及模型

``` shell
cd Serving/python/examples/imdb
sh get_data.sh
```

### 处理数据
由于处理数据需要用到相关库，请使用pip进行安装
``` shell
pip install paddlepaddle
pip install paddle-serving-app
pip install Shapely
````
您可以直接运行下面的命令来处理数据。

[python abtest_get_data.py](../python/examples/imdb/abtest_get_data.py)

文件中的Python代码将处理`test_data/part-0`的数据，并将处理后的数据生成并写入`processed.data`文件中。

### 启动Server端

这里采用[Docker方式](RUN_IN_DOCKER_CN.md)启动Server端服务。

首先启动BOW Server，该服务启用`8000`端口：

```bash
docker run -dit -v $PWD/imdb_bow_model:/model -p 8000:8000 --name bow-server registry.baidubce.com/paddlepaddle/serving:latest /bin/bash
docker exec -it bow-server /bin/bash
pip install paddle-serving-server -i https://pypi.tuna.tsinghua.edu.cn/simple
pip install paddle-serving-client -i https://pypi.tuna.tsinghua.edu.cn/simple
python -m paddle_serving_server.serve --model model --port 8000 >std.log 2>err.log &
exit
```

同理启动LSTM Server，该服务启用`9000`端口：

```bash
docker run -dit -v $PWD/imdb_lstm_model:/model -p 9000:9000 --name lstm-server registry.baidubce.com/paddlepaddle/serving:latest /bin/bash
docker exec -it lstm-server /bin/bash
pip install paddle-serving-server -i https://pypi.tuna.tsinghua.edu.cn/simple
pip install paddle-serving-client -i https://pypi.tuna.tsinghua.edu.cn/simple
python -m paddle_serving_server.serve --model model --port 9000 >std.log 2>err.log &
exit
```

### 启动Client端
为了模拟ABTEST工况，您可以在宿主机运行下面Python代码启动Client端，但需确保宿主机具备相关环境，您也可以在docker环境下运行.

运行前使用`pip install paddle-serving-client`安装paddle-serving-client包。


您可以直接使用下面的命令，进行ABTEST预测。

[python abtest_client.py](../python/examples/imdb/abtest_client.py)

```python
from paddle_serving_client import Client
import numpy as np

client = Client()
client.load_client_config('imdb_bow_client_conf/serving_client_conf.prototxt')
client.add_variant("bow", ["127.0.0.1:8000"], 10)
client.add_variant("lstm", ["127.0.0.1:9000"], 90)
client.connect()

print('please wait for about 10s')
with open('processed.data') as f:
    cnt = {"bow": {'acc': 0, 'total': 0}, "lstm": {'acc': 0, 'total': 0}}
    for line in f:
        word_ids, label = line.split(';')
        word_ids = [int(x) for x in word_ids.split(',')]
        word_len = len(word_ids)
        feed = {
            "words": np.array(word_ids).reshape(word_len, 1),
            "words.lod": [0, word_len]
        }
        fetch = ["acc", "cost", "prediction"]
        [fetch_map, tag] = client.predict(feed=feed, fetch=fetch, need_variant_tag=True,batch=True)
        if (float(fetch_map["prediction"][0][1]) - 0.5) * (float(label[0]) - 0.5) > 0:
            cnt[tag]['acc'] += 1
        cnt[tag]['total'] += 1

    for tag, data in cnt.items():
        print('[{}](total: {}) acc: {}'.format(tag, data['total'], float(data['acc'])/float(data['total']) ))
```
代码中，`client.add_variant(tag, clusters, variant_weight)`是为了添加一个标签为`tag`、流量权重为`variant_weight`的variant。在这个样例中，添加了一个标签为`bow`、流量权重为`10`的BOW variant，以及一个标签为`lstm`、流量权重为`90`的LSTM variant。Client端的流量会根据`10:90`的比例分发到两个variant。

Client端做预测时，若指定参数`need_variant_tag=True`，返回值则包含分发流量对应的variant标签。

### 预期结果
由于网络情况的不同，可能每次预测的结果略有差异。
``` bash
[lstm](total: 1867) acc: 0.490091055169
[bow](total: 217) acc: 0.73732718894
```
