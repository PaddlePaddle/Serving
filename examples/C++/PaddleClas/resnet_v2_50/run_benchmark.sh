if [ ! -x "ResNet50.tar.gz"]; then
  wget https://paddle-inference-dist.bj.bcebos.com/AI-Rank/models/Paddle/ResNet50.tar.gz
fi
tar -xzvf ResNet50.tar.gz
python3.6 -m paddle_serving_client.convert --dirname ./ResNet50 --model_filename model --params_filename params
bash benchmark.sh serving_server serving_client
