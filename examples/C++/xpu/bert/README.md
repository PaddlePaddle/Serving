
## Prepare
### download model and extract
```
wget https://paddle-inference-dist.cdn.bcebos.com/PaddleLite/models_and_data_for_unittests/bert_base_chinese.tar.gz
tar zxvf bert_base_chinese.tar.gz
```
### convert model
```
python3 -m paddle_serving_client.convert --dirname bert_base_chinese --model_filename model.pdmodel --params_filename model.pdiparams
```
### or, you can get the serving saved model directly
```
wget https://paddle-serving.bj.bcebos.com/models/xpu/bert.tar.gz
tar zxvf bert.tar.gz 
```
### Getting Dict and Sample Dataset

```
sh get_data.sh
```
this script will download Chinese Dictionary File vocab.txt and Chinese Sample Data data-c.txt

## RPC Service

### Start Service

```
python3 -m paddle_serving_server.serve --model serving_server --port 7703 --use_lite --use_xpu --ir_optim
```

### Client Prediction

```
head data-c.txt | python3 bert_client.py --model serving_client/serving_client_conf.prototxt
```
