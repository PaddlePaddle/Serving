wget https://paddle-serving.bj.bcebos.com/paddle_hub_models/text/SemanticModel/bert_chinese_L-12_H-768_A-12.tar.gz
tar -xzf bert_chinese_L-12_H-768_A-12.tar.gz
mv bert_chinese_L-12_H-768_A-12_model bert_seq128_model
mv bert_chinese_L-12_H-768_A-12_client bert_seq128_client
wget https://paddle-serving.bj.bcebos.com/bert_example/data-c.txt --no-check-certificate
wget https://paddle-serving.bj.bcebos.com/bert_example/vocab.txt --no-check-certificate
