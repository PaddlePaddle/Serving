FROM centos:7.3.1611

RUN yum -y install wget && \
    yum -y install epel-release && yum -y install patchelf && \
    yum -y install gcc gcc-c++ make python-devel && \
    yum -y install libSM-1.2.2-2.el7.x86_64 --setopt=protected_multilib=false && \
    yum -y install libXrender-0.9.10-1.el7.x86_64 --setopt=protected_multilib=false && \
    yum -y install libXext-1.3.3-3.el7.x86_64 --setopt=protected_multilib=false && \
    yum -y install python3 python3-devel && \
    yum clean all 

RUN curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py && \
    python get-pip.py && rm get-pip.py && \
    localedef -c -i en_US -f UTF-8 en_US.UTF-8 && \
    echo "export LANG=en_US.utf8" >> /root/.bashrc
