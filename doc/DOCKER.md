# 使用Docker编译Paddle Serving

## Docker编译环境要求

+ 开发机上已安装Docker
+ 编译GPU版本需要安装nvidia-docker

[CPU版本Dockerfile](../Dockerfile)

[GPU版本Dockerfile](../Dockerfile.gpu)

## 使用方法

### 构建Docker镜像

建立新目录，复制Dockerfile内容到该目录下Dockerfile文件

执行

```bash
docker build -t serving_compile:cpu .
```

或者

```bash
docker build -t serving_compile:cuda9 .
```

## 进入Docker

CPU版本请执行

```bash
docker run -it serving_compile:cpu bash
```

GPU版本请执行

```bash
docker run -it --runtime=nvidia -it serving_compile:cuda9 bash
```

## 预编译文件可执行环境列表

| Docker预编译版本可运行环境 |
| -------------------------- |
| Centos6                    |
| Centos7                    |
| Ubuntu16.04                |
| Ubuntu 18.04               |



| GPU Docker预编译版本支持的CUDA版本 |
| ---------------------------------- |
| cuda8_cudnn7                       |
| cuda9_cudnn7                       |
| cuda10_cudnn7                      |

**备注：** 
+ 若执行预编译版本出现找不到libcrypto.so.10、libssl.so.10的情况，可以将Docker环境中的/usr/lib64/libssl.so.10与/usr/lib64/libcrypto.so.10复制到可执行文件所在目录。
+ CPU预编译版本仅可在CPU机器上执行，GPU预编译版本仅可在GPU机器上执行
