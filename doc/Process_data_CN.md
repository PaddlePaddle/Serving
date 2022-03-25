## Paddle Serving 数据处理

### 综述

Paddle Serving提供了非常灵活的pipeline web/rpc服务，因此需要一个统一的教程来指导在数据流的各个阶段，我们的自然数据（文字/图片/稀疏参数表）会以何种形式存在并且传递。本文将以pipeline web service为例。

### pipeline客户端

pipeline客户端只做很简单的处理，他们把自然输入转化成可以序列化的JSON字典或者是对应的protubuf bytes字段即可。

#### 1）字符串/数字

字符串和数字在这个阶段都以字符串的形式存在。我们以[房价预测](../examples/Pipeline/simple_web_service)作为例子。房价预测的输入是13个维度的浮点数去描述一个住房的特征。在客户端阶段就可以直接如下所示

```
curl -X POST -k http://localhost:18082/uci/prediction -d '{"key": ["x"], "value": ["0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332"]}'
```

我们直接把13个数字当成一整个字符串，中间用逗号`,` 隔开。在这里 key所跟随的列表长度需要和 value所跟随的列表长度相等。

同理，如果是字符串文字输入，在这个阶段不妨直接明文输入，例如Bert在这个阶段不妨可以直接写成

```
curl -X POST -k http://localhost:18082/bert/prediction -d '{"key": ["x"], "value": ["hello world"]}'
```

当然，复杂的处理也可以把这个curl转换成python语言，详情参见[Bert Pipeline示例](../examples/Pipeline/PaddleNLP/bert). 

#### 2）图片

图片在Paddle的输入通常需要转换成numpy array，但是在客户端阶段，不需要转换成numpy array，因为那样比较耗费空间，在这个阶段我们用base64 string来传输就可以了，到了服务端的前处理再去解读base64转换成numpy array。详情参见[图像分类pipeline示例](../examples/Pipeline/PaddleClas/DarkNet53/pipeline_http_client.py)，我们也贴出部分代码

```python
def cv2_to_base64(image):
    return base64.b64encode(image).decode('utf8')
if __name__ == "__main__":
    url = "http://127.0.0.1:18080/imagenet/prediction"
    with open(os.path.join(".", "daisy.jpg"), 'rb') as file:
        image_data1 = file.read()
    image = cv2_to_base64(image_data1)
    data = {"key": ["image"], "value": [image]}
    for i in range(100):
        r = requests.post(url=url, data=json.dumps(data))
        print(r.json())
```

可以看出经过这样的操作，图片就可以像string一样，成为JSON或者GRPC Protobuf请求的一部分，发送到了服务端。

## pipeline服务端前处理

这些数据到了服务端之后，由于有一个auto batch的阶段，所以服务端程序接受到的是一个列表的python dict，列表里面的每一个dict，对应着我们从客户端发出去的请求。

#### 1）字符串/数字

刚才提到的房价预测示例，[服务端程序](../examples/Pipeline/simple_web_service/web_service.py)在这里。

```python
    def init_op(self):
        self.separator = ","
        self.batch_separator = ";"

    def preprocess(self, input_dicts, data_id, log_id):
        (_, input_dict), = input_dicts.items() 
        _LOGGER.error("UciOp::preprocess >>> log_id:{}, input:{}".format(
            log_id, input_dict))
        x_value = input_dict["x"].split(self.batch_separator)
        x_lst = []
        for x_val in x_value:
            x_lst.append(
                np.array([
                    float(x.strip()) for x in x_val.split(self.separator)
                ]).reshape(1, 13))
        input_dict["x"] = np.concatenate(x_lst, axis=0)
        proc_dict = {}
        return input_dict, False, None, ""

```

可以看到我们在接收到客户端的请求（请求字典如下）

```json
{"key": ["x"], "value": ["0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332"]}
```

之后，服务端对字符串的逗号`,`做了分隔。变成了 numpy array，并且shape是[1, 13]。最终需要确保 return的input_dict就是 能够和Paddle Predictor直接做交互的字典。

对于bert服务由于发送的已经是明文，服务端处理程序

```python
    def init_op(self):
        self.reader = ChineseBertReader({
            "vocab_file": "vocab.txt",
            "max_seq_len": 128
        })

    def preprocess(self, input_dicts, data_id, log_id):
        (_, input_dict), = input_dicts.items()
        print("input dict", input_dict)
        batch_size = len(input_dict.keys())
        feed_res = []
        for i in range(batch_size):
            feed_dict = self.reader.process(input_dict[str(i)].encode("utf-8"))
            for key in feed_dict.keys():
                feed_dict[key] = np.array(feed_dict[key]).reshape(
                    (1, len(feed_dict[key]), 1))
            feed_res.append(feed_dict)
        feed_dict = {}
        for key in feed_res[0].keys():
            feed_dict[key] = np.concatenate([x[key] for x in feed_res], axis=0)
            print(key, feed_dict[key].shape)
        return feed_dict, False, None, ""
```

就是由一个bert字典，来处理输入的明文数据，每一句话都生成 与bert seq len长度的浮点数。最终需要确保 return的input_dict就是 能够和Paddle Predictor直接做交互的字典。

#### 2）图片处理

图像的前处理阶段，前面提到的图像处理程序，[服务端程序](../examples/Pipeline/PaddleClas/DarkNet53/resnet50_web_service.py)如下。

```python
    def init_op(self):
        self.seq = Sequential([
            Resize(256), CenterCrop(224), RGB2BGR(), Transpose((2, 0, 1)),
            Div(255), Normalize([0.485, 0.456, 0.406], [0.229, 0.224, 0.225],
                                True)
        ])
        self.label_dict = {}
        label_idx = 0
        with open("imagenet.label") as fin:
            for line in fin:
                self.label_dict[label_idx] = line.strip()
                label_idx += 1

    def preprocess(self, input_dicts, data_id, log_id):
        (_, input_dict), = input_dicts.items()
        batch_size = len(input_dict.keys())
        imgs = []
        for key in input_dict.keys():
            data = base64.b64decode(input_dict[key].encode('utf8'))
            data = np.fromstring(data, np.uint8)
            im = cv2.imdecode(data, cv2.IMREAD_COLOR)
            img = self.seq(im)
            imgs.append(img[np.newaxis, :].copy())
        input_imgs = np.concatenate(imgs, axis=0)
        return {"image": input_imgs}, False, None, ""
```

可以看到我们在收到请求后，先要做base64的decode，然后再做np from string 最后用opencv库imcode，才能完成图片到numpy array的转换，这个时候的数据就可以直接用于Paddle的图像前处理。

我们最后再经过Sequential的 Resize（调整大小），CenterCrop（中央部分裁剪），RGB2BGR（颜色通道转换），Transpose（转置矩阵），Normalize（归一化），最终形成和Paddle模型输入需求相一致的numpy array。

## pipeline服务端预测

预测阶段和Paddle预测一样，我们在preprocess函数给到了所需的输入，就可以不需要额外添加代码，到postprocess端等待输出即可。

## pipeline服务端后处理

后处理阶段函数原型是`def postprocess(self, input_dicts, fetch_dict, log_id):`

我们会获取Paddle预测返回的fetch dict，后处理通常需要这个字典信息。

后处理的方式多种多样，例如前面的房价预测就不要后处理，预测的结果就已经给出了对房价的预测。

图像分类需要做后处理，代码如下

```python
def postprocess(self, input_dicts, fetch_dict, log_id):
        score_list = fetch_dict["prediction"]
        result = {"label": [], "prob": []}
        for score in score_list:
            score = score.tolist()
            max_score = max(score)
            result["label"].append(self.label_dict[score.index(max_score)]
                                   .strip().replace(",", ""))
            result["prob"].append(max_score)
        result["label"] = str(result["label"])
        result["prob"] = str(result["prob"])
        return result, None, ""
```

我们可以看到输出的字典只有 `prediction`的矩阵，只有通过后处理，才能得到这幅图模型判定的label（物体种类），和prob（对该物体的可信度）。

如果是数字和字符串信息，确保return的result可被JSON序列化即可。

通常后处理返回不再需要传输图片，如果需要传输图片，一样需要处理成base64的样子，交给客户端。

