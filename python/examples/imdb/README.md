## IMDB comment sentiment inference service

([简体中文](./README_CN.md)|English)

### Get model files and sample data

```
sh get_data.sh
```
the package downloaded contains cnn, lstm and bow model config along with their test_data and train_data.

### Start RPC inference service

```
python -m paddle_serving_server.serve --model imdb_cnn_model/ --port 9292
```
### RPC Infer
```
head test_data/part-0 | python test_client.py imdb_cnn_client_conf/serving_client_conf.prototxt imdb.vocab
```

it will get predict results of the first 10 test cases.

### Start HTTP inference service
```
python text_classify_service.py imdb_cnn_model/ workdir/ 9292 imdb.vocab
```
### HTTP Infer

```
curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"words": "i am very sad | 0"}], "fetch":["prediction"]}' http://127.0.0.1:9292/imdb/prediction
```
