## Java Demo

### Install package
```
mvn compile
mvn install
cd examples
mvn compile
mvn install
```

### Start Server

take the fit_a_line demo as example

```
 python -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9393 --use_multilang #CPU
python -m paddle_serving_server_gpu.serve --model uci_housing_model --thread 10 --port 9393 --use_multilang #GPU
```

### Client Predict
```
java -cp paddle-serving-sdk-java-examples-0.0.1-jar-with-dependencies.jar PaddleServingClientExample fit_a_line
```

The Java example also contains the prediction client of Bert, Model_enaemble, asyn_predict, batch_predict, Cube_local, Cube_quant, and Yolov4 models.
