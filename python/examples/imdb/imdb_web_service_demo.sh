wget https://paddle-serving.bj.bcebos.com/imdb-demo/imdb_service.tar.gz
tar -xzf imdb_service.tar.gz
wget --no-check-certificate https://fleet.bj.bcebos.com/text_classification_data.tar.gz
tar -zxvf text_classification_data.tar.gz
python text_classify_service.py serving_server_model/ workdir imdb.vocab
