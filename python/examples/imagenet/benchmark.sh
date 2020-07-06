rm profile_log
export CUDA_VISIBLE_DEVICES=0,1,2,3
export FLAGS_profile_server=1
export FLAGS_profile_client=1
python -m paddle_serving_server_gpu.serve --model $1 --port 9292 --thread 4 --gpu_ids 0,1,2,3 2> elog > stdlog &

sleep 5

#warm up
$PYTHONROOT/bin/python benchmark.py --thread 8 --batch_size 1 --model $2/serving_client_conf.prototxt --request rpc > profile 2>&1

for thread_num in 4 8 16
do
for batch_size in 1 4 16 64
do
    $PYTHONROOT/bin/python benchmark.py --thread $thread_num --batch_size $batch_size --model $2/serving_client_conf.prototxt --request rpc > profile 2>&1
    echo "model name :" $1
    echo "thread num :" $thread_num
    echo "batch size :" $batch_size
    echo "=================Done===================="
    echo "model name :$1" >> profile_log
    echo "batch size :$batch_size" >> profile_log
    $PYTHONROOT/bin/python ../util/show_profile.py profile $thread_num >> profile_log
    tail -n 8 profile >> profile_log
done
done

ps -ef|grep 'serving'|grep -v grep|cut -c 9-15 | xargs kill -9
