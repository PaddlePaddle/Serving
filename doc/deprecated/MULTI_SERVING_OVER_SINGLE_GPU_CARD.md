# Multiple Serving Instances over Single GPU Card

Paddle Serving依托PaddlePaddle预测库执行实际的预测计算。由于当前GPU预测库的限制，单个Serving实例只可以绑定1张GPU卡，且进程内所有worker线程共用1个GPU stream。也就是说，不管Serving启动多少个worker线程，所有的请求在GPU是严格串行计算的，起不到加速作用。这会带来一个问题，就是如果模型计算量不大，那么Serving进程实际上不会用满GPU的算力。

为了充分利用GPU卡的算力，考虑在单张卡上启动多个Serving实例，通过多个GPU stream，力争用满GPU的算力。启动命令可以如下所示：

```
bin/serving --gpuid=0 --bthread_concurrency=4 --bthread_min_concurrency=4 --port=8010&
bin/serving --gpuid=0 --bthread_concurrency=4 --bthread_min_concurrency=4 --port=8011&
```

上述2条命令，启动2个Serving实例，分别监听8010端口和8011端口。但他们都绑定同一张卡 (gpuid = 0)。

命令行参数含义：
```
-gpuid=N：用于指定所绑定的GPU卡ID
-bthread_concurrency和bthread_min_concurrency共同限制该进程启动的worker数：由于在GPU预测模式下，增加worker线程数并不能提高并发能力，为了节省部分资源，干脆将他们限制掉；均设为4，是因为这是bthread允许的最小值。
-port xxx：Serving实例监听的端口
```

但是，上述方式究竟是否能在不影响响应时间等其他指标的前提下，起到提高GPU使用率作用，受到多个限制因素的制约，具体的：

1. 单个stream占用GPU算力；假如单个stream已经将GPU算力占用超过50%，那么增加stream很可能会导致2个stream的job分别排队，拖慢各自的响应时间
2. GPU显存：Serving进程需要将模型参数加载到显存中，并且计算时要在GPU显存池分配临时变量；假如单个Serving进程已经用掉超过50%的显存，则增加Serving进程会造成显存不足，导致进程报错退出

为此，可采用如下步骤，进行测试：

1. 加载模型时，在model_toolkit.prototxt中，model type选择FLUID_GPU_ANALYSIS或FLUID_GPU_ANALYSIS_DIR；会对模型进行静态分析，进行一定程度显存优化
2. 在步骤1完成后，启动单个Serving进程，启动参数:`--gpuid=N --bthread_concurrency=4 --bthread_min_concurrency=4`；启动一个client，进行并发度为1的压力测试，batch size从小到大，记下平响；由于算力的限制，当batch size增大到一定程度，应该会出现响应时间明显变大；或虽然没有明显变大，但已经不满足系统需求
3. 再启动1个Serving进程，与步骤2启动时使用相同的参数略有不同: `--gpuid=N --bthread_concurrency=4 --bthread_min_concurrency=4 --port=8011` 其中--port=8011用来让新启动的进程使用一个新的服务端口；然后同时对这2个Serving进程进行压测，继续观察batch size从小到大时平均响应时间的变化，直到取得batch size和响应时间的折中
4. 重复步骤2-3
5. 以2-4步的测试，来决定：单张GPU卡可以由多少个Serving进程共用; 实际部署时，就在一张GPU卡上启动这么多个Serving进程同时提供服务
