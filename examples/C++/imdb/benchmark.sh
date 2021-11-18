rm profile_log*
export FLAGS_profile_server=1
export FLAGS_profile_client=1
export FLAGS_serving_latency=1
$PYTHONROOT/bin/python3 -m paddle_serving_server.serve --model $1 --port 9292 --thread 4 --mem_optim --ir_optim  2> elog > stdlog &
hostname=`echo $(hostname)|awk -F '.baidu.com' '{print $1}'`
#save cpu and gpu utilization log
if [ -d utilization ];then
    rm -rf utilization
else
    mkdir utilization
fi
sleep 5


#warm up
$PYTHONROOT/bin/python3 benchmark.py --thread 4 --batch_size 1 --model $2/serving_client_conf.prototxt --request rpc > profile 2>&1
echo -e "import psutil\ncpu_utilization=psutil.cpu_percent(1,False)\nprint('CPU_UTILIZATION:', cpu_utilization)\n" > cpu_utilization.py

for thread_num in 1 4 8 16
do
for batch_size in 1 4 16 64
do
    job_bt=`date '+%Y%m%d%H%M%S'`
    $PYTHONROOT/bin/python3 benchmark.py --thread $thread_num --batch_size $batch_size --model $2/serving_client_conf.prototxt --request rpc > profile 2>&1
    echo "model_name:" $1
    echo "thread_num:" $thread_num
    echo "batch_size:" $batch_size
    echo "=================Done===================="
    echo "model_name:$1" >> profile_log_$1
    echo "batch_size:$batch_size" >> profile_log_$1
    job_et=`date '+%Y%m%d%H%M%S'`
    $PYTHONROOT/bin/python3 ../../util/show_profile.py profile $thread_num >> profile_log_$1
    $PYTHONROOT/bin/python3 cpu_utilization.py >> profile_log_$1
    tail -n 8 profile >> profile_log_$1
    echo "" >> profile_log_$1
done
done

#Divided log
awk 'BEGIN{RS="\n\n"}{i++}{print > "imdb_log_"i}' profile_log_$1
mkdir $1_log && mv imdb_log_* $1_log
ps -ef|grep 'serving'|grep -v grep|cut -c 9-15 | xargs kill -9
