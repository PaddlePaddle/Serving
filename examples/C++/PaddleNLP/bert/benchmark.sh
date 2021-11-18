rm profile_log*
export CUDA_VISIBLE_DEVICES=0,1
export FLAGS_profile_server=1
export FLAGS_profile_client=1
export FLAGS_serving_latency=1

gpu_id=0
#save cpu and gpu utilization log
if [ -d utilization ];then
    rm -rf utilization
else
    mkdir utilization
fi
#start server
$PYTHONROOT/bin/python3 -m paddle_serving_server.serve --model $1 --port 9292 --thread 4 --gpu_ids 0,1 --mem_optim  --ir_optim >  elog  2>&1 &
sleep 5

#warm up
$PYTHONROOT/bin/python3 benchmark.py --thread 4 --batch_size 1 --model $2/serving_client_conf.prototxt --request rpc > profile 2>&1
echo -e "import psutil\nimport time\nwhile True:\n\tcpu_res = psutil.cpu_percent()\n\twith open('cpu.txt', 'a+') as f:\n\t\tf.write(f'{cpu_res}\\\n')\n\ttime.sleep(0.1)" > cpu.py
for thread_num in 1 4 8 16
do
for batch_size in 1 4 16 64
do
    job_bt=`date '+%Y%m%d%H%M%S'`
    nvidia-smi --id=0 --query-compute-apps=used_memory --format=csv -lms 100 > gpu_memory_use.log 2>&1 &
    nvidia-smi --id=0 --query-gpu=utilization.gpu --format=csv -lms 100 > gpu_utilization.log 2>&1 &
    rm -rf cpu.txt
    $PYTHONROOT/bin/python3 cpu.py &
    gpu_memory_pid=$!
    $PYTHONROOT/bin/python3 benchmark.py --thread $thread_num --batch_size $batch_size --model $2/serving_client_conf.prototxt --request rpc > profile 2>&1
    kill `ps -ef|grep used_memory|awk '{print $2}'` > /dev/null
    kill `ps -ef|grep utilization.gpu|awk '{print $2}'` > /dev/null
    kill `ps -ef|grep cpu.py|awk '{print $2}'` > /dev/null
    echo "model_name:" $1
    echo "thread_num:" $thread_num
    echo "batch_size:" $batch_size
    echo "=================Done===================="
    echo "model_name:$1" >> profile_log_$1
    echo "batch_size:$batch_size" >> profile_log_$1
    job_et=`date '+%Y%m%d%H%M%S'`
    awk 'BEGIN {max = 0} {if(NR>1){if ($1 > max) max=$1}} END {print "CPU_UTILIZATION:", max}' cpu.txt >> profile_log_$1
    awk 'BEGIN {max = 0} {if(NR>1){if ($1 > max) max=$1}} END {print "MAX_GPU_MEMORY:", max}' gpu_memory_use.log >> profile_log_$1
    awk 'BEGIN {max = 0} {if(NR>1){if ($1 > max) max=$1}} END {print "GPU_UTILIZATION:", max}' gpu_utilization.log >> profile_log_$1
    rm -rf gpu_use.log gpu_utilization.log
    $PYTHONROOT/bin/python3 ../../../util/show_profile.py profile $thread_num >> profile_log_$1
    tail -n 8 profile >> profile_log_$1
    echo "" >> profile_log_$1
done
done

#Divided log
awk 'BEGIN{RS="\n\n"}{i++}{print > "bert_log_"i}' profile_log_$1
mkdir bert_log && mv bert_log_* bert_log
ps -ef|grep 'serving'|grep -v grep|cut -c 9-15 | xargs kill -9
