# 加密模型预测

Padle Serving 提供了模型加密预测功能，本文档显示了详细信息。

## 原理

采用对称加密算法对模型进行加密。对称加密算法采用同一密钥进行加解密，它计算量小，速度快，是最常用的加密方法。

**一. 获得加密模型：**

普通的模型和参数可以理解为一个字符串，通过对其使用加密算法（参数是您的密钥），普通模型和参数就变成了一个加密的模型和参数。

我们提供了一个简单的演示来加密模型。请参阅[examples/C++/encryption/encrypt.py](../../examples/C++/encryption/encrypt.py)。


**二. 启动加密服务：**

假设您已经有一个已经加密的模型（在`encrypt_server/`路径下）,您可以通过添加一个额外的命令行参数 `--use_encryption_model`来启动加密模型服务。

CPU Service
```
python -m paddle_serving_server.serve --model encrypt_server/ --port 9300 --use_encryption_model
```
GPU Service
```
python -m paddle_serving_server.serve --model encrypt_server/ --port 9300 --use_encryption_model --gpu_ids 0
```

此时，服务器不会真正启动，而是等待密钥。

**三. Client Encryption Inference：**

首先，您必须拥有模型加密过程中使用的密钥。

然后你可以用这个密钥配置你的客户端，当你连接服务器时，这个密钥会发送到服务器，服务器会保留它。

一旦服务器获得密钥，它就使用该密钥解析模型并启动模型预测服务。


**四. 模型加密推理示例：**

模型加密推理示例, 请参见[examples/C++/encryption/](../../examples/C++/encryption/)。
