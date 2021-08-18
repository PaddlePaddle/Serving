## 中文分词模型

(简体中文|[English](./README.md))

### 获取模型
```
python -m paddle_serving_app.package --get_model lac
tar -xzvf lac.tar.gz
```

#### 开启预测服务(支持BRPC-Client/GRPC-Client/Http-Client)

```
python -m paddle_serving_server.serve --model lac_model/ --port 9292
```
### 执行BRPC预测
```
echo "我爱北京天安门" | python lac_client.py lac_client/serving_client_conf.prototxt
```

我们就能得到分词结果

### 执行GRPC/Http预测
```
echo "我爱北京天安门" | python lac_http_client.py lac_client/serving_client_conf.prototxt
```
