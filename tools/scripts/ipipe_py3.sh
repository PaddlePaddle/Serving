#!/bin/bash
echo "################################################################"
echo "#                                                              #"
echo "#                                                              #"
echo "#                                                              #"
echo "#          Paddle Serving  begin run  with python3.6.8!        #"
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

build_whl_list=(build_cpu_server build_gpu_server build_client build_app)
rpc_model_list=(grpc_fit_a_line grpc_yolov4 pipeline_imagenet bert_rpc_gpu bert_rpc_cpu ResNet50_rpc \
lac_rpc cnn_rpc bow_rpc lstm_rpc fit_a_line_rpc deeplabv3_rpc mobilenet_rpc unet_rpc resnetv2_rpc \
criteo_ctr_rpc_cpu criteo_ctr_rpc_gpu ocr_rpc yolov4_rpc_gpu faster_rcnn_hrnetv2p_w18_1x_encrypt)
http_model_list=(fit_a_line_http lac_http cnn_http bow_http lstm_http ResNet50_http bert_http\
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

function check_result() {
    if [ $? == 0 ]; then
        echo -e "${GREEN_COLOR}$1 execute normally${RES}"
        if [ $1 == "server" ]; then
            sleep $2
            tail ${dir}server_log.txt | tee -a ${log_dir}server_total.txt
        fi
        if [ $1 == "client" ]; then
            tail ${dir}client_log.txt | tee -a ${log_dir}client_total.txt
            grep -E "${error_words}" ${dir}client_log.txt > /dev/null
            if [ $? == 0 ]; then
                echo -e "${RED_COLOR}$1 error command${RES}\n" | tee -a ${log_dir}server_total.txt ${log_dir}client_total.txt
                error_log $2
            else
                echo -e "${GREEN_COLOR}$2${RES}\n" | tee -a ${log_dir}server_total.txt ${log_dir}client_total.txt
            fi
        fi
    else
        echo -e "${RED_COLOR}$1 error command${RES}\n" | tee -a ${log_dir}server_total.txt ${log_dir}client_total.txt
        tail ${dir}client_log.txt | tee -a ${log_dir}client_total.txt
        error_log $2
    fi
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
    echo "py_version: python3.6" | tee -a ${log_dir}error_models.txt
    echo "cuda_version: ${cuda_version}" | tee -a ${log_dir}error_models.txt
    echo "status: Failed" | tee -a ${log_dir}error_models.txt
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
    unsetproxy
    cd ${build_path}/python
    python3.6 -m pip install --upgrade pip
    python3.6 -m pip install requests
    python3.6 -m pip install -r requirements.txt -i https://mirror.baidu.com/pypi/simple
    python3.6 -m pip install numpy==1.16.4
    python3.6 -m pip install paddlehub -i https://mirror.baidu.com/pypi/simple
    echo "before hook configuration is successful.... "
}

function run_env() {
    setproxy
    python3.6 -m pip install --upgrade nltk==3.4
    python3.6 -m pip install --upgrade scipy==1.2.1
    python3.6 -m pip install --upgrade setuptools==41.0.0
    python3.6 -m pip install paddlehub ujson paddlepaddle==2.0.0
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
    cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python3.6m/ \
          -DPYTHON_LIBRARIES=$PYTHONROOT/lib64/libpython3.6.so \
          -DPYTHON_EXECUTABLE=$PYTHONROOT/bin/python3.6 \
          -DSERVER=ON \
          -DTENSORRT_ROOT=/usr \
          -DWITH_GPU=ON ..
    make -j32
    make -j32
    make install -j32
    python3.6 -m pip uninstall paddle-serving-server-gpu -y
    python3.6 -m pip install ${build_path}/build/python/dist/*
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
    cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python3.6m/ \
          -DPYTHON_LIBRARIES=$PYTHONROOT/lib64/libpython3.6.so \
          -DPYTHON_EXECUTABLE=$PYTHONROOT/bin/python3.6 \
          -DWITH_GPU=OFF \
          -DSERVER=ON ..
    make -j32
    make -j32
    make install -j32
    cp ${build_path}/build/python/dist/* ../
    python3.6 -m pip uninstall paddle-serving-server -y
    python3.6 -m pip install ${build_path}/build/python/dist/*
    cp -r ${build_path}/build/ ${build_path}/build_cpu
}

function build_client() {
    setproxy
    cd  ${build_path}
    if [ -d build ];then
        rm -rf build
    fi
    mkdir build && cd build
    cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python3.6m/ \
         -DPYTHON_LIBRARIES=$PYTHONROOT/lib64/libpython3.6.so \
         -DPYTHON_EXECUTABLE=$PYTHONROOT/bin/python3.6 \
         -DCLIENT=ON ..
    make -j32
    make -j32
    cp ${build_path}/build/python/dist/* ../
    python3.6 -m pip uninstall paddle-serving-client -y
    python3.6 -m pip install ${build_path}/build/python/dist/*
}

function build_app() {
    setproxy
    python3.6 -m pip install paddlehub ujson Pillow
    python3.6 -m pip install paddlepaddle==2.0.0
    cd ${build_path}
    if [ -d build ];then
        rm -rf build
    fi
    mkdir build && cd build
    cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python3.6m/ \
          -DPYTHON_LIBRARIES=$PYTHONROOT/lib/libpython3.6.so \
          -DPYTHON_EXECUTABLE=$PYTHONROOT/bin/python3.6 \
          -DCMAKE_INSTALL_PREFIX=./output -DAPP=ON ..
    make
    cp ${build_path}/build/python/dist/* ../
    python3.6 -m pip uninstall paddle-serving-app -y
    python3.6 -m pip install ${build_path}/build/python/dist/*
}

function faster_rcnn_hrnetv2p_w18_1x_encrypt() {
    dir=${log_dir}rpc_model/faster_rcnn_hrnetv2p_w18_1x/
    cd ${build_path}/python/examples/detection/faster_rcnn_hrnetv2p_w18_1x
    check_dir ${dir}
    data_dir=${data}detection/faster_rcnn_hrnetv2p_w18_1x/
    link_data ${data_dir}
    python3.6 encrypt.py
    unsetproxy
    echo -e "${GREEN_COLOR}faster_rcnn_hrnetv2p_w18_1x_ENCRYPTION_GPU_RPC server started${RES}" | tee -a ${log_dir}server_total.txt
    python3.6 -m paddle_serving_server.serve --model encrypt_server/ --port 9494 --use_trt --gpu_ids 0 --use_encryption_model > ${dir}server_log.txt 2>&1 &
    check_result server 3
    echo -e "${GREEN_COLOR}faster_rcnn_hrnetv2p_w18_1x_ENCRYPTION_GPU_RPC client started${RES}" | tee -a ${log_dir}client_total.txt
    python3.6 test_encryption.py 000000570688.jpg > ${dir}client_log.txt 2>&1
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
    python3.6 -m paddle_serving_server.serve --model bert_seq128_model/ --port 8860 --gpu_ids 0 > ${dir}server_log.txt 2>&1 &
    check_result server 15
    nvidia-smi
    head data-c.txt | python3.6 bert_client.py --model bert_seq128_client/serving_client_conf.prototxt > ${dir}client_log.txt 2>&1
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
    python3.6 -m paddle_serving_server.serve --model bert_seq128_model/ --port 8861 > ${dir}server_log.txt 2>&1 &
    check_result server 3
    cp data-c.txt.1 data-c.txt
    head data-c.txt | python3.6 bert_client.py --model bert_seq128_client/serving_client_conf.prototxt > ${dir}client_log.txt 2>&1
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
    python3.6 resnet50_web_service.py > ${dir}server_log.txt 2>&1 &
    check_result server 5
    nvidia-smi
    python3.6 pipeline_rpc_client.py > ${dir}client_log.txt 2>&1
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
    python3.6 -m paddle_serving_server.serve --model ResNet50_vd_model --port 8863 --gpu_ids 0 > ${dir}server_log.txt 2>&1 &
    check_result server 5
    nvidia-smi
    python3.6 resnet50_rpc_client.py ResNet50_vd_client_config/serving_client_conf.prototxt > ${dir}client_log.txt 2>&1
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
    python3.6 -m paddle_serving_server.serve --model ResNet101_vd_model --port 8864 --gpu_ids 0 > ${dir}server_log.txt 2>&1 &
    check_result server 5
    nvidia-smi
    python3.6 image_rpc_client.py ResNet101_vd_client_config/serving_client_conf.prototxt > ${dir}client_log.txt 2>&1
    check_result client "ResNet101_GPU_RPC server test completed"
    nvidia-smi
    kill_server_process
}

function cnn_rpc() {
    dir=${log_dir}rpc_model/cnn_rpc/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/imdb
    data_dir=${data}imdb/
    link_data ${data_dir}
    sed -i 's/9292/8865/g' test_client.py
    python3.6 -m paddle_serving_server.serve --model imdb_cnn_model/ --port 8865 > ${dir}server_log.txt 2>&1 &
    check_result server 5
    head test_data/part-0 | python3.6 test_client.py imdb_cnn_client_conf/serving_client_conf.prototxt imdb.vocab > ${dir}client_log.txt 2>&1
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
    python3.6 -m paddle_serving_server.serve --model imdb_bow_model/ --port 8866 > ${dir}server_log.txt 2>&1 &
    check_result server 5
    head test_data/part-0 | python3.6 test_client.py imdb_bow_client_conf/serving_client_conf.prototxt imdb.vocab > ${dir}client_log.txt 2>&1
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
    python3.6 -m paddle_serving_server.serve --model imdb_lstm_model/ --port 8867 > ${dir}server_log.txt 2>&1 &
    check_result server 5
    head test_data/part-0 | python3.6 test_client.py imdb_lstm_client_conf/serving_client_conf.prototxt imdb.vocab > ${dir}client_log.txt 2>&1
    check_result client "lstm_CPU_RPC server test completed"
    kill_server_process
}

function lac_rpc() {
    dir=${log_dir}rpc_model/lac_rpc/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/lac
    data_dir=${data}lac/
    link_data ${data_dir}
    sed -i 's/9292/8868/g' lac_client.py
    python3.6 -m paddle_serving_server.serve --model lac_model/ --port 8868 > ${dir}server_log.txt 2>&1 &
    check_result server 5
    echo "我爱北京天安门" | python3.6 lac_client.py lac_client/serving_client_conf.prototxt lac_dict/ > ${dir}client_log.txt 2>&1
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
    python3.6 -m paddle_serving_server.serve --model uci_housing_model --port 8869 > ${dir}server_log.txt 2>&1 &
    check_result server 5
    python3.6 test_client.py uci_housing_client/serving_client_conf.prototxt > ${dir}client_log.txt 2>&1
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
    python3.6 -m paddle_serving_server.serve --model serving_server --port 8870 --gpu_ids 0 --thread 2 --use_trt > ${dir}server_log.txt 2>&1 &
    echo "faster rcnn running ..."
    nvidia-smi
    check_result server 10
    python3.6 test_client.py 000000570688.jpg > ${dir}client_log.txt 2>&1
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
    python3.6 -m paddle_serving_server.serve --model serving_server --port 8879 --gpu_ids 0 --thread 2 > ${dir}server_log.txt 2>&1 &
    check_result server 5
    nvidia-smi
    python3.6 test_client.py > ${dir}client_log.txt 2>&1
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
    python3.6 -m paddle_serving_server.serve --model deeplabv3_server --gpu_ids 0 --port 8880 --thread 2 > ${dir}server_log.txt 2>&1 &
    check_result server 5
    nvidia-smi
    python3.6 deeplabv3_client.py > ${dir}client_log.txt 2>&1
    nvidia-smi
    check_result client "deeplabv3_GPU_RPC server test completed"
    kill_server_process
}

function mobilenet_rpc() {
    dir=${log_dir}rpc_model/mobilenet_rpc/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/mobilenet
    python3.6 -m paddle_serving_app.package --get_model mobilenet_v2_imagenet >/dev/null 2>&1
    tar xf mobilenet_v2_imagenet.tar.gz
    sed -i "s/9393/8881/g" mobilenet_tutorial.py
    python3.6 -m paddle_serving_server.serve --model mobilenet_v2_imagenet_model --gpu_ids 0 --port 8881 > ${dir}server_log.txt 2>&1 &
    check_result server 5
    nvidia-smi
    python3.6 mobilenet_tutorial.py > ${dir}client_log.txt 2>&1
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
    python3.6 -m paddle_serving_server.serve --model unet_model --gpu_ids 0 --port 8882 > ${dir}server_log.txt 2>&1 &
    check_result server 5
    nvidia-smi
    python3.6 seg_client.py > ${dir}client_log.txt 2>&1
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
    python3.6 -m paddle_serving_server.serve --model resnet_v2_50_imagenet_model --gpu_ids 0 --port 8883 > ${dir}server_log.txt 2>&1 &
    check_result server 10
    nvidia-smi
    python3.6 resnet50_v2_tutorial.py > ${dir}client_log.txt 2>&1
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
    python3.6 -m paddle_serving_app.package --get_model ocr_rec >/dev/null 2>&1
    tar xf ocr_rec.tar.gz
    sed -i 's/9292/8884/g' test_ocr_rec_client.py
    python3.6 -m paddle_serving_server.serve --model ocr_rec_model --port 8884 > ${dir}server_log.txt 2>&1 &
    check_result server 5
    python3.6 test_ocr_rec_client.py > ${dir}client_log.txt 2>&1
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
    python3.6 -m paddle_serving_server.serve --model ctr_serving_model/ --port 8885 > ${dir}server_log.txt 2>&1 &
    check_result server 5
    python3.6 test_client.py ctr_client_conf/serving_client_conf.prototxt raw_data/part-0 > ${dir}client_log.txt 2>&1
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
    python3.6 -m paddle_serving_server.serve --model ctr_serving_model/ --port 8886 --gpu_ids 0 > ${dir}server_log.txt 2>&1 &
    check_result server 5
    nvidia-smi
    python3.6 test_client.py ctr_client_conf/serving_client_conf.prototxt raw_data/part-0 > ${dir}client_log.txt 2>&1
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
    python3.6 -m paddle_serving_server.serve --model yolov4_model --port 8887 --gpu_ids 0 > ${dir}server_log.txt 2>&1 &
    nvidia-smi
    check_result server 5
    python3.6 test_client.py 000000570688.jpg > ${dir}client_log.txt 2>&1
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
    python3.6 -m paddle_serving_server.serve --model yolov4_model --port 8887 --gpu_ids 0 > ${dir}server_log.txt 2>&1 &
    nvidia-smi
    check_result server 5
    python3.6 test_client.py 000000570688.jpg > ${dir}client_log.txt 2>&1
    nvidia-smi
    check_result client "senta_GPU_RPC server test completed"
    kill_server_process
}


function fit_a_line_http() {
    dir=${log_dir}http_model/fit_a_line_http/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/fit_a_line
    sed -i "s/9393/8871/g" test_server.py
    python3.6 test_server.py > ${dir}server_log.txt 2>&1 &
    check_result server 10
    curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"x": [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332]}], "fetch":["price"]}' http://127.0.0.1:8871/uci/prediction > ${dir}client_log.txt 2>&1
    check_result client "fit_a_line_CPU_HTTP server test completed"
    kill_server_process
}

function lac_http() {
    dir=${log_dir}http_model/lac_http/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/lac
    python3.6 lac_web_service.py lac_model/ lac_workdir 8872 > ${dir}server_log.txt 2>&1 &
    check_result server 10
    curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"words": "我爱北京天安门"}], "fetch":["word_seg"]}' http://127.0.0.1:8872/lac/prediction > ${dir}client_log.txt 2>&1
    check_result client "lac_CPU_HTTP server test completed"
    kill_server_process
}

function cnn_http() {
    dir=${log_dir}http_model/cnn_http/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/imdb
    python3.6 text_classify_service.py imdb_cnn_model/ workdir/ 8873 imdb.vocab > ${dir}server_log.txt 2>&1 &
    check_result server 10
    curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"words": "i am very sad | 0"}], "fetch":["prediction"]}' http://127.0.0.1:8873/imdb/prediction > ${dir}client_log.txt 2>&1
    check_result client "cnn_CPU_HTTP server test completed"
    kill_server_process
}

function bow_http() {
    dir=${log_dir}http_model/bow_http/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/imdb
    python3.6 text_classify_service.py imdb_bow_model/ workdir/ 8874 imdb.vocab > ${dir}server_log.txt 2>&1 &
    check_result server 10
    curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"words": "i am very sad | 0"}], "fetch":["prediction"]}' http://127.0.0.1:8874/imdb/prediction > ${dir}client_log.txt 2>&1
    check_result client "bow_CPU_HTTP server test completed"
    kill_server_process
}

function lstm_http() {
    dir=${log_dir}http_model/lstm_http/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/imdb
    python3.6 text_classify_service.py imdb_bow_model/ workdir/ 8875 imdb.vocab > ${dir}server_log.txt 2>&1 &
    check_result server 10
    curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"words": "i am very sad | 0"}], "fetch":["prediction"]}' http://127.0.0.1:8875/imdb/prediction > ${dir}client_log.txt 2>&1
    check_result client "lstm_CPU_HTTP server test completed"
    kill_server_process
}

function ResNet50_http() {
    dir=${log_dir}http_model/ResNet50_http/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/imagenet
    python3.6 resnet50_web_service.py ResNet50_vd_model gpu 8876 > ${dir}server_log.txt 2>&1 &
    check_result server 10
    curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"image": "https://paddle-serving.bj.bcebos.com/imagenet-example/daisy.jpg"}], "fetch": ["score"]}' http://127.0.0.1:8876/image/prediction > ${dir}client_log.txt 2>&1
    check_result client "ResNet50_GPU_HTTP server test completed"
    kill_server_process
}

function bert_http() {
    dir=${log_dir}http_model/ResNet50_http/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/bert
    cp data-c.txt.1 data-c.txt
    cp vocab.txt.1 vocab.txt
    export CUDA_VISIBLE_DEVICES=0
    python3.6 bert_web_service.py bert_seq128_model/ 8878 > ${dir}server_log.txt 2>&1 &
    check_result server 5
    curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"words": "hello"}], "fetch":["pooled_output"]}' http://127.0.0.1:8878/bert/prediction > ${dir}client_log.txt 2>&1
    check_result client "bert_GPU_HTTP server test completed"
    kill_server_process
}

function grpc_fit_a_line() {
    dir=${log_dir}rpc_model/grpc_fit_a_line/
    check_dir ${dir}
    unsetproxy
    cd ${build_path}/python/examples/grpc_impl_example/fit_a_line
    data_dir=${data}fit_a_line/
    link_data ${data_dir}
    python3.6 test_server.py uci_housing_model/ > ${dir}server_log.txt 2>&1 &
    check_result server 5
    echo "sync predict" > ${dir}client_log.txt 2>&1
    python3.6 test_sync_client.py >> ${dir}client_log.txt 2>&1
    check_result client "grpc_impl_example_fit_a_line_sync_CPU_gRPC server sync test completed"
    echo "async predict" >> ${dir}client_log.txt 2>&1
    python3.6 test_asyn_client.py >> ${dir}client_log.txt 2>&1
    check_result client "grpc_impl_example_fit_a_line_asyn_CPU_gRPC server asyn test completed"
    echo "batch predict" >> ${dir}client_log.txt 2>&1
    python3.6 test_batch_client.py >> ${dir}client_log.txt 2>&1
    check_result client "grpc_impl_example_fit_a_line_batch_CPU_gRPC server batch test completed"
    echo "timeout predict" >> ${dir}client_log.txt 2>&1
    python3.6 test_timeout_client.py >> ${dir}client_log.txt 2>&1
    check_result client "grpc_impl_example_fit_a_line_timeout_CPU_gRPC server timeout test completed"
    kill_server_process
}

function grpc_yolov4() {
    dir=${log_dir}rpc_model/grpc_yolov4/
    cd ${build_path}/python/examples/grpc_impl_example/yolov4
    check_dir ${dir}
    data_dir=${data}yolov4/
    link_data ${data_dir}
    echo -e "${GREEN_COLOR}grpc_impl_example_yolov4_GPU_gRPC server started${RES}"
    python3.6 -m paddle_serving_server.serve --model yolov4_model --port 9393 --gpu_ids 0 --use_multilang > ${dir}server_log.txt 2>&1 &
    check_result server 5
    echo -e "${GREEN_COLOR}grpc_impl_example_yolov4_GPU_gRPC client started${RES}"
    python3.6 test_client.py 000000570688.jpg > ${dir}client_log.txt 2>&1
    check_result client "grpc_yolov4_GPU_GRPC server test completed"
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

main$@
