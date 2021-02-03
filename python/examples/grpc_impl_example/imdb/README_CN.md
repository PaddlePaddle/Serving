## IMDB评论情绪预测服务

(简体中文|[English](./README.md))

### 获取模型文件和样例数据

```
sh get_data.sh
```
脚本会下载和解压出cnn、lstm和bow三种模型的配置文文件以及test_data和train_data。

### 启动RPC预测服务

```
python -m paddle_serving_server.serve --model imdb_cnn_model/ --thread 10 --port 9393 --use_multilang
```
### 执行预测

`test_client.py`中使用了`paddlepaddle`包，需要进行下载（`pip install paddlepaddle`）。

```
head test_data/part-0 | python test_client.py
```
预测test_data/part-0的前十个样例。
