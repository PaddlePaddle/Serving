## IMDB comment sentiment inference service

([简体中文](./README_CN.md)|English)

### Get model files and sample data

```
sh get_data.sh
```
the package downloaded contains cnn, lstm and bow model config along with their test_data and train_data.

### Start inference service(Support BRPC-Client/GRPC-Client/Http-Client)

```
python3 -m paddle_serving_server.serve --model imdb_cnn_model/ --port 9292
```
### BRPC-Client Infer
```
head test_data/part-0 | python3 test_client.py imdb_cnn_client_conf/serving_client_conf.prototxt imdb.vocab
```

it will get predict results of the first 10 test cases.


### GRPC-Client/Http-Client Infer
```
head test_data/part-0 | python3 test_http_client.py imdb_cnn_client_conf/serving_client_conf.prototxt imdb.vocab
```
