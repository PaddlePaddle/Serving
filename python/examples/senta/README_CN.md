# 中文语句情感分类

## 获取模型文件和样例数据
```
sh get_data.sh
```
## 启动HTTP服务
```
python senta_web_service.py senta_bilstm_model/ workdir 9292
```
中文情感分类任务中需要先通过[LAC任务](../lac)进行中文分词，在脚本中通过```lac_model_path```参数配置LAC任务的模型文件路径,```lac_dict_path```参数配置LAC任务词典路径。
示例中将LAC任务放在情感分类任务的HTTP预测服务的预处理部分，LAC预测服务部署在CPU上，情感分类任务部署在GPU上,可以根据实际情况进行更改。

## 客户端预测
```
curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"words": "天气不错"}], "fetch":["class_probs"]}' http://127.0.0.1:9292/senta/prediction
```
