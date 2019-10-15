Paddle Serving的CTR预估任务会定期将访问大规模稀疏参数服务的响应时间等统计信息打印出来。用户在k8s集群一键部署好分布式训练+Serving方案后，可以在容器内通过CTR预估任务demo观察serving访问稀疏参数服务的响应时间等信息。具体的观察方法如下：

## 使用CTR预估任务客户端ctr_prediction向Serving发送批量请求

因Serving端每1000个请求打印一次请求，为了观察输出结果，需要客户端向serving端发送较大量请求。具体做法：

```bash
# 进入pdservingclient pod
$ kubectl exec -ti pdservingclient /bin/bash

# 以下命令在pdservingclient这个pod内执行
$ cd client/ctr_prediction/
$ bin/ctr_prediction --enable_profiling --concurrency=4 --repeat=100
```

## Serving端日志

```bash
# 进入Serving端pod
$ kubectl exec -ti paddleserving /bin/bash

# 以下命令在Serving pod内执行
$ grep 'Cube request count' log/serving.INFO -A 5 | more
```

示例输出：
```
I1014 12:57:20.699606    38 ctr_prediction_op.cpp:163] Cube request count: 1000
I1014 12:57:20.699631    38 ctr_prediction_op.cpp:164] Cube request key count: 1300000
I1014 12:57:20.699645    38 ctr_prediction_op.cpp:165] Cube request total time: 1465704us
I1014 12:57:20.699666    38 ctr_prediction_op.cpp:166] Average 1465.7us/req
I1014 12:57:20.699692    38 ctr_prediction_op.cpp:169] Average 1.12746us/key
```

## 说明

Paddle Serving实例中打印出的访问cube的平响时间，与[cube社区版本性能报告](https://github.com/PaddlePaddle/Serving/blob/develop/cube/doc/performance.md)中报告的性能数值有差别，影响Paddle Serving访问cube的因素：

1) 批量查询每个请求中key的个数

从日志可看到，Paddle Serving接收到CTR预估任务client发送的请求中，平均每个请求批量key的个数为1300，而性能报告中实测的有批量1个key、100个key和1000个key等

2) Serving实例所在机器CPU核数和客户端请求并发数

假设Paddle Serving所在云服务器上CPU核数为4，则Paddle Serving本身默认会启动4个worker线程。在client端发送4个并发情况下，Serving端约为占满4个CPU核。但由于Serving又要启动新的channel/thread来访问cube（采用的是异步模式），这些和Serving本身的server端代码共用bthread资源，就会出现竞争的情况。

以下是在CTR预估任务Client端向Serving发送不同并发请求数时，访问cube的平均响应时间 (1300key/req，分片数=1)

线程数 | 访问cube的平均响应时间 (us)
-------|-------
1 | 1465
2 | 1480
3 | 1450
4 | 1905

可以看到，当并发数等于（和大于）CPU核数时，访问cube的响应时间就会变大。

3) 稀疏参数字典分片数

假设分片数为N，每次cube访问，都会生成N个channel，每个来对应一个分片的请求，这些channel和Serving内其他工作线程共用bthread资源。但同时，较多的分片数，每个分片参数服务节点上查询的计算量会变小，使得总体响应时间变小。

以下是同一份词典分成1个分片和2个分片，serving端访问cube的平均响应时间 (1300key/req)

分片数 | 访问cube的平均响应时间 (us)
-------|--------------------------
1 | 1680
2 | 1450

4) 网络环境

百度云平台上机器间ping的时延平均为0.3ms - 0.5ms，在batch为1000个key时，平均响应时间为1312us \[[注1](https://github.com/PaddlePaddle/Serving/blob/master/doc/ELASTIC_CTR.md#annotation_1)\]

Paddle Serving发布的[cube社区版本性能报告](https://github.com/PaddlePaddle/Serving/blob/develop/cube/doc/performance.md)中测试环境为裸机部署，给出的机器间ping时延为0.06ms，在batch为1000个key时，平均响应时间为675us/req

两种环境的主要差别在于：

(1) 机器间固有的通信延迟 (百度云上位0.3ms-0.5ms，性能报告测试环境0.06ms)

(2) 字典分片数 (百度云上2个分片，性能报告测试环境10个分片)

