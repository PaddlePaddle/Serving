# Cube稀疏参数索引量化存储使用指南

(简体中文|[English](./Cube_Quant_EN.md))

## 总体概览

我们在之前的文章中，知道稀疏参数是维度很大的一系列浮点数，而浮点数在计算机中需要4 Byte的存储空间。事实上，我们并不需要很高的浮点数精度就可以实现相当的模型效果，换来大量的空间节约，加快模型的加载速度和查询速度。


## 前序要求

请先读取  [稀疏参数索引服务Cube单机版使用指南](./Cube_Local_CN.md)


##  组件介绍
### seq_generator:
此工具用于把Paddle的模型转换成Sequence File，在这里，我给出了两种模式，第一种是普通模式，生成的KV序列当中的Value以未压缩的浮点数来进行保存。第二种是量化模式，生成的KV序列当中的Value按照 [min, max, bytes]来存储。具体原理请参见 ([Post-Training 4-bit Quantization on Embedding Tables](https://arxiv.org/abs/1911.02079))


##  使用方法

在Serving主目录下，到criteo_ctr_with_cube目录下训练出模型

```
cd Serving/examples/C++/PaddleRec/criteo_ctr_with_cube
python local_train.py # 生成模型
```
接下来可以使用量化和非量化两种方式去生成Sequence File用于Cube稀疏参数索引。
```
seq_generator ctr_serving_model/SparseFeatFactors ./cube_model/feature # 未量化模式
seq_generator ctr_serving_model/SparseFeatFactors ./cube_model/feature 8 #量化模式
```
此命令会讲ctr_serving_model目录下的稀疏参数文件SparseFeatFactors转换为cube_model目录下的feature文件(Sequence File格式)。目前量化工具仅支持8bit量化，未来将支持压缩率更高和种类更多的量化方法。

## 用量化模型启动Serving

在Serving当中，使用general_dist_kv_quant_infer op来进行预测时使用量化模型。具体详见  Serving/examples/C++/PaddleRec/criteo_ctr_with_cube/test_server_quant.py。客户端部分不需要做任何改动。

为方便用户做demo，我们给出了从0开始启动量化模型Serving。
```
cd Serving/examples/C++/PaddleRec/criteo_ctr_with_cube
python local_train.py
cp ../../../build_server/core/predictor/seq_generator seq_generator
cp ../../../build_server/output/bin/cube* ./cube/
sh cube_quant_prepare.sh &
python test_server_quant.py ctr_serving_model_kv &
python test_client.py ctr_client_conf/serving_client_conf.prototxt ./raw_data
```

用户可以将量化后的AUC结果同量化前的AUC做比较
