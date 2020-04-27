# Chinese sentence sentiment classification

## Get model files and sample data
```
sh get_data.sh
```
## Start http service
```
python senta_web_service.py senta_bilstm_model/ workdir 9292
```
In the Chinese sentiment classification task, the Chinese word segmentation needs to be done through [LAC task] (../lac). Set model path by ```lac_model_path``` and dictionary path by ```lac_dict_path```. 
In this demo, the LAC task is placed in the preprocessing part of the HTTP prediction service of the sentiment classification task. The LAC prediction service is deployed on the CPU, and the sentiment classification task is deployed on the GPU, which can be changed according to the actual situation.
## Client prediction
```
curl -H "Content-Type:application/json" -X POST -d '{"words": "天气不错 | 0", "fetch":["sentence_feature"]}' http://127.0.0.1:9292/senta/prediction
```
