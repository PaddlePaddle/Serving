## 语义理解预测服务

(简体中文|[English](./README.md))

示例中采用BERT模型进行语义理解预测，将文本表示为向量的形式，可以用来做进一步的分析和预测。

若使用python的版本为3.X, 将以下命令中的pip 替换为pip3, python替换为python3.
### 获取模型
方法1：
示例中采用[Paddlehub](https://github.com/PaddlePaddle/PaddleHub)中的[BERT中文模型](https://www.paddlepaddle.org.cn/hubdetail?name=bert_chinese_L-12_H-768_A-12&en_category=SemanticModel)。
请先安装paddlehub
```
pip3 install paddlehub
```
执行
```
python3 prepare_model.py 128
```
参数128表示BERT模型中的max_seq_len，即预处理后的样本长度。
生成server端配置文件与模型文件，存放在bert_seq128_model文件夹。
生成client端配置文件，存放在bert_seq128_client文件夹。

方法2：
您也可以从bos上直接下载上述模型（max_seq_len=128），解压后server端配置文件与模型文件存放在bert_chinese_L-12_H-768_A-12_model文件夹，client端配置文件存放在bert_chinese_L-12_H-768_A-12_client文件夹：
```shell
wget https://paddle-serving.bj.bcebos.com/paddle_hub_models/text/SemanticModel/bert_chinese_L-12_H-768_A-12.tar.gz
tar -xzf bert_chinese_L-12_H-768_A-12.tar.gz
mv bert_chinese_L-12_H-768_A-12_model bert_seq128_model
mv bert_chinese_L-12_H-768_A-12_client bert_seq128_client
```
若使用bert_chinese_L-12_H-768_A-12_model模型，将下面命令中的bert_seq128_model字段替换为bert_chinese_L-12_H-768_A-12_model，bert_seq128_client字段替换为bert_chinese_L-12_H-768_A-12_client.




### 获取词典和样例数据

```
sh get_data.sh
```
脚本将下载中文词典vocab.txt和中文样例数据data-c.txt

### 启动RPC预测服务
启动cpu预测服务，执行
```
python -m paddle_serving_server.serve --model bert_seq128_model/ --port 9292  #启动cpu预测服务

```
或者，启动gpu预测服务，执行
```
python -m paddle_serving_server.serve --model bert_seq128_model/ --port 9292 --gpu_ids 0 #在gpu 0上启动gpu预测服务

```

### 执行预测

执行预测前需要安装paddle_serving_app，模块中提供了BERT模型的数据预处理方法。
```
pip install paddle_serving_app
```
执行
```
head data-c.txt | python bert_client.py --model bert_seq128_client/serving_client_conf.prototxt

```
启动client读取data-c.txt中的数据进行预测，预测结果为文本的向量表示（由于数据较多，脚本中没有将输出进行打印），server端的地址在脚本中修改。



### 启动HTTP预测服务
启动cpu HTTP预测服务，执行
```
python bert_web_service.py bert_seq128_model/ 9292 #启动CPU预测服务

```

或者，启动gpu HTTP预测服务，执行
```
 export CUDA_VISIBLE_DEVICES=0,1
```
通过环境变量指定gpu预测服务使用的gpu，示例中指定索引为0和1的两块gpu
```
python bert_web_service_gpu.py bert_seq128_model/ 9292 #启动gpu预测服务
```

### 执行预测

```
curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"words": "hello"}], "fetch":["pooled_output"]}' http://127.0.0.1:9292/bert/prediction
```
