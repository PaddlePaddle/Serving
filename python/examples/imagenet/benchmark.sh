rm profile_log
for thread_num in 1 2 4 8
do
for batch_size in 1 2 4 8 16 32 64 128
do
    $PYTHONROOT/bin/python benchmark.py --thread $thread_num --batch_size $batch_size --model ResNet50_vd_client_config/serving_client_conf.prototxt --request rpc > profile 2>&1
    echo "========================================"
    echo "batch size : $batch_size" >> profile_log
    $PYTHONROOT/bin/python ../util/show_profile.py profile $thread_num >> profile_log
    tail -n 1 profile >> profile_log
done
done
