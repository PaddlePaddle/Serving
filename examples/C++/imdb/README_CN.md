## IMDB 评论情绪预测 ABTest 服务

(简体中文|[English](./README.md))

### 获取模型文件和样例数据

```
sh get_data.sh
```
脚本会下载和解压出cnn、lstm和bow三种模型的配置文文件以及test_data和train_data。

### 启动预测服务(支持BRPC-Client/GRPC-Client/Http-Client)

```python
## 启动 bow 模型服务
python3 -m paddle_serving_server.serve --model imdb_bow_model/ --port 9297 >/dev/null 2>&1 &

## 启动 cnn 模型服务
python3 -m paddle_serving_server.serve --model imdb_cnn_model/ --port 9298  >/dev/null 2>&1 &

## 启动 lstm 模型服务
python3 -m paddle_serving_server.serve --model imdb_lstm_model/ --port 9299 >/dev/null 2>&1 &
```

### ABTest 预测
```
head test_data/part-0 | python3 abtest_client.py imdb_cnn_client_conf/serving_client_conf.prototxt imdb.vocab
```
预测test_data/part-0的前十个样例。

### http预测
```
head test_data/part-0 | python3 test_http_client.py imdb_cnn_client_conf/serving_client_conf.prototxt imdb.vocab
```
