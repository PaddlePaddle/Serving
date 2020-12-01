# FAQ



## 基础知识

#### Q: Paddle Serving 、Paddle Inference、PaddleHub Serving三者的区别及联系？

**A:** paddle serving是远程服务，即发起预测的设备（手机、浏览器、客户端等）与实际预测的硬件不在一起。	paddle inference是一个library，适合嵌入到一个大系统中保证预测效率，paddle serving调用了paddle       inference做远程服务。paddlehub serving可以认为是一个示例，都会使用paddle serving作为统一预测服务入口。如果在web端交互，一般是调用远程服务的形式，可以使用paddle serving的web service搭建。

#### Q: paddle-serving是否支持Int32支持

**A:** 在protobuf定feed_type和fetch_type编号与数据类型对应如下

​     0-int64

​	  1-float32

​	  2-int32

#### Q: paddle-serving是否支持windows和Linux环境下的多线程调用 

**A:** 客户端可以发起多线程访问调用服务端 

#### Q: paddle-serving如何修改消息大小限制

**A:** 在server端和client但通过FLAGS_max_body_size来扩大数据量限制，单位为字节，默认为64MB

#### Q: paddle-serving客户端目前支持哪些语言

**A:** java c++ python 

#### Q: paddle-serving目前支持哪些协议

**A:** http rpc 


## 编译问题

#### Q: 如何使用自己编译的Paddle Serving进行预测？

**A:** 通过pip命令安装自己编译出的whl包，并设置SERVING_BIN环境变量为编译出的serving二进制文件路径。

#### Q: 使用Java客户端，mvn compile过程出现"No compiler is provided in this environment. Perhaps you are running on a JRE rather than a JDK?"错误

**A:** 没有安装JDK，或者JAVA_HOME路径配置错误（正确配置是JDK路径，常见错误配置成JRE路径，例如正确路径参考JAVA_HOME="/usr/lib/jvm/java-1.8.0-openjdk-1.8.0.262.b10-0.el7_8.x86_64/"）。Java JDK安装参考https://segmentfault.com/a/1190000015389941



## 部署问题

#### Q: GPU环境运行Serving报错，GPU count is: 0。

```
terminate called after throwing an instance of 'paddle::platform::EnforceNotMet'
what():
--------------------------------------------
C++ Call Stacks (More useful to developers):
--------------------------------------------
0   std::string paddle::platform::GetTraceBackString<std::string const&>(std::string const&, char const*, int)
1   paddle::platform::SetDeviceId(int)
2   paddle::AnalysisConfig::fraction_of_gpu_memory_for_pool() const
3   std::unique_ptr<paddle::PaddlePredictor, std::default_delete<paddle::PaddlePredictor> > paddle::CreatePaddlePredictor<paddle::AnalysisConfig, (paddle::PaddleEngineKind)2>(paddle::AnalysisConfig const&)
4   std::unique_ptr<paddle::PaddlePredictor, std::default_delete<paddle::PaddlePredictor> > paddle::CreatePaddlePredictor<paddle::AnalysisConfig>(paddle::AnalysisConfig const&)
----------------------
Error Message Summary:
----------------------
InvalidArgumentError: Device id must be less than GPU count, but received id is: 0. GPU count is: 0.
[Hint: Expected id < GetCUDADeviceCount(), but received id:0 >= GetCUDADeviceCount():0.] at (/home/scmbuild/workspaces_cluster.dev/baidu.lib.paddlepaddle/baidu/lib/paddlepaddle/Paddle/paddle/fluid/platform/gpu_info.cc:211)
```

**A:** libcuda.so没有链接成功。首先在机器上找到libcuda.so，ldd检查libnvidia版本与nvidia-smi中版本一致（libnvidia-fatbinaryloader.so.418.39，与NVIDIA-SMI 418.39 Driver Version: 418.39）,然后用export导出libcuda.so的路径即可（例如libcuda.so在/usr/lib64/，export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib64/）

#### Q: 遇到 GPU not found, please check your environment or use cpu version by "pip install paddle_serving_server"

**A:** 检查环境中是否有N卡：ls /dev/ | grep nvidia

#### Q: 目前Paddle Serving支持哪些镜像环境？

**A:** 目前（0.4.0）仅支持CentOS，具体列表查阅[这里](https://github.com/PaddlePaddle/Serving/blob/develop/doc/DOCKER_IMAGES.md)

#### Q: python编译的GCC版本与serving的版本不匹配

**A:**:1)使用[GPU docker](https://github.com/PaddlePaddle/Serving/blob/develop/doc/RUN_IN_DOCKER.md#gpunvidia-docker)解决环境问题

​	   2)修改anaconda的虚拟环境下安装的python的gcc版本[参考](https://www.jianshu.com/p/c498b3d86f77) 

#### Q: paddle-serving是否支持本地离线安装 

**A:** 支持离线部署，需要把一些相关的[依赖包](https://github.com/PaddlePaddle/Serving/blob/develop/doc/COMPILE.md)提前准备安装好

## 预测问题

#### Q: 使用GPU第一次预测时特别慢，如何调整RPC服务的等待时间避免超时？ 

**A:** GPU第一次预测需要初始化。使用set_rpc_timeout_ms设置更长的等待时间，单位为毫秒，默认时间为20秒。

示例：

```
from paddle_serving_client import Client

client = Client()
client.load_client_config(sys.argv[1])
client.set_rpc_timeout_ms(100000)
client.connect(["127.0.0.1:9393"])
```

#### Q: 执行GPU预测时遇到InvalidArgumentError: Device id must be less than GPU count, but received id is: 0. GPU count is: 0.

**A:** 将显卡驱动对应的libcuda.so的目录添加到LD_LIBRARY_PATH环境变量中

#### Q: 执行GPU预测时遇到ExternalError: Cudnn error, CUDNN_STATUS_BAD_PARAM at (../batch_norm_op.cu:198)

**A:** 将cudnn的lib64路径添加到LD_LIBRARY_PATH，安装自pypi的Paddle Serving中post9版使用的是cudnn 7.3,post10使用的是cudnn 7.5。如果是使用自己编译的Paddle Serving，可以在log/serving.INFO日志文件中查看对应的cudnn版本。

#### Q: 执行GPU预测时遇到Error: Failed to find dynamic library: libcublas.so

**A:** 将cuda的lib64路径添加到LD_LIBRARY_PATH, post9版本的Paddle Serving使用的是cuda 9.0，post10版本使用的cuda 10.0。

#### Q: Client端fetch的变量名如何设置

**A:** 可以查看配置文件serving_server_conf.prototxt，获取需要的变量名

#### Q: 如何使用多语言客户端

**A:** 多语言客户端要与多语言服务端配套使用。当前版本下（0.4.0），服务端需要将Server改为MultiLangServer（如果是以命令行启动的话只需要添加--use_multilang参数），Python客户端需要将Client改为MultiLangClient，同时去除load_client_config的过程。[Java客户端参考文档](https://github.com/PaddlePaddle/Serving/blob/develop/doc/JAVA_SDK_CN.md)

#### Q: 如何在Windows下使用Paddle Serving

**A:** 当前版本（0.4.0）在Windows上可以运行多语言RPC客户端，或使用HTTP方式访问。如果使用多语言RPC客户端，需要在Linux环境（比如本机容器，或远程Linux机器）中运行多语言服务端；如果使用HTTP方式，需要在Linux环境中运行普通服务端

#### Q: libnvinfer.so: cannot open shared object file: No such file or directory)

 **A:** 参考该文档安装TensorRT: https://blog.csdn.net/hesongzefairy/article/details/105343525



## 日志排查

#### Q: 部署和预测中的日志信息在哪里查看？

**A:** server端的日志分为两部分，一部分打印到标准输出，一部分打印到启动服务时的目录下的log/serving.INFO文件中。

client端的日志直接打印到标准输出。

通过在部署服务之前 'export  GLOG_v=3'可以输出更为详细的日志信息。

#### Q: paddle-serving启动成功后，相关的日志在哪里设置

**A:** 1)警告是glog组件打印的，告知glog初始化之前日志打印在STDERR

​	   2)一般采用GLOG_v方式启动服务同时设置日志级别。

例如：
```
GLOG_v=2 python -m paddle_serving_server.serve --model xxx_conf/ --port 9999 
```



#### Q: （GLOG_v=2下）Server端日志一切正常，但Client端始终得不到正确的预测结果

**A:** 可能是配置文件有问题，检查下配置文件（is_load_tensor，fetch_type等有没有问题）

#### Q: 如何给Server传递Logid

**A:** Logid默认为0（后续应该有自动生成Logid的计划，当前版本0.4.0），Client端通过在predict函数中指定log_id参数传递



## 性能优化
