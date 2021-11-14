# Yolov4 检测服务

(简体中文|[English](README.md))

## 获取模型

```
python3 -m paddle_serving_app.package --get_model yolov4
tar -xzvf yolov4.tar.gz
```

## 启动RPC服务

```
python3 -m paddle_serving_server.serve --model yolov4_model --port 9393 --gpu_ids 0
```

## 预测

```
python3 test_client.py 000000570688.jpg
```

预测完成会在`./output`文件夹下生成保存预测结果的json文件以及标出检测结果框的图片。
