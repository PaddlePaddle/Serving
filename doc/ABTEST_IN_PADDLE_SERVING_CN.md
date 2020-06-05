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

下面Python代码将处理`test_data/part-0`的数据，写入`processed.data`文件中。

```python
from paddle_serving_app.reader import IMDBDataset
imdb_dataset = IMDBDataset()
imdb_dataset.load_resource('imdb.vocab')

with open('test_data/part-0') as fin:
    with open('processed.data', 'w') as fout:
        for line in fin:
            word_ids, label = imdb_dataset.get_words_and_label(line)
            fout.write("{};{}\n".format(','.join([str(x) for x in word_ids]), label[0]))
```

### 启动Server端

这里采用[Docker方式](https://github.com/PaddlePaddle/Serving/blob/develop/doc/RUN_IN_DOCKER_CN.md)启动Server端服务。

首先启动BOW Server，该服务启用`8000`端口：

```bash
docker run -dit -v $PWD/imdb_bow_model:/model -p 8000:8000 --name bow-server hub.baidubce.com/paddlepaddle/serving:latest
docker exec -it bow-server bash
pip install paddle-serving-server -i https://pypi.tuna.tsinghua.edu.cn/simple
python -m paddle_serving_server.serve --model model --port 8000 >std.log 2>err.log &
exit
```

同理启动LSTM Server，该服务启用`9000`端口：

```bash
docker run -dit -v $PWD/imdb_lstm_model:/model -p 9000:9000 --name lstm-server hub.baidubce.com/paddlepaddle/serving:latest
docker exec -it lstm-server bash
pip install paddle-serving-server -i https://pypi.tuna.tsinghua.edu.cn/simple
python -m paddle_serving_server.serve --model model --port 9000 >std.log 2>err.log &
exit
```

### 启动Client端

在宿主机运行下面Python代码启动Client端，需要确保宿主机装好`paddle-serving-client`包。

```python
from paddle_serving_client import Client

client = Client()
client.load_client_config('imdb_bow_client_conf/serving_client_conf.prototxt')
client.add_variant("bow", ["127.0.0.1:8000"], 10)
client.add_variant("lstm", ["127.0.0.1:9000"], 90)
client.connect()

with open('processed.data') as f:
    cnt = {"bow": {'acc': 0, 'total': 0}, "lstm": {'acc': 0, 'total': 0}}
    for line in f:
        word_ids, label = line.split(';')
        word_ids = [int(x) for x in word_ids.split(',')]
        feed = {"words": word_ids}
        fetch = ["acc", "cost", "prediction"]
        [fetch_map, tag] = client.predict(feed=feed, fetch=fetch, need_variant_tag=True)
        if (float(fetch_map["prediction"][0][1]) - 0.5) * (float(label[0]) - 0.5) > 0:
            cnt[tag]['acc'] += 1
        cnt[tag]['total'] += 1

    for tag, data in cnt.items():
        print('[{}](total: {}) acc: {}'.format(tag, data['total'], float(data['acc']) / float(data['total'])))
```

代码中，`client.add_variant(tag, clusters, variant_weight)`是为了添加一个标签为`tag`、流量权重为`variant_weight`的variant。在这个样例中，添加了一个标签为`bow`、流量权重为`10`的BOW variant，以及一个标签为`lstm`、流量权重为`90`的LSTM variant。Client端的流量会根据`10:90`的比例分发到两个variant。

Client端做预测时，若指定参数`need_variant_tag=True`，返回值则包含分发流量对应的variant标签。

### 预期结果

``` bash
[lstm](total: 1867) acc: 0.490091055169
[bow](total: 217) acc: 0.73732718894
```
