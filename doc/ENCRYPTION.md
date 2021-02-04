# MOEDL ENCRYPTION INFERENCE

([简体中文](ENCRYPTION_CN.md)|English)

Paddle Serving provides model encryption inference, This document shows the details.

## Principle

We use symmetric encryption algorithm to encrypt the model. Symmetric encryption algorithm uses the same key for encryption and decryption, it has small amount of calculation, fast speed, is the most commonly used encryption method.

### Got an encrypted model

First of all, you got have a key for encryption.

Normal model and parameters can be understood as a string, by using the encryption algorithm (parameter is your key) on them, the normal model and parameters become an encrypted one.

We provide a simple demo to encrypt the model. See the file 
https://github.com/HexToString/Serving/blob/develop/python/examples/encryption/README.md
```
- Java 8 or higher
- Apache Maven
```

The following table shows compatibilities between Paddle Serving Server and Java SDK.

| Paddle Serving Server version | Java SDK version |
| :---------------------------: | :--------------: |
|             0.3.2             |      0.0.1       |

1、Directly use the provided Java SDK as the client for prediction
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

2、Use it after compiling from the source code. See the file:
https://github.com/PaddlePaddle/Serving/blob/develop/java/README_CN.md

3、examples for using the java client, see the file：
https://github.com/PaddlePaddle/Serving/blob/develop/java/README_CN.md

