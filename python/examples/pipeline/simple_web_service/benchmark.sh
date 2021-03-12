ps -ef | grep web_service | awk '{print $2}' | xargs kill -9 
rm -rf PipelineServingLogs
rm -rf cpu_utilization.py
sleep 3
python3 benchmark.py yaml local_predictor 1 

for thread_num in 1 8 32
do
python3 web_service.py >web.log 2>&1 &
sleep 3
echo "import psutil\ncpu_utilization=psutil.cpu_percent(1,False)\nprint('CPU_UTILIZATION:', cpu_utilization)\n" > cpu_utilization.py
python3 benchmark.py run http $thread_num 1
python3 cpu_utilization.py
echo "------------Fit a line pipeline benchmark (Thread: $thread_num) (Benchmark: 1)"
tail -n 25 PipelineServingLogs/pipeline.tracer 
ps -ef | grep web_service | awk '{print $2}' | xargs kill -9
done
