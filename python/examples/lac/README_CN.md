## 中文分词模型

(简体中文|[English](./README.md))

### 获取模型
```
python -m paddle_serving_app.package --get_model lac
tar -xzvf lac.tar.gz
```

#### 开启RPC预测服务

```
python -m paddle_serving_server.serve --model lac_model/ --port 9292
```
### 执行RPC预测
```
echo "我爱北京天安门" | python lac_client.py lac_client/serving_client_conf.prototxt
```

我们就能得到分词结果

### 开启HTTP预测服务
```
python lac_web_service.py lac_model/ lac_workdir 9292
```
### 执行HTTP预测

```
curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"words": "我爱北京天安门"}], "fetch":["word_seg"]}' http://127.0.0.1:9292/lac/prediction
```
