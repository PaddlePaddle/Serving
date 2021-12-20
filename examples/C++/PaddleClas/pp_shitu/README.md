# PP-Shitu

## Get Model
```
wget https://paddle-imagenet-models-name.bj.bcebos.com/dygraph/rec/models/inference/serving/pp_shitu.tar.gz
tar -xzvf pp_shitu.tar.gz
```

## Get test images and index
```
wget https://paddle-imagenet-models-name.bj.bcebos.com/dygraph/rec/data/drink_dataset_v1.0.tar
tar -xvf drink_dataset_v1.0.tar
```

## RPC Service
### Start Service
```
sh run_cpp_serving.sh
```

### Client Prediction
```
python3 test_cpp_serving_pipeline.py ./drint_dataset_v1.0/test_images/nongfu_spring.jpeg
```
