#!/usr/bin/env bash

function init() {
    source /root/.bashrc
    set -v
    #export http_proxy=http://172.19.56.199:3128
    #export https_proxy=http://172.19.56.199:3128
    export PYTHONROOT=/usr
    cd Serving
}

function check_cmd() {
    eval $@
    if [ $? -ne 0 ]; then
        exit 1
    fi
}

function build_client() {
    local TYPE=$1
    local DIRNAME=build-client-$TYPE
    mkdir $DIRNAME && cd $DIRNAME
    case $TYPE in
        CPU|GPU)
            cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python2.7/ \
                  -DPYTHON_LIBRARIES=$PYTHONROOT/lib64/libpython2.7.so \
                  -DPYTHON_EXECUTABLE=$PYTHONROOT/bin/python \
                  -DCLIENT_ONLY=ON ..
            check_cmd "make -j2 >/dev/null"
            pip install python/dist/paddle_serving_client* >/dev/null
            ;;
        *)
            echo "error type"
            exit 1
            ;;
    esac
    echo "build client $TYPE part finished as expected."
    cd ..
    rm -rf $DIRNAME
}

function build_server() {
    local TYPE=$1
    local DIRNAME=build-server-$TYPE
    mkdir $DIRNAME && cd $DIRNAME
    case $TYPE in
        CPU)
            cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python2.7/ \
                  -DPYTHON_LIBRARIES=$PYTHONROOT/lib64/libpython2.7.so \
                  -DPYTHON_EXECUTABLE=$PYTHONROOT/bin/python \
                  -DCLIENT_ONLY=OFF ..
            check_cmd "make -j2 >/dev/null && make install -j2 >/dev/null"
            pip install python/dist/paddle_serving_server* >/dev/null
            ;;
        GPU)
            cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python2.7/ \
                  -DPYTHON_LIBRARIES=$PYTHONROOT/lib64/libpython2.7.so \
                  -DPYTHON_EXECUTABLE=$PYTHONROOT/bin/python \
                  -DCLIENT_ONLY=OFF \
                  -DWITH_GPU=ON ..
            check_cmd "make -j2 >/dev/null && make install -j2 >/dev/null"
            pip install python/dist/paddle_serving_server* >/dev/null
            ;;
        *)
            echo "error type"
            exit 1
            ;;
    esac
    echo "build server $TYPE part finished as expected."
    cd ..
    rm -rf $DIRNAME
}

function python_test_fit_a_line() {
    cd fit_a_line
    sh get_data.sh
    local TYPE=$1
    echo $TYPE
    case $TYPE in
        CPU)
            # test rpc
            check_cmd "python test_server.py uci_housing_model/ > /dev/null &"
            sleep 5
            check_cmd "python test_client.py uci_housing_client/serving_client_conf.prototxt > /dev/null"
            ps -ef | grep "paddle_serving_server" | grep -v grep | awk '{print $2}' | xargs kill
            # test web
            check_cmd "python -m paddle_serving_server.serve --model uci_housing_model/ --name uci --port 9399 --name uci > /dev/null &"
            sleep 5
            check_cmd "curl -H \"Content-Type:application/json\" -X POST -d '{\"x\": [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332], \"fetch\":[\"price\"]}' http://127.0.0.1:9399/uci/prediction"
            ps -ef | grep "paddle_serving_server" | grep -v grep | awk '{print $2}' | xargs kill
            ;;
        GPU)
            echo "not support yet"
            exit 1
            ;;
        *)
            echo "error type"
            exit 1
            ;;
    esac
    echo "test fit_a_line $TYPE part finished as expected."
    rm -rf image kvdb log uci_housing* work*
    cd ..
}

function python_run_criteo_ctr_with_cube() {
    TYPE="CPU"
    yum install -y bc >/dev/null
    cd criteo_ctr_with_cube
    check_cmd "wget https://paddle-serving.bj.bcebos.com/unittest/ctr_cube_unittest.tar.gz"
    check_cmd "tar xf ctr_cube_unittest.tar.gz"
    check_cmd "mv models/ctr_client_conf ./"
    check_cmd "mv models/ctr_serving_model_kv ./"
    check_cmd "mv models/data ./cube/"
    check_cmd "mv models/ut_data ./"
    cp ../../../build-server-$TYPE/output/bin/cube* ./cube/ 
    sh cube_prepare.sh &
    check_cmd "mkdir work_dir1 && cp cube/conf/cube.conf ./work_dir1/"    
    python test_server.py ctr_serving_model_kv &
    check_cmd "python test_client.py ctr_client_conf/serving_client_conf.prototxt ./ut_data >score"
    AUC=$(tail -n 2  score | awk 'NR==1')
    VAR2="0.70"
    RES=$( echo "$AUC>$VAR2" | bc )
    if [[ $RES -eq 0 ]]; then
        echo "error with criteo_ctr_with_cube inference auc test, auc should > 0.70"
        exit 1
    fi
    echo "criteo_ctr_with_cube inference auc test success"
    ps -ef | grep "paddle_serving_server" | grep -v grep | awk '{print $2}' | xargs kill
    ps -ef | grep "cube" | grep -v grep | awk '{print $2}' | xargs kill
}

function python_run_test() {
    cd python/examples
    local TYPE=$1
    # Frist time run, downloading PaddleServing components ...
    python -c "from paddle_serving_server import Server; server = Server(); server.download_bin()"
    python_test_fit_a_line $TYPE
    python_run_criteo_ctr_with_cube
    echo "test python $TYPE part finished as expected."
    cd ../..
}

function main() {
    local TYPE=$1
    init
    build_client $TYPE
    build_server $TYPE
    cd Serving/
    python_run_test $TYPE
    echo "serving $TYPE part finished as expected."
}

main $@
