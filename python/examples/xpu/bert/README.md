
## Prepare
### download model and extract
```
wget https://paddle-serving.bj.bcebos.com/models/xpu/bert.tgz
```
### convert model
```
python -m paddle_serving_client.convert --dirname infer_bert-base-chinese_ft_model_4000.pdparams
```
### Getting Dict and Sample Dataset

```
sh get_data.sh
```
this script will download Chinese Dictionary File vocab.txt and Chinese Sample Data data-c.txt

## RPC Service

### Start Service

```
python bert_web_service.py serving_server 7703
```

### Client Prediction

```
head data-c.txt | python bert_client.py
```
