# Benchmark

## C++ Serving 性能测试

**一.测试环境**
- 机器型号：4 × Tesla P4-8GB ，48 core Intel(R) Xeon(R) Gold 5117 @ 2.00GHz
- CUDA：11.0，cuDNN：v8.0.4
- Serving：v0.7.0
- Paddle：v2.2.0
- 模型：ResNet_v2_50
- batch：1
- 使用的测试代码和使用的数据集：[resnet_v2_50](../../examples/C++/PaddleClas/resnet_v2_50)

**二.测试方法**
- 请求数量递增：不断增加 client 数量，指标稳定后统计 client 的耗时信息
- 同步模式：网络线程同步处理，保证显存占用相同的情况下，开启最大线程数
- 异步模式：异步线程处理方式，保证显存占用相同，最大批量为32，异步线程数为2
- 性能对比：
  - 竞品选择：C++ Serving（蓝色） 与 Tenserflow Serving（灰色）都是 C++ 实现，且同为业界主流 Serving 框架
  - 吞吐性能（QPS）：折线图，数值越大表示每秒钟处理的请求数量越大，性能就越好
  - 平均处理时延（ms）：柱状图，数值越大表示单个请求处理时间越长，性能就越差

**三.同步模式**

结论：同步模型默认参数配置情况下，C++ Serving 吞吐和平均时延指标均优于 Tensorflow Serving。

<p align="center">
    <br>
<img src='../images/syn_benchmark.png'">
    <br>
<p>

|client_num |	model_name |	qps(samples/s) |	mean(ms) |	model_name |	qps(samples/s) |	mean(ms) |
| --- | --- | --- | --- | --- | --- | --- |
| 10 |	pd-serving |	111.336 |	89.787|	tf-serving|	84.632|	118.13|
|30	|pd-serving	|165.928	|180.761	|tf-serving	|106.572	|281.473|
|50|	pd-serving|	207.244|	241.211|	tf-serving|	80.002	|624.959|
|70	|pd-serving	|214.769	|325.894	|tf-serving	|105.17	|665.561|
|100|	pd-serving|	235.405|	424.759|	tf-serving|	93.664	|1067.619|
|150	|pd-serving	|239.114	|627.279	|tf-serving	|86.312	|1737.848|

**四.异步模式**

结论：client数据较少时，Tensorflow Serving 性能略优于 C++ Serving ，但当 client 并发数超过70后，Tensorflow Serving 服务出现大量超时，而 C++ Serving 仍能正常运行

<p align="center">
    <br>
<img src='../images/asyn_benchmark.png'">
    <br>
<p>

|client_num |	model_name |	qps(samples/s) |	mean(ms) |	model_name |	qps(samples/s) |	mean(ms) |
| --- | --- | --- | --- | --- | --- | --- |
|10|	pd-serving|	130.631|	76.502|	tf-serving	|172.64	|57.916|
|30|	pd-serving|	201.062|	149.168|	tf-serving|	241.669|	124.128|
|50|	pd-serving|	286.01|	174.764|	tf-serving	|278.744	|179.367|
|70|	pd-serving|	313.58|	223.187|	tf-serving|	298.241|	234.7|
|100|	pd-serving|	323.369|	309.208|	tf-serving|	0|	∞|
|150|	pd-serving|	328.248|	456.933|	tf-serving|	0|	∞|

