# Paddle Serving Client Java SDK

([简体中文](JAVA_SDK_CN.md)|English)

Paddle Serving provides Java SDK，which supports predict on the Client side with Java language. This document shows how to use the Java SDK.

## Getting started


### Prerequisites

```
- Java 8 or higher
- Apache Maven
```

The following table shows compatibilities between Paddle Serving Server and Java SDK.

| Paddle Serving Server version | Java SDK version |
| :---------------------------: | :--------------: |
|             0.3.2             |      0.0.1       |

### Install Java SDK

You can download jar and install it to the local Maven repository:

```shell
wget https://paddle-serving.bj.bcebos.com/jar/paddle-serving-sdk-java-0.0.1.jar
mvn install:install-file -Dfile=$PWD/paddle-serving-sdk-java-0.0.1.jar -DgroupId=io.paddle.serving.client -DartifactId=paddle-serving-sdk-java -Dversion=0.0.1 -Dpackaging=jar
```

Or compile from the source code and install it to the local Maven repository:

```shell
cd Serving/java
mvn compile
mvn install
```

### Maven configure

```text
 <dependency>
     <groupId>io.paddle.serving.client</groupId>
     <artifactId>paddle-serving-sdk-java</artifactId>
     <version>0.0.1</version>
 </dependency>
```



## Example

Here we will show how to use Java SDK for Boston house price prediction. Please refer to [examples](../java/examples) folder for more examples.

### Get model

```shell
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/uci_housing.tar.gz
tar -xzf uci_housing.tar.gz
```

### Start Python Server

```shell
python -m paddle_serving_server.serve --model uci_housing_model --port 9393 --use_multilang 
```

#### Client side code example

```java
import io.paddle.serving.client.*;
import org.nd4j.linalg.api.ndarray.INDArray;
import org.nd4j.linalg.factory.Nd4j;
import java.util.*;

public class PaddleServingClientExample {
    public static void main( String[] args ) {
        float[] data = {0.0137f, -0.1136f, 0.2553f, -0.0692f,
            0.0582f, -0.0727f, -0.1583f, -0.0584f,
            0.6283f, 0.4919f, 0.1856f, 0.0795f, -0.0332f};
        INDArray npdata = Nd4j.createFromArray(data);
        HashMap<String, INDArray> feed_data
            = new HashMap<String, INDArray>() {{
                put("x", npdata);
            }};
        List<String> fetch = Arrays.asList("price");

        Client client = new Client();
        String target = "localhost:9393";
        boolean succ = client.connect(target);
        if (succ != true) {
            System.out.println("connect failed.");
            return ;
        }

        Map<String, INDArray> fetch_map = client.predict(feed_data, fetch);
        if (fetch_map == null) {
            System.out.println("predict failed.");
            return ;
        }

        for (Map.Entry<String, INDArray> e : fetch_map.entrySet()) {
            System.out.println("Key = " + e.getKey() + ", Value = " + e.getValue());
        }
        return ;
    }
}
```
