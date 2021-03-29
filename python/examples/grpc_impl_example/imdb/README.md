## IMDB comment sentiment inference service

([简体中文](./README_CN.md)|English)

### Get model files and sample data

```
sh get_data.sh
```
the package downloaded contains cnn, lstm and bow model config along with their test_data and train_data.

### Start RPC inference service

```
python -m paddle_serving_server.serve --model imdb_cnn_model/ --thread 10 --port 9393 --use_multilang
```
### RPC Infer

The `paddlepaddle` package is used in `test_client.py`, and you may need to download the corresponding package(`pip install paddlepaddle`).

```
head test_data/part-0 | python test_client.py
```

it will get predict results of the first 10 test cases.
