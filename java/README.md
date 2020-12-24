## Tutorial of Java Client for Paddle Serving

(English|[简体中文](./README_CN.md))

### Development Environment

In order to facilitate users to use java for development, we provide the compiled Serving project to be placed in the java mirror. The way to get the mirror and enter the development environment is

```
docker pull hub.baidubce.com/paddlepaddle/serving:0.4.0-java
docker run --rm -dit --name java_serving hub.baidubce.com/paddlepaddle/serving:0.4.0-java
docker exec -it java_serving bash
cd Serving/java
```

The Serving folder is at the develop branch when the docker image is generated. You need to git pull to the latest version or git checkout to the desired branch.

### Install client dependencies

Due to the large number of dependent libraries, the image has been compiled once at the time of generation, and the user can perform the following operations

```
mvn compile
mvn install
cd examples
mvn compile
mvn install
```

### Start the server

Take the fit_a_line model as an example, the server starts

```
cd ../../python/examples/fit_a_line
sh get_data.sh
python -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9393 --use_multilang &
```

Client prediction

```
cd ../../../java/examples/target
java -cp paddle-serving-sdk-java-examples-0.0.1-jar-with-dependencies.jar PaddleServingClientExample fit_a_line
```

Take yolov4 as an example, the server starts

```
python -m paddle_serving_app.package --get_model yolov4
tar -xzvf yolov4.tar.gz
python -m paddle_serving_server_gpu.serve --model yolov4_model --port 9393 --gpu_ids 0 --use_multilang & #It needs to be executed in GPU Docker, otherwise the execution method of CPU must be used.
```

Client prediction

```
# in /Serving/java/examples/target
java -cp paddle-serving-sdk-java-examples-0.0.1-jar-with-dependencies.jar PaddleServingClientExample yolov4 ../../../python/examples/yolov4/000000570688.jpg
# The case of yolov4 needs to specify a picture as input
```

### Customization guidance

The above example is running in CPU mode. If GPU mode is required, there are two options.

The first is that GPU Serving and Java Client are in the same image. After starting the corresponding image, the user needs to move /Serving/java in the java image to the corresponding image.

The second is to deploy GPU Serving and Java Client separately. If they are on the same host, you can learn the IP address of the corresponding container through ifconfig, and then when you connect to client.connect in `examples/src/main/java/PaddleServingClientExample.java` Make changes to the endpoint, and then compile it again. Or select `--net=host` to bind the network device of docker and host when docker starts, so that it can run directly without customizing java code.

**It should be noted that in the example, all models need to use `--use_multilang` to start GRPC multi-programming language support, and the port number is 9393. If you need another port, you need to modify it in the java file**

**Currently Serving has launched the Pipeline mode (see [Pipeline Serving](../doc/PIPELINE_SERVING.md) for details). The next version (0.4.1) of the Pipeline Serving Client for Java will be released. **


