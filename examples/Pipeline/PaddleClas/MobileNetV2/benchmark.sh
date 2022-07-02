export FLAGS_profile_pipeline=1
alias python3="python3.6"
modelname="clas-MobileNetV2"

# HTTP
#ps -ef | grep web_service | awk '{print $2}' | xargs kill -9
sleep 3
# Create yaml，If you already have the config.yaml, ignore it.
#python3 benchmark.py yaml local_predictor 1 gpu
rm -rf profile_log_$modelname

echo "Starting HTTP Clients..."
# Start a client in each thread, tesing the case of multiple threads.
for thread_num in 1 2 4 8 12 16
do
  for batch_size in 1
  do
    echo "----${modelname} thread num: ${thread_num} batch size: ${batch_size} mode:http ----" >>profile_log_$modelname
    # Start one web service, If you start the service yourself, you can ignore it here.
    #python3 web_service.py >web.log 2>&1 &
    #sleep 3

    # --id is the serial number of the GPU card, Must be the same as the gpu id used by the server.
    nvidia-smi --id=3 --query-gpu=memory.used --format=csv -lms 1000 > gpu_use.log 2>&1 &
    nvidia-smi --id=3 --query-gpu=utilization.gpu --format=csv -lms 1000 > gpu_utilization.log 2>&1 &
    echo "import psutil\ncpu_utilization=psutil.cpu_percent(1,False)\nprint('CPU_UTILIZATION:', cpu_utilization)\n" > cpu_utilization.py
    # Start http client
    python3 benchmark.py run http $thread_num $batch_size > profile 2>&1

    # Collect CPU metrics, Filter data that is zero momentarily, Record the maximum value of GPU memory and the average value of GPU utilization
    python3 cpu_utilization.py >> profile_log_$modelname
    grep -av '^0 %' gpu_utilization.log > gpu_utilization.log.tmp
    awk 'BEGIN {max = 0} {if(NR>1){if ($modelname > max) max=$modelname}} END {print "MAX_GPU_MEMORY:", max}' gpu_use.log >> profile_log_$modelname
    awk -F' ' '{sum+=$1} END {print "GPU_UTILIZATION:", sum/NR, sum, NR }' gpu_utilization.log.tmp >> profile_log_$modelname

    # Show profiles
    python3 ../../../util/show_profile.py profile $thread_num >> profile_log_$modelname
    tail -n 8 profile >> profile_log_$modelname
    echo '' >> profile_log_$modelname
  done
done

# Kill all nvidia-smi background task.
pkill nvidia-smi
