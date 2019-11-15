FROM centos:centos6.10
RUN export http_proxy=http://172.19.56.199:3128 \
    && export https_proxy=http://172.19.56.199:3128 \
    && yum -y install wget \
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


