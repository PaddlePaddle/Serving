# Encryption Model Prediction

([简体中文](README_CN.md)|English)

## Get Origin Model

The example uses the model file of the fit_a_line example as a origin model

```
sh get_data.sh
```

## Encrypt Model

The `paddlepaddle` package is used in this example, you may need to download the corresponding package(`pip3 install paddlepaddle`).

[python3 encrypt.py](./encrypt.py)

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
dirname is the folder path where the model is located. If the parameter is discrete, it is unnecessary to specify params_filename, else you need to set `params_filename="__params__"`.

The key is stored in the `key` file, and the encrypted model file and server-side configuration file are stored in the `encrypt_server` directory.
client-side configuration file are stored in the `encrypt_client` directory.

**Notice：** When encryption prediction is used, the model configuration and parameter folder loaded by server and client should be encrypt_server/ and encrypt_client/
## Start Encryption Service
CPU Service
```
python3 -m paddle_serving_server.serve --model encrypt_server/ --port 9393 --use_encryption_model
```
GPU Service
```
python3 -m paddle_serving_server.serve --model encrypt_server/ --port 9393 --use_encryption_model --gpu_ids 0
```

## Prediction
```
python3 test_client.py encrypt_client/serving_client_conf.prototxt
```
