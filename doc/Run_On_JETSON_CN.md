## Paddle Serving使用JETSON部署

Paddle Serving支持使用JETSON进行预测部署。目前仅支持Pipeline模式。

### 安装PaddlePaddle

可以参考[NV Jetson部署示例](https://paddleinference.paddlepaddle.org.cn/demo_tutorial/cuda_jetson_demo.html) 安装python版本的paddlepaddle


### 安装PaddleServing

安装ARM版本的whl包
```
# paddle-serving-server
https://paddle-serving.bj.bcebos.com/whl/xpu/arm/paddle_serving_server_xpu-0.0.0.post2-py3-none-any.whl
# paddle-serving-client
https://paddle-serving.bj.bcebos.com/whl/xpu/arm/paddle_serving_client-0.0.0-cp36-none-any.whl
# paddle-serving-app
https://paddle-serving.bj.bcebos.com/whl/xpu/arm/paddle_serving_app-0.0.0-py3-none-any.whl
```

### 部署使用

以[Uci](../examples/Pipeline/simple_web_service/README_CN.md)为例

启动服务
```
python3 web_service.py &>log.txt &
```
其中修改config.yml中的对应配置项
```
            #计算硬件类型: 空缺时由devices决定(CPU/GPU)，0=cpu, 1=gpu, 2=tensorRT, 3=arm cpu, 4=kunlun xpu
            device_type: 1

            #计算硬件ID，优先由device_type决定硬件类型。devices为""或空缺时为CPU预测；当为"0", "0,1,2"时为GPU预测，表示使用的GPU卡
            devices: "0,1"
```

## 其他说明

### Jetson支持相关参考资料
* [Jetson运行飞桨](https://paddleinference.paddlepaddle.org.cn/demo_tutorial/cuda_jetson_demo.html)