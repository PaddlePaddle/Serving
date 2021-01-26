## 用于Paddle Serving的Java客户端

([English](./README.md)|简体中文)

### 开发环境

为了方便用户使用java进行开发，我们提供了编译好的Serving工程放置在java镜像当中，获取镜像并进入开发环境的方式是

```
docker pull hub.baidubce.com/paddlepaddle/serving:0.4.0-java
docker run --rm -dit --name java_serving hub.baidubce.com/paddlepaddle/serving:0.4.0-java
docker exec -it java_serving bash
cd Serving/java
```

Serving文件夹是镜像生成时的develop分支工程目录，需要git pull 到最新版本，或者git checkout 到想要的分支。

### 安装客户端依赖

由于依赖库数量庞大，因此镜像已经在生成时编译过一次，用户执行以下操作即可

```
mvn compile
mvn install
cd examples
mvn compile
mvn install
```

### 启动服务端(非pipeline方式)

以fit_a_line模型为例，服务端启动

```
cd ../../python/examples/fit_a_line
sh get_data.sh
python -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9393 --use_multilang &
```

客户端预测

```
cd ../../../java/examples/target
java -cp paddle-serving-sdk-java-examples-0.0.1-jar-with-dependencies.jar PaddleServingClientExample fit_a_line
```

以yolov4为例子，服务端启动

```
python -m paddle_serving_app.package --get_model yolov4
tar -xzvf yolov4.tar.gz
python -m paddle_serving_server_gpu.serve --model yolov4_model --port 9393 --gpu_ids 0 --use_multilang &  #需要在GPU Docker当中执行，否则要使用CPU的执行方式。
```

客户端预测

```
# in /Serving/java/examples/target
java -cp paddle-serving-sdk-java-examples-0.0.1-jar-with-dependencies.jar PaddleServingClientExample yolov4 ../../../python/examples/yolov4/000000570688.jpg
# yolov4的案例需要指定一个图片作为输入

```

### 启动服务端(Pipeline方式)

对于input data type = string类型，以IMDB model ensemble模型为例，服务端启动

```
cd ../../python/examples/pipeline/imdb_model_ensemble
sh get_data.sh
python -m paddle_serving_server.serve --model imdb_cnn_model --port 9292 &> cnn.log &
python -m paddle_serving_server.serve --model imdb_bow_model --port 9393 &> bow.log &
python test_pipeline_server.py &>pipeline.log &
```

客户端预测(同步)

```
cd ../../../java/examples/target
java -cp paddle-serving-sdk-java-examples-0.0.1-jar-with-dependencies.jar PipelineClientExample string_imdb_predict
```

客户端预测(异步)

```
cd ../../../java/examples/target
java -cp paddle-serving-sdk-java-examples-0.0.1-jar-with-dependencies.jar PipelineClientExample asyn_predict
```


对于input data type = INDArray类型，以Simple Pipeline WebService中的uci_housing_model模型为例，服务端启动

```
cd ../../python/examples/pipeline/simple_web_service
sh get_data.sh
python web_service_java.py &>log.txt &
```

客户端预测(同步)

```
cd ../../../java/examples/target
java -cp paddle-serving-sdk-java-examples-0.0.1-jar-with-dependencies.jar PipelineClientExample indarray_predict
```

### 二次开发指导

上述示例是在CPU模式下运行，如果需要GPU模式，可以有两种选择。

第一种是GPU Serving和Java Client在同一个镜像，需要用户在启动对应的镜像后，把java镜像当中的/Serving/java移动到对应的镜像中。

第二种是GPU Serving和Java Client分开部署，如果在同一台宿主机，可以通过ifconfig了解对应容器的IP地址，然后在`examples/src/main/java/PaddleServingClientExample.java`当中对client.connect时的endpoint做修改，然后再编译一次。 或者在docker启动时选择 `--net=host`来绑定docker和宿主机的网络设备，这样不需要定制java代码可以直接运行。

**需要注意的是，在示例中，所有非Pipeline模型都需要使用`--use_multilang`来启动GRPC多编程语言支持，以及端口号都是9393，如果需要别的端口，需要在java文件里修改**

**目前Serving已推出Pipeline模式（详见[Pipeline Serving](../doc/PIPELINE_SERVING_CN.md)），面向Java的Pipeline Serving Client已发布。**

**需要注意的是，Java Pipeline Client相关示例在/Java/Examples和/Java/src/main中，对应的Pipeline server在/python/examples/pipeline/中
注意java/examples/src/main/java/PipelineClientExample.java中的ip和port，需要与/python/examples/pipeline/中对应Pipeline server的config.yaml文件中配置的ip和port相对应。**
