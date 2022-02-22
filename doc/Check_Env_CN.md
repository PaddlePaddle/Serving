# Paddle Serving 环境检查功能介绍

## 概览
Paddle Serving 提供了一键运行示例，检查 Paddle Serving 环境是否安装正确。


## 启动方式
```
python3 -m paddle_serving_server.serve check
```

|命令|描述|
|---------|----|
|check_all|检查 Paddle Inference、Pipeline Serving、C++ Serving。只打印检测结果，不记录日志|
|check_pipeline|检查 Pipeline Serving，只打印检测结果，不记录日志|
|check_cpp|检查 C++ Serving，只打印检测结果，不记录日志|
|check_inference|检查 Paddle Inference 是否安装正确，只打印检测结果，不记录日志|
|debug|发生报错后，该命令将打印提示日志到屏幕，并记录详细日志文件|
|exit|退出|
>> **注意**:<br>
>> 1.当 C++ Serving 启动报错且是自己编译后 pip 安装的paddle_serving_server, 确认是否执行 `export SERVING_BIN` 导入`SERVING_BIN`真实路径。<br>
>> 2.可以通过 `export SERVING_LOG_PATH` 指定`debug`命令生成log的路径，默认是在当前路径下记录日志。
