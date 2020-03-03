### 使用方法

假设数据文件为test.data，配置文件为inference.conf

单进程client
```
cat test.data | python test_client.py inference.conf > result
```
多进程client，若进程数为4
```
python test_client_multithread.py inference.conf test.data 4 > result
```
batch clienit，若batch size为4
```
cat test.data | python test_client_batch.py inference.conf 4 > result
```

### Benchmark

设备 ：Intel(R) Xeon(R)  Gold 6271 CPU @ 2.60GHz * 48

模型 ：IMDB-CNN

server thread num ： 16

测试中，client共发送2500条测试样本，图中数据为单个线程的耗时，时间单位为秒。可以看出，client端多线程的预测速度相比单线程有明显提升，在16线程时预测速度是单线程的8.7倍。

| client  thread num | prepro | client infer | op0    | op1   | op2    | postpro | total |
| ------------------ | ------ | ------------ | ------ | ----- | ------ | ------- | ----- |
| 1                  | 1.09   | 28.79        | 0.094  | 20.59 | 0.047  | 0.034   | 31.41 |
| 4                  | 0.22   | 7.41         | 0.023  | 5.01  | 0.011  | 0.0098  | 8.01  |
| 8                  | 0.11   | 4.7          | 0.012  | 2.61  | 0.0062 | 0.0049  | 5.01  |
| 12                 | 0.081  | 4.69         | 0.0078 | 1.72  | 0.0042 | 0.0035  | 4.91  |
| 16                 | 0.058  | 3.46         | 0.0061 | 1.32  | 0.0033 | 0.003   | 3.63  |
| 20                 | 0.049  | 3.77         | 0.0047 | 1.03  | 0.0025 | 0.0022  | 3.91  |
| 24                 | 0.041  | 3.86         | 0.0039 | 0.85  | 0.002  | 0.0017  | 3.98  |

预测总耗时变化规律如下：

![total cost](../../../doc/imdb-benchmark-server-16.png)

### Flask WebServe Benchmark使用说明

假设数据文件为test.data，配置文件在serving_server_model 文件夹下

```
python imdb_flask.py serving_server_model
```

多进程client，若进程数为4

```
python flask_client_multithread.py serving_client_conf/serving_client_conf.prototxt test.data 4
```

测试中，client共发送2500条测试样本，图中数据为单个线程的耗时，时间单位为秒。可以看出，client端多线程的预测速度相比单线程有明显提升，但是在4个线程开始，进步幅度就很不明显。

| client  thread num | serving time  | total time    |
| ------------------ | ------------- | ------------- |
| 1                  | 3.28515577316 | 33.2859699726 |
| 2                  | 3.84341430664 | 18.6617088318 |
| 4                  | 5.80341076851 | 11.7521441    |
| 8                  | 6.637332201   | 10.8628261089 |
| 12                 | 6.75784659386 | 11.1788439751 |
| 16                 | 6.51199436188 | 10.9849720001 |
| 20                 | 6.71371102333 | 11.1929299831 |
| 24                 | 6.66537022591 | 11.1105921268 |

为方便复现benchmark，可以直接运行

```
bash flask_benchmark.sh
```
