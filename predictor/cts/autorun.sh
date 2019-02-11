#!/bin/bash
# ����·��
start_path="$(pwd)"
sh build.sh stop

# ��λ��ctsĿ¼��
cd "$(dirname "$0")"/

if [[ "x"$@ = x*--module_name=* ]]
then
    all_arg=$@
    tmp=${all_arg##*--module_name=}
    mod_name=${tmp%% *}
    sed -i "/^run_mod=/s/run_mod.*/run_mod=$mod_name/" install-all.conf
else
    sed -i "/^run_mod=/s/run_mod.*/run_mod=lr_engine/" install-all.conf
fi
 
env_num=`grep env_num install-all.conf | awk -F '=' '{print $2}'`
# ���û�������
export PATH="$(pwd)"/frame/tools/python27/bin:$PATH
export PYTHONPATH="$(pwd)"
alias | grep "alias cp=" >/dev/null
if [ $? -eq 0 ];then
    unalias cp
fi
# �ص�����·����ִ��main.py
cd "$start_path"
mem_free=`free -m | awk '{print $4}'| head -3 | awk 'END{print}'`
let thread_max=$mem_free/5000
if [ $thread_max -eq 0 ];then
    echo "ϵͳ�ڴ治��, ���������κ�case"
    exit 1
fi
if [ $thread_max -lt $env_num ];then
    env_num=$thread_max
    echo "Ŀǰϵͳ�ڴ����֧������$env_num���߳�"
fi
temp_args="--paral=$env_num"
python "$(dirname "$0")"/control/main.py $temp_args $@
ret=$?
sh build.sh stop
if [ $ret -ne 0 ]
then
    exit 1
fi
