## 中文分词模型

(简体中文|[English](./README.md))

### 获取模型和字典文件
```
sh get_data.sh
```

下载包里包含了lac模型和lac模型预测需要的字典文件

#### 开启RPC预测服务

```
python -m paddle_serving_server.serve --model jieba_server_model/ --port 9292
```
### 执行RPC预测
```
echo "我爱北京天安门" | python lac_client.py jieba_client_conf/serving_client_conf.prototxt lac_dict/
```

我们就能得到分词结果

### 开启HTTP预测服务
```
python lac_web_service.py jieba_server_model/ lac_workdir 9292
```
### 执行HTTP预测

```
curl -H "Content-Type:application/json" -X POST -d '{"words": "我爱北京天安门", "fetch":["word_seg"]}' http://127.0.0.1:9292/lac/prediction
```



