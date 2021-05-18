# IMDB model ensemble 样例

## 获取模型
```
sh get_data.sh
```

## 启动服务

```
python3 -m paddle_serving_server.serve --model imdb_cnn_model --port 9292 &> cnn.log &
python3 -m paddle_serving_server.serve --model imdb_bow_model --port 9393 &> bow.log &
python3 test_pipeline_server.py &>pipeline.log &
```

## 启动客户端
```
python3 test_pipeline_client.py
```
