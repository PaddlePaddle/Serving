#!/bin/bash
function install_pdserving_lib(){
    ret=1
    local pdserving_lib_mode=$1
    case $pdserving_lib_mode in
        local)
            local pdserving_local_path=$2
            if [ ! -d $pdserving_local_path ]; then
                echo "[WARN failed to find local path]"
                return ret
            fi
            lib_name=`basename $pdserving_local_path`
            if [ -d ${CITOOLS}/$lib_name ]; then
                rm -rf ${CITOOLS}/$lib_name
            fi
            cp -rf $pdserving_local_path ${CITOOLS}/
            source ${CITOOLS}/$lib_name/predictor_build_lib.sh
        ;;
        ftp)
            local wgetOptions="--tries=3 --retry-connrefused -r -l0 -nv --limit-rate=50m -nH --cut-dirs=5"
            pdserving_lib_ftp_path="ftp://tc-orp-app2.tc.baidu.com:/home/heqing/scmbak/common_lib/pdserving_cts/pdserving_lib"
            lib_name=`basename $pdserving_lib_ftp_path`
            if [ -d ${CITOOLS}/$lib_name ]; then
                rm -rf ${CITOOLS}/$lib_name
            fi
            echo "wget cmd is :$wgetOptions  $pdserving_lib_ftp_path"
            echo "lib_name is :${lib_name}"
            wget $wgetOptions$cur_dirs  $pdserving_lib_ftp_path
            mv ${lib_name} ${CITOOLS}/
            source ${CITOOLS}/${lib_name}/predictor_build_lib.sh
        ;;
        *)
            ret=0
            echo "todo"
        ;;
    esac
    return $ret
}

CUR_PATH=$(pwd)
WORK_PATH=$(pwd)
WORK_ROOT=${WORK_PATH%%/baidu/*}
#co citools
CITOOLS="${WORK_ROOT}/baidu/fengchao-qa/citools"
if [ -d ${CITOOLS} ];then
    rm -rf ${CITOOLS}
fi
git clone --depth 1 ssh://git@icode.baidu.com:8235/baidu/fengchao-qa/citools $CITOOLS >/dev/null
[[ $? != 0 ]] && exit 1
source  $CITOOLS/lib/localbuild_lib.sh
#source过后路径可能改变，需要重新赋值
CITOOLS="${WORK_ROOT}/baidu/fengchao-qa/citools"

#install_pdserving_lib
pdserving_lib_mode="ftp"
install_pdserving_lib ${pdserving_lib_mode}    #两种模式:如果是local，需要指定本机上pdserving_lib的路径
#source ${CITOOLS}/pdserving_lib/predictor_build_lib.sh

COVMODULEID=8652
TYPE=framework
#执行本模块构建初始化
predictor_build_init
WORKROOT=$WORK_ROOT
#执行构建命令
predictor_build_do $@

exit 0
