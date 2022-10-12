# Bert

## Get Model

```
wget https://paddle-serving.bj.bcebos.com/paddle_hub_models/text/SemanticModel/bert_chinese_L-12_H-768_A-12.tar.gz
tar -xzf bert_chinese_L-12_H-768_A-12.tar.gz
mv bert_chinese_L-12_H-768_A-12_model bert_seq128_model
mv bert_chinese_L-12_H-768_A-12_client bert_seq128_client
```

## Get data

```
sh get_data.sh
```

## RPC Service

### Start Service

```
python3 -m paddle_serving_server.serve --model bert_seq128_model/ --port 9292  --use_ipu #启动ipu预测服务

```

### Client Prediction

```
head data-c.txt | python3 bert_client.py --model bert_seq128_client/serving_client_conf.prototxt
```
