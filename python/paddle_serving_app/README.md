([简体中文](./README.md)|English)

paddle_serivng_app is a tool component of the Paddle Serving framework, and includes functions such as pre-training model download and data pre-processing methods.
It is convenient for users to quickly test and deploy model examples, analyze the performance of prediction services, and debug model prediction services.

## Install

```shell
pip install paddle_serving_app
```

## Get model list

```shell
python -m paddle_serving_app.package --model_list
```

## Download pre-training model

```shell
python -m paddle_serving_app.package --get_model senta_bilstm
```

11 pre-trained models are built into paddle_serving_app, covering 6 kinds of prediction tasks.
The model files can be directly used for deployment, and the `--tutorial` argument can be added to obtain the deployment method.

| Prediction task | Model name                                         |
| ------------ | ------------------------------------------------ |
| SentimentAnalysis | 'senta_bilstm', 'senta_bow', 'senta_cnn'         |
| SemanticRepresentation | 'ernie_base'                                     |
| ChineseWordSegmentation     | 'lac'                                            |
| ObjectDetection     | 'faster_rcnn', 'yolov3'                          |
| ImageSegmentation     | 'unet', 'deeplabv3'                              |
| ImageClassification     | 'resnet_v2_50_imagenet', 'mobilenet_v2_imagenet' |
