# Paddle Serving Client Java SDK

(简体中文|[English](JAVA_SDK.md))

Paddle Serving 提供了 Java SDK，支持 Client 端用 Java 语言进行预测，本文档说明了如何使用 Java SDK。

## 快速开始

### 环境要求

```
- Java 8 or higher
- Apache Maven
```

下表显示了 Paddle Serving Server 和 Java SDK 之间的兼容性

| Paddle Serving Server version | Java SDK version |
| :---------------------------: | :--------------: |
|             0.3.2             |      0.0.1       |

### 安装

您可以直接下载 jar，安装到本地 Maven 库：

```shell
wget https://paddle-serving.bj.bcebos.com/jar/paddle-serving-sdk-java-0.0.1.jar
mvn install:install-file -Dfile=$PWD/paddle-serving-sdk-java-0.0.1.jar -DgroupId=io.paddle.serving.client -DartifactId=paddle-serving-sdk-java -Dversion=0.0.1 -Dpackaging=jar
```

或者从源码进行编译，安装到本地 Maven 库：

```shell
cd Serving/java
mvn compile
mvn install
```

### Maven 配置

```text
 <dependency>
     <groupId>io.paddle.serving.client</groupId>
     <artifactId>paddle-serving-sdk-java</artifactId>
     <version>0.0.1</version>
 </dependency>
```




## 使用样例

这里将展示如何使用 Java SDK 进行房价预测，更多例子详见 [examples](../java/examples) 文件夹。

### 获取房价预测模型

```shell
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/uci_housing.tar.gz
tar -xzf uci_housing.tar.gz
```

### 启动 Python 端 Server

```shell
python -m paddle_serving_server.serve --model uci_housing_model --port 9393 --use_multilang 
```

### Client 端代码示例

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
