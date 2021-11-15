# MOEDL ENCRYPTION INFERENCE

([简体中文](./Encryption_CN.md)|English)

Paddle Serving provides model encryption inference, This document shows the details.

## Principle

We use symmetric encryption algorithm to encrypt the model. Symmetric encryption algorithm uses the same key for encryption and decryption, it has small amount of calculation, fast speed, is the most commonly used encryption method.

### Got an Encrypted Model

Normal model and parameters can be understood as a string, by using the encryption algorithm (parameter is your key) on them, the normal model and parameters become an encrypted one.

We provide a simple demo to encrypt the model. See the [examples/C++/encryption/encrypt.py](../../examples/C++/encryption/encrypt.py)。


### Start Encryption Service

Suppose you already have an encrypted model（in the `encrypt_server/`）,you can start the encryption model service by adding an additional command line parameter `--use_encryption_model`

CPU Service
```
python -m paddle_serving_server.serve --model encrypt_server/ --port 9300 --use_encryption_model
```
GPU Service
```
python -m paddle_serving_server.serve --model encrypt_server/ --port 9300 --use_encryption_model --gpu_ids 0
```

At this point, the server does not really start, but waits for the key。

### Client Encryption Inference

First of all, you got have the key which is used in the process of model encryption.

Then you can configure your client with the key, when you connect the server, this key will send to the server and the server will keep it.

Once the server gets the key, it uses the key to parse the model and starts the model prediction service.


### Example of Model Encryption Inference
Example of model encryption inference, See the [examples/C++/encryption/](../../examples/C++/encryption/)。
