# In-batch Negatives

 **目录**

* [模型下载](#模型下载)
* [模型部署](#模型部署)


<a name="模型下载"></a>

## 1. 语义索引模型

**语义索引训练模型下载链接：**

以下模型结构参数为: `TrasformerLayer:12, Hidden:768, Heads:12, OutputEmbSize: 256`

|Model|训练参数配置|硬件|MD5|
| ------------ | ------------ | ------------ |-----------|
|[batch_neg](https://bj.bcebos.com/v1/paddlenlp/models/inbatch_model.zip)|<div style="width: 150pt">margin:0.2 scale:30 epoch:3 lr:5E-5 bs:64 max_len:64 </div>|<div style="width: 100pt">4卡 v100-16g</div>|f3e5c7d7b0b718c2530c5e1b136b2d74|

```
wget https://bj.bcebos.com/v1/paddlenlp/models/inbatch_model.zip
unzip inbatch_model.zip -d checkpoints
```

<a name="模型部署"></a>

## 2. 模型部署

### 2.1 动转静导出

首先把动态图模型转换为静态图：

```
python export_model.py --params_path checkpoints/model_40/model_state.pdparams --output_path=./output
```
也可以运行下面的bash脚本：

```
sh scripts/export_model.sh
```

### 2.2 Paddle Inference预测

预测既可以抽取向量也可以计算两个文本的相似度。

修改id2corpus的样本：

```
# 抽取向量
id2corpus={0:'国有企业引入非国有资本对创新绩效的影响——基于制造业国有上市公司的经验证据'}
# 计算相似度
corpus_list=[['中西方语言与文化的差异','中西方文化差异以及语言体现中西方文化,差异,语言体现'],
                    ['中西方语言与文化的差异','飞桨致力于让深度学习技术的创新与应用更简单']]

```

然后使用PaddleInference

```
python deploy/python/predict.py --model_dir=./output
```
也可以运行下面的bash脚本：

```
sh deploy.sh
```
最终输出的是256维度的特征向量和句子对的预测概率：

```
(1, 256)
[[-0.0394925  -0.04474756 -0.065534    0.00939134  0.04359895  0.14659195
  -0.0091779  -0.07303623  0.09413272 -0.01255222 -0.08685658  0.02762237
   0.10138468  0.00962821  0.10888419  0.04553023  0.05898942  0.00694253
   ....

[0.959269642829895, 0.04725276678800583]
```

### 2.3 Paddle Serving部署

Paddle Serving 的详细文档请参考 [Pipeline_Design](https://github.com/PaddlePaddle/Serving/blob/v0.7.0/doc/Python_Pipeline/Pipeline_Design_CN.md)和[Serving_Design](https://github.com/PaddlePaddle/Serving/blob/v0.7.0/doc/Serving_Design_CN.md),首先把静态图模型转换成Serving的格式：

```
python export_to_serving.py \
    --dirname "output" \
    --model_filename "inference.get_pooled_embedding.pdmodel" \
    --params_filename "inference.get_pooled_embedding.pdiparams" \
    --server_path "./serving_server" \
    --client_path "./serving_client" \
    --fetch_alias_names "output_embedding"

```

参数含义说明
* `dirname`: 需要转换的模型文件存储路径，Program 结构文件和参数文件均保存在此目录。
* `model_filename`： 存储需要转换的模型 Inference Program 结构的文件名称。如果设置为 None ，则使用 `__model__` 作为默认的文件名
* `params_filename`: 存储需要转换的模型所有参数的文件名称。当且仅当所有模型参数被保>存在一个单独的二进制文件中，它才需要被指定。如果模型参数是存储在各自分离的文件中，设置它的值为 None
* `server_path`: 转换后的模型文件和配置文件的存储路径。默认值为 serving_server
* `client_path`: 转换后的客户端配置文件存储路径。默认值为 serving_client
* `fetch_alias_names`: 模型输出的别名设置，比如输入的 input_ids 等，都可以指定成其他名字，默认不指定
* `feed_alias_names`: 模型输入的别名设置，比如输出 pooled_out 等，都可以重新指定成其他模型，默认不指定

也可以运行下面的 bash 脚本：
```
sh scripts/export_to_serving.sh
```

Paddle Serving的部署有两种方式，第一种方式是Pipeline的方式，第二种是C++的方式，下面分别介绍这两种方式的用法：

#### 2.3.1 Pipeline方式

启动 Pipeline Server:

```
python web_service.py
```

启动客户端调用 Server。

首先修改rpc_client.py中需要预测的样本：

```
list_data = [
    "国有企业引入非国有资本对创新绩效的影响——基于制造业国有上市公司的经验证据",
    "试论翻译过程中的文化差异与语言空缺翻译过程,文化差异,语言空缺,文化对比"
]
```
然后运行：

```
python rpc_client.py
```
模型的输出为：

```
{'0': '国有企业引入非国有资本对创新绩效的影响——基于制造业国有上市公司的经验证据', '1': '试论翻译过程中的文化差异与语言空缺翻译过程,文化差异,语言空缺,文化对比'}
PipelineClient::predict pack_data time:1641450851.3752182
PipelineClient::predict before time:1641450851.375738
['output_embedding']
(2, 256)
[[ 0.07830612 -0.14036864  0.03433796 -0.14967982 -0.03386067  0.06630666
   0.01357943  0.03531194  0.02411093  0.02000859  0.05724002 -0.08119463
   ......
```

可以看到客户端发送了2条文本，返回了2个 embedding 向量

#### 2.3.2 C++的方式

启动C++的Serving：

```
python -m paddle_serving_server.serve --model serving_server --port 9393 --gpu_id 2 --thread 5 --ir_optim True --use_trt --precision FP16
```
也可以使用脚本：

```
sh deploy/C++/start_server.sh
```
Client 可以使用 http 或者 rpc 两种方式，rpc 的方式为：

```
python deploy/C++/rpc_client.py
```
运行的输出为：
```
I0209 20:40:07.978225 20896 general_model.cpp:490] [client]logid=0,client_cost=395.695ms,server_cost=392.559ms.
time to cost :0.3960278034210205 seconds
{'output_embedding': array([[ 9.01343748e-02, -1.21870913e-01,  1.32834800e-02,
        -1.57673359e-01, -2.60387752e-02,  6.98455423e-02,
         1.58108603e-02,  3.89952064e-02,  3.22783105e-02,
         3.49135026e-02,  7.66086206e-02, -9.12970975e-02,
         6.25643134e-02,  7.21886680e-02,  7.03565404e-02,
         5.44054210e-02,  3.25332815e-03,  5.01751155e-02,
......
```
可以看到服务端返回了向量

或者使用 http 的客户端访问模式：

```
python deploy/C++/http_client.py
```
运行的输出为：

```
(2, 64)
(2, 64)
outputs {
  tensor {
    float_data: 0.09013437479734421
    float_data: -0.12187091261148453
    float_data: 0.01328347995877266
    float_data: -0.15767335891723633
......
```
可以看到服务端返回了向量



