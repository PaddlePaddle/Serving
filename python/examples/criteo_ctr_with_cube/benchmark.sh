rm profile_log
export FLAGS_profile_client=1
export FLAGS_profile_server=1
for thread_num in 1 2 4 8 16
do
for batch_size in 1 4 16 64 256
do
    $PYTHONROOT/bin/python benchmark.py --thread $thread_num --batch_size $batch_size --model serving_client_conf/serving_client_conf.prototxt --request rpc > profile 2>&1
    echo "batch size : $batch_size"
    echo "thread num : $thread_num"
    echo "========================================"
    echo "batch size : $batch_size" >> profile_log
    $PYTHONROOT/bin/python ../util/show_profile.py profile $thread_num >> profile_log
    tail -n 2 profile >> profile_log
done
done
