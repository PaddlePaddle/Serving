## Tutorial of Java Client for Paddle Serving

(English|[简体中文](./README_CN.md))

### Development Environment

In order to facilitate users to use java for development, we provide the compiled Serving project to be placed in the java mirror. The way to get the mirror and enter the development environment is

```
docker pull registry.baidubce.com/paddlepaddle/serving:0.5.0-java
docker run --rm -dit --name java_serving registry.baidubce.com/paddlepaddle/serving:0.5.0-java
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

### Start the server(pipeline)

as for input data type = string，take IMDB model ensemble as an example，the server starts

```
cd ../../python/examples/pipeline/imdb_model_ensemble
sh get_data.sh
python -m paddle_serving_server.serve --model imdb_cnn_model --port 9292 &> cnn.log &
python -m paddle_serving_server.serve --model imdb_bow_model --port 9393 &> bow.log &
python test_pipeline_server.py &>pipeline.log &
```

Client prediction(Synchronous)

```
cd ../../../java/examples/target
java -cp paddle-serving-sdk-java-examples-0.0.1-jar-with-dependencies.jar PipelineClientExample string_imdb_predict
```

Client prediction(Asynchronous)

```
cd ../../../java/examples/target
java -cp paddle-serving-sdk-java-examples-0.0.1-jar-with-dependencies.jar PipelineClientExample asyn_predict
```


as for input data type = INDArray，take uci_housing_model as an example，the server starts

```
cd ../../python/examples/pipeline/simple_web_service
sh get_data.sh
python web_service_java.py &>log.txt &
```

Client prediction(Synchronous)

```
cd ../../../java/examples/target
java -cp paddle-serving-sdk-java-examples-0.0.1-jar-with-dependencies.jar PipelineClientExample indarray_predict
```

### Precautions for details

1.In the example, all models(not pipeline) need to use `--use_multilang` to start GRPC multi-programming language support, and the port number is 9393. If you need another port, you need to modify it in the java file

2.Currently Serving has launched the Pipeline mode (see [Pipeline Serving](../doc/Python_Pipeline/Pipeline_Design_EN.md) for details). Pipeline Serving Client for Java is released.

3.The parameters`ip` and`port` in PipelineClientExample.java(path:java/examples/src/main/java/[PipelineClientExample.java](./examples/src/main/java/PipelineClientExample.java)），needs to be connected with the corresponding pipeline server parameters`ip` and`port` which is defined in the config.yaml(Taking IMDB model ensemble as an example，path:python/examples/pipeline/imdb_model_ensemble/[config.yaml](../examples/Pipeline/imdb_model_ensemble/config.yml)）

### Customization guidance

Because the docker image of Java does not contain the compilation and development environment required by serving, and the regular docker image of serving does not contain the compilation and development environment required by Java, the secondary compilation and development of GPU serving and Java client need to be completed under their respective docker images. So, we take GPU mode as an example to explain the two ways of development and deployment.


The first is that when GPU serving and Java client are running in the same GPU image, the user needs to copy the files compiled in the java image (path:/serving /Java) to the path /serving/Java of the GPU image.


The second is that GPU serving and Java client are deployed in their respective docker images (or different hosts with compilation and development environment). At this time, you only need to pay attention to the `ip` and`port` correspondence between the Java client and GPU serving. See item 3 of the above precautions for details.


