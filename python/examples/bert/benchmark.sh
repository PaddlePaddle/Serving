rm profile_log
export CUDA_VISIBLE_DEVICES=0,1,2,3
export FLAGS_profile_server=1
export FLAGS_profile_client=1
export FLAGS_serving_latency=1
python3 -m paddle_serving_server_gpu.serve --model $1 --port 9292 --thread 4 --gpu_ids 0,1,2,3 --mem_optim False --ir_optim True 2> elog > stdlog &
hostname=`echo $(hostname)|awk -F '.baidu.com' '{print $1}'`
sleep 5
gpu_id=0

#warm up
python3 benchmark.py --thread 8 --batch_size 1 --model $2/serving_client_conf.prototxt --request rpc > profile 2>&1

for thread_num in 4 8 16
do
for batch_size in 1 4 16 64 256
do
    job_bt=`date '+%Y%m%d%H%M%S'`
    nvidia-smi --id=$gpu_id --query-compute-apps=used_memory --format=csv -lms 100 > gpu_use.log 2>&1 &
    gpu_memory_pid=$!
    python3 benchmark.py --thread $thread_num --batch_size $batch_size --model $2/serving_client_conf.prototxt --request rpc > profile 2>&1
    kill ${gpu_memory_pid}
    echo "model_name:" $1
    echo "thread_num:" $thread_num
    echo "batch_size:" $batch_size
    echo "=================Done===================="
    echo "model_name:$1" >> profile_log_$1
    echo "batch_size:$batch_size" >> profile_log_$1
    job_et=`date '+%Y%m%d%H%M%S'`
    awk 'BEGIN {max = 0} {if(NR>1){if ($1 > max) max=$1}} END {print "MAX_GPU_MEMORY_USE:", max}' gpu_use.log >> profile_log_$1
    monquery -n ${hostname} -i GPU_AVERAGE_UTILIZATION -s $job_bt -e $job_et -d 10 > gpu_log_file_${job_bt}
    monquery -n ${hostname} -i CPU_USER -s $job_bt -e $job_et  -d 10 > cpu_log_file_${job_bt}
    cpu_num=$(cat /proc/cpuinfo | grep processor | wc -l)
    gpu_num=$(nvidia-smi -L|wc -l)
    python ../util/show_profile.py profile $thread_num >> profile_log_$1
    tail -n 8 profile >> profile_log_$1
    echo "" >> profile_log_$1
done
done

ps -ef|grep 'serving'|grep -v grep|cut -c 9-15 | xargs kill -9
