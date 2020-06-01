## Chinese Word Segmentation

([简体中文](./README_CN.md)|English)

### Get Model
```
python -m paddle_serving_app.package --get_model lac
tar -xzvf lac.tar.gz
```

#### Start RPC inference service

```
python -m paddle_serving_server.serve --model lac_model/ --port 9292
```
### RPC Infer
```
echo "我爱北京天安门" | python lac_client.py lac_client/serving_client_conf.prototxt
```

It will get the segmentation result. 

### Start HTTP inference service
```
python lac_web_service.py lac_model/ lac_workdir 9292
```
### HTTP Infer

```
curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"words": "我爱北京天安门"}], "fetch":["word_seg"]}' http://127.0.0.1:9292/lac/prediction
```
