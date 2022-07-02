#!/bin/bash
echo "################################################################"
echo "#                                                              #"
echo "#                                                              #"
echo "#                                                              #"
echo "#          Paddle Serving  begin run  with python$1!           #"
echo "#                                                              #"
echo "#                                                              #"
echo "#                                                              #"
echo "################################################################"
export GOPATH=$HOME/go
export PATH=$PATH:$GOROOT/bin:$GOPATH/bin
export CUDA_INCLUDE_DIRS=/usr/local/cuda-10.2/include
export PYTHONROOT=/usr/local
export PYTHONIOENCODING=utf-8

build_path=/workspace/Serving/
error_words="Fail|DENIED|UNKNOWN|None"
log_dir=${build_path}logs/
data=/root/.cache/serving_data/
OPENCV_DIR=${data}/opencv-3.4.7/opencv3
dir=`pwd`
RED_COLOR='\E[1;31m'
GREEN_COLOR='\E[1;32m'
YELOW_COLOR='\E[1;33m'
RES='\E[0m'
cuda_version=`cat /usr/local/cuda/version.txt`

if [ $? -ne 0 ]; then
    cuda_version=11
fi

go env -w GO111MODULE=on
go env -w GOPROXY=https://goproxy.cn,direct
go get -u github.com/grpc-ecosystem/grpc-gateway/protoc-gen-grpc-gateway@v1.15.2
go get -u github.com/grpc-ecosystem/grpc-gateway/protoc-gen-swagger@v1.15.2
go get -u github.com/golang/protobuf/protoc-gen-go@v1.4.3
go get -u google.golang.org/grpc@v1.33.0
go env -w GO111MODULE=auto

build_whl_list=(build_cpu_server build_gpu_server build_client build_app)
rpc_model_list=(grpc_fit_a_line grpc_yolov4 pipeline_imagenet bert_rpc_gpu bert_rpc_cpu ResNet50_rpc \
lac_rpc_asyn cnn_rpc_asyn bow_rpc lstm_rpc fit_a_line_rpc deeplabv3_rpc mobilenet_rpc unet_rpc resnetv2_rpc \
criteo_ctr_rpc_cpu criteo_ctr_rpc_gpu ocr_rpc yolov4_rpc_gpu faster_rcnn_hrnetv2p_w18_1x_encrypt \
faster_rcnn_model_rpc low_precision_resnet50_int8 ocr_c++_service ocr_c++_service_asyn)
http_model_list=(fit_a_line_http lac_http imdb_http_proto imdb_http_json imdb_grpc ResNet50_http bert_http \
pipeline_ocr_cpu_http)

function setproxy() {
    export http_proxy=${proxy}
    export https_proxy=${proxy}
}

function unsetproxy() {
    unset http_proxy
    unset https_proxy
}

function kill_server_process() {
    kill `ps -ef | grep serving | awk '{print $2}'` > /dev/null 2>&1
    kill `ps -ef | grep python | awk '{print $2}'` > /dev/null 2>&1
    echo -e "${GREEN_COLOR}process killed...${RES}"
}

function set_env() {
    if [ $1 == 36 ]; then
        export PYTHONROOT=/usr/local/
        export PYTHON_INCLUDE_DIR=$PYTHONROOT/include/python3.6m
        export PYTHON_LIBRARIES=$PYTHONROOT/lib/libpython3.6m.so
        export PYTHON_EXECUTABLE=$PYTHONROOT/bin/python3.6
        py_version="python3.6"
    elif [ $1 == 37 ]; then
        export PYTHONROOT=/usr/local/
        export PYTHON_INCLUDE_DIR=$PYTHONROOT/include/python3.7m
        export PYTHON_LIBRARIES=$PYTHONROOT/lib/libpython3.7m.so
        export PYTHON_EXECUTABLE=$PYTHONROOT/bin/python3.7
        py_version="python3.7"
    elif [ $1 == 38 ]; then
        export PYTHONROOT=/usr/local/
        export PYTHON_INCLUDE_DIR=$PYTHONROOT/include/python3.8
        export PYTHON_LIBRARIES=$PYTHONROOT/lib/libpython3.8.so
        export PYTHON_EXECUTABLE=$PYTHONROOT/bin/python3.8
        py_version="python3.8"
    else
        echo -e "${RED_COLOR}Error py version$1${RES}"
        exit
    fi
    export CUDA_PATH='/usr/local/cuda'
    export CUDNN_LIBRARY='/usr/local/cuda/lib64/'
    export CUDA_CUDART_LIBRARY="/usr/local/cuda/lib64/"
    if [ $2 == 101 ]; then
        export TENSORRT_LIBRARY_PATH="/usr/local/TensorRT6-cuda10.1-cudnn7/targets/x86_64-linux-gnu/"
    elif [ $2 == 102 ]; then
        export TENSORRT_LIBRARY_PATH="/usr/local/TensorRT-7.1.3.4/targets/x86_64-linux-gnu/"
    elif [ $2 == 110 ]; then
        export TENSORRT_LIBRARY_PATH="/usr/local/TensorRT-7.1.3.4/targets/x86_64-linux-gnu/"
    elif [ $2 == "cpu" ]; then
        export TENSORRT_LIBRARY_PATH="/usr/local/TensorRT6-cuda9.0-cudnn7/targets/x86_64-linux-gnu"
    else
        echo -e "${RED_COLOR}Error cuda version$1${RES}"
        exit
    fi
}

function check() {
    cd ${build_path}
    if [ ! -f paddle_serving_app* ]; then
        echo "paddle_serving_app is compiled failed, please check your pull request"
        exit 1
    elif [ ! -f paddle_serving_server-* ]; then
        echo "paddle_serving_server-cpu is compiled failed, please check your pull request"
        exit 1
    elif [ ! -f paddle_serving_server_* ]; then
        echo "paddle_serving_server_gpu is compiled failed, please check your pull request"
        exit 1
    elif [ ! -f paddle_serving_client* ]; then
        echo "paddle_serving_server_client is compiled failed, please check your pull request"
        exit 1
    else
        echo "paddle serving build passed"
    fi
}

function check_gpu_memory() {
    gpu_memory=`nvidia-smi --id=$1 --format=csv,noheader --query-gpu=memory.used | awk '{print $1}'`
    echo -e "${GREEN_COLOR}-------id-$1 gpu_memory_used: ${gpu_memory}${RES}"
    if [ ${gpu_memory} -le 100 ]; then
        echo "-------GPU-$1 is not used"
        status="GPU-$1 is not used"
    else
        echo "-------GPU_memory used is expected"
    fi
}

function check_result() {
    if [ $? == 0 ]; then
        echo -e "${GREEN_COLOR}$1 execute normally${RES}"
        if [ $1 == "server" ]; then
            sleep $2
            cat ${dir}server_log.txt | tee -a ${log_dir}server_total.txt
        fi
        if [ $1 == "client" ]; then
            cat ${dir}client_log.txt | tee -a ${log_dir}client_total.txt
            grep -E "${error_words}" ${dir}client_log.txt > /dev/null
            if [ $? == 0 ]; then
                if [ "${status}" != "" ]; then
                    status="${status}|Failed"
                else
                    status="Failed"
                fi
                echo -e "${RED_COLOR}$1 error command${RES}\n" | tee -a ${log_dir}server_total.txt ${log_dir}client_total.txt
                echo "--------------server log:--------------"
                cat ${dir}server_log.txt
                echo "--------------client log:--------------"
                cat ${dir}client_log.txt
                echo "--------------pipeline.log:----------------"
                cat PipelineServingLogs/pipeline.log
                echo "-------------------------------------------\n"
                error_log $2
            else
                if [ "${status}" != "" ]; then
                    error_log $2
                fi
                echo -e "${GREEN_COLOR}$2${RES}\n" | tee -a ${log_dir}server_total.txt ${log_dir}client_total.txt
            fi
        fi
    else
        echo -e "${RED_COLOR}$1 error command${RES}\n" | tee -a ${log_dir}server_total.txt ${log_dir}client_total.txt
        echo "--------------server log:--------------"
        cat ${dir}server_log.txt
        echo "--------------client log:--------------"
        cat ${dir}client_log.txt
        echo "--------------pipeline.log:----------------"
        cat PipelineServingLogs/pipeline.log
        echo "-------------------------------------------\n"
        if [ "${status}" != "" ]; then
            status="${status}|Failed"
        else
            status="Failed"
        fi
        error_log $2
    fi
    status=""
}

function error_log() {
    arg=${1//\//_}
    echo "-----------------------------" | tee -a ${log_dir}error_models.txt
    arg=${arg%% *}
    arr=(${arg//_/ })
    if [ ${arr[@]: -1} == 1 -o ${arr[@]: -1} == 2 ]; then
        model=${arr[*]:0:${#arr[*]}-3}
        deployment=${arr[*]: -3}
    else
        model=${arr[*]:0:${#arr[*]}-2}
        deployment=${arr[*]: -2}
    fi
    echo "model: ${model// /_}" | tee -a ${log_dir}error_models.txt
    echo "deployment: ${deployment// /_}" | tee -a ${log_dir}error_models.txt
    echo "py_version: ${py_version}" | tee -a ${log_dir}error_models.txt
    echo "cuda_version: ${cuda_version}" | tee -a ${log_dir}error_models.txt
    echo "status: ${status}" | tee -a ${log_dir}error_models.txt
    echo -e "-----------------------------\n\n" | tee -a ${log_dir}error_models.txt
    prefix=${arg//\//_}
    for file in ${dir}*
    do
        cp ${file} ${log_dir}error/${prefix}_${file##*/}
    done
}

function check_dir() {
    if [ ! -d "$1" ]
    then
        mkdir -p $1
    fi
}

function link_data() {
    for file in $1*
    do
        if [ ! -h ${file##*/} ]
        then
            ln -s ${file} ./${file##*/}
        fi
    done
}

function before_hook() {
    setproxy
    cd ${build_path}/python
    ${py_version} -m pip install --upgrade pip==21.1.3 -i https://pypi.douban.com/simple
    ${py_version} -m pip install requests
    ${py_version} -m pip install -r requirements.txt
    ${py_version} -m pip install numpy==1.16.4
    ${py_version} -m pip install paddlehub
    echo "before hook configuration is successful.... "
}

function run_env() {
    setproxy
    ${py_version} -m pip install nltk==3.4 -i https://mirror.baidu.com/pypi/simple
    ${py_version} -m pip install setuptools==41.0.0 -i https://mirror.baidu.com/pypi/simple
    ${py_version} -m pip install paddlehub -i https://mirror.baidu.com/pypi/simple
    if [ ${py_version} == "python3.6" ]; then
        ${py_version} -m pip install paddlepaddle-gpu==2.1.0
    elif [ ${py_version} == "python3.8" ]; then
        cd ${build_path}
        wget https://paddle-wheel.bj.bcebos.com/with-trt/2.1.0-gpu-cuda11.0-cudnn8-mkl-gcc8.2/paddlepaddle_gpu-2.1.0.post110-cp38-cp38-linux_x86_64.whl
        ${py_version} -m pip install paddlepaddle_gpu-2.1.0.post110-cp38-cp38-linux_x86_64.whl
    fi
    echo "run env configuration is successful.... "
}

function run_gpu_env() {
    cd ${build_path}
    export LD_LIBRARY_PATH=/usr/local/lib64/python3.6/site-packages/paddle/libs/:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH=/workspace/Serving/build_gpu/third_party/install/Paddle/lib/:/workspace/Serving/build_gpu/third_party/Paddle/src/extern_paddle/third_party/install/mklml/lib/:/workspace/Serving/build_gpu/third_party/Paddle/src/extern_paddle/third_party/install/mkldnn/lib/:$LD_LIBRARY_PATH
    export SERVING_BIN=${build_path}/build_gpu/core/general-server/serving
    echo "run gpu env configuration is successful.... "
}

function build_gpu_server() {
    setproxy
    cd ${build_path}
    git submodule update --init --recursive
    if [ -d build_gpu ];then
        rm -rf build_gpu
    fi
    if [ -d build ];then
        rm -rf build
    fi
    mkdir build && cd build
    cmake -DPYTHON_INCLUDE_DIR=$PYTHON_INCLUDE_DIR \
        -DPYTHON_LIBRARIES=$PYTHON_LIBRARIES \
        -DPYTHON_EXECUTABLE=$PYTHON_EXECUTABLE \
        -DCUDA_TOOLKIT_ROOT_DIR=${CUDA_PATH} \
        -DCUDNN_LIBRARY=${CUDNN_LIBRARY} \
        -DCUDA_CUDART_LIBRARY=${CUDA_CUDART_LIBRARY} \
        -DTENSORRT_ROOT=${TENSORRT_LIBRARY_PATH} \
        -DOPENCV_DIR=${OPENCV_DIR} \
        -DWITH_OPENCV=ON \
        -DSERVER=ON \
        -DWITH_GPU=ON ..
    make -j32
    make -j32
    make install -j32
    ${py_version} -m pip uninstall paddle-serving-server-gpu -y
    ${py_version} -m pip install ${build_path}/build/python/dist/*
    cp  ${build_path}/build/python/dist/* ../
    cp -r ${build_path}/build/ ${build_path}/build_gpu
}

function build_cpu_server(){
    setproxy
    cd ${build_path}
    if [ -d build_cpu ];then
        rm -rf build_cpu
    fi
    if [ -d build ];then
        rm -rf build
    fi
    mkdir build && cd build
    cmake -DPYTHON_INCLUDE_DIR=$PYTHON_INCLUDE_DIR/ \
        -DPYTHON_LIBRARIES=$PYTHON_LIBRARIES \
        -DPYTHON_EXECUTABLE=$PYTHON_EXECUTABLE \
        -DOPENCV_DIR=${OPENCV_DIR} \
        -DWITH_OPENCV=ON \
        -DSERVER=ON ..
    make -j32
    make -j32
    make install -j32
    cp ${build_path}/build/python/dist/* ../
    ${py_version} -m pip uninstall paddle-serving-server -y
    ${py_version} -m pip install ${build_path}/build/python/dist/*
    cp -r ${build_path}/build/ ${build_path}/build_cpu
}

function build_client() {
    setproxy
    cd  ${build_path}
    if [ -d build ];then
        rm -rf build
    fi
    mkdir build && cd build
    cmake -DPYTHON_INCLUDE_DIR=$PYTHON_INCLUDE_DIR \
        -DPYTHON_LIBRARIES=$PYTHON_LIBRARIES \
        -DPYTHON_EXECUTABLE=$PYTHON_EXECUTABLE \
        -DCLIENT=ON ..
    make -j32
    make -j32
    cp ${build_path}/build/python/dist/* ../
    ${py_version} -m pip uninstall paddle-serving-client -y
    ${py_version} -m pip install ${build_path}/build/python/dist/*
}

function build_app() {
    setproxy
    ${py_version} -m pip install paddlehub ujson Pillow
    ${py_version} -m pip install paddlepaddle==2.0.2
    cd ${build_path}
    if [ -d build ];then
        rm -rf build
    fi
    mkdir build && cd build
    cmake -DPYTHON_INCLUDE_DIR=$PYTHON_INCLUDE_DIR \
        -DPYTHON_LIBRARIES=$PYTHON_LIBRARIES \
        -DPYTHON_EXECUTABLE=$PYTHON_EXECUTABLE \
        -DAPP=ON ..
    make
    cp ${build_path}/build/python/dist/* ../
    ${py_version} -m pip uninstall paddle-serving-app -y
    ${py_version} -m pip install ${build_path}/build/python/dist/*
}

function low_precision_resnet50_int8 () {
    dir=${log_dir}rpc_model/low_precision/resnet50/
    cd ${build_path}/python/examples/low_precision/resnet50/
    check_dir ${dir}
    wget https://paddle-inference-dist.bj.bcebos.com/inference_demo/python/resnet50/ResNet50_quant.tar.gz
    tar zxvf ResNet50_quant.tar.gz
    ${py_version} -m paddle_serving_client.convert --dirname ResNet50_quant
    echo -e "${GREEN_COLOR}low_precision_resnet50_int8_GPU_RPC server started${RES}" | tee -a ${log_dir}server_total.txt
    ${py_version} -m paddle_serving_server.serve --model serving_server --port 9393 --gpu_ids 0 --use_trt --precision int8 > ${dir}server_log.txt 2>&1 &
    check_result server 15
    echo -e "${GREEN_COLOR}low_precision_resnet50_int8_GPU_RPC client started${RES}" | tee -a ${log_dir}client_total.txt
    ${py_version} resnet50_client.py > ${dir}client_log.txt 2>&1
    check_result client "low_precision_resnet50_int8_GPU_RPC server test completed"
    kill_server_process
}

function faster_rcnn_hrnetv2p_w18_1x_encrypt() {
    dir=${log_dir}rpc_model/faster_rcnn_hrnetv2p_w18_1x/
    cd ${build_path}/python/examples/detection/faster_rcnn_hrnetv2p_w18_1x
    check_dir ${dir}
    data_dir=${data}detection/faster_rcnn_hrnetv2p_w18_1x/
    link_data ${data_dir}
    ${py_version} encrypt.py
    unsetproxy
    echo -e "${GREEN_COLOR}faster_rcnn_hrnetv2p_w18_1x_ENCRYPTION_GPU_RPC server started${RES}" | tee -a ${log_dir}server_total.txt
    ${py_version} -m paddle_serving_server.serve --model encrypt_server/ --port 9494 --gpu_ids 0 --use_encryption_model > ${dir}server_log.txt 2>&1 &
    check_result server 3
    echo -e "${GREEN_COLOR}faster_rcnn_hrnetv2p_w18_1x_ENCRYPTION_GPU_RPC client started${RES}" | tee -a ${log_dir}client_total.txt
    ${py_version} test_encryption.py 000000570688.jpg > ${dir}client_log.txt 2>&1
    check_result client "faster_rcnn_hrnetv2p_w18_1x_ENCRYPTION_GPU_RPC server test completed"
    kill_server_process
}

function pipeline_ocr_cpu_http() {
    dir=${log_dir}rpc_model/pipeline_ocr_cpu_http/
    check_dir ${dir}
    cd ${build_path}/python/examples/pipeline/ocr
    data_dir=${data}ocr/
    link_data ${data_dir}
    unsetproxy
    echo -e "${GREEN_COLOR}pipeline_ocr_CPU_HTTP server started${RES}" | tee -a ${log_dir}server_total.txt
    $py_version web_service.py > ${dir}server_log.txt 2>&1 &
    check_result server 5
    echo -e "${GREEN_COLOR}pipeline_ocr_CPU_HTTP client started${RES}" | tee -a ${log_dir}client_total.txt
    timeout 15s $py_version pipeline_http_client.py > ${dir}client_log.txt 2>&1
    check_result client "pipeline_ocr_CPU_HTTP server test completed"
    kill_server_process
}

function bert_rpc_gpu() {
    dir=${log_dir}rpc_model/bert_rpc_gpu/
    check_dir ${dir}
    run_gpu_env
    unsetproxy
    cd ${build_path}/python/examples/bert
    data_dir=${data}bert/
    link_data ${data_dir}
    sh get_data.sh >/dev/null 2>&1
    sed -i 's/9292/8860/g' bert_client.py
    sed -i '$aprint(result)' bert_client.py
    ls -hlst
    ${py_version} -m paddle_serving_server.serve --model bert_seq128_model/ --port 8860 --gpu_ids 0 > ${dir}server_log.txt 2>&1 &
    check_result server 15
    check_gpu_memory 0
    nvidia-smi
    head data-c.txt | ${py_version} bert_client.py --model bert_seq128_client/serving_client_conf.prototxt > ${dir}client_log.txt 2>&1
    check_result client "bert_GPU_RPC server test completed"
    nvidia-smi
    kill_server_process
}

function bert_rpc_cpu() {
    dir=${log_dir}rpc_model/bert_rpc_cpu/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/bert
    data_dir=${data}bert/
    link_data ${data_dir}
    sed -i 's/8860/8861/g' bert_client.py
    ${py_version} -m paddle_serving_server.serve --model bert_seq128_model/ --port 8861 > ${dir}server_log.txt 2>&1 &
    check_result server 5
    cp data-c.txt.1 data-c.txt
    head data-c.txt | ${py_version} bert_client.py --model bert_seq128_client/serving_client_conf.prototxt > ${dir}client_log.txt 2>&1
    check_result client "bert_CPU_RPC server test completed"
    kill_server_process
}

function pipeline_imagenet() {
    dir=${log_dir}rpc_model/pipeline_imagenet/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/pipeline/imagenet
    data_dir=${data}imagenet/
    link_data ${data_dir}
    ${py_version} resnet50_web_service.py > ${dir}server_log.txt 2>&1 &
    check_result server 8
    nvidia-smi
    timeout 30s ${py_version} pipeline_rpc_client.py > ${dir}client_log.txt 2>&1
    echo "pipeline_log:-----------"
    check_result client "pipeline_imagenet_GPU_RPC server test completed"
    nvidia-smi
    kill_server_process
}

function ResNet50_rpc() {
    dir=${log_dir}rpc_model/ResNet50_rpc/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/imagenet
    data_dir=${data}imagenet/
    link_data ${data_dir}
    sed -i 's/9696/8863/g' resnet50_rpc_client.py
    ${py_version} -m paddle_serving_server.serve --model ResNet50_vd_model --port 8863 --gpu_ids 0 > ${dir}server_log.txt 2>&1 &
    check_result server 8
    check_gpu_memory 0
    nvidia-smi
    ${py_version} resnet50_rpc_client.py ResNet50_vd_client_config/serving_client_conf.prototxt > ${dir}client_log.txt 2>&1
    check_result client "ResNet50_GPU_RPC server test completed"
    nvidia-smi
    kill_server_process
}

function ResNet101_rpc() {
    dir=${log_dir}rpc_model/ResNet101_rpc/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/imagenet
    data_dir=${data}imagenet/
    link_data ${data_dir}
    sed -i "22cclient.connect(['127.0.0.1:8864'])" image_rpc_client.py
    ${py_version} -m paddle_serving_server.serve --model ResNet101_vd_model --port 8864 --gpu_ids 0 > ${dir}server_log.txt 2>&1 &
    check_result server 8
    check_gpu_memory 0
    nvidia-smi
    ${py_version} image_rpc_client.py ResNet101_vd_client_config/serving_client_conf.prototxt > ${dir}client_log.txt 2>&1
    check_result client "ResNet101_GPU_RPC server test completed"
    nvidia-smi
    kill_server_process
}

function cnn_rpc_asyn() {
    dir=${log_dir}rpc_model/cnn_rpc/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/imdb
    data_dir=${data}imdb/
    link_data ${data_dir}
    sed -i 's/9292/8865/g' test_client.py
    ${py_version} -m paddle_serving_server.serve --model imdb_cnn_model/ --port 8865 --op_num 4 --thread 10 --gpu_ids 0 > ${dir}server_log.txt 2>&1 &
    check_result server 8
    check_gpu_memory 0
    head test_data/part-0 | ${py_version} test_client.py imdb_cnn_client_conf/serving_client_conf.prototxt imdb.vocab > ${dir}client_log.txt 2>&1
    check_result client "cnn_CPU_RPC server test completed"
    kill_server_process
}

function bow_rpc() {
    dir=${log_dir}rpc_model/bow_rpc/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/imdb
    data_dir=${data}imdb/
    link_data ${data_dir}
    sed -i 's/8865/8866/g' test_client.py
    ${py_version} -m paddle_serving_server.serve --model imdb_bow_model/ --port 8866 > ${dir}server_log.txt 2>&1 &
    check_result server 5
    head test_data/part-0 | ${py_version} test_client.py imdb_bow_client_conf/serving_client_conf.prototxt imdb.vocab > ${dir}client_log.txt 2>&1
    check_result client "bow_CPU_RPC server test completed"
    kill_server_process
}

function lstm_rpc() {
    dir=${log_dir}rpc_model/lstm_rpc/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/imdb
    data_dir=${data}imdb/
    link_data ${data_dir}
    sed -i 's/8866/8867/g' test_client.py
    ${py_version} -m paddle_serving_server.serve --model imdb_lstm_model/ --port 8867 > ${dir}server_log.txt 2>&1 &
    check_result server 5
    head test_data/part-0 | ${py_version} test_client.py imdb_lstm_client_conf/serving_client_conf.prototxt imdb.vocab > ${dir}client_log.txt 2>&1
    check_result client "lstm_CPU_RPC server test completed"
    kill_server_process
}

function lac_rpc_asyn() {
    dir=${log_dir}rpc_model/lac_rpc/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/lac
    data_dir=${data}lac/
    link_data ${data_dir}
    sed -i 's/9292/8868/g' lac_client.py
    ${py_version} -m paddle_serving_server.serve --model lac_model/ --port 8868 --gpu_ids 0 --op_num 2 > ${dir}server_log.txt 2>&1 &
    check_result server 8
    check_gpu_memory 0
    echo "我爱北京天安门" | ${py_version} lac_client.py lac_client/serving_client_conf.prototxt lac_dict/ > ${dir}client_log.txt 2>&1
    check_result client "lac_CPU_RPC server test completed"
    kill_server_process
}

function fit_a_line_rpc() {
    dir=${log_dir}rpc_model/fit_a_line_rpc/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/fit_a_line
    data_dir=${data}fit_a_line/
    link_data ${data_dir}
    sed -i 's/9393/8869/g' test_client.py
    ${py_version} -m paddle_serving_server.serve --model uci_housing_model --port 8869 > ${dir}server_log.txt 2>&1 &
    check_result server 5
    ${py_version} test_client.py uci_housing_client/serving_client_conf.prototxt > ${dir}client_log.txt 2>&1
    check_result client "fit_a_line_CPU_RPC server test completed"
    kill_server_process
}

function faster_rcnn_model_rpc() {
    dir=${log_dir}rpc_model/faster_rcnn_rpc/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/detection/faster_rcnn_r50_fpn_1x_coco
    data_dir=${data}detection/faster_rcnn_r50_fpn_1x_coco/
    link_data ${data_dir}
    sed -i 's/9494/8870/g' test_client.py
    ${py_version} -m paddle_serving_server.serve --model serving_server --port 8870 --gpu_ids 1 --thread 8 > ${dir}server_log.txt 2>&1 &
    echo "faster rcnn running ..."
    nvidia-smi
    check_result server 10
    check_gpu_memory 1
    ${py_version} test_client.py 000000570688.jpg > ${dir}client_log.txt 2>&1
    nvidia-smi
    check_result client "faster_rcnn_GPU_RPC server test completed"
    kill_server_process
}

function cascade_rcnn_rpc() {
    dir=${log_dir}rpc_model/cascade_rcnn_rpc/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/cascade_rcnn
    data_dir=${data}cascade_rcnn/
    link_data ${data_dir}
    sed -i "s/9292/8879/g" test_client.py
    ${py_version} -m paddle_serving_server.serve --model serving_server --port 8879 --gpu_ids 0 --thread 2 > ${dir}server_log.txt 2>&1 &
    check_result server 8
    check_gpu_memory 0
    nvidia-smi
    ${py_version} test_client.py > ${dir}client_log.txt 2>&1
    nvidia-smi
    check_result client "cascade_rcnn_GPU_RPC server test completed"
    kill_server_process
}

function deeplabv3_rpc() {
    dir=${log_dir}rpc_model/deeplabv3_rpc/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/deeplabv3
    data_dir=${data}deeplabv3/
    link_data ${data_dir}
    sed -i "s/9494/8880/g" deeplabv3_client.py
    ${py_version} -m paddle_serving_server.serve --model deeplabv3_server --gpu_ids 0 --port 8880 --thread 4 > ${dir}server_log.txt 2>&1 &
    check_result server 10
    check_gpu_memory 0
    nvidia-smi
    ${py_version} deeplabv3_client.py > ${dir}client_log.txt 2>&1
    nvidia-smi
    check_result client "deeplabv3_GPU_RPC server test completed"
    kill_server_process
}

function mobilenet_rpc() {
    dir=${log_dir}rpc_model/mobilenet_rpc/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/mobilenet
    ${py_version} -m paddle_serving_app.package --get_model mobilenet_v2_imagenet >/dev/null 2>&1
    tar xf mobilenet_v2_imagenet.tar.gz
    sed -i "s/9393/8881/g" mobilenet_tutorial.py
    ${py_version} -m paddle_serving_server.serve --model mobilenet_v2_imagenet_model --gpu_ids 0 --port 8881 > ${dir}server_log.txt 2>&1 &
    check_result server 8
    check_gpu_memory 0
    nvidia-smi
    ${py_version} mobilenet_tutorial.py > ${dir}client_log.txt 2>&1
    nvidia-smi
    check_result client "mobilenet_GPU_RPC server test completed"
    kill_server_process
}

function unet_rpc() {
    dir=${log_dir}rpc_model/unet_rpc/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/unet_for_image_seg
    data_dir=${data}unet_for_image_seg/
    link_data ${data_dir}
    sed -i "s/9494/8882/g" seg_client.py
    ${py_version} -m paddle_serving_server.serve --model unet_model --gpu_ids 1 --port 8882 > ${dir}server_log.txt 2>&1 &
    check_result server 8
    check_gpu_memory 1
    nvidia-smi
    ${py_version} seg_client.py > ${dir}client_log.txt 2>&1
    nvidia-smi
    check_result client "unet_GPU_RPC server test completed"
    kill_server_process
}

function resnetv2_rpc() {
    dir=${log_dir}rpc_model/resnetv2_rpc/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/resnet_v2_50
    data_dir=${data}resnet_v2_50/
    link_data ${data_dir}
    sed -i 's/9393/8883/g' resnet50_v2_tutorial.py
    ${py_version} -m paddle_serving_server.serve --model resnet_v2_50_imagenet_model --gpu_ids 0 --port 8883 > ${dir}server_log.txt 2>&1 &
    check_result server 10
    check_gpu_memory 0
    nvidia-smi
    ${py_version} resnet50_v2_tutorial.py > ${dir}client_log.txt 2>&1
    nvidia-smi
    check_result client "resnetv2_GPU_RPC server test completed"
    kill_server_process
}

function ocr_rpc() {
    dir=${log_dir}rpc_model/ocr_rpc/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/ocr
    data_dir=${data}ocr/
    link_data ${data_dir}
    ${py_version} -m paddle_serving_app.package --get_model ocr_rec >/dev/null 2>&1
    tar xf ocr_rec.tar.gz
    sed -i 's/9292/8884/g' test_ocr_rec_client.py
    ${py_version} -m paddle_serving_server.serve --model ocr_rec_model --port 8884 > ${dir}server_log.txt 2>&1 &
    check_result server 5
    ${py_version} test_ocr_rec_client.py > ${dir}client_log.txt 2>&1
    check_result client "ocr_CPU_RPC server test completed"
    kill_server_process
}

function criteo_ctr_rpc_cpu() {
    dir=${log_dir}rpc_model/criteo_ctr_rpc_cpu/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/criteo_ctr
    data_dir=${data}criteo_ctr/
    link_data ${data_dir}
    sed -i "s/9292/8885/g" test_client.py
    ${py_version} -m paddle_serving_server.serve --model ctr_serving_model/ --port 8885 > ${dir}server_log.txt 2>&1 &
    check_result server 5
    ${py_version} test_client.py ctr_client_conf/serving_client_conf.prototxt raw_data/part-0 > ${dir}client_log.txt 2>&1
    check_result client "criteo_ctr_CPU_RPC server test completed"
    kill_server_process
}

function criteo_ctr_rpc_gpu() {
    dir=${log_dir}rpc_model/criteo_ctr_rpc_gpu/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/criteo_ctr
    data_dir=${data}criteo_ctr/
    link_data ${data_dir}
    sed -i "s/8885/8886/g" test_client.py
    ${py_version} -m paddle_serving_server.serve --model ctr_serving_model/ --port 8886 --gpu_ids 1 > ${dir}server_log.txt 2>&1 &
    check_result server 8
    check_gpu_memory 1
    nvidia-smi
    ${py_version} test_client.py ctr_client_conf/serving_client_conf.prototxt raw_data/part-0 > ${dir}client_log.txt 2>&1
    nvidia-smi
    check_result client "criteo_ctr_GPU_RPC server test completed"
    kill_server_process
}

function yolov4_rpc_gpu() {
    dir=${log_dir}rpc_model/yolov4_rpc_gpu/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/yolov4
    data_dir=${data}yolov4/
    link_data ${data_dir}
    sed -i "s/9393/8887/g" test_client.py
    ${py_version} -m paddle_serving_server.serve --model yolov4_model --port 8887 --gpu_ids 0 > ${dir}server_log.txt 2>&1 &
    nvidia-smi
    check_result server 8
    check_gpu_memory 0
    ${py_version} test_client.py 000000570688.jpg > ${dir}client_log.txt 2>&1
    nvidia-smi
    check_result client "yolov4_GPU_RPC server test completed"
    kill_server_process
}

function senta_rpc_cpu() {
    dir=${log_dir}rpc_model/senta_rpc_cpu/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/senta
    data_dir=${data}senta/
    link_data ${data_dir}
    sed -i "s/9393/8887/g" test_client.py
    ${py_version} -m paddle_serving_server.serve --model yolov4_model --port 8887 --gpu_ids 0 > ${dir}server_log.txt 2>&1 &
    nvidia-smi
    check_result server 8
    check_gpu_memory 0
    ${py_version} test_client.py 000000570688.jpg > ${dir}client_log.txt 2>&1
    nvidia-smi
    check_result client "senta_GPU_RPC server test completed"
    kill_server_process
}


function fit_a_line_http() {
    dir=${log_dir}http_model/fit_a_line_http/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/fit_a_line
    ${py_version} -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9393 > ${dir}server_log.txt 2>&1 &
    check_result server 10
    ${py_version} test_httpclient.py uci_housing_client/serving_client_conf.prototxt > ${dir}client_log.txt 2>&1
    check_result client "fit_a_line_CPU_HTTP server test completed"
    kill_server_process
}

function lac_http() {
    dir=${log_dir}http_model/lac_http/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/lac
    ${py_version} -m paddle_serving_server.serve --model lac_model/ --port 9292 > ${dir}server_log.txt 2>&1 &
    check_result server 10
    echo "我爱北京天安门" | ${py_version} lac_http_client.py lac_client/serving_client_conf.prototxt > ${dir}client_log.txt 2>&1
    check_result client "lac_CPU_HTTP server test completed"
    kill_server_process
}

function imdb_http_proto() {
    dir=${log_dir}http_model/imdb_http_proto/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/imdb
    ${py_version} -m paddle_serving_server.serve --model imdb_cnn_model/ --port 9292 > ${dir}server_log.txt 2>&1 &
    check_result server 10
    head test_data/part-0 | ${py_version} test_http_client.py imdb_cnn_client_conf/serving_client_conf.prototxt imdb.vocab > ${dir}client_log.txt 2>&1
    check_result client "imdb_CPU_HTTP-proto server test completed"
    kill_server_process
}

function imdb_http_json() {
    dir=${log_dir}http_model/imdb_http_json/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/imdb
    ${py_version} -m paddle_serving_server.serve --model imdb_cnn_model/ --port 9292 > ${dir}server_log.txt 2>&1 &
    check_result server 10
    sed -i "s/#client.set_http_proto(True)/client.set_http_proto(False)/g" test_http_client.py
    head test_data/part-0 | ${py_version} test_http_client.py imdb_cnn_client_conf/serving_client_conf.prototxt imdb.vocab > ${dir}client_log.txt 2>&1
    check_result client "imdb_CPU_HTTP-json server test completed"
    kill_server_process
}

function imdb_grpc() {
    dir=${log_dir}http_model/imdb_grpc/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/imdb
    ${py_version} -m paddle_serving_server.serve --model imdb_cnn_model/ --port 9292 --gpu_ids 1 > ${dir}server_log.txt 2>&1 &
    check_result server 10
    check_gpu_memory 1
    sed -i "s/client.set_http_proto(False)/#client.set_http_proto(False)/g" test_http_client.py
    sed -i "s/#client.set_use_grpc_client(True)/client.set_use_grpc_client(True)/g" test_http_client.py
    head test_data/part-0 | ${py_version} test_http_client.py imdb_cnn_client_conf/serving_client_conf.prototxt imdb.vocab > ${dir}client_log.txt 2>&1
    check_result client "imdb_GPU_GRPC server test completed"
    kill_server_process
}

function ResNet50_http() {
    dir=${log_dir}http_model/ResNet50_http/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/imagenet
    ${py_version} -m paddle_serving_server.serve --model ResNet50_vd_model --port 9696 --gpu_ids 0 > ${dir}server_log.txt 2>&1 &
    check_result server 10
    check_gpu_memory 0
    ${py_version} resnet50_http_client.py ResNet50_vd_client_config/serving_client_conf.prototxt > ${dir}client_log.txt 2>&1
    check_result client "ResNet50_GPU_HTTP server test completed"
    kill_server_process
}

function bert_http() {
    dir=${log_dir}http_model/bert_http/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/bert
    cp data-c.txt.1 data-c.txt
    cp vocab.txt.1 vocab.txt
    export CUDA_VISIBLE_DEVICES=0,1
    ${py_version} -m paddle_serving_server.serve --model bert_seq128_model/ --port 9292 --gpu_ids 0 > ${dir}server_log.txt 2>&1 &
    check_result server 10
    check_gpu_memory 0
    head data-c.txt | ${py_version} bert_httpclient.py --model bert_seq128_client/serving_client_conf.prototxt > ${dir}client_log.txt 2>&1
    check_result client "bert_GPU_HTTP server test completed"
    kill_server_process
}

function grpc_fit_a_line() {
    echo "pass"
#    dir=${log_dir}rpc_model/grpc_fit_a_line/
#    check_dir ${dir}
#    unsetproxy
#    cd ${build_path}/python/examples/grpc_impl_example/fit_a_line
#    data_dir=${data}fit_a_line/
#    link_data ${data_dir}
#    ${py_version} test_server.py uci_housing_model/ > ${dir}server_log.txt 2>&1 &
#    check_result server 5
#    echo "sync predict" > ${dir}client_log.txt 2>&1
#    ${py_version} test_sync_client.py >> ${dir}client_log.txt 2>&1
#    check_result client "grpc_impl_example_fit_a_line_sync_CPU_gRPC server sync test completed"
#    echo "async predict" >> ${dir}client_log.txt 2>&1
#    ${py_version} test_asyn_client.py >> ${dir}client_log.txt 2>&1
#    check_result client "grpc_impl_example_fit_a_line_asyn_CPU_gRPC server asyn test completed"
#    echo "batch predict" >> ${dir}client_log.txt 2>&1
#    ${py_version} test_batch_client.py >> ${dir}client_log.txt 2>&1
#    check_result client "grpc_impl_example_fit_a_line_batch_CPU_gRPC server batch test completed"
#    echo "timeout predict" >> ${dir}client_log.txt 2>&1
#    ${py_version} test_timeout_client.py >> ${dir}client_log.txt 2>&1
#    check_result client "grpc_impl_example_fit_a_line_timeout_CPU_gRPC server timeout test completed"
#    kill_server_process
}

function grpc_yolov4() {
    echo "pass"
#    dir=${log_dir}rpc_model/grpc_yolov4/
#    cd ${build_path}/python/examples/grpc_impl_example/yolov4
#    check_dir ${dir}
#    data_dir=${data}yolov4/
#    link_data ${data_dir}
#    echo -e "${GREEN_COLOR}grpc_impl_example_yolov4_GPU_gRPC server started${RES}"
#    ${py_version} -m paddle_serving_server.serve --model yolov4_model --port 9393 --gpu_ids 0 --use_multilang > ${dir}server_log.txt 2>&1 &
#    check_result server 15
#    check_gpu_memory 0
#    echo -e "${GREEN_COLOR}grpc_impl_example_yolov4_GPU_gRPC client started${RES}"
#    ${py_version} test_client.py 000000570688.jpg > ${dir}client_log.txt 2>&1
#    check_result client "grpc_yolov4_GPU_GRPC server test completed"
#    kill_server_process
}

function ocr_c++_service() {
    dir=${log_dir}rpc_model/ocr_c++_serving/
    cd ${build_path}/python/examples/ocr
    check_dir ${dir}
    data_dir=${data}ocr/
    link_data ${data_dir}
    cp -r ocr_det_client/ ./ocr_det_client_cp
    rm -rf ocr_det_client
    mv ocr_det_client_cp ocr_det_client
    sed -i "s/feed_type: 1/feed_type: 20/g" ocr_det_client/serving_client_conf.prototxt
    sed -i "s/shape: 3/shape: 1/g" ocr_det_client/serving_client_conf.prototxt
    sed -i '7,8d' ocr_det_client/serving_client_conf.prototxt
    echo -e "${GREEN_COLOR}OCR_C++_Service_GPU_RPC server started${RES}"
    $py_version -m paddle_serving_server.serve --model ocr_det_model ocr_rec_model --port 9293 --gpu_id 0 > ${dir}server_log.txt 2>&1 &
    check_result server 8
    check_gpu_memory 0
    echo -e "${GREEN_COLOR}OCR_C++_Service_GPU_RPC client started${RES}"
    echo "------------------first:"
    $py_version ocr_cpp_client.py ocr_det_client ocr_rec_client
    echo "------------------second:"
    $py_version ocr_cpp_client.py ocr_det_client ocr_rec_client > ${dir}client_log.txt 2>&1
    check_result client "OCR_C++_Service_GPU_RPC server test completed"
    kill_server_process
}

function ocr_c++_service_asyn() {
    dir=${log_dir}rpc_model/ocr_c++_serving/
    cd ${build_path}/python/examples/ocr
    check_dir ${dir}
    echo -e "${GREEN_COLOR}OCR_C++_Service_GPU_RPC asyn_server started${RES}"
    $py_version -m paddle_serving_server.serve --model ocr_det_model ocr_rec_model --port 9293 --gpu_id 0 --op_num 4 > ${dir}server_log.txt 2>&1 &
    check_result server 8
    check_gpu_memory 0
    echo -e "${GREEN_COLOR}OCR_C++_Service_GPU_RPC client started${RES}"
    echo "------------------first:"
    $py_version ocr_cpp_client.py ocr_det_client ocr_rec_client
    echo "------------------second:"
    $py_version ocr_cpp_client.py ocr_det_client ocr_rec_client > ${dir}client_log.txt 2>&1
    check_result client "OCR_C++_Service_GPU_RPC server test completed"
    kill_server_process
}

function build_all_whl() {
    for whl in ${build_whl_list[@]}
    do
        echo "===========${whl} begin build==========="
        $whl
        sleep 3
        echo "===========${whl} build over ==========="
    done
}

function run_rpc_models() {
    for model in ${rpc_model_list[@]}
    do
        echo "===========${model} run begin==========="
        $model
        sleep 3
        echo "===========${model} run  end ==========="
    done
}

function run_http_models() {
    for model in ${http_model_list[@]}
    do
        echo "===========${model} run begin==========="
        $model
        sleep 3
        echo "===========${model} run  end ==========="
    done
}

function end_hook() {
    cd ${build_path}
    kill_server_process
    kill `ps -ef|grep python|awk '{print $2}'`
    sleep 5
    echo "===========files==========="
    ls -hlst
    echo "=========== end ==========="
}

function main() {
    set_env $1 $2
    before_hook
    build_all_whl
    check
    run_env
    unsetproxy
    run_gpu_env
    check_dir ${log_dir}rpc_model/
    check_dir ${log_dir}http_model/
    check_dir ${log_dir}error/
    run_rpc_models
    run_http_models
    end_hook
    if [ -f ${log_dir}error_models.txt ]; then
        cat ${log_dir}error_models.txt
        echo "error occurred!"
        exit 1
    fi
}

main $@
