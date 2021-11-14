# Chinese Sentence Sentiment Classification
([简体中文](./README_CN.md)|English)

## Get Model
```
python3 -m paddle_serving_app.package --get_model senta_bilstm
python3 -m paddle_serving_app.package --get_model lac
tar -xzvf senta_bilstm.tar.gz
tar -xzvf lac.tar.gz
```

## Start HTTP Service
```
python3 -m paddle_serving_server.serve --model lac_model --port 9300
python3 senta_web_service.py
```
In the Chinese sentiment classification task, the Chinese word segmentation needs to be done through [LAC task] (../lac). 
In this demo, the LAC task is placed in the preprocessing part of the HTTP prediction service of the sentiment classification task.

## Client prediction
```
curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"words": "天气不错"}], "fetch":["class_probs"]}' http://127.0.0.1:9393/senta/prediction
```
