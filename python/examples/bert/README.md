## Bert as service

([简体中文](./README_CN.md)|English)

In the example, a BERT model is used for semantic understanding prediction, and the text is represented as a vector, which can be used for further analysis and prediction.

### Getting Model

This example use model [BERT Chinese Model](https://www.paddlepaddle.org.cn/hubdetail?name=bert_chinese_L-12_H-768_A-12&en_category=SemanticModel) from [Paddlehub](https://github.com/PaddlePaddle/PaddleHub).

Install paddlehub first
```
pip install paddlehub
```

run 
```
python prepare_model.py 128
```

the 128 in the command above means max_seq_len in BERT model, which is the length of sample after preprocessing.
the config file and model file for server side are saved in the folder bert_seq128_model.
the config file generated for client side is saved in the folder bert_seq128_client.

You can also download the above model from BOS(max_seq_len=128). After decompression, the config file and model file for server side are stored in the bert_chinese_L-12_H-768_A-12_model folder, and the config file generated for client side is stored in the bert_chinese_L-12_H-768_A-12_client folder:
```shell
wget https://paddle-serving.bj.bcebos.com/paddle_hub_models/text/SemanticModel/bert_chinese_L-12_H-768_A-12.tar.gz
tar -xzf bert_chinese_L-12_H-768_A-12.tar.gz
```

### Getting Dict and Sample Dataset

```
sh get_data.sh
```
this script will download Chinese Dictionary File vocab.txt and Chinese Sample Data data-c.txt

### RPC Inference Service
Run
```
python -m paddle_serving_server.serve --model bert_seq128_model/ --port 9292  #cpu inference service
```
Or
```
python -m paddle_serving_server_gpu.serve --model bert_seq128_model/ --port 9292 --gpu_ids 0 #launch gpu inference service at GPU 0
```

### RPC Inference

before prediction we should install paddle_serving_app. This module provides data preprocessing for BERT model.
```
pip install paddle_serving_app
```
Run
```
head data-c.txt | python bert_client.py --model bert_seq128_client/serving_client_conf.prototxt
```

the client reads data from data-c.txt and send prediction request, the prediction is given by word vector. (Due to massive data in the word vector, we do not print it).

### HTTP Inference Service
```
 export CUDA_VISIBLE_DEVICES=0,1
```
set environmental variable to specify which gpus are used, the command above means gpu 0 and gpu 1 is used.
```
 python bert_web_service.py bert_seq128_model/ 9292 #launch gpu inference service
```
### HTTP Inference 

```
curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"words": "hello"}], "fetch":["pooled_output"]}' http://127.0.0.1:9292/bert/prediction
```

### Benchmark

Model：bert_chinese_L-12_H-768_A-12

GPU：GPU V100 * 1

CUDA/cudnn Version：CUDA 9.2，cudnn 7.1.4


In the test, 10 thousand samples in the sample data are copied into 100 thousand samples. Each client thread sends a sample of the number of threads. The batch size is 1, the max_seq_len is 20(not 128 as described above), and the time unit is seconds.

When the number of client threads is 4, the prediction speed can reach 432 samples per second.
Because a single GPU can only perform serial calculations internally, increasing the number of client threads can only reduce the idle time of the GPU. Therefore, after the number of threads reaches 4, the increase in the number of threads does not improve the prediction speed.

| client  thread num | prepro | client infer | op0   | op1    | op2  | postpro | total  |
| ------------------ | ------ | ------------ | ----- | ------ | ---- | ------- | ------ |
| 1                  | 3.05   | 290.54       | 0.37  | 239.15 | 6.43 | 0.71    | 365.63 |
| 4                  | 0.85   | 213.66       | 0.091 | 200.39 | 1.62 | 0.2     | 231.45 |
| 8                  | 0.42   | 223.12       | 0.043 | 110.99 | 0.8  | 0.098   | 232.05 |
| 12                 | 0.32   | 225.26       | 0.029 | 73.87  | 0.53 | 0.078   | 231.45 |
| 16                 | 0.23   | 227.26       | 0.022 | 55.61  | 0.4  | 0.056   | 231.9  |

the following is the client thread num - latency bar chart:
![bert benchmark](../../../doc/bert-benchmark-batch-size-1.png)
