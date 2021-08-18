## Chinese Word Segmentation

([简体中文](./README_CN.md)|English)

### Get Model
```
python -m paddle_serving_app.package --get_model lac
tar -xzvf lac.tar.gz
```

#### Start inference service(Support BRPC-Client/GRPC-Client/Http-Client)

```
python -m paddle_serving_server.serve --model lac_model/ --port 9292
```
### BRPC Infer
```
echo "我爱北京天安门" | python lac_client.py lac_client/serving_client_conf.prototxt
```

It will get the segmentation result. 

### GRPC/Http Infer
```
echo "我爱北京天安门" | python lac_http_client.py lac_client/serving_client_conf.prototxt
```
