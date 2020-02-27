rm profile_log
thread_num=1
for batch_size in 1 4 8 16 32 64 128 256
do
    $PYTHONROOT/bin/python benchmark_batch.py serving_client_conf/serving_client_conf.prototxt data.txt $thread_num $batch_size > profile 2>&1
    $PYTHONROOT/bin/python ../imdb/show_profile.py profile $thread_num >> profile_log
    tail -n 1 profile >> profile_log
done
