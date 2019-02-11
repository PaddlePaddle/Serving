#!/bin/bash

function cfont()
{
    while (($#!=0))
    do
        case $1 in
            -b)
                echo -ne " ";
                ;;
            -t)
                echo -ne "\t";
                ;;
            -n)
                echo -ne "\n";
                ;;
            -black)
                echo -ne "\033[30m";
                ;;
            -red)
                echo -ne "\033[31m";
                echo -ne "\033[1m";
                ;;
            -green)
                echo -ne "\033[32m";
                echo -ne "\033[1m";
                ;;
            -yellow)
                echo -ne "\033[33m";
                ;;
            -blue)
                echo -ne "\033[34m";
                echo -ne "\033[1m";
                ;;
            -purple)
                echo -ne "\033[35m";
                ;;
            -cyan)
                echo -ne "\033[36m";
                echo -ne "\033[1m";
                ;;
            -white|-gray)
                echo -ne "\033[37m";
                ;;
            -reset)
                echo -ne "\033[0m";
                ;;
            -h|-help|--help)
                echo "Usage: cfont -color1 message1 -color2 message2 ...";
                echo "eg:       cfont -red [ -blue message1 message2 -red ]";
                ;;
            *)
                echo -ne "$1"
                ;;
        esac
        shift
    done
    echo -ne "\033[0m";
}

cur_path=`pwd`
work_root=${cur_path%%/baidu/*}
CITOOLS="${work_root}/baidu/fengchao-qa/citools"
if [ ! -e ${CITOOLS}/lib/localbuild_lib.sh ];then
   cfont -blue "=============== localbuild_lib.sh is not exist, downloading ...================" -n
   git clone ssh://git@icode.baidu.com:8235/baidu/fengchao-qa/citools $CITOOLS >/dev/null
fi

source  ${CITOOLS}/lib/localbuild_lib.sh

function get_framework_baseenv()
{
    onlineFtp="ftp://tc-orp-app2.tc.baidu.com/home/heqing"
    wgetOptions="--tries=3 --retry-connrefused -r -l0 -nv --limit-rate=50m -nH"

    cfont -blue "##################################################" -n ;
    cfont -blue "###   build pdserving_framework xts base env   ###" -n ;
    cfont -blue "##################################################" -n ;
    cfont -reset;

    run_path="$(grep "run_path" "./install-all.conf" | cut -d "=" -f 2)"
    cd $run_path
    wget $wgetOptions --cut-dirs=4 "$onlineFtp"/scmbak/pdserving/framework_tester -o wget.log
    ret=$?
    retry=0
    while [[ $retry -lt 3 ]]; do
        if [[ $ret -eq 0 ]];then
            break;
        fi
        wget $wgetOptions --cut-dirs=4 "$onlineFtp"/scmbak/pdserving/framework_tester  -o wget.log
        ret=$?
        ((retry++))
    done
    [[ $ret -ne 0 ]] && return 1


    cfont -blue "[XTS] " -green "[ finish download: pdserving-framework ]" -n    
    cd -

    return 0
}

# 搭建cts环境
function build_ctsenv()
{    
    # 搭建cts环境
    if [ -z $1 ]; then
        ENV_NUM=0
    else
        ENV_NUM=$1
    fi

    #更新安装配置设置
    hostname=$(uname -n)
    username="$(echo "`whoami`" | awk '{print $1}')"
    LIBPATH=${PWD}/lib
    echo "libpath is : $LIBPATH"

    # 生成install-all.conf
    {
        echo "[config]"
        echo "host=$hostname"
        echo "user=$username"
        echo "passwd=CAPHI2008"
        echo "env_file=${PWD}/envfile"
        echo "lib_path=$LIBPATH"
        echo "run_path=${PWD}/run_env"
        echo "env_num=$ENV_NUM"
    }  > ./install-all.conf
     
    # 安装cts环境
    {
        cfont -blue "============= predictor env install =============" -n
        
        rm -rf run_env && mkdir -p run_env 
        echo "current path is :${cur_path}"
        #get_framework_baseenv        
        #if [ $? -ne 0 ]; then
        #    echo "pdserving-framework is not ready!!!"
        #    exit 1
        #fi
        mkdir -p run_env/predictor/bin
        mkdir -p run_env/predictor/conf
        # 拷贝pdserving到环境中        
        [[ -e  ../output/bin/pdserving ]] && cp -rf ../output/bin/pdserving  run_env/predictor/bin/predictor       
        [[ -e  ../output/lib ]] && cp -rf ../output/lib/  run_env/predictor/
        [[ -e  ../conf ]] && cp -rf ../conf/*  run_env/predictor/conf/        

        #搭建并行环境
        if [ $ENV_NUM -ne 0 ]; then
            cfont -blue "=============== build multi env ===============" -n          
            mkdir -p ${PWD}/run_env/1
            mv -f ${PWD}/run_env/framework_tester ${PWD}/run_env/1/framework_tester
            mv -f ${PWD}/run_env/model ${PWD}/run_env/1/model
            mv -f ${PWD}/run_env/dict ${PWD}/run_env/1/dict
            for ((i=2; i<=$ENV_NUM; i=i+1))
            do
                cp -rf ${PWD}/run_env/1 ${PWD}/run_env/$i
            done
        fi
    }

   #安装XTS环境
    {
        echo "now pwd is :`pwd`"
        cfont -blue "=============== XTS(cts) install ================" -n
        svn co https://svn.baidu.com/general-test/trunk/xts/frame frame> /dev/null
        svn co https://svn.baidu.com/general-test/trunk/xts/im/core/control control>/dev/null
        echo "now dir list is :`ls`"
        cd lib
        svn co https://svn.baidu.com/general-test/trunk/xts/im/core/lib/commonlib commonlib>/dev/null
        cd -
    }
    cfont -blue "[XTS] " -green "[ finish XTS(cts) install ]" -n

    onlineFtp="ftp://tc-orp-app2.tc.baidu.com/home/heqing"
    wgetOptions="--tries=3 --retry-connrefused -r -l0 -nv --limit-rate=50m -nH"

    #安装bidinfo 和基础protolib
    {
        cd lib
        [[ -e  bidinfo ]] && rm -rf bidinfo
        [[ -e  protolib ]] && rm -rf protolib
        [[ -e  pluginlib ]] && rm -rf pluginlib
        wget $wgetOptions --cut-dirs=5 "$onlineFtp"/scmbak/common_lib/pdserving_cts/bidinfo -o wget.log
        wget $wgetOptions --cut-dirs=5 "$onlineFtp"/scmbak/common_lib/pdserving_cts/protolib -o wget.log
        wget $wgetOptions --cut-dirs=6 "$onlineFtp"/scmbak/common_lib/pdserving_cts/framework/pluginlib -o wget.log
        cd -    
    }

    #安装protolib 
    {
        cfont -blue "============== protoc install ==================" -n
        [[ -e  protoc_tools ]] && rm -rf protoc_tools
        wget $wgetOptions --cut-dirs=5 "$onlineFtp"/scmbak/common_lib/pdserving_cts/protoc_tools -o wget.log
        [[ -e  ../proto ]] && cp -rf ../proto/*  ./protoc_tools/proto/
        cd protoc_tools
        chmod +x ./protobuf-2.4.1/bin/protoc
        chmod +x ./protobuf-2.4.1/lib/*
        [[ -e protolib ]] && rm -rf protolib
        mkdir ./protolib
        export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`/protobuf-2.4.1/lib
        ./protobuf-2.4.1/bin/protoc -I=./proto --python_out=./protolib/ ./proto/*.proto
        cd -
        cp ./protoc_tools/protolib/*.py ./lib/protolib/
    }
    cfont -reset
    return 0
}

function get_pid
{
    local prog=$1
    local user=$2
    local prog_path=$3
    local ret=-1
    local trash_path="/home/$(echo "`whoami`" | awk '{print $1}')/.__trash/"
    pids=`pgrep $prog -u $user`
    for pid in $pids
    do
        tmp_path=`ls -l /proc/$pid/exe 2>/dev/null | awk '{print $NF}'`
        if [ "$tmp_path" == "$prog_path" ] || [ ! -e $tmp_path ] || [ 0 == `echo $tmp_path | grep -qs $trash_path;echo $?` ]
        then
            echo $pid
            ret=0
        fi
    done
    return $ret
}

function kill_prog()
{
    name=$1
    username=$2
    prog_path=$3
    pids=`get_pid $name $username $prog_path`
    echo $pids>/dev/null
    if [ $? -eq 0 ] ;then
        for pid in $pids
        do
        #echo "$name,$pid"
        kill -9 $pid
        done
    fi
}

function kill_predictor_prog()
{
    username="$(echo "`whoami`" | awk '{print $1}')"
    if [ -f install-all.conf ]
    then
        env_num=`grep env_num= install-all.conf|awk -F '=' '{print $2}'`
    else
        env_num=0
    fi
    for ((i=0; i<=$env_num; i=i+1))
    do
    if [ $i -eq 0 ]
    then
        run_path="${PWD}/run_env"
    else
        run_path="${PWD}/run_env/$i"
    fi
    kill_prog predictor $username $run_path/framework_tester/bin/predictor
    done
}

function clean_ctsenv()
{
    rm -rf install-all.conf ccover
    rm -rf run_env fail_env output log  frame control lib/commonlib lib/protolib
    return 0
}


if [ $# -eq 1 ] && [ $1 == "clean" ] 
then
    clean_ctsenv
    exit 0
fi

if [ $# -eq 1 ] && [ $1 == "stop" ] 
then
    kill_predictor_prog
    exit 0
fi

clean_ctsenv
build_ctsenv "$1"
exit $?
