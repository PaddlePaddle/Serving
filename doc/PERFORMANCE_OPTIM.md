# Performance Optimization

([简体中文](./PERFORMANCE_OPTIM_CN.md)|English)

Due to different model structures, different prediction services consume different computing resources when performing predictions. For online prediction services, models that require less computing resources will have a higher proportion of communication time cost, which is called communication-intensive service. Models that require more computing resources have a higher time cost for inference calculations, which is called computation-intensive services.

For a prediction service, the easiest way to determine the type of service is to look at the time ratio. Paddle Serving provides [Timeline tool](../python/examples/util/README_CN.md), which can intuitively display the time spent in each stage of the prediction service.

For communication-intensive prediction services, requests can be aggregated, and within a limit that can tolerate delay, multiple prediction requests can be combined into a batch for prediction.

For computation-intensive prediction services, you can use GPU prediction services instead of CPU prediction services, or increase the number of graphics cards for GPU prediction services.

Under the same conditions, the communication time of the HTTP prediction service provided by Paddle Serving is longer than that of the RPC prediction service, so for communication-intensive services, please give priority to using RPC communication.

Parameters for performance optimization:

The memory/graphic memory optimization option is enabled by default in Paddle Serving, which can reduce the memory/video memory usage and usually does not affect performance. If you need to turn it off, you can use --mem_optim_off in the command line.

r_optim can optimize the calculation graph and increase the inference speed. It is turned off by default and turned on by --ir_optim in the command line.

| Parameters | Type | Default | Description                                                  |
| ---------- | ---- | ------- | ------------------------------------------------------------ |
| mem_optim_off  | - | - | Disable memory / graphic memory optimization                                   |
| ir_optim   | - | -  | Enable analysis and optimization of calculation graph,including OP fusion, etc |


For the mode of using Python code to start the prediction service, the API of the above two parameters is as follows:

RPC Service
```
from paddle_serving_server import Server
server = Server()
...
server.set_memory_optimize(mem_optim)
server.set_ir_optimize(ir_optim)
...
```

HTTP Service
```
from paddle_serving_server import WebService
class NewService(WebService):
...
new_service = NewService(name="new")
...
new_service.prepare_server(mem_optim=True, ir_optim=False)
...
```
