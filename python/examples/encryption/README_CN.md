# 加密模型预测

(简体中文|[English](README.md))

## 获取明文模型

示例中使用fit_a_line示例的模型文件作为明文模型

```
sh get_data.sh
```

## 模型加密
本示例中使用了`paddlepaddle`包中的模块，需要进行下载（`pip install paddlepaddle`）。

运行[python encrypt.py](./encrypt.py)进行模型加密

[//file]:#encrypt.py
``` python
def serving_encryption():
    inference_model_to_serving(
        dirname="./uci_housing_model",
        params_filename=None,
        serving_server="encrypt_server",
        serving_client="encrypt_client",
        encryption=True)
```
其中dirname为模型所在的文件夹路径

当参数为离散参数时，无须指定params_filename，当参数为__params__时，需指定`params_filename="__params__"`

密钥保存在`key`文件中，加密模型文件以及server端配置文件保存在`encrypt_server`目录下，client端配置文件保存在`encrypt_client`目录下。

## 启动加密预测服务
CPU预测服务
```
python -m paddle_serving_server.serve --model encrypt_server/ --port 9300 --use_encryption_model
```
GPU预测服务
```
python -m paddle_serving_server.serve --model encrypt_server/ --port 9300 --use_encryption_model --gpu_ids 0
```

## 预测
```
python test_client.py uci_housing_client/serving_client_conf.prototxt
```
