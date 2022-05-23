# Paddle Serving Client Java SDK

(简体中文|[English](Java_SDK_EN.md))

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
|             0.9.0             |      0.0.1       |

1.    直接使用提供的Java SDK作为Client进行预测
### 安装

您可以直接下载 jar，安装到本地 Maven 库：

```shell
wget https://paddle-serving.bj.bcebos.com/jar/paddle-serving-sdk-java-0.0.1.jar
mvn install:install-file -Dfile=$PWD/paddle-serving-sdk-java-0.0.1.jar -DgroupId=io.paddle.serving.client -DartifactId=paddle-serving-sdk-java -Dversion=0.0.1 -Dpackaging=jar
```

### Maven 配置

```text
 <dependency>
     <groupId>io.paddle.serving.client</groupId>
     <artifactId>paddle-serving-sdk-java</artifactId>
     <version>0.0.1</version>
 </dependency>
```

2.    从源码进行编译后使用，详细步骤见[文档](../java/README.md).

3.    相关使用示例见[文档](../java/README.md).

