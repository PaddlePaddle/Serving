FROM registry.baidu.com/paddlecloud/paddlecloud-runenv-centos6u3-bce:paddlecloud-fluid-gcc482-cuda8.0_cudnn5_bce
MAINTAINER predictor@baidu.com
LABEL Description="paddle serving docker image"
USER root
RUN echo "Enjoy your paddle serving journey!"
ADD conf /home/work/paddle-serving/conf
ADD data /home/work/paddle-serving/data
ADD bin /home/work/paddle-serving/bin
RUN wget ftp://st01-rdqa-dev055-wanlijin01.epc.baidu.com/home/users/wanlijin01/workspace/baidu/paddle-serving/predictor/data.tar.gz -O /tmp/data.tar.gz \
    && tar -C /home/work/paddle-serving -xvzf /tmp/data.tar.gz \
    && rm /tmp/data.tar.gz \
    && cd /home/work/paddle-serving/ \
    && chmod a+x bin/pdserving  \
    && chmod a+x bin/start.sh \
    && sed -i 's/\.\/conf/\/home\/work\/paddle-serving\/conf/g' conf/workflow.conf \
    && sed -i 's/\.\/conf/\/home\/work\/paddle-serving\/conf/g' conf/resource.conf \
    && sed -i 's/\.\/log/\/home\/work\/paddle-serving\/log/g' conf/log.conf \
    && sed -i 's/\.\/data/\/home\/work\/paddle-serving\/data/g' conf/model_toolkit.conf \
    && mkdir -p /home/work/paddle-serving/log
CMD sh /home/work/paddle-serving/bin/start.sh -c "trap : TERM INT; sleep infinity & wait"
