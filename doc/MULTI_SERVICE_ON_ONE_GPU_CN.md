# 单卡多模型预测服务

当客户端发送的请求数并不频繁的情况下，会造成服务端机器计算资源尤其是GPU资源的浪费，这种情况下，可以在服务端启动多个预测服务来提高资源利用率。Paddle Serving支持在单张显卡上部署多个预测服务，使用时只需要在启动单个服务时通过--gpu_ids参数将服务与显卡进行绑定，这样就可以将多个服务都绑定到同一张卡上。

例如：

```shell
python -m paddle_serving_server_gpu.serve --model bert_seq20_model --port 9292 --gpu_ids 0
python -m paddle_serving_server_gpu.serve --model ResNet50_vd_model --port 9393 --gpu_ids 0
```

在卡0上，同时部署了bert示例和iamgenet示例。

**注意：** 单张显卡内部进行推理计算时仍然为串行计算，这种方式是为了减少server端显卡的空闲时间。
 
