#使用docker编译Paddle Serving

## docker编译环境要求

+ 开发机上已安装docker
+ 编译GPU版本需要安装nvidia-docker

## CPU版本dockerfile

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



##GPU版本dockerfile

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

###构建docker镜像

建立新目录，复制dockerfile内容到目录下dockerfile文件

执行

```bash
docker build -t serving_compile:cpu .
```

或者

```bash
docker build -t serving_compile:cuda9 .
```

## 进入docker

CPU版本请执行

```bash
docker run -it serving_compile:cpu bash
```

GPU版本请执行

```bash
docker run -it --runtime=nvidia -it serving_compile:cuda9 bash
```
