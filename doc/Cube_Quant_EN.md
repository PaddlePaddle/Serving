# Quantization Storage on Cube Sparse Parameter Indexing

([简体中文](./Cube_Quant_CN.md)|English)

## Overview

In our previous article, we know that the sparse parameter is a series of floating-point numbers with large dimensions, and floating-point numbers require 4 Bytes of storage space in the computer. In fact, we don't need very high precision of floating point numbers to achieve a comparable model effect, in exchange for a lot of space savings, speeding up model loading and query speed.

## Precondition

Please Read  [Cube: Sparse Parameter Indexing Service (Local Mode)](./Cube_Local_EN.md)


##  Components
### seq_generator:
This tool is used to convert the Paddle model into a Sequence File. Here, two modes are given. The first is the normal mode. The value in the generated KV sequence is saved as an uncompressed floating point number. The second is the quantization mode. The Value in the generated KV sequence is stored according to [min, max, bytes]. See the specific principle ([Post-Training 4-bit Quantization on Embedding Tables](https://arxiv.org/abs/1911.02079))


##  Usage

In Serving Directory，train the model in the criteo_ctr_with_cube directory

```
cd Serving/examples/C++/PaddleRec/criteo_ctr_with_cube
python local_train.py # save model
```
Next, you can use quantization and non-quantization to generate Sequence File for Cube sparse parameter indexing.

```
seq_generator ctr_serving_model/SparseFeatFactors ./cube_model/feature # naive mode
seq_generator ctr_serving_model/SparseFeatFactors ./cube_model/feature 8 #quantization
```
This command will convert the sparse parameter file SparseFeatFactors in the ctr_serving_model directory into a feature file (Sequence File format) in the cube_model directory. At present, the quantization tool only supports 8-bit quantization. In the future, it will support higher compression rates and more types of quantization methods.

## Launch Serving by Quantized Model

In Serving, a quantized model is used when using general_dist_kv_quant_infer op to make predictions. See Serving/examples/C++/PaddleRec/criteo_ctr_with_cube/test_server_quant.py for details. No changes are required on the client side.

In order to make the demo easier for users, the following script is to train the quantized criteo ctr model and launch serving by it.
```
cd Serving/examples/C++/PaddleRec/criteo_ctr_with_cube
python local_train.py
cp ../../../build_server/core/predictor/seq_generator seq_generator
cp ../../../build_server/output/bin/cube* ./cube/
sh cube_quant_prepare.sh &
python test_server_quant.py ctr_serving_model_kv &
python test_client.py ctr_client_conf/serving_client_conf.prototxt ./raw_data
```

Users can compare AUC results after quantization with AUC before quantization. 
