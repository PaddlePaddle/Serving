## 怎样保存用于Paddle Serving的模型？

(简体中文|[English](./SAVE.md))

- 目前，Paddle服务提供了一个save_model接口供用户访问，该接口与Paddle的`save_inference_model`类似。

``` python
import paddle_serving_client.io as serving_io
serving_io.save_model("imdb_model", "imdb_client_conf",
                      {"words": data}, {"prediction": prediction},
                      fluid.default_main_program())
```
imdb_model是具有服务配置的服务器端模型。 imdb_client_conf是客户端rpc配置。 Serving有一个 提供给用户存放Feed和Fetch变量信息的字典。 在示例中，{{words”：data}是用于指定已保存推理模型输入的提要字典。 {{"prediction"：projection}是指定保存的推理模型输出的字典。可以为feed和fetch变量定义一个别名。 如何使用别名的例子 示例如下：

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
    fetch = ["acc", "cost", "prediction"]
    fetch_map = client.predict(feed=feed, fetch=fetch)
    print("{} {}".format(fetch_map["prediction"][1], label[0]))
 ```
