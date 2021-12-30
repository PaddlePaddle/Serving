# PP-Shitu
```
# 需安装相关依赖包
pip install faiss-cpu==1.7.1.post2 -i https://mirror.baidu.com/pypi/simple
```

## 获取模型
```
wget https://paddle-imagenet-models-name.bj.bcebos.com/dygraph/rec/models/inference/serving/pp_shitu.tar.gz
tar -xvf pp_shitu.tar.gz
```

## 获取测试图像和index
```
wget https://paddle-imagenet-models-name.bj.bcebos.com/dygraph/rec/data/drink_dataset_v1.0.tar
tar -xvf drink_dataset_v1.0.tar
```

## RPC 服务
### 启动服务端
```
sh run_cpp_serving.sh
```

### 客户端预测
```
python3 test_cpp_serving_pipeline.py ./drink_dataset_v1.0/test_images/nongfu_spring.jpeg
```
