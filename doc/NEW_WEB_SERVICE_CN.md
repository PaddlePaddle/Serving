# 如何开发一个新的Web Service？

(简体中文|[English](NEW_WEB_SERVICE.md))

本文档将以Imagenet图像分类服务为例，来介绍如何开发一个新的Web Service。您可以在[这里](https://github.com/PaddlePaddle/Serving/blob/develop/python/examples/imagenet/image_classification_service.py)查阅完整的代码。

## WebService基类

Paddle Serving实现了[WebService](https://github.com/PaddlePaddle/Serving/blob/develop/python/paddle_serving_server/web_service.py#L23)基类，您需要重写它的`preprocess`方法和`postprocess`方法，默认实现如下：

```python
class WebService(object):
  
    def preprocess(self, feed={}, fetch=[]):
        return feed, fetch
    def postprocess(self, feed={}, fetch=[], fetch_map=None):
        return fetch_map
```

### preprocess方法

preprocess方法有两个输入参数，`feed`和`fetch`。对于一个HTTP请求`request`：

- `feed`的值为请求数据`request.json`
- `fetch`的值为请求数据中的fetch部分`request.json["fetch"]`

返回值分别是预测过程中用到的feed和fetch值。

### postprocess方法

postprocess方法有三个输入参数，`feed`、`fetch`和`fetch_map`：

- `feed`的值为请求数据`request.json`
- `fetch`的值为请求数据中的fetch部分`request.json["fetch"]`
- `fetch_map`的值为fetch到的模型输出值

返回值将会被处理成`{"reslut": fetch_map}`作为HTTP请求的返回。

## 开发ImageService类

```python
class ImageService(WebService):
    def preprocess(self, feed={}, fetch=[]):
        reader = ImageReader()
        if "image" not in feed:
            raise ("feed data error!")
        if isinstance(feed["image"], list):
            feed_batch = []
            for image in feed["image"]:
                sample = base64.b64decode(image)
                img = reader.process_image(sample)
                res_feed = {}
                res_feed["image"] = img.reshape(-1)
                feed_batch.append(res_feed)
            return feed_batch, fetch
        else:
            sample = base64.b64decode(feed["image"])
            img = reader.process_image(sample)
            res_feed = {}
            res_feed["image"] = img.reshape(-1)
            return res_feed, fetch
```

对于上述的`ImageService`，只重写了前处理方法，将base64格式的图片数据处理成模型预测需要的数据格式。
