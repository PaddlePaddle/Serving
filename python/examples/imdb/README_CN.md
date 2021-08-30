## IMDB评论情绪预测服务

(简体中文|[English](./README.md))

### 获取模型文件和样例数据

```
sh get_data.sh
```
脚本会下载和解压出cnn、lstm和bow三种模型的配置文文件以及test_data和train_data。

### 启动预测服务(支持BRPC-Client/GRPC-Client/Http-Client)

```
python3 -m paddle_serving_server.serve --model imdb_cnn_model/ --port 9292
```
### BRPC-Client预测
```
head test_data/part-0 | python3 test_client.py imdb_cnn_client_conf/serving_client_conf.prototxt imdb.vocab
```
预测test_data/part-0的前十个样例。

### BRPC-Client预测
```
head test_data/part-0 | python3 test_http_client.py imdb_cnn_client_conf/serving_client_conf.prototxt imdb.vocab
```
