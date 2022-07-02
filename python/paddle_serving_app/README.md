([简体中文](./README_CN.md)|English)

paddle_serving_app is a tool component of the Paddle Serving framework, and includes functions such as pre-training model download and data pre-processing methods.
It is convenient for users to quickly test and deploy model examples, analyze the performance of prediction services, and debug model prediction services.

## Install

```shell
pip install paddle_serving_app
```

## Get model list

```shell
python -m paddle_serving_app.package --list_model
```

## Download pre-training model

```shell
python -m paddle_serving_app.package --get_model senta_bilstm
```

1 pre-trained models are built into paddle_serving_app, covering 6 kinds of prediction tasks.
The model files can be directly used for deployment, and the `--tutorial` argument can be added to obtain the deployment method.

| Prediction task | Model name                                         |
| ------------ | ------------------------------------------------ |
| SentimentAnalysis | 'senta_bilstm', 'senta_bow', 'senta_cnn'         |
| SemanticRepresentation | 'ernie'                                     |
| ChineseWordSegmentation     | 'lac'                                            |
| ObjectDetection     | 'faster_rcnn'                         |
| ImageSegmentation     | 'unet', 'deeplabv3','deeplabv3+cityscapes'      |
| ImageClassification     | 'resnet_v2_50_imagenet', 'mobilenet_v2_imagenet' |

## Data preprocess API

paddle_serving_app provides a variety of data preprocessing methods for prediction tasks in the field of CV and NLP.

- class ChineseBertReader 
  

Preprocessing for Chinese semantic representation task.

  - `__init__(vocab_file, max_seq_len=20)`

    - vocab_file（st ）：Path of dictionary file.

    - max_seq_len（in ，optional）：The length of sample after processing. The excess part will be truncated, and the insufficient part will be padding 0. Default 20.

  - `process(line)`

    - line（st ）：Text input.

  [example](../../examples/C++/PaddleNLP/bert/bert_client.py)

- class LACReader 
  

Preprocessing for Chinese word segmentation task.

  - `__init__(dict_floder)`
    - dict_floder（st ）Path of dictionary file.
  - `process(sent)`
    - sent（st ）：Text input.
  - `parse_result`
    - words（st ）：Original text input.
    - crf_decode（np.array）：CRF code predicted by model.

  [example](../../examples/C++/PaddleNLP/lac/lac_http_client.py)

- class SentaReader

  - `__init__(vocab_path)`
    - vocab_path（st ）：Path of dictionary file.
  - `process(cols)`
    - cols（st ）：Word segmentation result.

  [example](../../examples/C++/PaddleNLP/senta/senta_web_service.py)

- The image preprocessing method is more flexible than the above method, and can be combined by the following multiple classes，[example](../../examples/C++/PaddleClas/imagenet/resnet50_rpc_client.py)

- class Sequentia

  - `__init__(transforms)`
    - transforms（list）：List of image preprocessing classes
  - `__call__(img)`
    - img：The input of image preprocessing. The data type is is related to the first preprocessing method in transforms.

- class File2Image

  - `__call__(img_path)`
    - img_path（str）：Path of image file.

- class URL2Image

  - `__call__(img_url)`
    - img_url（str）：url of image file.

- class Normalize

  - `__init__(mean,std)`
    - mean（float）：Mean
    - std（float）：Variance
  - `__call__(img)`
    - img（np.array）：Image data in (C,H,W) channels.

- class CenterCrop

  - `__init__(size)`
    - size（list/int）：
  - `__call__(img)`
    - img（np.array）：Image data.

- class Resize

  - `__init__(size, max_size=2147483647, interpolation=None)`
    - size（list/int）：The expected image size, when the input is a list type, it needs to contain the expected length and width. When the input is int type, the short side will be set to the length of size, and the long side will be scaled proportionally.
  - `__call__(img)`
    - img（numpy array）：Image data.


## Timeline tools

The Timeline tool can be used to visualize the start and end time of various stages such as the preparation data of the prediction service, client wait and server op.
This tool is convenient to analyze the proportion of time occupancy in the prediction service. On this basis, prediction services can be optimized in a targeted manner.

### How to use

1. Before making predictions on the client side, turn on the timeline function of each stage in the Paddle Serving framework by environment variables. It will print timeline information in log.

   ```shell
   export FLAGS_profile_client=1 # Turn on timeline function of client
   export FLAGS_profile_server=1 # Turn on timeline function of server
   ```
2. Perform predictions and redirect client-side logs to files, for example, named as profile.

3. Export the information in the log file into a trace file.
   ```shell
   python -m paddle_serving_app.trace --profile_file profile --trace_file trace
   ```

4. Open the `chrome: // tracing /` URL using Chrome browser. 
Load the trace file generated in the previous step through the load button, you can
Visualize the time information of each stage of the forecast service.

As shown in next figure, the figure shows the timeline of GPU prediction service using [bert example](../../examples/C++/PaddleNLP/bert).
The server side starts service with 4 GPU cards, the client side starts 4 processes to request, and the batch size is 1.
In the figure, bert_pre represents the data pre-processing stage of the client, and client_infer represents the stage where the client completes the sending of the prediction request to the receiving result.
The process in the figure represents the process number of the client, and the second line of each process shows the timeline of each op of the server.

![timeline](../../doc/images/timeline-example.png)

## Debug tools

The inference op of Paddle Serving is implemented based on Paddle inference lib.
Before deploying the prediction service, you may need to check the input and output of the prediction service or check the resource consumption.
Therefore, a local prediction tool is built into the paddle_serving_app, which is used in the same way as sending a request to the server through the client.

Taking [fit_a_line prediction service](../../examples/C++/fit_a_line) as an example, the following code can be used to run local prediction.

```python
from paddle_serving_app.local_predict import LocalPredictor
import numpy as np

debugger = LocalPredictor()
debugger.load_model_config("./uci_housing_model", gpu=False)
data = [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727,
        -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332]
fetch_map = debugger.predict(feed={"x":data}, fetch = ["price"])
```
