rm profile_log
for thread_num in 1 4 8 12 16 20 24
do
    $PYTHONROOT/bin/python benchmark.py serving_client_conf/serving_client_conf.prototxt data.txt $thread_num $batch_size > profile 2>&1
    $PYTHONROOT/bin/python ../imdb/show_profile.py profile $thread_num >> profile_log
    tail -n 1 profile >> profile_log
done
