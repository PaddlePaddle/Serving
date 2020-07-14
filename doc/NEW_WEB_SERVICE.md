# How to develop a new Web service?

([简体中文](NEW_WEB_SERVICE_CN.md)|English)

This document will take the image classification service based on the Imagenet data set as an example to introduce how to develop a new web service. The complete code can be visited at [here](../python/examples/imagenet/resnet50_web_service.py).

## WebService base class

Paddle Serving implements the [WebService](https://github.com/PaddlePaddle/Serving/blob/develop/python/paddle_serving_server/web_service.py#L23) base class. You need to override its `preprocess` and `postprocess` method. The default implementation is as follows:

```python
class WebService(object):
  
    def preprocess(self, feed={}, fetch=[]):
        return feed, fetch
    def postprocess(self, feed={}, fetch=[], fetch_map=None):
        return fetch_map
```

### preprocess

The preprocess method has two input parameters, `feed` and `fetch`. For an HTTP request `request`:

- The value of `feed` is the feed part `request.json["feed"]` in the request data 
- The value of `fetch` is the fetch part `request.json["fetch"]` in the request data

The return values are the feed and fetch values used in the prediction.

### postprocess

The postprocess method has three input parameters, `feed`, `fetch` and `fetch_map`:

- The value of `feed` is the feed part `request.json["feed"]` in the request data 
- The value of `fetch` is the fetch part `request.json["fetch"]` in the request data
- The value of `fetch_map` is the model output value.

The return value will be processed as `{"reslut": fetch_map}` as the return of the HTTP request.

## Develop ImageService class

```python
class ImageService(WebService):

    def preprocess(self, feed={}, fetch=[]):
        reader = ImageReader()
        feed_batch = []
        for ins in feed:
            if "image" not in ins:
                raise ("feed data error!")
            sample = base64.b64decode(ins["image"])
            img = reader.process_image(sample)
            feed_batch.append({"image": img})
        return feed_batch, fetch
```

For the above `ImageService`, only the `preprocess` method is rewritten to process the image data in Base64 format into the data format required by prediction.
