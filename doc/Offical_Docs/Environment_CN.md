# 环境配置

使用 PaddleServing 前需要准备相关环境。

## 环境准备

* **Linux 版本 (64 bit)**
    * **CentOS 7 (GPU 版本支持CUDA 10.1/10.2/11.0/11.1/11.2)**
    * **Ubuntu 16.04 (GPU 版本支持 CUDA 10.1/10.2/11.0/11.1/11.2)**
    * **Ubuntu 18.04 (GPU 版本支持 CUDA 10.1/10.2/11.0/11.1/11.2)**
* **Python 版本 3.6/3.7/3.8/3.9 (64 bit)**

## 选择 CPU/GPU

* 如果您的计算机有 NVIDIA® GPU，请确保满足以下条件

    * **CUDA 工具包 10.1/10.2 配合 cuDNN 7 (cuDNN 版本>=7.6.5)**
    * **CUDA 工具包 11.2 配合 cuDNN v8.1.1**
    * **配套版本的 TensorRT**
    * **GPU运算能力超过3.5的硬件设备**

        您可参考NVIDIA官方文档了解CUDA和CUDNN的安装流程和配置方法，请见[CUDA](https://docs.nvidia.com/cuda/cuda-installation-guide-linux/)，[cuDNN](https://docs.nvidia.com/deeplearning/sdk/cudnn-install/)，[TensorRT](https://docs.nvidia.com/deeplearning/tensorrt/index.html)

## 安装必要工具

**一.更新系统源**

* Centos 环境

    更新`yum`的源：

    ```
    yum update
    ```

    并添加必要的yum源：

    ```
    yum install -y epel-release
    ```

* Ubuntu 环境

    更新`apt`的源：

    ```
    apt update
    ```

**二.安装工具**

* Centos 环境

    `bzip2`以及`make`：

    ```
    yum install -y bzip2
    ```

    ```
    yum install -y make
    ```

    cmake 需要3.15以上，建议使用3.16.0:

    ```
    wget -q https://cmake.org/files/v3.16/cmake-3.16.0-Linux-x86_64.tar.gz
    ```

    ```
    tar -zxvf cmake-3.16.0-Linux-x86_64.tar.gz
    ```

    ```
    rm cmake-3.16.0-Linux-x86_64.tar.gz
    ```

    ```
    PATH=/home/cmake-3.16.0-Linux-x86_64/bin:$PATH
    ```

    gcc 需要5.4以上，建议使用8.2.0:

    ```
    wget -q https://paddle-docker-tar.bj.bcebos.com/home/users/tianshuo/bce-python-sdk-0.8.27/gcc-8.2.0.tar.xz && \
    tar -xvf gcc-8.2.0.tar.xz && \
    cd gcc-8.2.0 && \
    sed -i 's#ftp://gcc.gnu.org/pub/gcc/infrastructure/#https://paddle-ci.gz.bcebos.com/#g' ./contrib/download_prerequisites && \
    unset LIBRARY_PATH CPATH C_INCLUDE_PATH PKG_CONFIG_PATH CPLUS_INCLUDE_PATH INCLUDE && \
    ./contrib/download_prerequisites && \
    cd .. && mkdir temp_gcc82 && cd temp_gcc82 && \
    ../gcc-8.2.0/configure --prefix=/usr/local/gcc-8.2 --enable-threads=posix --disable-checking --disable-multilib && \
    make -j8 && make install
    ```

* Ubuntu 环境

    `bzip2`以及`make`：

    ```
    apt install -y bzip2
    ```
    ```
    apt install -y make
    ```

    cmake 需要3.15以上，建议使用3.16.0:

    ```
    wget -q https://cmake.org/files/v3.16/cmake-3.16.0-Linux-x86_64.tar.gz
    ```

    ```
    tar -zxvf cmake-3.16.0-Linux-x86_64.tar.gz
    ```

    ```
    rm cmake-3.16.0-Linux-x86_64.tar.gz
    ```

    ```
    PATH=/home/cmake-3.16.0-Linux-x86_64/bin:$PATH
    ```

    gcc 需要5.4以上，建议使用8.2.0:

    ```
    wget -q https://paddle-docker-tar.bj.bcebos.com/home/users/tianshuo/bce-python-sdk-0.8.27/gcc-8.2.0.tar.xz && \
    tar -xvf gcc-8.2.0.tar.xz && \
    cd gcc-8.2.0 && \
    sed -i 's#ftp://gcc.gnu.org/pub/gcc/infrastructure/#https://paddle-ci.gz.bcebos.com/#g' ./contrib/download_prerequisites && \
    unset LIBRARY_PATH CPATH C_INCLUDE_PATH PKG_CONFIG_PATH CPLUS_INCLUDE_PATH INCLUDE && \
    ./contrib/download_prerequisites && \
    cd .. && mkdir temp_gcc82 && cd temp_gcc82 && \
    ../gcc-8.2.0/configure --prefix=/usr/local/gcc-8.2 --enable-threads=posix --disable-checking --disable-multilib && \
    make -j8 && make install
    ```

**三.安装GOLANG**

    建议使用 go1.17.2:

    ```
    wget -qO- https://go.dev/dl/go1.17.2.linux-amd64.tar.gz | \
    tar -xz -C /usr/local && \
    mkdir /root/go && \
    mkdir /root/go/bin && \
    mkdir /root/go/src && \
    echo "GOROOT=/usr/local/go" >> /root/.bashrc && \
    echo "GOPATH=/root/go" >> /root/.bashrc && \
    echo "PATH=/usr/local/go/bin:/root/go/bin:$PATH" >> /root/.bashrc
    source /root/.bashrc
    ```
  
**四.安装依赖库**

* Centos 环境

    安装相关依赖库 patchelf：

    ```
    yum install patchelf
    ```

    配置 ssl 依赖库

    ```
    wget https://paddle-serving.bj.bcebos.com/others/centos_ssl.tar && \
    tar xf centos_ssl.tar && rm -rf centos_ssl.tar && \
    mv libcrypto.so.1.0.2k /usr/lib/libcrypto.so.1.0.2k && mv libssl.so.1.0.2k /usr/lib/libssl.so.1.0.2k && \
    ln -sf /usr/lib/libcrypto.so.1.0.2k /usr/lib/libcrypto.so.10 && \
    ln -sf /usr/lib/libssl.so.1.0.2k /usr/lib/libssl.so.10 && \
    ln -sf /usr/lib/libcrypto.so.10 /usr/lib/libcrypto.so && \
    ln -sf /usr/lib/libssl.so.10 /usr/lib/libssl.so
    ```

* Ubuntu 环境

    安装相关依赖库 patchelf：

    ```
    apt-get install patchelf
    ```

    配置 ssl 依赖库

    ```
    wget https://paddle-serving.bj.bcebos.com/others/centos_ssl.tar && \
    tar xf centos_ssl.tar && rm -rf centos_ssl.tar && \
    mv libcrypto.so.1.0.2k /usr/lib/libcrypto.so.1.0.2k && mv libssl.so.1.0.2k /usr/lib/libssl.so.1.0.2k && \
    ln -sf /usr/lib/libcrypto.so.1.0.2k /usr/lib/libcrypto.so.10 && \
    ln -sf /usr/lib/libssl.so.1.0.2k /usr/lib/libssl.so.10 && \
    ln -sf /usr/lib/libcrypto.so.10 /usr/lib/libcrypto.so && \
    ln -sf /usr/lib/libssl.so.10 /usr/lib/libssl.so
    ```