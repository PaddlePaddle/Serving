# C++ PP-Shitu Service

**注意：** Since opencv library is used in the pre-processing and post-processing of C++ code, you need to open `WITH_OPENCV` option to compile the C++ server, see [COMPILE.md](../../../../doc/Compile_CN.md)

```
# Dependent packages need to be installed
pip install faiss-cpu==1.7.1.post2 -i https://mirror.baidu.com/pypi/simple
```

## Get Model
```
wget https://paddle-imagenet-models-name.bj.bcebos.com/dygraph/rec/models/inference/serving/pp_shitu.tar.gz
tar -xvf pp_shitu.tar.gz
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
python3 test_cpp_serving_pipeline.py ./drink_dataset_v1.0/test_images/nongfu_spring.jpeg
```
