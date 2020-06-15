set -xe
rm -rf test19293
#python -m test_multilang_server uci_housing_model

export  PADDLE_SERVING_MAX_BATCH_SIZE=16

python -m paddle_serving_server_gpu.serve \
  --model ./uci_housing_model/ \
  --port 19293 \
  --thread 8 \
  --mem_optim  \
  --async  \
  --hidden_port 19294 \
  --gpu_ids 0
