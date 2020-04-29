## IMDB comment sentiment inference service

([简体中文](./README_CN.md)|English)

### Get model files and sample data

```
sh get_data.sh
```
the package downloaded contains cnn, lstm and bow model config along with their test_data and train_data.

### Start RPC inference service

```
python -m paddle_serving_server.serve --model imdb_cnn_model/ --port 9292
```
### RPC Infer
```
head test_data/part-0 | python test_client.py imdb_cnn_client_conf/serving_client_conf.prototxt imdb.vocab
```

it will get predict results of the first 10 test cases.

### Start HTTP inference service
```
python text_classify_service.py imdb_cnn_model/ workdir/ 9292 imdb.vocab
```
### HTTP Infer

```
curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"words": "i am very sad | 0"}], "fetch":["prediction"]}' http://127.0.0.1:9292/imdb/prediction
```

### Benchmark

CPU ：Intel(R) Xeon(R)  Gold 6271 CPU @ 2.60GHz * 48

Model ：[CNN](https://github.com/PaddlePaddle/Serving/blob/develop/python/examples/imdb/nets.py)

server thread num ： 16

In this test, client sends 25000 test samples totally, the bar chart given later is the latency of single thread, the unit is second, from which we know the predict efficiency is improved greatly by multi-thread compared to single-thread. 8.7 times improvement is made by 16 threads prediction.

| client  thread num | prepro | client infer | op0    | op1   | op2    | postpro | total |
| ------------------ | ------ | ------------ | ------ | ----- | ------ | ------- | ----- |
| 1                  | 1.09   | 28.79        | 0.094  | 20.59 | 0.047  | 0.034   | 31.41 |
| 4                  | 0.22   | 7.41         | 0.023  | 5.01  | 0.011  | 0.0098  | 8.01  |
| 8                  | 0.11   | 4.7          | 0.012  | 2.61  | 0.0062 | 0.0049  | 5.01  |
| 12                 | 0.081  | 4.69         | 0.0078 | 1.72  | 0.0042 | 0.0035  | 4.91  |
| 16                 | 0.058  | 3.46         | 0.0061 | 1.32  | 0.0033 | 0.003   | 3.63  |
| 20                 | 0.049  | 3.77         | 0.0047 | 1.03  | 0.0025 | 0.0022  | 3.91  |
| 24                 | 0.041  | 3.86         | 0.0039 | 0.85  | 0.002  | 0.0017  | 3.98  |

The thread-latency bar chart is as follow：

![total cost](../../../doc/imdb-benchmark-server-16.png)
