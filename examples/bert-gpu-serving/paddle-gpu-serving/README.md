# paddle-gpu-serving

paddle-gpu-serving简介是基于Paddle Serving框架的模型预测服务库，通过少量代码即可部署和使用指定的模型进行远程预测。目前支持使用PaddleHub语义理解模型库中的BERT类模型获取文本对应的向量表示。其中serving服务的代码参考[bert模型服务demo server端](../../demo-serving/op/bert_service_op.cpp)。

## 安装

### server端

环境要求：python3，paddlepaddle>=1.6，paddlehub>=1.4

```bash
pip install paddle-gpu-serving
```

### client端

环境要求：ujson，python2或python3

下载[bert_service.py脚本](../client/bert_service/bert_service.py)使用

## 使用

### server端

```python
from paddle_gpu_serving.run import BertServer
bs = BertServer(with_gpu=True)
bs.with_model('bert_chinese_L-12_H-768_A-12')
bs.run(gpu_index = 0, port = 8010)
```



### client端

```python
bc = BertService(
    model_name='bert_chinese_L-12_H-768_A-12',
    max_seq_len=20,
    show_ids=False,
    do_lower_case=True)
bc.add_server('127.0.0.1:8010')
result = bc.encode([["远上寒山石径斜"], ])
print(result[0])
```
