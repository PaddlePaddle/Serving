## CTR Prediction Service

([简体中文](./README_CN.md)|English)

### download criteo dataset
```
sh get_data.sh
```

### download inference model
```
wget https://paddle-serving.bj.bcebos.com/criteo_ctr_example/criteo_ctr_demo_model.tar.gz
tar xf criteo_ctr_demo_model.tar.gz
mv models/ctr_client_conf .
mv models/ctr_serving_model .
```
the directories like `ctr_serving_model` and `ctr_client_conf` will appear.

### Start RPC Inference Service

```
python3 -m paddle_serving_server.serve --model ctr_serving_model/ --port 9292 #CPU RPC Service
python3 -m paddle_serving_server.serve --model ctr_serving_model/ --port 9292 --gpu_ids 0 #RPC Service on GPU 0
```

### RPC Infer

```
python3 test_client.py ctr_client_conf/serving_client_conf.prototxt raw_data/part-0
```
the latency will display in the end.
