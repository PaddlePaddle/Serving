
# Getting Started

请先按照[编译安装说明](INSTALL.md)完成编译

## 运行示例
说明：Imagenet图像分类模型，默认采用CPU模式（GPU模式当前版本暂未提供支持）

Step1：启动Server端：
```shell
cd /path/to/paddle-serving/output/demo/serving/ && ./bin/serving &
```

默认启动后日志写在./log/下，可tail日志查看serving端接收请求的日志：
```shell
tail -f log/serving.INFO
```

Step2：启动Client端：
```shell
cd path/to/paddle-serving/output/demo/client/image_class &&  ./bin/ximage &
```

默认启动后日志写在./log/下，可tail日志查看分类结果：
```shell
tail -f log/ximage.INFO
```
