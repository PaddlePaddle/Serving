# 中文语句情感分类
(简体中文|[English](./README.md))

## 获取模型文件
```
python3 -m paddle_serving_app.package --get_model senta_bilstm
python3 -m paddle_serving_app.package --get_model lac
tar -xzvf lac.tar.gz
tar -xzvf senta_bilstm.tar.gz
```

## 启动HTTP服务
```
python3 -m paddle_serving_server.serve --model lac_model --port 9300
python3 senta_web_service.py
```
中文情感分类任务中需要先通过[LAC任务](../lac)进行中文分词。
示例中将LAC任务放在情感分类任务的HTTP预测服务的预处理部分。

## 客户端预测
```
curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"words": "天气不错"}], "fetch":["class_probs"]}' http://127.0.0.1:9393/senta/prediction
```
