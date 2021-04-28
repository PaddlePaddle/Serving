export CUDA_VISIBLE_DEVICES=0,1
python -m paddle_serving_server.serve --model bert_seq20_model/ --port 9295 --thread 4 --gpu_ids 0,1 2> elog > stdlog &
export FLAGS_profile_client=1
export FLAGS_profile_server=1
sleep 5
thread_num=4
python benchmark_batch.py --thread ${thread_num} --batch_size 64 --model serving_client_conf/serving_client_conf.prototxt 2> profile

python show_profile.py profile ${thread_num}
python timeline_trace.py profile trace
