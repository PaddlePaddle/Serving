# FAQ
## 1. 如何修改端口配置？
使用该框架搭建的服务需要申请一个端口，可以通过以下方式修改端口号：

- 如果在inferservice_file里指定了port:xxx，那么就去申请该端口号；
- 否则，如果在gflags.conf里指定了--port:xxx，那就去申请该端口号；
- 否则，使用程序里指定的默认端口号：8010。

## 2. GPU预测中为何请求的响应时间波动会非常大？
PaddleServing依托PaddlePaddle预测库执行预测计算；在GPU设备上，由于同一个进程内目前共用1个GPU stream，进程内的多个请求的预测计算会被严格串行。所以如果有2个请求同时到达某个Serving实例，不管该实例启动时创建了多少个worker线程，都不能起到加速作用，后到的请求会被排队，直到前面请求计算完成。

## 3. 如何充分利用GPU卡的计算能力？
如问题2所说，由于预测库的限制，单个Serving进程只能绑定单张GPU卡，且进程内共用1个GPU stream，所有请求必须串行计算。

为提高GPU卡使用率，目前可以想到的方法是：在单张GPU卡上启动多个Serving进程，每个进程绑定一个GPU stream，多个stream并行计算。这种方法是否能起到加速作用，受限于多个因素，主要有：

1. 单个stream占用GPU算力；假如单个stream已经将GPU算力占用超过50%，那么增加stream很可能会导致2个stream的job分别排队，拖慢各自的响应时间
2. GPU显存：Serving进程需要将模型参数加载到显存中，并且计算时要在GPU显存池分配临时变量；假如单个Serving进程已经用掉超过50%的显存，则增加Serving进程会造成显存不足，导致进程报错退出

为此，可采用如下步骤，进行测试：

1. 加载模型时，在model_toolkit.prototxt中，model type选择FLUID_GPU_ANALYSIS或FLUID_GPU_ANALYSIS_DIR；会对模型进行静态分析，进行一定程度显存优化
2. 在步骤1完成后，启动单个Serving进程，启动参数:`--gpuid=N --bthread_concurrency=4 --bthread_min_concurrency=4`；启动一个client，进行并发度为1的压力测试，batch size从小到大，记下平响；由于算力的限制，当batch size增大到一定程度，应该会出现响应时间明显变大；或虽然没有明显变大，但已经不满足系统需求
3. 再启动1个Serving进程，与步骤2启动时使用相同的参数略有不同: `--gpuid=N --bthread_concurrency=4 --bthread_min_concurrency=4 --port=8011` 其中--port=8011用来让新启动的进程使用一个新的服务端口；然后同时对这2个Serving进程进行压测，继续观察batch size从小到大时平均响应时间的变化，直到取得batch size和响应时间的折中
4. 重复步骤2-3
5. 以2-4步的测试，来决定：单张GPU卡可以由多少个Serving进程共用; 实际部署时，就在一张GPU卡上启动这么多个Serving进程同时提供服务
