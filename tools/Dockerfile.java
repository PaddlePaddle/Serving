# A image for building paddle binaries
# # Use cuda devel base image for both cpu and gpu environment
# # When you modify it, please be aware of cudnn-runtime version
FROM registry.baidubce.com/paddlepaddle/serving:0.8.0-cuda10.2-cudnn8-devel  
MAINTAINER PaddlePaddle Authors <paddle-dev@baidu.com>


# The docker has already installed maven.
RUN apt update && \
    apt install -y default-jre && \
    apt install -y default-jdk && \
    apt install -y maven

EXPOSE 22
