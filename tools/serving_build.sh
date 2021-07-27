#!/usr/bin/env bash
set -x
function unsetproxy() {
    HTTP_PROXY_TEMP=$http_proxy
    HTTPS_PROXY_TEMP=$https_proxy
    unset http_proxy
    unset https_proxy
}

function setproxy() {
    export http_proxy=$HTTP_PROXY_TEMP
    export https_proxy=$HTTPS_PROXY_TEMP
}

function init() {
    source /root/.bashrc
    set -v
    export PYTHONROOT=/usr
    cd Serving
    export SERVING_WORKDIR=$PWD

    $PYTHONROOT/bin/python -m pip install -r python/requirements.txt
    $PYTHONROOT/bin/python -m pip install paddlepaddle

    export GOPATH=$HOME/go
    export PATH=$PATH:$GOPATH/bin

    go env -w GO111MODULE=on
    go env -w GOPROXY=https://goproxy.cn,direct

    go get -u github.com/grpc-ecosystem/grpc-gateway/protoc-gen-grpc-gateway@v1.15.2
    go get -u github.com/grpc-ecosystem/grpc-gateway/protoc-gen-swagger@v1.15.2
    go get -u github.com/golang/protobuf/protoc-gen-go@v1.4.3
    go get -u google.golang.org/grpc@v1.33.0
}

function check_cmd() {
    eval $@
    if [ $? -ne 0 ]; then
        exit 1
    fi
}

function rerun() {
    if [ $# -ne 2 ]; then
        echo "usage: rerun command rerun-times"
        exit 1
    fi
    local command=$1
    local times=$2
    for((i=1;i<=${times};i++))
    do
        if [ ${i} != 1 ]; then
            echo "${i}-th run command: ${command}..."
        fi
        eval $command
        if [ $? -eq 0 ]; then
            return 0
        fi
        echo "${i}-th run(command: ${command}) failed."
    done
    exit 1
}

function build_app() {
    local TYPE=$1
    local DIRNAME=build-app-$TYPE
    mkdir $DIRNAME # pwd: /Serving
    cd $DIRNAME # pwd: /Serving/build-app-$TYPE
    case $TYPE in
        CPU|GPU)
            cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python2.7/ \
                  -DPYTHON_LIBRARIES=$PYTHONROOT/lib/libpython2.7.so \
                  -DPYTHON_EXECUTABLE=$PYTHONROOT/bin/python \
                  -DAPP=ON ..
            rerun "make -j10 >/dev/null" 3 # due to some network reasons, compilation may fail
            pip install -U python/dist/paddle_serving_app* >/dev/null
            ;;
        *)
            echo "error type"
            exit 1
            ;;
    esac
    echo "build app $TYPE part finished as expected."
    cd .. # pwd: /Serving
}

function build_client() {
    local TYPE=$1
    local DIRNAME=build-client-$TYPE
    mkdir $DIRNAME # pwd: /Serving
    cd $DIRNAME # pwd: /Serving/build-client-$TYPE
    case $TYPE in
        CPU|GPU)
            cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python2.7/ \
                  -DPYTHON_LIBRARIES=$PYTHONROOT/lib64/libpython2.7.so \
                  -DPYTHON_EXECUTABLE=$PYTHONROOT/bin/python \
                  -DCLIENT=ON ..
            rerun "make -j10 >/dev/null" 3 # due to some network reasons, compilation may fail
            pip install -U python/dist/paddle_serving_client* >/dev/null
            ;;
        *)
            echo "error type"
            exit 1
            ;;
    esac
    echo "build client $TYPE part finished as expected."
    cd .. # pwd: /Serving
    # rm -rf $DIRNAME
}

function build_server() {
    local TYPE=$1
    local DIRNAME=build-server-$TYPE
    mkdir $DIRNAME # pwd: /Serving
    cd $DIRNAME # pwd: /Serving/build-server-$TYPE
    case $TYPE in
        CPU)
            cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python2.7/ \
                  -DPYTHON_LIBRARIES=$PYTHONROOT/lib64/libpython2.7.so \
                  -DPYTHON_EXECUTABLE=$PYTHONROOT/bin/python \
                  -DSERVER=ON ..
            rerun "make -j10 >/dev/null" 3 # due to some network reasons, compilation may fail
            check_cmd "make install -j2 >/dev/null"
            pip install -U python/dist/paddle_serving_server* >/dev/null
            ;;
        GPU)
            cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python2.7/ \
                  -DPYTHON_LIBRARIES=$PYTHONROOT/lib64/libpython2.7.so \
                  -DPYTHON_EXECUTABLE=$PYTHONROOT/bin/python \
                  -DSERVER=ON \
                  -DWITH_GPU=ON ..
            rerun "make -j10 >/dev/null" 3 # due to some network reasons, compilation may fail
            check_cmd "make install -j2 >/dev/null"
            pip install -U python/dist/paddle_serving_server* >/dev/null
            ;;
        *)
            echo "error type"
            exit 1
            ;;
    esac
    echo "build server $TYPE part finished as expected."
    cd .. # pwd: /Serving
    # rm -rf $DIRNAME    for export SERVING_BIN
}

function kill_server_process() {
    ps -ef | grep "serving" | grep -v serving_build | grep -v grep | awk '{print $2}' | xargs kill
    sleep 1
}

function kill_process_by_port() {
    if [ $# != 1 ]; then
        echo "usage: kill_process_by_port <PID>"
        exit 1
    fi
    local PID=$1
    lsof -i:$PID | awk 'NR == 1 {next} {print $2}' | xargs kill
}

function python_test_fit_a_line() {
    # pwd: /Serving/python/examples
    cd fit_a_line # pwd: /Serving/python/examples/fit_a_line
    sh get_data.sh
    local TYPE=$1
    export SERVING_BIN=${SERVING_WORKDIR}/build-server-${TYPE}/core/general-server/serving
    case $TYPE in
        CPU)
            # test rpc
            check_cmd "python -m paddle_serving_server.serve --model uci_housing_model --port 9393 --thread 4 > /dev/null &"
            sleep 5 # wait for the server to start
            check_cmd "python test_client.py uci_housing_client/serving_client_conf.prototxt > /dev/null"
            kill_server_process

            # test web
            unsetproxy # maybe the proxy is used on iPipe, which makes web-test failed.
            check_cmd "python test_server.py > /dev/null &"
            sleep 5 # wait for the server to start
            check_cmd "curl -H \"Content-Type:application/json\" -X POST -d '{\"feed\":[{\"x\": [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332]}], \"fetch\":[\"price\"]}' http://127.0.0.1:9393/uci/prediction"
            # check http code
            http_code=`curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"x": [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332]}], "fetch":["price"]}' -s -w "%{http_code}" -o /dev/null http://127.0.0.1:9393/uci/prediction`
            if [ ${http_code} -ne 200 ]; then
                echo "HTTP status code -ne 200"
                exit 1
            fi
            setproxy # recover proxy state
            kill_server_process
            ;;
        GPU)
            export CUDA_VISIBLE_DEVICES=0
            # test rpc
            check_cmd "python -m paddle_serving_server_gpu.serve --model uci_housing_model --port 9393 --thread 4 --gpu_ids 0 > /dev/null &"
            sleep 5 # wait for the server to start
            check_cmd "python test_client.py uci_housing_client/serving_client_conf.prototxt > /dev/null"
            kill_server_process

            ;;
        *)
            echo "error type"
            exit 1
            ;;
    esac
    echo "test fit_a_line $TYPE part finished as expected."
    rm -rf image kvdb log uci_housing* work*
    unset SERVING_BIN
    cd .. # pwd: /Serving/python/examples
}

function python_run_criteo_ctr_with_cube() {
    # pwd: /Serving/python/examples
    local TYPE=$1
    yum install -y bc >/dev/null
    cd criteo_ctr_with_cube # pwd: /Serving/python/examples/criteo_ctr_with_cube
    export SERVING_BIN=${SERVING_WORKDIR}/build-server-${TYPE}/core/general-server/serving
    case $TYPE in
        CPU)
            check_cmd "wget https://paddle-serving.bj.bcebos.com/unittest/ctr_cube_unittest.tar.gz"
            check_cmd "tar xf ctr_cube_unittest.tar.gz"
            check_cmd "mv models/ctr_client_conf ./"
            check_cmd "mv models/ctr_serving_model_kv ./"
            check_cmd "mv models/data ./cube/"
            check_cmd "mv models/ut_data ./"
            cp ../../../build-server-$TYPE/output/bin/cube* ./cube/
            sh cube_prepare.sh &
            python test_server.py ctr_serving_model_kv &
            sleep 5
            check_cmd "python test_client.py ctr_client_conf/serving_client_conf.prototxt ./ut_data >score"
            tail -n 2 score | awk 'NR==1'
            AUC=$(tail -n 2  score | awk 'NR==1')
            VAR2="0.67" #TODO: temporarily relax the threshold to 0.67
            RES=$( echo "$AUC>$VAR2" | bc )
            if [[ $RES -eq 0 ]]; then
                echo "error with criteo_ctr_with_cube inference auc test, auc should > 0.67"
                exit 1
            fi
            echo "criteo_ctr_with_cube inference auc test success"
            kill_server_process
            ps -ef | grep "cube" | grep -v grep | awk '{print $2}' | xargs kill
            sleep 1
            ;;
        GPU)
            check_cmd "wget https://paddle-serving.bj.bcebos.com/unittest/ctr_cube_unittest.tar.gz"
            check_cmd "tar xf ctr_cube_unittest.tar.gz"
            check_cmd "mv models/ctr_client_conf ./"
            check_cmd "mv models/ctr_serving_model_kv ./"
            check_cmd "mv models/data ./cube/"
            check_cmd "mv models/ut_data ./"
            cp ../../../build-server-$TYPE/output/bin/cube* ./cube/
            sh cube_prepare.sh &
            python test_server_gpu.py ctr_serving_model_kv &
            sleep 5
            # for warm up
            python test_client.py ctr_client_conf/serving_client_conf.prototxt ./ut_data > /dev/null || true
            check_cmd "python test_client.py ctr_client_conf/serving_client_conf.prototxt ./ut_data >score"
            tail -n 2 score | awk 'NR==1'
            AUC=$(tail -n 2  score | awk 'NR==1')
            VAR2="0.67" #TODO: temporarily relax the threshold to 0.67
            RES=$( echo "$AUC>$VAR2" | bc )
            if [[ $RES -eq 0 ]]; then
                echo "error with criteo_ctr_with_cube inference auc test, auc should > 0.67"
                exit 1
            fi
            echo "criteo_ctr_with_cube inference auc test success"
            kill_server_process
            ps -ef | grep "cube" | grep -v grep | awk '{print $2}' | xargs kill
            sleep 1
            ;;
        *)
            echo "error type"
            exit 1
            ;;
    esac
    unset SERVING_BIN
    echo "test criteo_ctr_with_cube $TYPE part finished as expected."
    cd .. # pwd: /Serving/python/examples
}

function python_test_bert() {
    # pwd: /Serving/python/examples
    local TYPE=$1
    export SERVING_BIN=${SERVING_WORKDIR}/build-server-${TYPE}/core/general-server/serving
    cd bert # pwd: /Serving/python/examples/bert
    case $TYPE in
        CPU)
            # Because download from paddlehub may timeout,
            # download the model from bos(max_seq_len=128).
            wget https://paddle-serving.bj.bcebos.com/paddle_hub_models/text/SemanticModel/bert_chinese_L-12_H-768_A-12.tar.gz
            tar -xzf bert_chinese_L-12_H-768_A-12.tar.gz
            sh get_data.sh
            check_cmd "python -m paddle_serving_server.serve --model bert_chinese_L-12_H-768_A-12_model --port 9292 &"
            sleep 5
            check_cmd "head -n 10 data-c.txt | python bert_client.py --model bert_chinese_L-12_H-768_A-12_client/serving_client_conf.prototxt"
            kill_server_process
            echo "bert RPC inference pass"
            ;;
        GPU)
            export CUDA_VISIBLE_DEVICES=0
            # Because download from paddlehub may timeout,
            # download the model from bos(max_seq_len=128).
            wget https://paddle-serving.bj.bcebos.com/paddle_hub_models/text/SemanticModel/bert_chinese_L-12_H-768_A-12.tar.gz
            tar -xzf bert_chinese_L-12_H-768_A-12.tar.gz
            sh get_data.sh
            check_cmd "python -m paddle_serving_server_gpu.serve --model bert_chinese_L-12_H-768_A-12_model --port 9292 --gpu_ids 0 &"
            sleep 5
            check_cmd "head -n 10 data-c.txt | python bert_client.py --model bert_chinese_L-12_H-768_A-12_client/serving_client_conf.prototxt"
            kill_server_process
            echo "bert RPC inference pass"
            ;;
        *)
            echo "error type"
            exit 1
            ;;
    esac
    echo "test bert $TYPE finished as expected."
    unset SERVING_BIN
    cd ..
}

function python_test_multi_fetch() {
    # pwd: /Serving/python/examples
    local TYPT=$1
    export SERVING_BIN=${SERVING_WORKDIR}/build-server-${TYPE}/core/general-server/serving
    cd bert # pwd: /Serving/python/examples/bert
    case $TYPE in
        CPU)
            #download model (max_seq_len=32)
            wget https://paddle-serving.bj.bcebos.com/bert_example/bert_multi_fetch.tar.gz
            tar -xzvf bert_multi_fetch.tar.gz
            check_cmd "python -m paddle_serving_server.serve --model bert_seq32_model --port 9292 &"
            sleep 5
            check_cmd "head -n 8 data-c.txt | python test_multi_fetch_client.py"
            kill_server_process
            echo "bert mutli fetch RPC inference pass"
            ;;
        GPU)
            #download model (max_seq_len=32)
            wget https://paddle-serving.bj.bcebos.com/bert_example/bert_multi_fetch.tar.gz
            tar -xzvf bert_multi_fetch.tar.gz
            check_cmd "python -m paddle_serving_server_gpu.serve --model bert_seq32_model --port 9292 --gpu_ids 0 &"
            sleep 5
            check_cmd "head -n 8 data-c.txt | python test_multi_fetch_client.py"
            kill_server_process
            echo "bert mutli fetch RPC inference pass"
            ;;
        *)
            echo "error type"
            exit 1
            ;;
    esac
    echo "test multi fetch $TYPE finished as expected."
    unset SERVING_BIN
    cd ..
}

function python_test_multi_process(){
    # pwd: /Serving/python/examples
    local TYPT=$1
    export SERVING_BIN=${SERVING_WORKDIR}/build-server-${TYPE}/core/general-server/serving
    cd fit_a_line # pwd: /Serving/python/examples/fit_a_line
    sh get_data.sh
    case $TYPE in
        CPU)
            check_cmd "python -m paddle_serving_server.serve --model uci_housing_model --port 9292 --workdir test9292 &"
            check_cmd "python -m paddle_serving_server.serve --model uci_housing_model --port 9293 --workdir test9293 &"
            sleep 5
            check_cmd "python test_multi_process_client.py"
            kill_server_process
            echo "bert mutli rpc RPC inference pass"
            ;;
        GPU)
            rm -rf ./image #TODO: The following code tried to create this folder, but no corresponding code was found
            check_cmd "python -m paddle_serving_server_gpu.serve --model uci_housing_model --port 9292 --workdir test9292 --gpu_ids 0 &"
            check_cmd "python -m paddle_serving_server_gpu.serve --model uci_housing_model --port 9293 --workdir test9293 --gpu_ids 0 &"
            sleep 5
            check_cmd "python test_multi_process_client.py"
            kill_server_process
            echo "bert mutli process RPC inference pass"
            ;;
        *)
            echo "error type"
            exit 1
            ;;
    esac
    echo "test multi process $TYPE finished as expected."
    unset SERVING_BIN
    cd ..
}

function python_test_imdb() {
    # pwd: /Serving/python/examples
    local TYPE=$1
    export SERVING_BIN=${SERVING_WORKDIR}/build-server-${TYPE}/core/general-server/serving
    cd imdb # pwd: /Serving/python/examples/imdb
    case $TYPE in
        CPU)
            sh get_data.sh
            check_cmd "python -m paddle_serving_server.serve --model imdb_cnn_model/ --port 9292 &"
            sleep 5
            check_cmd "head test_data/part-0 | python test_client.py imdb_cnn_client_conf/serving_client_conf.prototxt imdb.vocab"
            # test batch predict
            check_cmd "python benchmark.py --thread 4 --batch_size 8 --model imdb_bow_client_conf/serving_client_conf.prototxt --request rpc --endpoint 127.0.0.1:9292"
            echo "imdb CPU RPC inference pass"
            kill_server_process
            rm -rf work_dir1
            sleep 5

            unsetproxy # maybe the proxy is used on iPipe, which makes web-test failed.
            check_cmd "python text_classify_service.py imdb_cnn_model/ workdir/ 9292 imdb.vocab &"
            sleep 5
            check_cmd "curl -H \"Content-Type:application/json\" -X POST -d '{\"feed\":[{\"words\": \"i am very sad | 0\"}], \"fetch\":[\"prediction\"]}' http://127.0.0.1:9292/imdb/prediction"
            http_code=`curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"words": "i am very sad | 0"}], "fetch":["prediction"]}' -s -w "%{http_code}" -o /dev/null http://127.0.0.1:9292/imdb/prediction`
            if [ ${http_code} -ne 200 ]; then
                echo "HTTP status code -ne 200"
                exit 1
            fi
            # test batch predict
            check_cmd "python benchmark.py --thread 4 --batch_size 8 --model imdb_bow_client_conf/serving_client_conf.prototxt --request http --endpoint 127.0.0.1:9292"
            setproxy # recover proxy state
            kill_server_process
            ps -ef | grep "text_classify_service.py" | grep -v grep | awk '{print $2}' | xargs kill
            echo "imdb CPU HTTP inference pass"
            ;;
        GPU)
            echo "imdb ignore GPU test"
            ;;
        *)
            echo "error type"
            exit 1
            ;;
    esac
    echo "test imdb $TYPE finished as expected."
    unset SERVING_BIN
    cd ..
}

function python_test_lac() {
    # pwd: /Serving/python/examples
    local TYPE=$1
    export SERVING_BIN=${SERVING_WORKDIR}/build-server-${TYPE}/core/general-server/serving
    cd lac # pwd: /Serving/python/examples/lac
    case $TYPE in
        CPU)
            python -m paddle_serving_app.package --get_model lac
            tar -xzvf lac.tar.gz
            check_cmd "python -m paddle_serving_server.serve --model lac_model/ --port 9292 &"
            sleep 5
            check_cmd "echo \"我爱北京天安门\" | python lac_client.py lac_client/serving_client_conf.prototxt "
            echo "lac CPU RPC inference pass"
            kill_server_process

            unsetproxy # maybe the proxy is used on iPipe, which makes web-test failed.
            check_cmd "python lac_web_service.py lac_model/ lac_workdir 9292 &"
            sleep 5
            check_cmd "curl -H \"Content-Type:application/json\" -X POST -d '{\"feed\":[{\"words\": \"我爱北京天安门\"}], \"fetch\":[\"word_seg\"]}' http://127.0.0.1:9292/lac/prediction"
            # check http code
            http_code=`curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"words": "我爱北京天安门"}], "fetch":["word_seg"]}' -s -w "%{http_code}" -o /dev/null http://127.0.0.1:9292/lac/prediction`
            if [ ${http_code} -ne 200 ]; then
                echo "HTTP status code -ne 200"
                exit 1
            fi
            # http batch
            check_cmd "curl -H \"Content-Type:application/json\" -X POST -d '{\"feed\":[{\"words\": \"我爱北京天安门\"}, {\"words\": \"我爱北京天安门\"}], \"fetch\":[\"word_seg\"]}' http://127.0.0.1:9292/lac/prediction"
            # check http code
            http_code=`curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"words": "我爱北京天安门"}, {"words": "我爱北京天安门"}], "fetch":["word_seg"]}' -s -w "%{http_code}" -o /dev/null http://127.0.0.1:9292/lac/prediction`
            if [ ${http_code} -ne 200 ]; then
                echo "HTTP status code -ne 200"
                exit 1
            fi
            setproxy # recover proxy state
            kill_server_process
            ps -ef | grep "lac_web_service" | grep -v grep | awk '{print $2}' | xargs kill
            sleep 1
            echo "lac CPU HTTP inference pass"
            ;;
        GPU)
            echo "lac ignore GPU test"
            ;;
        *)
            echo "error type"
            exit 1
            ;;
    esac
    echo "test lac $TYPE finished as expected."
    unset SERVING_BIN
    cd ..
}


function python_test_encryption(){
    #pwd: /Serving/python/examples
    cd encryption
    sh get_data.sh
    local TYPE=$1
    export SERVING_BIN=${SERIVNG_WORKDIR}/build-server-${TYPE}/core/general-server/serving
    case $TYPE in
        CPU)
            #check_cmd "python encrypt.py"
            #sleep 5
            check_cmd "python -m paddle_serving_server.serve --model encrypt_server/ --port 9300 --use_encryption_model > /dev/null &"
            sleep 5
            check_cmd "python test_client.py encrypt_client/serving_client_conf.prototxt"
            kill_server_process
            ;;
        GPU)
            #check_cmd "python encrypt.py"
            #sleep 5
            check_cmd "python -m paddle_serving_server_gpu.serve --model encrypt_server/ --port 9300 --use_encryption_model --gpu_ids 0"
            sleep 5
            check_cmd "python test_client.py encrypt_client/serving_client_conf.prototxt"
            kill_servere_process
            ;;
        *)
            echo "error type"
            exit 1
            ;;
    esac
    echo "encryption $TYPE test finished as expected"
    setproxy
    unset SERVING_BIN
    cd ..
}


function java_run_test() {
    # pwd: /Serving
    local TYPE=$1
    export SERVING_BIN=${SERVING_WORKDIR}/build-server-${TYPE}/core/general-server/serving
    unsetproxy
    case $TYPE in
        CPU)
            # compile java sdk
            cd java # pwd: /Serving/java
            mvn compile > /dev/null
            mvn install > /dev/null
            # compile java sdk example
            cd examples # pwd: /Serving/java/examples
            mvn compile > /dev/null
            mvn install > /dev/null
            
            # fit_a_line (general, asyn_predict, batch_predict)
            cd ../../python/examples/grpc_impl_example/fit_a_line # pwd: /Serving/python/examples/grpc_impl_example/fit_a_line
            sh get_data.sh
            check_cmd "python -m paddle_serving_server.serve --model uci_housing_model --port 9393 --thread 4 --use_multilang > /dev/null &"
            sleep 5 # wait for the server to start
            cd ../../../java/examples # /Serving/java/examples
            java -cp target/paddle-serving-sdk-java-examples-0.0.1-jar-with-dependencies.jar PaddleServingClientExample fit_a_line
            java -cp target/paddle-serving-sdk-java-examples-0.0.1-jar-with-dependencies.jar PaddleServingClientExample asyn_predict
            java -cp target/paddle-serving-sdk-java-examples-0.0.1-jar-with-dependencies.jar PaddleServingClientExample batch_predict
            kill_server_process

            # imdb (model_ensemble)
            cd ../../python/examples/grpc_impl_example/imdb # pwd: /Serving/python/examples/grpc_impl_example/imdb
            sh get_data.sh > /dev/null
            check_cmd "python test_multilang_ensemble_server.py > /dev/null &"
            sleep 5 # wait for the server to start
            cd ../../../java/examples # /Serving/java/examples
            java -cp target/paddle-serving-sdk-java-examples-0.0.1-jar-with-dependencies.jar PaddleServingClientExample model_ensemble
            kill_server_process

            # yolov4 (int32)
            cd ../../python/examples/grpc_impl_example/yolov4 # pwd: /Serving/python/examples/grpc_impl_example/yolov4
            python -m paddle_serving_app.package --get_model yolov4 > /dev/null
            tar -xzf yolov4.tar.gz > /dev/null
            check_cmd "python -m paddle_serving_server.serve --model yolov4_model --port 9393 --use_multilang --mem_optim > /dev/null &"
            cd ../../../java/examples # /Serving/java/examples
            java -cp target/paddle-serving-sdk-java-examples-0.0.1-jar-with-dependencies.jar PaddleServingClientExample yolov4 src/main/resources/000000570688.jpg
            kill_server_process
            cd ../../ # pwd: /Serving
            ;;
        GPU)
            ;;
        *)
            echo "error type"
            exit 1
            ;;
    esac
    echo "java-sdk $TYPE part finished as expected."
    setproxy
    unset SERVING_BIN
}

function python_test_grpc_impl() {
    # pwd: /Serving/python/examples
    cd grpc_impl_example # pwd: /Serving/python/examples/grpc_impl_example
    local TYPE=$1
    export SERVING_BIN=${SERVING_WORKDIR}/build-server-${TYPE}/core/general-server/serving
    unsetproxy
    case $TYPE in
        CPU)
            # test general case
            cd fit_a_line # pwd: /Serving/python/examples/grpc_impl_example/fit_a_line
            sh get_data.sh

            # one line command start
            check_cmd "python -m paddle_serving_server.serve --model uci_housing_model --port 9393 --thread 4 --use_multilang > /dev/null &"
            sleep 5 # wait for the server to start
            check_cmd "python test_sync_client.py > /dev/null"
            check_cmd "python test_asyn_client.py > /dev/null"
            check_cmd "python test_timeout_client.py > /dev/null"
            kill_server_process
            kill_process_by_port 9393

            check_cmd "python test_server.py uci_housing_model > /dev/null &"
            sleep 5 # wait for the server to start
            check_cmd "python test_sync_client.py > /dev/null"
            check_cmd "python test_asyn_client.py > /dev/null"
            check_cmd "python test_timeout_client.py > /dev/null"
            kill_server_process
            kill_process_by_port 9393

            cd .. # pwd: /Serving/python/examples/grpc_impl_example

            # test load server config and client config in Server side
            cd criteo_ctr_with_cube # pwd: /Serving/python/examples/grpc_impl_example/criteo_ctr_with_cube
<<COMMENT #comment for compile bug, todo fix conflict between grpc-gateway and cube-agent 
            check_cmd "wget https://paddle-serving.bj.bcebos.com/unittest/ctr_cube_unittest.tar.gz > /dev/null"
            check_cmd "tar xf ctr_cube_unittest.tar.gz"
            check_cmd "mv models/ctr_client_conf ./"
            check_cmd "mv models/ctr_serving_model_kv ./"
            check_cmd "mv models/data ./cube/"
            check_cmd "mv models/ut_data ./"
            cp ../../../../build-server-$TYPE/output/bin/cube* ./cube/
            sh cube_prepare.sh &
            check_cmd "mkdir work_dir1 && cp cube/conf/cube.conf ./work_dir1/"
            python test_server.py ctr_serving_model_kv ctr_client_conf/serving_client_conf.prototxt &
            sleep 5
            check_cmd "python test_client.py ./ut_data >score"
            tail -n 2 score | awk 'NR==1'
            AUC=$(tail -n 2  score | awk 'NR==1')
            VAR2="0.67" #TODO: temporarily relax the threshold to 0.67
            RES=$( echo "$AUC>$VAR2" | bc )
            if [[ $RES -eq 0 ]]; then
                echo "error with criteo_ctr_with_cube inference auc test, auc should > 0.67"
                exit 1
            fi
COMMENT

            echo "grpc impl test success"
            kill_server_process
            #ps -ef | grep "cube" | grep -v grep | awk '{print $2}' | xargs kill

            cd .. # pwd: /Serving/python/examples/grpc_impl_example
            ;;
        GPU)
            export CUDA_VISIBLE_DEVICES=0
            # test general case
            cd fit_a_line # pwd: /Serving/python/examples/grpc_impl_example/fit_a_line
            sh get_data.sh

            # one line command start
            check_cmd "python -m paddle_serving_server_gpu.serve --model uci_housing_model --port 9393 --thread 4 --gpu_ids 0 --use_multilang > /dev/null &"
            sleep 5 # wait for the server to start
            check_cmd "python test_sync_client.py > /dev/null"
            check_cmd "python test_asyn_client.py > /dev/null"
            #check_cmd "python test_batch_client.py > /dev/null"
            check_cmd "python test_timeout_client.py > /dev/null"
            kill_server_process
            kill_process_by_port 9393

            check_cmd "python test_server_gpu.py uci_housing_model > /dev/null &"
            sleep 5 # wait for the server to start
            check_cmd "python test_sync_client.py > /dev/null"
            check_cmd "python test_asyn_client.py > /dev/null"
            #check_cmd "python test_batch_client.py > /dev/null"
            check_cmd "python test_timeout_client.py > /dev/null"
            kill_server_process
            kill_process_by_port 9393
            #ps -ef | grep "test_server_gpu" | grep -v serving_build | grep -v grep | awk '{print $2}' | xargs kill

            cd .. # pwd: /Serving/python/examples/grpc_impl_example

            # test load server config and client config in Server side
<<COMMENT #comment for compile bug, todo fix conflict between grpc-gateway and cube-agent 
            cd criteo_ctr_with_cube # pwd: /Serving/python/examples/grpc_impl_example/criteo_ctr_with_cube

            check_cmd "wget https://paddle-serving.bj.bcebos.com/unittest/ctr_cube_unittest.tar.gz"
            check_cmd "tar xf ctr_cube_unittest.tar.gz"
            check_cmd "mv models/ctr_client_conf ./"
            check_cmd "mv models/ctr_serving_model_kv ./"
            check_cmd "mv models/data ./cube/"
            check_cmd "mv models/ut_data ./"
            cp ../../../../build-server-$TYPE/output/bin/cube* ./cube/
            sh cube_prepare.sh &
            check_cmd "mkdir work_dir1 && cp cube/conf/cube.conf ./work_dir1/"
            python test_server_gpu.py ctr_serving_model_kv ctr_client_conf/serving_client_conf.prototxt &
            sleep 5
            # for warm up
            python test_client.py ./ut_data &> /dev/null || true
            check_cmd "python test_client.py ./ut_data >score"
            tail -n 2 score | awk 'NR==1'
            AUC=$(tail -n 2  score | awk 'NR==1')
            VAR2="0.67" #TODO: temporarily relax the threshold to 0.67
            RES=$( echo "$AUC>$VAR2" | bc )
            if [[ $RES -eq 0 ]]; then
                echo "error with criteo_ctr_with_cube inference auc test, auc should > 0.67"
                exit 1
            fi
COMMENT
            echo "grpc impl test success"
            kill_server_process
            ps -ef | grep "test_server_gpu" | grep -v serving_build | grep -v grep | awk '{print $2}' | xargs kill
            #ps -ef | grep "cube" | grep -v grep | awk '{print $2}' | xargs kill
            cd .. # pwd: /Serving/python/examples/grpc_impl_example
            ;;
        *)
            echo "error type"
            exit 1
            ;;
    esac
    echo "test grpc impl $TYPE part finished as expected."
    setproxy
    unset SERVING_BIN
    cd .. # pwd: /Serving/python/examples
}


function python_test_yolov4(){
    #pwd:/ Serving/python/examples
    local TYPE=$1
    export SERVING_BIN=${SERVING_WORKDIR}/build-server-${TYPE}/core/general-server/serving
    cd yolov4
    case $TYPE in
        CPU)
            echo "no implement for cpu type"
            ;;
        GPU)
            python -m paddle_serving_app.package --get_model yolov4
            tar -xzvf yolov4.tar.gz
            check_cmd "python -m paddle_serving_server_gpu.serve --model yolov4_model/ --port 9393 --gpu_ids 0 &"
            sleep 5
            check_cmd "python test_client.py 000000570688.jpg"
            echo "yolov4 GPU RPC inference pass"
            kill_server_process
            ;;
        *)
            echo "error type"
            exit 1
            ;;
    esac
    echo "test yolov4 $TYPE finished as expected."
    unset SERVING_BIN
    cd ..
}

function python_test_resnet50(){
    #pwd:/ Serving/python/examples
    local TYPE=$1
    export SERVING_BIN=${SERVING_WORKDIR}/build-server-${TYPE}/core/general-server/serving
    cd imagenet
    case $TYPE in
        CPU)
            echo "no implement for cpu type"
            ;;
        GPU)
            sh get_model.sh
            check_cmd"python -m paddle_serving_server_gpu.serve --model ResNet50_vd_model --port 9696 --gpu_ids 0"
            sleep 5
            check_cmd"python resnet50_rpc_client.py ResNet50_vd_client_config/serving_client_conf.prototxt"
            echo "resnet50 GPU RPC inference pass"
            kill_server_process
            ;;
        *)
            echo "error type"
            exit 1
            ;;
    esac
    echo "test resnet $TYPE finished as expected"
    unset SERVING_BIN
    cd ..
}

function python_test_pipeline(){
    # pwd: /Serving/python/examples
    local TYPE=$1
    export SERVING_BIN=${SERVING_WORKDIR}/build-server-${TYPE}/core/general-server/serving
    unsetproxy
    cd pipeline # pwd: /Serving/python/examples/pipeline
    case $TYPE in
        CPU)
            cd imdb_model_ensemble # pwd: /Serving/python/examples/pipeline/imdb_model_ensemble
            # start paddle serving service (brpc)
            sh get_data.sh
            python -m paddle_serving_server.serve --model imdb_cnn_model --port 9292 --workdir test9292 &> cnn.log &
            python -m paddle_serving_server.serve --model imdb_bow_model --port 9393 --workdir test9393 &> bow.log &
            sleep 5
            
            # test: thread servicer & thread op
            cat << EOF > config.yml
rpc_port: 18080
worker_num: 4
build_dag_each_worker: false
dag:
    is_thread_op: true
    client_type: brpc
    retry: 1
    use_profile: false
EOF
            python test_pipeline_server.py > /dev/null &
            sleep 5
            check_cmd "python test_pipeline_client.py"
            ps -ef | grep "pipeline_server" | grep -v grep | awk '{print $2}' | xargs kill
            kill_process_by_port 18080

            # test: thread servicer & process op
            cat << EOF > config.yml
rpc_port: 18080
worker_num: 4
build_dag_each_worker: false
dag:
    is_thread_op: false
    client_type: brpc
    retry: 1
    use_profile: false
EOF
            python test_pipeline_server.py > /dev/null &
            sleep 5
            check_cmd "python test_pipeline_client.py"
            ps -ef | grep "pipeline_server" | grep -v grep | awk '{print $2}' | xargs kill
            kill_process_by_port 18080

            # test: process servicer & process op
            cat << EOF > config.yml
rpc_port: 18080
worker_num: 4
build_dag_each_worker: false
dag:
    is_thread_op: false
    client_type: brpc
    retry: 1
    use_profile: false
EOF
            python test_pipeline_server.py > /dev/null &
            sleep 5
            check_cmd "python test_pipeline_client.py"
            ps -ef | grep "pipeline_server" | grep -v grep | awk '{print $2}' | xargs kill
            kill_process_by_port 18080
            
            # test: process servicer & thread op
            #pip uninstall grpcio -y
            #pip install grpcio --no-binary=grpcio
            cat << EOF > config.yml
rpc_port: 18080
worker_num: 4
build_dag_each_worker: true
dag:
    is_thread_op: false
    client_type: brpc
    retry: 1
    use_profile: false
EOF
            python test_pipeline_server.py > /dev/null &
            sleep 5
            check_cmd "python test_pipeline_client.py"
            ps -ef | grep "pipeline_server" | grep -v grep | awk '{print $2}' | xargs kill
            kill_process_by_port 18080

            kill_server_process
            kill_process_by_port 9292
            kill_process_by_port 9393

            # start paddle serving service (grpc)
            python -m paddle_serving_server.serve --model imdb_cnn_model --port 9292 --use_multilang --workdir test9292 &> cnn.log &
            python -m paddle_serving_server.serve --model imdb_bow_model --port 9393 --use_multilang --workdir test9393 &> bow.log &
            sleep 5
            cat << EOF > config.yml
rpc_port: 18080
worker_num: 4
build_dag_each_worker: false
dag:
    is_thread_op: false
    client_type: grpc
    retry: 1
    use_profile: false
EOF
            python test_pipeline_server.py > /dev/null &
            sleep 5
            check_cmd "python test_pipeline_client.py"
            ps -ef | grep "pipeline_server" | grep -v grep | awk '{print $2}' | xargs kill
            kill_process_by_port 18080
            kill_server_process
            kill_process_by_port 9292
            kill_process_by_port 9393
            cd ..

            cd simple_web_service # pwd: /Serving/python/examples/pipeline/simple_web_service
            sh get_data.sh
            python web_service.py >/dev/null &
            sleep 5
            curl -X POST -k http://localhost:18080/uci/prediction -d '{"key": ["x"], "value": ["0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332"]}'
             check http code
            http_code=`curl -X POST -k -d '{"key":["x"], "value": ["0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332"]}' -s -w "%{http_code}" -o /dev/null http://localhost:18080/uci/prediction`
            if [ ${http_code} -ne 200 ]; then
                echo "HTTP status code -ne 200"
                exit 1
            fi
            ps -ef | grep "web_service" | grep -v grep | awk '{print $2}' | xargs kill
            ps -ef | grep "pipeline" | grep -v grep | awk '{print $2}' | xargs kill
            kill_server_process
            cd ..
            ;;
        GPU)
            cd simple_web_service # pwd: /Serving/python/examples/pipeline/simple_web_service
            sh get_data.sh
            python web_service.py >/dev/null &
            sleep 5
            curl -X POST -k http://localhost:18080/uci/prediction -d '{"key": ["x"], "value": ["0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332"]}'
            # check http code
            http_code=`curl -X POST -k -d '{"key":["x"], "value": ["0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332"]}' -s -w "%{http_code}" -o /dev/null http://localhost:18080/uci/prediction`
            if [ ${http_code} -ne 200 ]; then
                echo "HTTP status code -ne 200"
                exit 1
            fi
            ps -ef | grep "web_service" | grep -v grep | awk '{print $2}' | xargs kill
            ps -ef | grep "pipeline" | grep -v grep | awk '{print $2}' | xargs kill
            kill_server_process
            cd .. # pwd: /Serving/python/examples/pipeline
            ;;
        *)
            echo "error type"
            exit 1
            ;;
    esac
    cd ..
    setproxy
    unset SERVING_BIN
}

function python_app_api_test(){
    #pwd:/ Serving/python/examples
    #test image reader
    local TYPE=$1
    cd imagenet
    case $TYPE in
        CPU)
            ;;
        GPU)
            echo "no implement for cpu type"
            ;;
        *)
            echo "error type"
            exit 1
            ;;
    esac
    echo "test app api finised as expected"
    cd ..
}

function python_run_test() {
    # Using the compiled binary
    local TYPE=$1 # pwd: /Serving
    cd python/examples # pwd: /Serving/python/examples
    python_test_fit_a_line $TYPE # pwd: /Serving/python/examples
    #python_run_criteo_ctr_with_cube $TYPE # pwd: /Serving/python/examples
    python_test_bert $TYPE # pwd: /Serving/python/examples
    python_test_imdb $TYPE # pwd: /Serving/python/examples
    python_test_lac $TYPE # pwd: /Serving/python/examples
    python_test_multi_process $TYPE # pwd: /Serving/python/examples
    python_test_multi_fetch $TYPE # pwd: /Serving/python/examples
    python_test_encryption $TYPE # pwd: /Serving/python/examples
    python_test_yolov4 $TYPE # pwd: /Serving/python/examples
    python_test_grpc_impl $TYPE # pwd: /Serving/python/examples
    python_test_resnet50 $TYPE # pwd: /Serving/python/examples
    #python_test_pipeline $TYPE # pwd: /Serving/python/examples
    echo "test python $TYPE part finished as expected."
    cd ../.. # pwd: /Serving
}

function monitor_test() {
    local TYPE=$1 # pwd: /Serving
    mkdir _monitor_test && cd _monitor_test # pwd: /Serving/_monitor_test
    case $TYPE in
        CPU):
            # The CPU part and GPU part are identical.
            # In order to avoid Travis CI timeout (50 min), the CPU version is not checked
            ;;
        GPU):
            pip install pyftpdlib
            mkdir remote_path
            mkdir local_path
            cd remote_path # pwd: /Serving/_monitor_test/remote_path
            check_cmd "python -m pyftpdlib -p 8000 &>/dev/null &"
            cd .. # pwd: /Serving/_monitor_test

            # type: ftp
            # remote_path: /
            # remote_model_name: uci_housing.tar.gz
            # local_tmp_path: ___tmp
            # local_path: local_path
            cd remote_path # pwd: /Serving/_monitor_test/remote_path
            wget --no-check-certificate https://paddle-serving.bj.bcebos.com/uci_housing.tar.gz
            touch donefile
            cd ..  # pwd: /Serving/_monitor_test
            mkdir -p local_path/uci_housing_model
            python -m paddle_serving_server_gpu.monitor \
                    --type='ftp' --ftp_host='127.0.0.1' --ftp_port='8000' \
                    --remote_path='/' --remote_model_name='uci_housing.tar.gz' \
                    --remote_donefile_name='donefile' --local_path='local_path' \
                    --local_model_name='uci_housing_model' --local_timestamp_file='fluid_time_file' \
                    --local_tmp_path='___tmp' --unpacked_filename='uci_housing_model' \
                    --interval='1' >/dev/null &
            sleep 10
            if [ ! -f "local_path/uci_housing_model/fluid_time_file" ]; then
                echo "local_path/uci_housing_model/fluid_time_file not exist."
                exit 1
            fi
            ps -ef | grep "monitor" | grep -v grep | awk '{print $2}' | xargs kill
            rm -rf remote_path/*
            rm -rf local_path/*

            # type: ftp
            # remote_path: /tmp_dir
            # remote_model_name: uci_housing_model
            # local_tmp_path: ___tmp
            # local_path: local_path
            mkdir -p remote_path/tmp_dir && cd remote_path/tmp_dir # pwd: /Serving/_monitor_test/remote_path/tmp_dir
            wget --no-check-certificate https://paddle-serving.bj.bcebos.com/uci_housing.tar.gz
            tar -xzf uci_housing.tar.gz
            touch donefile
            cd ../.. # pwd: /Serving/_monitor_test
            mkdir -p local_path/uci_housing_model
            python -m paddle_serving_server_gpu.monitor \
                    --type='ftp' --ftp_host='127.0.0.1' --ftp_port='8000' \
                    --remote_path='/tmp_dir' --remote_model_name='uci_housing_model' \
                    --remote_donefile_name='donefile' --local_path='local_path' \
                    --local_model_name='uci_housing_model' --local_timestamp_file='fluid_time_file' \
                    --local_tmp_path='___tmp' --interval='1' >/dev/null &
            sleep 10
            if [ ! -f "local_path/uci_housing_model/fluid_time_file" ]; then
                echo "local_path/uci_housing_model/fluid_time_file not exist."
                exit 1
            fi
            ps -ef | grep "monitor" | grep -v grep | awk '{print $2}' | xargs kill
            rm -rf remote_path/*
            rm -rf local_path/*

            # type: general
            # remote_path: /
            # remote_model_name: uci_housing.tar.gz
            # local_tmp_path: ___tmp
            # local_path: local_path
            cd remote_path # pwd: /Serving/_monitor_test/remote_path
            wget --no-check-certificate https://paddle-serving.bj.bcebos.com/uci_housing.tar.gz
            touch donefile
            cd ..  # pwd: /Serving/_monitor_test
            mkdir -p local_path/uci_housing_model
            python -m paddle_serving_server_gpu.monitor \
                    --type='general' --general_host='ftp://127.0.0.1:8000' \
                    --remote_path='/' --remote_model_name='uci_housing.tar.gz' \
                    --remote_donefile_name='donefile' --local_path='local_path' \
                    --local_model_name='uci_housing_model' --local_timestamp_file='fluid_time_file' \
                    --local_tmp_path='___tmp' --unpacked_filename='uci_housing_model' \
                    --interval='1' >/dev/null &
            sleep 10
            if [ ! -f "local_path/uci_housing_model/fluid_time_file" ]; then
                echo "local_path/uci_housing_model/fluid_time_file not exist."
                exit 1
            fi
            ps -ef | grep "monitor" | grep -v grep | awk '{print $2}' | xargs kill
            rm -rf remote_path/*
            rm -rf local_path/*

            # type: general
            # remote_path: /tmp_dir
            # remote_model_name: uci_housing_model
            # local_tmp_path: ___tmp
            # local_path: local_path
            mkdir -p remote_path/tmp_dir && cd remote_path/tmp_dir # pwd: /Serving/_monitor_test/remote_path/tmp_dir
            wget --no-check-certificate https://paddle-serving.bj.bcebos.com/uci_housing.tar.gz
            tar -xzf uci_housing.tar.gz
            touch donefile
            cd ../.. # pwd: /Serving/_monitor_test
            mkdir -p local_path/uci_housing_model
            python -m paddle_serving_server_gpu.monitor \
                    --type='general' --general_host='ftp://127.0.0.1:8000' \
                    --remote_path='/tmp_dir' --remote_model_name='uci_housing_model' \
                    --remote_donefile_name='donefile' --local_path='local_path' \
                    --local_model_name='uci_housing_model' --local_timestamp_file='fluid_time_file' \
                    --local_tmp_path='___tmp' --interval='1' >/dev/null &
            sleep 10
            if [ ! -f "local_path/uci_housing_model/fluid_time_file" ]; then
                echo "local_path/uci_housing_model/fluid_time_file not exist."
                exit 1
            fi
            ps -ef | grep "monitor" | grep -v grep | awk '{print $2}' | xargs kill
            rm -rf remote_path/*
            rm -rf local_path/*

            ps -ef | grep "pyftpdlib" | grep -v grep | awk '{print $2}' | xargs kill
            ;;
        *)
            echo "error type"
            exit 1
            ;;
    esac
    cd .. # pwd: /Serving
    rm -rf _monitor_test
    echo "test monitor $TYPE finished as expected."
}

function main() {
    local TYPE=$1 # pwd: /
    init # pwd: /Serving
    build_client $TYPE # pwd: /Serving
    build_server $TYPE # pwd: /Serving
    build_app $TYPE # pwd: /Serving
    java_run_test $TYPE # pwd: /Serving
    python_run_test $TYPE # pwd: /Serving
    #monitor_test $TYPE # pwd: /Serving
    echo "serving $TYPE part finished as expected."
}

main $@
exit 0
