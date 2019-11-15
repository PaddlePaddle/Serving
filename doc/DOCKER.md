# 使用Docker编译Paddle Serving

## Docker编译环境要求

+ 开发机上已安装Docker
+ 编译GPU版本需要安装nvidia-docker

## CPU版本Dockerfile

```bash
FROM centos:centos6.10
RUN yum -y install wget \
    && wget http://people.centos.org/tru/devtools-2/devtools-2.repo -O /etc/yum.repos.d/devtoolset-2.repo \
    && yum -y install devtoolset-2-gcc devtoolset-2-gcc-c++ devtoolset-2-binutils \
    && source /opt/rh/devtoolset-2/enable \
    && echo "source /opt/rh/devtoolset-2/enable" >> /etc/profile \
    && yum -y install git openssl-devel curl-devel bzip2-devel \
    && wget https://cmake.org/files/v3.5/cmake-3.5.2.tar.gz \
    && tar xvf cmake-3.5.2.tar.gz \
    && cd cmake-3.5.2 \
    &&  ./bootstrap --prefix=/usr \
    && make \
    && make install \
    && cd .. \
    && rm -r cmake-3.5.2* \
    && wget https://dl.google.com/go/go1.12.12.linux-amd64.tar.gz \
    && tar -xzvf go1.12.12.linux-amd64.tar.gz \
    && mv go /usr/local/go \
    && rm go1.12.12.linux-amd64.tar.gz \
    && echo "export GOROOT=/usr/local/go" >> /root/.bashrc \
    && echo "export GOPATH=$HOME/go" >> /root/.bashrc \
    && echo "export PATH=$PATH:/usr/local/go/bin" >> /root/.bashrc
```



##GPU版本Dockerfile

```bash
FROM paddlepaddle/paddle_manylinux_devel:cuda9.0_cudnn7
RUN yum -y install git openssl-devel curl-devel bzip2-devel \
    && wget https://dl.google.com/go/go1.12.12.linux-amd64.tar.gz \
    && tar -xzvf go1.12.12.linux-amd64.tar.gz \
    && rm -rf /usr/local/go \
    && mv go /usr/local/go \
    && rm go1.12.12.linux-amd64.tar.gz \
    && echo "GOROOT=/usr/local/go" >> /root/.bashrc \
    && echo "GOPATH=$HOME/go" >> /root/.bashrc \
    && echo "PATH=$PATH:$GOROOT/bin" >> /root/.bashrc
```



## 使用方法

###构建Docker镜像

建立新目录，复制Dockerfile内容到目录下Dockerfile文件

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
+ 若执行预编译版本出现找不到libcrypto.so.10、libssl.so.10的情况，可以将Docker环境中的/usr/lib64/libssl.so.10与/usr/lib64/libcrypto.so.10复制到可执行文件输在目录。
+ CPU预编译版本仅可在CPU机器上执行，GPU预编译版本仅可在GPU机器上执行
