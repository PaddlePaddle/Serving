
## Prepare
### convert model
```
python -m paddle_serving_client.convert --dirname infer_bert-base-chinese_ft_model_4000.pdparams
```

## RPC Service

### Start Service

```
pytyon bert_web_service.py serving_server 7703
```

### Client Prediction

```
python bert_client.py
```
