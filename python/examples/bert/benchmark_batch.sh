rm profile_log
export CUDA_VISIBLE_DEVICES=0,1,2,3
python -m paddle_serving_server_gpu.serve --model bert_seq20_model/ --port 9295 --thread 4 --gpu_ids 0,1,2,3 2> elog > stdlog &

sleep 5

for thread_num in 1 2 4 8 16
do
for batch_size in 1 2 4 8 16 32 64 128 256 512
do
    $PYTHONROOT/bin/python benchmark_batch.py --thread $thread_num --batch_size $batch_size --model serving_client_conf/serving_client_conf.prototxt --request rpc > profile 2>&1
    echo "========================================"
    echo "thread num: ", $thread_num
    echo "batch size: ", $batch_size
    echo "batch size : $batch_size" >> profile_log
    $PYTHONROOT/bin/python ../util/show_profile.py profile $thread_num >> profile_log
    tail -n 1 profile >> profile_log
done
done
