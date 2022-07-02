export FLAGS_profile_pipeline=1
alias python3="python3.7"
modelname="bert"
# HTTP
ps -ef | grep web_service | awk '{print $2}' | xargs kill -9 
sleep 3
python3 benchmark.py yaml local_predictor 1 gpu 
rm -rf profile_log_$modelname
for thread_num in 1 8 16 
do
  for batch_size in 1 10 100
  do
    echo "----Bert thread num: $thread_num batch size: $batch_size mode:http ----" >>profile_log_$modelname
    rm -rf PipelineServingLogs
    rm -rf cpu_utilization.py
    python3 web_service.py >web.log 2>&1 &
    sleep 3
    nvidia-smi --id=2 --query-compute-apps=used_memory --format=csv -lms 100 > gpu_use.log 2>&1 &
    nvidia-smi --id=2 --query-gpu=utilization.gpu --format=csv -lms 100 > gpu_utilization.log 2>&1 &
    echo "import psutil\ncpu_utilization=psutil.cpu_percent(1,False)\nprint('CPU_UTILIZATION:', cpu_utilization)\n" > cpu_utilization.py
    python3 benchmark.py run http $thread_num $batch_size
    python3 cpu_utilization.py >>profile_log_$modelname
    ps -ef | grep web_service | awk '{print $2}' | xargs kill -9
    python3 benchmark.py dump benchmark.log benchmark.tmp
    mv benchmark.tmp benchmark.log
    awk 'BEGIN {max = 0} {if(NR>1){if ($modelname > max) max=$modelname}} END {print "MAX_GPU_MEMORY:", max}' gpu_use.log >> profile_log_$modelname
    awk 'BEGIN {max = 0} {if(NR>1){if ($modelname > max) max=$modelname}} END {print "GPU_UTILIZATION:", max}' gpu_utilization.log >> profile_log_$modelname
    cat benchmark.log >> profile_log_$modelname
    #rm -rf gpu_use.log gpu_utilization.log
  done
done
# RPC
ps -ef | grep web_service | awk '{print $2}' | xargs kill -9
sleep 3
python3 benchmark.py yaml local_predictor 1 gpu

for thread_num in 1 8 16
do
  for batch_size in 1 10 100
  do
    echo "----Bert thread num: $thread_num batch size: $batch_size mode:rpc ----" >>profile_log_$modelname
    rm -rf PipelineServingLogs
    rm -rf cpu_utilization.py
    python3 web_service.py >web.log 2>&1 &
    sleep 3
    nvidia-smi --id=2 --query-compute-apps=used_memory --format=csv -lms 100 > gpu_use.log 2>&1 &
    nvidia-smi --id=2 --query-gpu=utilization.gpu --format=csv -lms 100 > gpu_utilization.log 2>&1 &
    echo "import psutil\ncpu_utilization=psutil.cpu_percent(1,False)\nprint('CPU_UTILIZATION:', cpu_utilization)\n" > cpu_utilization.py
    python3 benchmark.py run rpc $thread_num $batch_size
    python3 cpu_utilization.py >>profile_log_$modelname
    ps -ef | grep web_service | awk '{print $2}' | xargs kill -9
    python3 benchmark.py dump benchmark.log benchmark.tmp
    mv benchmark.tmp benchmark.log
    awk 'BEGIN {max = 0} {if(NR>1){if ($modelname > max) max=$modelname}} END {print "MAX_GPU_MEMORY:", max}' gpu_use.log >> profile_log_$modelname
    awk 'BEGIN {max = 0} {if(NR>1){if ($modelname > max) max=$modelname}} END {print "GPU_UTILIZATION:", max}' gpu_utilization.log >> profile_log_$modelname
    #rm -rf gpu_use.log gpu_utilization.log
    cat benchmark.log >> profile_log_$modelname
  done
done
