# Paddle Serving Client Java SDK

([简体中文](Java_SDK_CN.md)|English)

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
|             0.9.0             |      0.0.1       |

1.    Directly use the provided Java SDK as the client for prediction
### Install Java SDK

You can download jar and install it to the local Maven repository:

```shell
wget https://paddle-serving.bj.bcebos.com/jar/paddle-serving-sdk-java-0.0.1.jar
mvn install:install-file -Dfile=$PWD/paddle-serving-sdk-java-0.0.1.jar -DgroupId=io.paddle.serving.client -DartifactId=paddle-serving-sdk-java -Dversion=0.0.1 -Dpackaging=jar
```

### Maven configure

```text
 <dependency>
     <groupId>io.paddle.serving.client</groupId>
     <artifactId>paddle-serving-sdk-java</artifactId>
     <version>0.0.1</version>
 </dependency>
```

2.    Use it after compiling from the source code. See the [document](../java/README.md).

3.    examples for using the java client, see the See the [document](../java/README.md).
