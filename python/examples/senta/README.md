# Chinese sentence sentiment classification

## Get model files and sample data
```
sh get_data.sh
```

## Start rpc service


## Start http service
```
python senta_web_service.py senta_bilstm_model/ workdir 9292
```

## Client prediction
```
curl -H "Content-Type:application/json" -X POST -d '{"words": "天气不错 | 0", "fetch":["sentence_feature"]}' http://127.0.0.1:9393/senta/prediction
```
