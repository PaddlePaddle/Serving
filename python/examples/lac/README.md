## Chinese Word Segmentation

([简体中文](./README_CN.md)|English)

### Get model files and sample data
```
sh get_data.sh
```

the package downloaded contains lac model config along with lac dictionary.

#### Start RPC inference service

```
python -m paddle_serving_server.serve --model jieba_server_model/ --port 9292
```
### RPC Infer
```
echo "我爱北京天安门" | python lac_client.py jieba_client_conf/serving_client_conf.prototxt lac_dict/
```

it will get the segmentation result

### Start HTTP inference service
```
python lac_web_service.py jieba_server_model/ lac_workdir 9292
```
### HTTP Infer

```
curl -H "Content-Type:application/json" -X POST -d '{"words": "我爱北京天安门", "fetch":["word_seg"]}' http://127.0.0.1:9292/lac/prediction
```



