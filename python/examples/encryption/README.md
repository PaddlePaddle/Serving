# Encryption Model Prediction

([简体中文](README_CN.md)|English)

## Get Origin Model

The example uses the model file of the fit_a_line example as a origin model

```
sh get_data.sh
```

## Encrypt Model

```
python encrypt.py
```
The key is stored in the `key` file, and the encrypted model file and server-side configuration file are stored in the `encrypt_server` directory.
client-side configuration file are stored in the `encrypt_client` directory.

## Start Encryption Service
CPU Service
```
python -m paddle_serving_server.serve --model encrypt_server/ --port 9300 --use_encryption_model
```
GPU Service
```
python -m paddle_serving_server_gpu.serve --model encrypt_server/ --port 9300 --use_encryption_model --gpu_ids 0
```

## Prediction
```
python test_client.py uci_housing_client/serving_client_conf.prototxt
```
