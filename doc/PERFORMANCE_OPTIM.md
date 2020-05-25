# Performance Optimization

([简体中文](./PERFORMANCE_OPTIM_CN.md)|English)

Due to different model structures, different prediction services consume different computing resources when performing predictions. For online prediction services, models that require less computing resources will have a higher proportion of communication time cost, which is called communication-intensive service. Models that require more computing resources have a higher time cost for inference calculations, which is called computationa-intensive services.

For a prediction service, the easiest way to determine what type it is is to look at the time ratio. Paddle Serving provides [Timeline tool](../python/examples/util/README_CN.md), which can intuitively display the time spent in each stage of the prediction service.

For communication-intensive prediction services, requests can be aggregated, and within a limit that can tolerate delay, multiple prediction requests can be combined into a batch for prediction.

For computation-intensive prediction services, you can use GPU prediction services instead of CPU prediction services, or increase the number of graphics cards for GPU prediction services.

Under the same conditions, the communication time of the HTTP prediction service provided by Paddle Serving is longer than that of the RPC prediction service, so for communication-intensive services, please give priority to using RPC communication.

Parameters for performance optimization:

| Parameters | Type | Default | Description                                                  |
| ---------- | ---- | ------- | ------------------------------------------------------------ |
| mem_optim  | bool | False   | Enable memory / graphic memory optimization                                   |
| ir_optim   | bool | Fasle   | Enable analysis and optimization of calculation graph,including OP fusion, etc |
