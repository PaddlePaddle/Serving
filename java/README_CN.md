## Java 示例

### 安装客户端依赖
```
mvn compile
mvn install
cd examples
mvn compile
mvn install
```

### 启动服务端

以fit_a_line模型为例

```
 python -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9393 --use_multilang #CPU
python -m paddle_serving_server_gpu.serve --model uci_housing_model --thread 10 --port 9393 --use_multilang #GPU
```

### 客户端预测
```
java -cp paddle-serving-sdk-java-examples-0.0.1-jar-with-dependencies.jar PaddleServingClientExample fit_a_line
```

java示例中还包含了bert、model_enaemble、asyn_predict、batch_predict、cube_local、cube_quant、yolov4模型的预测客户端。
