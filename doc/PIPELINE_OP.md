# 如何配置Web Service的Op



## ## rpc和local predictor

目前一共支持两种Serving的运行方式，一种是rpc，一种是local predictor，二者各有优劣。

| 版本            | 特点                 | 适用场景                                 |
| --------------- | -------------------- | ---------------------------------------- |
| Rpc             | 稳定性高，分布式部署 | 适用于吞吐量大，需要跨机房部署的情况     |
| local predictor | 部署方便，预测速度快 | 适用于对预测速度要求高，迭代速度快的场景 |

rpc模式的原理是启动一个rpc服务，客户端用protobuf格式打包预测请求的内容，在rpc服务端完成预测。适合稳定性较高的场景，web服务和预测服务可以解耦合实现多地部署。

local predictor的原理是启动一个python的predictor，客户端可以直接调用python的predictor来实现。适合快速迭代以及规模较小的场景，web服务和预测服务需要在同一台机器上，

在web模式下，我们通过配置Op的方式来配置每一个Serving模型。

我们以OCR的识别模型作为例子，以下是RecOp的实现。

```python
class RecOp(Op):
    def init_op(self):
        self.ocr_reader = OCRReader()
        self.get_rotate_crop_image = GetRotateCropImage()
        self.sorted_boxes = SortedBoxes()

    def preprocess(self, input_dicts):
        (_, input_dict), = input_dicts.items()
        im = input_dict["image"]
        dt_boxes = input_dict["dt_boxes"]
        dt_boxes = self.sorted_boxes(dt_boxes)
        feed_list = []
        img_list = []
        max_wh_ratio = 0
        for i, dtbox in enumerate(dt_boxes):
            boximg = self.get_rotate_crop_image(im, dt_boxes[i])
            img_list.append(boximg)
            h, w = boximg.shape[0:2]
            wh_ratio = w * 1.0 / h
            max_wh_ratio = max(max_wh_ratio, wh_ratio)
        for img in img_list:
            norm_img = self.ocr_reader.resize_norm_img(img, max_wh_ratio)
            feed = {"image": norm_img}
            feed_list.append(feed)
        return feed_list

    def postprocess(self, input_dicts, fetch_dict):
        rec_res = self.ocr_reader.postprocess(fetch_dict, with_score=True)
        res_lst = []
        for res in rec_res:
            res_lst.append(res[0])
        res = {"res": str(res_lst)}
        return res
```

在做好init_op，preprocess和postprocess这些函数的重载之后，我们就在调用这个Op的地方去控制rpc和local predictor。

```python
#RPC
rec_op = RecOp(
    name="rec",
    input_ops=[det_op],
    server_endpoints=["127.0.0.1:12001"], #if server endpoint exist, use rpc
    fetch_list=["ctc_greedy_decoder_0.tmp_0", "softmax_0.tmp_0"],
   client_config="ocr_rec_client/serving_client_conf.prototxt",
    concurrency=1)
# local predictor
rec_op = RecOp(
    name="rec",
    input_ops=[det_op],
    fetch_list=["ctc_greedy_decoder_0.tmp_0", "softmax_0.tmp_0"],
 model_config="ocr_rec_server/serving_server_conf.prototxt",
    concurrency=1)
```

在上面的例子可以看到，当我们在Op的构造函数里，指定了server_endpoints和client_config时，就会采用rpc的方式。因为这些在运算Op的时候需要先执行preprocess，然后访问rpc服务请求预测，最后再执行postprocess。请求预测的过程，可能预测服务在本地，也可能在远端，可能是单点可能是分布式，需要给出对应的IP地址作为server_endpoints

如果是local predictor，我们就不需要指定endpoints。

| 属性名              | 定义            | 其他                                                         |
| ------------------- | --------------- | ------------------------------------------------------------ |
| name                | op名            |                                                              |
| input_ops           | 前向输入 op     | 可以为多个，前向Op的结果会作为此Op的输入                     |
| fetch_list          | fetch字段名     | 模型预测服务的结果字典包含所有在此定义的fetch字段            |
| rpc限定             |                 |                                                              |
| server_endpoints    | rpc服务地址列表 | 分布式部署时可以有多个rpc地址                                |
| concurrency         | 并行度          | 并行线程数                                                   |
| client_config       | 客户端配置文件  | Op接收请求作为客户端访问rpc服务，因此需要客户端的配置文件    |
| local predictor限定 |                 |                                                              |
| model_config        | 模型配置文件    | 由于local predictor和Op运行在一台机器上，因此需要模型配置来启动local predictor |
