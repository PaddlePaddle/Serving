## Bert as service

([简体中文](./README_CN.md)|English)

In the example, a BERT model is used for semantic understanding prediction, and the text is represented as a vector, which can be used for further analysis and prediction.
If your python version is 3.X, replace the 'pip' field in the following command with 'pip3',replace 'python' with 'python3'.

### Getting Model
method 1:
This example use model [BERT Chinese Model](https://www.paddlepaddle.org.cn/hubdetail?name=bert_chinese_L-12_H-768_A-12&en_category=SemanticModel) from [Paddlehub](https://github.com/PaddlePaddle/PaddleHub).

Install paddlehub first
```
pip3 install paddlehub
```

run 
```
python3 prepare_model.py 128
```

**PaddleHub only support Python 3.5+**

the 128 in the command above means max_seq_len in BERT model, which is the length of sample after preprocessing.
the config file and model file for server side are saved in the folder bert_seq128_model.
the config file generated for client side is saved in the folder bert_seq128_client.

method 2:
You can also download the above model from BOS(max_seq_len=128). After decompression, the config file and model file for server side are stored in the bert_chinese_L-12_H-768_A-12_model folder, and the config file generated for client side is stored in the bert_chinese_L-12_H-768_A-12_client folder:
```shell
wget https://paddle-serving.bj.bcebos.com/paddle_hub_models/text/SemanticModel/bert_chinese_L-12_H-768_A-12.tar.gz
tar -xzf bert_chinese_L-12_H-768_A-12.tar.gz
mv bert_chinese_L-12_H-768_A-12_model bert_seq128_model
mv bert_chinese_L-12_H-768_A-12_client bert_seq128_client
```
if your model is bert_chinese_L-12_H-768_A-12_model, replace the 'bert_seq128_model' field in the following command with 'bert_chinese_L-12_H-768_A-12_model',replace 'bert_seq128_client' with 'bert_chinese_L-12_H-768_A-12_client'.

### Getting Dict and Sample Dataset

```
sh get_data.sh
```
this script will download Chinese Dictionary File vocab.txt and Chinese Sample Data data-c.txt

### Inference Service(Support BRPC-Client、GRPC-Client、Http-Client)
start cpu inference service,Run
```
python3 -m paddle_serving_server.serve --model bert_seq128_model/ --port 9292  #cpu inference service
```
Or,start gpu inference service,Run
```
python3 -m paddle_serving_server.serve --model bert_seq128_model/ --port 9292 --gpu_ids 0 #launch gpu inference service at GPU 0
```

### BRPC-Client Inference

before prediction we should install paddle_serving_app. This module provides data preprocessing for BERT model.
```
pip3 install paddle_serving_app
```
Run
```
head data-c.txt | python3 bert_client.py --model bert_seq128_client/serving_client_conf.prototxt
```

the client reads data from data-c.txt and send prediction request, the prediction is given by word vector. (Due to massive data in the word vector, we do not print it).

#### GRPC-Client/HTTP-Client
Run
```
head data-c.txt | python3 bert_httpclient.py --model bert_seq128_client/serving_client_conf.prototxt

```


## Benchmark
``` shell
bash benchmark.sh bert_seq128_model bert_seq128_client
```
The output log file of benchmark named `profile_log_bert_seq128_model`
