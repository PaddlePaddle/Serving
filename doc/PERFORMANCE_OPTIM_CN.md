# 性能优化

(简体中文|[English](./PERFORMANCE_OPTIM.md))

由于模型结构的不同，在执行预测时不同的预测服务对计算资源的消耗也不相同。对于在线的预测服务来说，对计算资源要求较少的模型，通信的时间成本占比就会较高，称为通信密集型服务，对计算资源要求较多的模型，推理计算的时间成本较高，称为计算密集型服务。对于这两种服务类型，可以根据实际需求采取不同的方式进行优化

对于一个预测服务来说，想要判断属于哪种类型，最简单的方法就是看时间占比，Paddle Serving提供了[Timeline工具](../python/examples/util/README_CN.md)，可以直观的展现预测服务中各阶段的耗时。

对于通信密集型的预测服务，可以将请求进行聚合，在对延时可以容忍的限度内，将多个预测请求合并成一个batch进行预测。

对于计算密集型的预测服务，可以使用GPU预测服务代替CPU预测服务，或者增加GPU预测服务的显卡数量。

在相同条件下，Paddle Serving提供的HTTP预测服务的通信时间是大于RPC预测服务的，因此对于通信密集型的服务请优先考虑使用RPC的通信方式。

性能优化相关参数：

Paddle Serving中默认开启内存/显存优化选项，可以减少对内存/显存的占用，通常不会对性能造成影响，如果需要关闭可以在命令行启动模式中使用--mem_optim_off。
ir_optim可以优化计算图，提升推理速度，默认关闭，在命令行启动的模式中通过--ir_optim开启。

| 参数      | 类型 | 默认值 | 含义                      |
| --------- | ---- | ------ | -------------------------------- |
| mem_optim_off | - | -  | 关闭内存/显存优化                |
| ir_optim  | - | -  | 开启计算图分析优化，包括OP融合等 |


对于使用Python代码启动预测服务的模式，以上两个参数的接口如下：
RPC服务
```
from paddle_serving_server import Server
server = Server()
...
server.set_memory_optimize(mem_optim)
server.set_ir_optimize(ir_optim)
...
```

HTTP服务
```
from paddle_serving_server import WebService
class NewService(WebService):
...
new_service = NewService(name="new")
...
new_service.prepare_server(mem_optim=True, ir_optim=False)
...
```
