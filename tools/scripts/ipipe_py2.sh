#!/bin/bash
echo "################################################################"
echo "#                                                              #"
echo "#                                                              #"
echo "#                                                              #"
echo "#          Paddle Serving  begin run with python2.7.15!!        #"
echo "#                                                              #"
echo "#                                                              #"
echo "#                                                              #"
echo "################################################################"

export GOPATH=$HOME/go
export PATH=$PATH:$GOROOT/bin:$GOPATH/bin
export CUDA_INCLUDE_DIRS=/usr/local/cuda-10.2/include
export PYTHONROOT=/usr/local/python2.7.15/

go env -w GO111MODULE=on
go env -w GOPROXY=https://goproxy.cn,direct
go get -u github.com/grpc-ecosystem/grpc-gateway/protoc-gen-grpc-gateway@v1.15.2
go get -u github.com/grpc-ecosystem/grpc-gateway/protoc-gen-swagger@v1.15.2
go get -u github.com/golang/protobuf/protoc-gen-go@v1.4.3
go get -u google.golang.org/grpc@v1.33.0

build_path=/workspace/Serving
build_whl_list=(build_gpu_server build_client build_cpu_server build_app)
rpc_model_list=(grpc_impl pipeline_imagenet bert_rpc_gpu bert_rpc_cpu faster_rcnn_model_rpc ResNet50_rpc lac_rpc \
cnn_rpc bow_rpc lstm_rpc fit_a_line_rpc cascade_rcnn_rpc deeplabv3_rpc mobilenet_rpc unet_rpc resnetv2_rpc \
ocr_rpc criteo_ctr_rpc_cpu criteo_ctr_rpc_gpu yolov4_rpc_gpu)
http_model_list=(fit_a_line_http lac_http cnn_http bow_http lstm_http ResNet50_http bert_http)

function setproxy(){
  export http_proxy=${proxy}
  export https_proxy=${proxy}
}

function unsetproxy(){
  unset http_proxy
  unset https_proxy
}

function kill_server_process(){
  kill `ps -ef|grep serving|awk '{print $2}'`
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
      echo "paddle serving Build Passed"
    fi
}

function check_result() {
    if [ $? -ne 0 ];then
      echo -e "\033[4;31;42m$1 model runs failed, please check your pull request or modify test case! \033[0m"
      exit 1
    else
      echo -e "\033[4;37;42m$1 model runs successfully, congratulations! \033[0m"
    fi
}

function before_hook(){
  setproxy
  cd ${build_path}/python
  pip2.7 install --upgrade pip
  pip2.7 install opencv-python==4.2.0.32 requests
  pip2.7 install -r requirements.txt
  echo "before hook configuration is successful.... "
}

function run_env(){
  setproxy
  pip2.7 install --upgrade nltk==3.4
  pip2.7 install --upgrade scipy==1.2.1
  pip2.7 install --upgrade setuptools
  pip2.7 install paddlehub ujson paddlepaddle==2.0.0
  echo "run env configuration is successful.... "
}

function run_gpu_env(){
  cd ${build_path}
  export LD_LIBRARY_PATH=/usr/local/python2.7.15/lib/python2.7/site-packages/paddle/libs/:$LD_LIBRARY_PATH
  export LD_LIBRARY_PATH=/workspace/Serving/build_gpu/third_party/install/Paddle/lib/:/workspace/Serving/build_gpu/third_party/Paddle/src/extern_paddle/third_party/install/mklml/lib/:/workspace/Serving/build_gpu/third_party/Paddle/src/extern_paddle/third_party/install/mkldnn/lib/:$LD_LIBRARY_PATH
  export SERVING_BIN=${build_path}/build_gpu/core/general-server/serving
  echo "run gpu env configuration is successful.... "
}

function run_cpu_env(){
  cd ${build_path}
  export LD_LIBRARY_PATH=/usr/local/python2.7.15/lib/python2.7/site-packages/paddle/libs/:$LD_LIBRARY_PATH
  export LD_LIBRARY_PATH=/workspace/Serving/build_cpu/third_party/install/Paddle/lib/:$LD_LIBRARY_PATH
  export SERVING_BIN=${build_path}/build_cpu/core/general-server/serving
  echo "run cpu env configuration is successful.... "
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
    cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python2.7/ \
          -DPYTHON_LIBRARIES=$PYTHONROOT/lib64/libpython2.7.so \
          -DPYTHON_EXECUTABLE=$PYTHONROOT/bin/python \
          -DSERVER=ON \
          -DTENSORRT_ROOT=/usr \
          -DWITH_GPU=ON ..
    make -j18
    make -j18
    make install -j18
    pip2.7 uninstall paddle-serving-server-gpu -y
    pip2.7 install ${build_path}/build/python/dist/*
    cp  ${build_path}/build/python/dist/* ../
    cp -r ${build_path}/build/ ${build_path}/build_gpu
}

function build_client() {
     setproxy
     cd  ${build_path}
     if [ -d build ];then
          rm -rf build
     fi
     mkdir build && cd build
     cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python2.7/ \
           -DPYTHON_LIBRARIES=$PYTHONROOT/lib64/libpython2.7.so \
           -DPYTHON_EXECUTABLE=$PYTHONROOT/bin/python \
           -DCLIENT=ON ..
     make -j18
     make -j18
     cp ${build_path}/build/python/dist/* ../
     pip2.7 uninstall paddle-serving-client -y
     pip2.7 install ${build_path}/build/python/dist/*
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
      pwd
      cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python2.7/ \
            -DPYTHON_LIBRARIES=$PYTHONROOT/lib64/libpython2.7.so \
            -DPYTHON_EXECUTABLE=$PYTHONROOT/bin/python \
            -DWITH_GPU=OFF \
            -DSERVER=ON ..
      make -j18
      make -j18
      make install -j18
      cp ${build_path}/build/python/dist/* ../
      pip2.7 uninstall paddle-serving-server -y
      pip2.7 install ${build_path}/build/python/dist/*
      cp -r ${build_path}/build/ ${build_path}/build_cpu
}

function build_app() {
  setproxy
  cd ${build_path}
  if [ -d build ];then
      rm -rf build
  fi
  mkdir build && cd build
  cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python2.7/ \
        -DPYTHON_LIBRARIES=$PYTHONROOT/lib/libpython2.7.so \
        -DPYTHON_EXECUTABLE=$PYTHONROOT/bin/python \
        -DCMAKE_INSTALL_PREFIX=./output -DAPP=ON ..
  make
  cp ${build_path}/build/python/dist/* ../
  pip2.7 uninstall paddle-serving-app -y
  pip2.7 install ${build_path}/build/python/dist/*
}

function bert_rpc_gpu(){
  run_gpu_env
  setproxy
  cd ${build_path}/python/examples/bert
  sh get_data.sh >/dev/null 2>&1
  sed -i 's/9292/8860/g' bert_client.py
  sed -i '$aprint(result)' bert_client.py
  cp -r /root/.cache/dist_data/serving/bert/bert_seq128_* ./
  ls -hlst
  python2.7 -m paddle_serving_server_gpu.serve --model bert_seq128_model/ --port 8860 --gpu_ids 0 > bert_rpc_gpu 2>&1 &
  sleep 15
  head data-c.txt | python2.7 bert_client.py --model bert_seq128_client/serving_client_conf.prototxt
  cat bert_rpc_gpu
  check_result $FUNCNAME
  kill_server_process
}

function bert_rpc_cpu(){
  run_cpu_env
  setproxy
  cd ${build_path}/python/examples/bert
  sed -i 's/8860/8861/g' bert_client.py
  python2.7 -m paddle_serving_server.serve --model bert_seq128_model/ --port 8861 > bert_rpc_cpu 2>&1 &
  sleep 3
  cp data-c.txt.1 data-c.txt
  head data-c.txt | python2.7 bert_client.py --model bert_seq128_client/serving_client_conf.prototxt
  cat bert_rpc_cpu
  check_result $FUNCNAME
  kill_server_process
}

function criteo_ctr_with_cube_rpc(){
  setproxy
  run_cpu_env
  cd ${build_path}/python/examples/criteo_ctr_with_cube
  ln -s /root/.cache/dist_data/serving/criteo_ctr_with_cube/raw_data ./
  sed -i "s/9292/8888/g" test_server.py
  sed -i "s/9292/8888/g" test_client.py
  wget https://paddle-serving.bj.bcebos.com/unittest/ctr_cube_unittest.tar.gz >/dev/null 2>&1
  tar xf ctr_cube_unittest.tar.gz
  mv models/ctr_client_conf ./
  mv models/ctr_serving_model_kv ./
  mv models/data ./cube/
  wget https://paddle-serving.bj.bcebos.com/others/cube_app.tar.gz >/dev/null 2>&1
  tar xf cube_app.tar.gz
  mv cube_app/cube* ./cube/
  sh cube_prepare.sh > haha 2>&1 &
  sleep 5
  python2.7 test_server.py ctr_serving_model_kv > criteo_ctr_rpc 2>&1 &
  sleep 5
  python2.7 test_client.py ctr_client_conf/serving_client_conf.prototxt ./raw_data
  cat criteo_ctr_rpc
  check_result $FUNCNAME
  kill `ps -ef|grep cube|awk '{print $2}'`
  kill_server_process
}

function pipeline_imagenet(){
  run_gpu_env
  setproxy
  cd ${build_path}/python/examples/pipeline/imagenet
  cp -r /root/.cache/dist_data/serving/imagenet/* ./
  ls -a
  python2.7 resnet50_web_service.py > pipelog 2>&1 &
  sleep 5
  python2.7 pipeline_rpc_client.py
  # check_result $FUNCNAME
  kill_server_process
}

function ResNet50_rpc(){
  run_gpu_env
  setproxy
  cd ${build_path}/python/examples/imagenet
  cp -r /root/.cache/dist_data/serving/imagenet/* ./
  sed -i 's/9696/8863/g' resnet50_rpc_client.py
  python2.7 -m paddle_serving_server_gpu.serve --model ResNet50_vd_model --port 8863 --gpu_ids 0 > ResNet50_rpc 2>&1 &
  sleep 5
  python2.7 resnet50_rpc_client.py ResNet50_vd_client_config/serving_client_conf.prototxt
  tail ResNet50_rpc
  check_result $FUNCNAME
  kill_server_process
  sleep 5
}

function ResNet101_rpc(){
  run_gpu_env
  setproxy
  cd ${build_path}/python/examples/imagenet
  sed -i 's/9292/8864/g' image_rpc_client.py
  python2.7 -m paddle_serving_server_gpu.serve --model ResNet101_vd_model --port 8864 --gpu_ids 0 > ResNet101_rpc 2>&1 &
  sleep 5
  python2.7 image_rpc_client.py ResNet101_vd_client_config/serving_client_conf.prototxt
  tail ResNet101_rpc
  kill_server_process
  check_result $FUNCNAME
}

function cnn_rpc(){
  setproxy
  run_cpu_env
  cd ${build_path}/python/examples/imdb
  cp -r /root/.cache/dist_data/serving/imdb/* ./
  tar xf imdb_model.tar.gz && tar xf text_classification_data.tar.gz
  sed -i 's/9292/8865/g' test_client.py
  python2.7 -m paddle_serving_server.serve --model imdb_cnn_model/ --port 8865 > cnn_rpc 2>&1 &
  sleep 5
  head test_data/part-0 | python2.7 test_client.py imdb_cnn_client_conf/serving_client_conf.prototxt imdb.vocab
  tail cnn_rpc
  check_result $FUNCNAME
  kill_server_process
}

function bow_rpc(){
  setproxy
  run_cpu_env
  cd ${build_path}/python/examples/imdb
  sed -i 's/8865/8866/g' test_client.py
  python2.7 -m paddle_serving_server.serve --model imdb_bow_model/ --port 8866 > bow_rpc 2>&1 &
  sleep 5
  head test_data/part-0 | python2.7 test_client.py imdb_bow_client_conf/serving_client_conf.prototxt imdb.vocab
  tail bow_rpc
  check_result $FUNCNAME
  kill_server_process
  sleep 5
}

function lstm_rpc(){
  setproxy
  run_cpu_env
  cd ${build_path}/python/examples/imdb
  sed -i 's/8866/8867/g' test_client.py
  python2.7 -m paddle_serving_server.serve --model imdb_lstm_model/ --port 8867 > lstm_rpc 2>&1 &
  sleep 5
  head test_data/part-0 | python2.7 test_client.py imdb_lstm_client_conf/serving_client_conf.prototxt imdb.vocab
  tail lstm_rpc
  check_result $FUNCNAME
  kill_server_process
  kill `ps -ef|grep imdb|awk '{print $2}'`
}

function lac_rpc(){
  setproxy
  run_cpu_env
  cd ${build_path}/python/examples/lac
  python2.7 -m paddle_serving_app.package --get_model lac >/dev/null 2>&1
  tar xf lac.tar.gz
  sed -i 's/9292/8868/g' lac_client.py
  python2.7 -m paddle_serving_server.serve --model lac_model/ --port 8868 > lac_rpc 2>&1 &
  sleep 5
  echo "我爱北京天安门" | python2.7 lac_client.py lac_client/serving_client_conf.prototxt lac_dict/
  tail lac_rpc
  check_result $FUNCNAME
  kill_server_process
}

function fit_a_line_rpc(){
  setproxy
  run_cpu_env
  cd ${build_path}/python/examples/fit_a_line
  sh get_data.sh >/dev/null 2>&1
  sed -i 's/9393/8869/g' test_client.py
  python2.7 -m paddle_serving_server.serve --model uci_housing_model --port 8869 > line_rpc 2>&1 &
  sleep 5
  python2.7 test_client.py uci_housing_client/serving_client_conf.prototxt
  tail line_rpc
  check_result $FUNCNAME
  kill_server_process
}

function faster_rcnn_model_rpc(){
  run_gpu_env
  setproxy
  cd ${build_path}/python/examples/faster_rcnn_model
  cp -r /root/.cache/dist_data/serving/faster_rcnn/faster_rcnn_model.tar.gz ./
  tar xf faster_rcnn_model.tar.gz
  wget https://paddle-serving.bj.bcebos.com/pddet_demo/infer_cfg.yml >/dev/null 2>&1
  mv faster_rcnn_model/pddet* ./
  sed -i 's/9494/8870/g' test_client.py
  python2.7 -m paddle_serving_server_gpu.serve --model pddet_serving_model --port 8870 --gpu_id 0 > faster_rcnn_rpc 2>&1 &
  sleep 3
  python2.7 test_client.py pddet_client_conf/serving_client_conf.prototxt infer_cfg.yml 000000570688.jpg
  tail faster_rcnn_rpc
  check_result $FUNCNAME
  kill_server_process
}

function cascade_rcnn_rpc(){
  setproxy
  run_gpu_env
  cd ${build_path}/python/examples/cascade_rcnn
  cp -r /root/.cache/dist_data/serving/cascade_rcnn/cascade_rcnn_r50_fpx_1x_serving.tar.gz ./
  tar xf cascade_rcnn_r50_fpx_1x_serving.tar.gz
  sed -i "s/9292/8879/g" test_client.py
  python2.7 -m paddle_serving_server_gpu.serve --model serving_server --port 8879 --gpu_id 0 > rcnn_rpc 2>&1 &
  ls -hlst
  sleep 5
  python2.7 test_client.py
  tail rcnn_rpc
  check_result $FUNCNAME
  kill_server_process
}

function deeplabv3_rpc() {
  setproxy
  run_gpu_env
  cd ${build_path}/python/examples/deeplabv3
  cp -r /root/.cache/dist_data/serving/deeplabv3/deeplabv3.tar.gz ./
  tar xf deeplabv3.tar.gz
  sed -i "s/9494/8880/g" deeplabv3_client.py
  python2.7 -m paddle_serving_server_gpu.serve --model deeplabv3_server --gpu_ids 0 --port 8880 > deeplab_rpc 2>&1 &
  sleep 5
  python2.7 deeplabv3_client.py
  tail deeplab_rpc
  check_result $FUNCNAME
  kill_server_process
}

function mobilenet_rpc() {
  setproxy
  run_gpu_env
  cd ${build_path}/python/examples/mobilenet
  python2.7 -m paddle_serving_app.package --get_model mobilenet_v2_imagenet >/dev/null 2>&1
  tar xf mobilenet_v2_imagenet.tar.gz
  sed -i "s/9393/8881/g" mobilenet_tutorial.py
  python2.7 -m paddle_serving_server_gpu.serve --model mobilenet_v2_imagenet_model --gpu_ids 0 --port 8881 > mobilenet_rpc 2>&1 &
  sleep 5
  python2.7 mobilenet_tutorial.py
  tail mobilenet_rpc
  check_result $FUNCNAME
  kill_server_process
}

function unet_rpc() {
 setproxy
 run_gpu_env
 cd ${build_path}/python/examples/unet_for_image_seg
 python2.7 -m paddle_serving_app.package --get_model unet >/dev/null 2>&1
 tar xf unet.tar.gz
 sed -i "s/9494/8882/g" seg_client.py
 python2.7 -m paddle_serving_server_gpu.serve --model unet_model --gpu_ids 0 --port 8882 > unet_rpc 2>&1 &
 sleep 5
 python2.7 seg_client.py
 tail unet_rpc
 check_result $FUNCNAME
 kill_server_process
}

function resnetv2_rpc() {
  setproxy
  run_gpu_env
  cd ${build_path}/python/examples/resnet_v2_50
  cp /root/.cache/dist_data/serving/resnet_v2_50/resnet_v2_50_imagenet.tar.gz ./
  tar xf resnet_v2_50_imagenet.tar.gz
  sed -i 's/9393/8883/g' resnet50_v2_tutorial.py
  python2.7 -m paddle_serving_server_gpu.serve --model resnet_v2_50_imagenet_model --gpu_ids 0 --port 8883 > v2_log 2>&1 &
  sleep 10
  python2.7 resnet50_v2_tutorial.py
  tail v2_log
  check_result $FUNCNAME
  kill_server_process
}

function ocr_rpc() {
  setproxy
  run_cpu_env
  cd ${build_path}/python/examples/ocr
  cp -r /root/.cache/dist_data/serving/ocr/test_imgs ./
  python2.7 -m paddle_serving_app.package --get_model ocr_rec >/dev/null 2>&1
  tar xf ocr_rec.tar.gz
  sed -i 's/9292/8884/g' test_ocr_rec_client.py
  python2.7 -m paddle_serving_server.serve --model ocr_rec_model --port 8884 > ocr_rpc 2>&1 &
  sleep 5
  python2.7 test_ocr_rec_client.py
  tail ocr_rpc
  check_result $FUNCNAME
  kill_server_process
}

function criteo_ctr_rpc_cpu() {
  setproxy
  run_cpu_env
  cd ${build_path}/python/examples/criteo_ctr
  sed -i "s/9292/8885/g" test_client.py
  ln -s /root/.cache/dist_data/serving/criteo_ctr_with_cube/raw_data ./
  wget https://paddle-serving.bj.bcebos.com/criteo_ctr_example/criteo_ctr_demo_model.tar.gz >/dev/null 2>&1
  tar xf criteo_ctr_demo_model.tar.gz
  mv models/ctr_client_conf .
  mv models/ctr_serving_model .
  python2.7 -m paddle_serving_server.serve --model ctr_serving_model/ --port 8885 > criteo_ctr_cpu_rpc 2>&1 &
  sleep 5
  python2.7 test_client.py ctr_client_conf/serving_client_conf.prototxt raw_data/part-0
  tail criteo_ctr_cpu_rpc
  check_result $FUNCNAME
  kill_server_process
}

function criteo_ctr_rpc_gpu() {
  setproxy
  run_gpu_env
  cd ${build_path}/python/examples/criteo_ctr
  sed -i "s/8885/8886/g" test_client.py
  python2.7 -m paddle_serving_server_gpu.serve --model ctr_serving_model/ --port 8886 --gpu_ids 0 > criteo_ctr_gpu_rpc 2>&1 &
  sleep 5
  python2.7 test_client.py ctr_client_conf/serving_client_conf.prototxt raw_data/
  tail criteo_ctr_gpu_rpc
  check_result $FUNCNAME
  kill_server_process
}

function yolov4_rpc_gpu() {
  setproxy
  run_gpu_env
  cd ${build_path}/python/examples/yolov4
  sed -i "s/9393/8887/g" test_client.py
  cp -r /root/.cache/dist_data/serving/yolov4/yolov4.tar.gz ./
  tar xf yolov4.tar.gz
  python2.7 -m paddle_serving_server_gpu.serve --model yolov4_model --port 8887 --gpu_ids 0 > yolov4_rpc_log 2>&1 &
  sleep 5
  python2.7 test_client.py 000000570688.jpg
  tail yolov4_rpc_log
# check_result $FUNCNAME
  kill_server_process
}

function senta_rpc_cpu() {
  setproxy
  run_gpu_env
  cd ${build_path}/python/examples/senta
  sed -i "s/9393/8887/g" test_client.py
  cp -r /data/.cache/dist_data/serving/yolov4/yolov4.tar.gz ./
  tar xf yolov4.tar.gz
  python2.7 -m paddle_serving_server_gpu.serve --model yolov4_model --port 8887 --gpu_ids 0 > yolov4_rpc_log 2>&1 &
  sleep 5
  python2.7 test_client.py 000000570688.jpg
  tail yolov4_rpc_log
  check_result $FUNCNAME
  kill_server_process
}

function fit_a_line_http() {
  unsetproxy
  run_cpu_env
  cd ${build_path}/python/examples/fit_a_line
  sed -i "s/9292/8871/g" test_server.py
  python2.7 test_server.py > http_log2 2>&1 &
  sleep 10
  curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"x": [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332]}], "fetch":["price"]}' http://${host}:8871/uci/prediction
  check_result $FUNCNAME
  kill_server_process
}

function lac_http() {
  unsetproxy
  run_cpu_env
  cd ${build_path}/python/examples/lac
  python2.7 lac_web_service.py lac_model/ lac_workdir 8872 > http_lac_log2 2>&1 &
  sleep 10
  curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"words": "我爱北京天安门"}], "fetch":["word_seg"]}' http://${host}:8872/lac/prediction
  check_result $FUNCNAME
  kill_server_process
}

function cnn_http() {
  unsetproxy
  run_cpu_env
  cd ${build_path}/python/examples/imdb
  python2.7 text_classify_service.py imdb_cnn_model/ workdir/ 8873 imdb.vocab > cnn_http 2>&1 &
  sleep 10
  curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"words": "i am very sad | 0"}], "fetch":["prediction"]}' http://${host}:8873/imdb/prediction
  check_result $FUNCNAME
  kill_server_process
}

function bow_http() {
  unsetproxy
  run_cpu_env
  cd ${build_path}/python/examples/imdb
  python2.7 text_classify_service.py imdb_bow_model/ workdir/ 8874 imdb.vocab > bow_http 2>&1 &
  sleep 10
  curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"words": "i am very sad | 0"}], "fetch":["prediction"]}' http://${host}:8874/imdb/prediction
  check_result $FUNCNAME
  kill_server_process
}

function lstm_http() {
  unsetproxy
  run_cpu_env
  cd ${build_path}/python/examples/imdb
  python2.7 text_classify_service.py imdb_bow_model/ workdir/ 8875 imdb.vocab > bow_http 2>&1 &
  sleep 10
  curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"words": "i am very sad | 0"}], "fetch":["prediction"]}' http://${host}:8875/imdb/prediction
  check_result $FUNCNAME
  kill_server_process
}

function ResNet50_http() {
  unsetproxy
  run_gpu_env
  cd ${build_path}/python2.7/examples/imagenet
  python2.7 resnet50_web_service.py ResNet50_vd_model gpu 8876 > resnet50_http 2>&1 &
  sleep 10
  curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"image": "https://paddle-serving.bj.bcebos.com/imagenet-example/daisy.jpg"}], "fetch": ["score"]}' http://${host}:8876/image/prediction
  check_result $FUNCNAME
  kill_server_process
}

bert_http(){
  run_gpu_env
  unsetproxy
  cd ${build_path}/python/examples/bert
  cp data-c.txt.1 data-c.txt
  cp vocab.txt.1 vocab.txt
  export CUDA_VISIBLE_DEVICES=0
  python2.7 bert_web_service.py bert_seq128_model/ 8878 > bert_http 2>&1 &
  sleep 10
  curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"words": "hello"}], "fetch":["pooled_output"]}' http://${host}:8878/bert/prediction
  check_result $FUNCNAME
  kill_server_process
}

grpc_impl(){
  run_gpu_env
  cd ${build_path}/python/examples/grpc_impl_example/fit_a_line
  sh get_data.sh >/dev/null 2>&1
  python2.7 test_server.py uci_housing_model/ > grpclog 2>&1 &
  sleep 5
  echo "sync predict"
  python2.7 test_sync_client.py
  echo "async predict"
  python2.7 test_asyn_client.py
  echo "batch predict"
  python2.7 test_batch_client.py
  echo "timeout predict"
  python2.7 test_timeout_client.py
  # check_result $FUNCNAME
  kill_server_process
}


function build_all_whl(){
  for whl in ${build_whl_list[@]}
  do
    echo "===========${whl} begin build==========="
    $whl
    sleep 3
    echo "===========${whl} build over ==========="
  done
}

function run_rpc_models(){
  for model in ${rpc_model_list[@]}
  do
    echo "===========${model} run begin==========="
    $model
    sleep 3
    echo "===========${model} run  end ==========="
  done
}

function run_http_models(){
  for model in ${http_model_list[@]}
  do
    echo "===========${model} run begin==========="
    $model
    sleep 3
    echo "===========${model} run  end ==========="
  done
}
function end_hook(){
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
  run_rpc_models
#   run_http_models
  end_hook

}


main$@
