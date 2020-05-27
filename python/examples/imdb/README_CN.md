## IMDB评论情绪预测服务

(简体中文|[English](./README.md))

### 获取模型文件和样例数据

```
sh get_data.sh
```
脚本会下载和解压出cnn、lstm和bow三种模型的配置文文件以及test_data和train_data。

### 启动RPC预测服务

```
python -m paddle_serving_server.serve --model imdb_cnn_model/ --port 9292
```
### 执行预测
```
head test_data/part-0 | python test_client.py imdb_cnn_client_conf/serving_client_conf.prototxt imdb.vocab
```
预测test_data/part-0的前十个样例。

### 启动HTTP预测服务
```
python text_classify_service.py imdb_cnn_model/ workdir/ 9292 imdb.vocab
```
### 执行预测

```
curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"words": "i am very sad | 0"}], "fetch":["prediction"]}' http://127.0.0.1:9292/imdb/prediction
```
