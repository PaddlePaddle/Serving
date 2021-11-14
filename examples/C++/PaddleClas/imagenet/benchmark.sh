rm profile_log*
export CUDA_VISIBLE_DEVICES=0,1,2,3
export FLAGS_profile_server=1
export FLAGS_profile_client=1
python -m paddle_serving_server.serve --model $1 --port 9292 --thread 4 --gpu_ids 0,1,2,3 --mem_optim --ir_optim  2> elog > stdlog &

sleep 5
gpu_id=0
#save cpu and gpu utilization log
if [ -d utilization ];then
    rm -rf utilization
else
    mkdir utilization
fi

#warm up
$PYTHONROOT/bin/python3 benchmark.py --thread 4 --batch_size 1 --model $2/serving_client_conf.prototxt --request rpc > profile 2>&1
echo -e "import psutil\ncpu_utilization=psutil.cpu_percent(1,False)\nprint('CPU_UTILIZATION:', cpu_utilization)\n" > cpu_utilization.py

for thread_num in 1 4 8 16
do
for batch_size in 1 4 16 64
do
    job_bt=`date '+%Y%m%d%H%M%S'`
    nvidia-smi --id=0 --query-compute-apps=used_memory --format=csv -lms 100 > gpu_use.log 2>&1 &
    nvidia-smi --id=0 --query-gpu=utilization.gpu --format=csv -lms 100 > gpu_utilization.log 2>&1 &
    gpu_memory_pid=$!
    $PYTHONROOT/bin/python benchmark.py --thread $thread_num --batch_size $batch_size --model $2/serving_client_conf.prototxt --request rpc > profile 2>&1
    kill ${gpu_memory_pid}
    kill `ps -ef|grep used_memory|awk '{print $2}'`
    echo "model name :" $1
    echo "thread num :" $thread_num
    echo "batch size :" $batch_size
    echo "=================Done===================="
    echo "model name :$1" >> profile_log
    echo "batch size :$batch_size" >> profile_log
    job_et=`date '+%Y%m%d%H%M%S'`
    awk 'BEGIN {max = 0} {if(NR>1){if ($1 > max) max=$1}} END {print "MAX_GPU_MEMORY:", max}' gpu_use.log >> profile_log_$1
    awk 'BEGIN {max = 0} {if(NR>1){if ($1 > max) max=$1}} END {print "GPU_UTILIZATION:", max}' gpu_utilization.log >> profile_log_$1
    rm -rf gpu_use.log gpu_utilization.log
    $PYTHONROOT/bin/python ../../../util/show_profile.py profile $thread_num >> profile_log
    tail -n 8 profile >> profile_log
    echo "" >> profile_log_$1
done
done

#Divided log
awk 'BEGIN{RS="\n\n"}{i++}{print > "ResNet_log_"i}' profile_log_$1
mkdir $1_log && mv ResNet_log_* $1_log
ps -ef|grep 'serving'|grep -v grep|cut -c 9-15 | xargs kill -9
