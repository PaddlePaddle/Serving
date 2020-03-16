sh get_data.sh
python -m paddle_serving_server_gpu.serve --model ernie_crf_model/ --port 9295 --thread 12 --gpu_ids 0,1,2,3 2> service.elog > service.log &
python trainer.py --endpoint 127.0.0.1:9295-127.0.0.1:9296-127.0.0.1:9297-127.0.0.1:9298
