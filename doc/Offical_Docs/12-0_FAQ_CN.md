# 常见问题与解答

常见问题解答分为8大类问题：
- [版本升级问题](#1)
- [基础知识](#2)
- [安装问题](#3)
- [编译问题](#4)
- [环境问题](#5)
- [部署问题](#6)
- [预测问题](#7)
- [日志排查](#8)

<a name="1"></a>

## 版本升级问题

#### Q: 从 `v0.6.x` 升级到 `v0.7.0` 版本时，运行 Python Pipeline 程序时报错信息如下：
```
Failed to predict: (data_id=1 log_id=0) [det|0] Failed to postprocess: postprocess() takes 4 positional arguments but 5 were given
```
**A:** 在服务端程序（例如 web_service.py)的postprocess函数定义中增加参数data_id，改为 def postprocess(self, input_dicts, fetch_dict, **data_id**, log_id) 即可。

<a name="2"></a>

## 基础知识

#### Q: Paddle Serving 、Paddle Inference、PaddleHub Serving 三者的区别及联系？

**A:** Paddle Serving 是远程服务，即发起预测的设备（手机、浏览器、客户端等）与实际预测的硬件不在一起。   paddle inference 是一个 library，适合嵌入到一个大系统中保证预测效率，Paddle Serving 调用 paddle inference 做远程服务。paddlehub serving 可以认为是一个示例，都会使用 Paddle Serving 作为统一预测服务入口。如果在 web 端交互，一般是调用远程服务的形式，可以使用 Paddle Serving 的 web service 搭建。

#### Q: Paddle Serving 支持哪些数据类型?

**A:** 在 protobuf 定义中 `feed_type` 和 `fetch_type` 编号与数据类型对应如下，完整信息可参考[保存用于 Serving 部署的模型参数](./5-1_Save_Model_Params_CN.md)

| 类型 | 类型值 |
|------|------|
| int64 | 0   |
| float32  |1 |
| int32 | 2 |
| float64 | 3 |
| int16 | 4 |
| float16 | 5 |
| bfloat16 | 6 |
| uint8 | 7 |
| int8 | 8 |
| bool | 9 |
| complex64 | 10 
| complex128 | 11 |

#### Q: Paddle Serving 是否支持 Windows 和 Linux 原生环境部署？

**A:** 安装 `Linux Docker`，在 Docker 中部署 Paddle Serving，参考[安装指南](./2-0_Index_CN.md)

#### Q: Paddle Serving 如何修改消息大小限制

**A:** Server 和 Client 通过修改 `FLAGS_max_body_size` 参数来扩大数据量限制，单位为字节，默认为64MB

#### Q: Paddle Serving 客户端目前支持哪些开发语言？

**A:** 提供 Python、C++ 和 Java SDK

#### Q: Paddle Serving 支持哪些网络协议？

**A:** C++ Serving 同时支持 HTTP、gRPC 和 bRPC 协议。其中 HTTP 协议既支持 HTTP + Json 格式，同时支持 HTTP + proto 格式。完整信息请阅读[C++ Serving 通讯协议](./6-2_Cpp_Serving_Protocols_CN.md)；Python Pipeline 支持 HTTP 和 gRPC 协议，更多信息请阅读[Python Pipeline 框架设计](./6-2_Cpp_Serving_Protocols_CN.md)

<a name="3"></a>

## 安装问题

#### Q: `pip install` 安装 `python wheel` 过程中，报错信息如何修复？

```
Collecting opencv-python
  Getting requirements to build wheel ... error
  ERROR: Command errored out with exit status 1:
   command: /home/work/Python-2.7.17/build/bin/python /home/work/Python-2.7.17/build/lib/python2.7/site-packages/pip/_vendor/pep517/_in_process.py get_requires_for_build_wheel /tmp/tmpLiweA9
       cwd: /tmp/pip-install-_w6AUI/opencv-python
  Complete output (22 lines):
  Traceback (most recent call last):
    File "setup.py", line 99, in main
      % {"ext": re.escape(sysconfig.get_config_var("EXT_SUFFIX"))}
    File "/home/work/Python-2.7.17/build/lib/python2.7/re.py", line 210, in escape
      s = list(pattern)
  TypeError: 'NoneType' object is not iterable
```

**A:** 指定 `opencv-python` 安装版本4.2.0.32，运行 `pip3 install opencv-python==4.2.0.32`

#### Q: pip3 install wheel包过程报错，详细信息如下：

```
    Complete output from command python setup.py egg_info:
    Found cython-generated files...
    error in grpcio setup command: 'install_requires' must be a string or list of strings containing valid project/version requirement specifiers; Expected ',' or end-of-list in futures>=2.2.0; python_version<'3.2' at ; python_version<'3.2'

    ----------------------------------------
Command "python setup.py egg_info" failed with error code 1 in /tmp/pip-install-taoxz02y/grpcio/
```

**A:** 需要升级 pip3 版本，再重新执行安装命令。

```
pip3 install --upgrade pip
pip3 install --upgrade setuptools
```

#### Q: 运行过程中出现 `No module named xxx` 错误，信息如下：

```
Traceback (most recent call last):
  File "../../deploy/serving/test_client.py", line 18, in <module>
    from paddle_serving_app.reader import *
  File "/usr/local/python2.7.15/lib/python2.7/site-packages/paddle_serving_app/reader/__init__.py", line 15, in <module>
    from .image_reader import ImageReader, File2Image, URL2Image, Sequential, Normalize, Base64ToImage
  File "/usr/local/python2.7.15/lib/python2.7/site-packages/paddle_serving_app/reader/image_reader.py", line 24, in <module>
    from shapely.geometry import Polygon
ImportError: No module named shapely.geometry
```

**A:** 有2种方法，第一种通过 pip3 安装shapely，第二种通过 pip3 安装所有依赖组件[requirements.txt](https://github.com/PaddlePaddle/Serving/blob/develop/python/requirements.txt)。

```
方法1：
pip3 install shapely==1.7.0

方法2：
pip3 install -r python/requirements.txt
```

<a name="4"></a>

## 编译问题

#### Q: 如何使用自己编译的 Paddle Serving 进行预测？

**A:** 编译 Paddle Serving 请阅读[编译 Serving](https://github.com/PaddlePaddle/Serving/blob/v0.8.3/doc/Compile_CN.md)。

#### Q: 使用 Java 客户端，mvn compile 过程出现 "No compiler is provided in this environment. Perhaps you are running on a JRE rather than a JDK?" 错误

**A:** 没有安装 JDK，或者 `JAVA_HOME` 路径配置错误（正确配置是 JDK 路径，常见错误配置成 JRE 路径，例如正确路径参考 `JAVA_HOME="/usr/lib/jvm/java-1.8.0-openjdk-1.8.0.262.b10-0.el7_8.x86_64/"`）。Java JDK 安装参考 https://segmentfault.com/a/1190000015389941。

#### Q: 编译过程报错 /usr/local/bin/ld: cannot find -lbz2
```
/usr/local/bin/ld: cannot find -lbz2
collect2: error: ld returned 1 exit status
core/general-server/CMakeFiles/serving.dir/build.make:276: recipe for target 'core/general-server/serving' failed
make[2]: *** [core/general-server/serving] Error 1
CMakeFiles/Makefile2:1181: recipe for target 'core/general-server/CMakeFiles/serving.dir/all' failed
make[1]: *** [core/general-server/CMakeFiles/serving.dir/all] Error 2
Makefile:129: recipe for target 'all' failed
make: *** [all] Error 2
```

**A:**  Ubuntu 系统运行命令安装 libbz2: `apt install libbz2-dev`，

<a name="5"></a>

## 环境问题

#### Q：程序运行出现 `CXXABI` 相关错误。

错误原因是编译 Python 使用的 GCC 版本和编译 Serving 的 GCC 版本不一致。对于 Docker 用户，推荐使用[Docker容器](https://github.com/PaddlePaddle/Serving/blob/develop/doc/Docker_Images_CN.md)，由于 Docker 容器内的 Python 版本与 Serving 在发布前都做过适配，这样就不会出现类似的错误。

推荐使用 GCC 8.2 预编译包 [Python3.6](https://paddle-serving.bj.bcebos.com/others/Python3.6.10-gcc82.tar) 。下载解压后，需要将对应的目录设置为 `PYTHONROOT`，并设置 `PATH` 和 `LD_LIBRARY_PATH`。

```bash
export PYTHONROOT=/path/of/python # 对应解压后的Python目录
export PATH=$PYTHONROOT/bin:$PATH
export LD_LIBRARY_PATH=$PYTHONROOT/lib:$LD_LIBRARY_PATH
```

#### Q：遇到 `libstdc++.so.6` 的版本不够的问题

触发该问题的原因在于，编译 Paddle Serving 相关可执行程序和动态库，所采用的是 GCC 8.2(Cuda 9.0 和 10.0 的 Server 可执行程序受限 CUDA 兼容性采用 GCC 4.8编译)。Python 在调用的过程中，有可能链接到了其他 GCC 版本的 `libstdc++.so`。 需要做的就是受限确保所在环境具备 GCC 8.2，其次将 GCC8.2 的`libstdc++.so.*`拷贝到某个目录例如`/home/libstdcpp` 下。最后 `export LD_LIBRARY_PATH=/home/libstdcpp:$LD_LIBRARY_PATH` 即可。

#### Q: 遇到 `OPENSSL_1.0.1EC` 符号找不到的问题。

目前 Serving 的可执行程序和客户端动态库需要链接 `1.0.2k` 版本的 `openssl` 动态库。如果环境当中没有，可以执行

```bash
wget https://paddle-serving.bj.bcebos.com/others/centos_ssl.tar && \
    tar xf centos_ssl.tar && rm -rf centos_ssl.tar && \
    mv libcrypto.so.1.0.2k /usr/lib/libcrypto.so.1.0.2k && mv libssl.so.1.0.2k /usr/lib/libssl.so.1.0.2k && \
    ln -sf /usr/lib/libcrypto.so.1.0.2k /usr/lib/libcrypto.so.10 && \
    ln -sf /usr/lib/libssl.so.1.0.2k /usr/lib/libssl.so.10 && \
    ln -sf /usr/lib/libcrypto.so.10 /usr/lib/libcrypto.so && \
    ln -sf /usr/lib/libssl.so.10 /usr/lib/libssl.so
```

其中 `/usr/lib` 可以换成其他目录，并确保该目录在 `LD_LIBRARY_PATH` 下。

### GPU相关环境问题

#### Q：需要做哪些检查确保 Serving 可以运行在 GPU 环境

**注：如果是使用 Serving 提供的镜像不需要做下列检查，如果是其他开发环境可以参考以下指导。**

首先需要确保`nvidia-smi`可用，其次需要确保所需的动态库so文件在`LD_LIBRARY_PATH`所在的目录（包括系统lib库）。

（1）CUDA 显卡驱动：文件名通常为 `libcuda.so.$DRIVER_VERSION` 例如驱动版本为440.10.15，文件名就是 `libcuda.so.440.10.15`。

（2）CUDA 和 cuDNN 动态库：文件名通常为 `libcudart.so.$CUDA_VERSION`，和 `libcudnn.so.$CUDNN_VERSION`。例如 CUDA9 就是 `libcudart.so.9.0`，Cudnn7就是 `libcudnn.so.7`。CUDA 和 cuDNN 与 Serving 的版本匹配参见[Serving所有镜像列表](Docker_Images_CN.md#%E9%99%84%E5%BD%95%E6%89%80%E6%9C%89%E9%95%9C%E5%83%8F%E5%88%97%E8%A1%A8).

 (3) CUDA 10.1及更高版本需要 TensorRT。安装 TensorRT 相关文件的脚本参考 [install_trt.sh](../tools/dockerfiles/build_scripts/install_trt.sh).

<a name="6"></a>

## 部署问题

#### Q: GPU 环境运行 Serving 报错，GPU count is: 0。

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

**A:** 原因是 `libcuda.so` 没有链接成功。首先在机器上找到 `libcuda.so`，使用 `ldd` 命令检查 libnvidia 版本与 nvidia-smi 中版本是否一致（libnvidia-fatbinaryloader.so.418.39，与NVIDIA-SMI 418.39 Driver Version: 418.39），然后用 export 导出 `libcuda.so` 的路径即可（例如 libcuda.so 在 /usr/lib64/，export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib64/）

#### Q: 遇到 GPU not found, please check your environment or use cpu version by "pip install paddle_serving_server"

**A:** 检查环境中是否有N卡：`ls /dev/ | grep nvidia`

#### Q: Paddle Serving 支持哪些镜像环境？

**A:** 支持 CentOS 和 Ubuntu 环境镜像 ，完整列表查阅[这里](https://github.com/PaddlePaddle/Serving/blob/develop/doc/Docker_Images_CN.md)

#### Q: Paddle Serving 是否支持本地离线安装 

**A:** 支持离线部署，需要把一些相关的[依赖包](https://github.com/PaddlePaddle/Serving/blob/develop/doc/Compile_CN.md) 提前准备安装好

#### Q: Docker 中启动 Server IP地址 127.0.0.1 与 0.0.0.0 差异
**A:** 必须将容器的主进程设置为绑定到特殊的 `0.0.0.0` 表示“所有接口”地址，否则它将无法从容器外部访问。在 Docker 中 `127.0.0.1` 仅代表“这个容器”，而不是“这台机器”。如果您从容器建立到 `127.0.0.1` 的出站连接，它将返回到同一个容器；如果您将服务器绑定到 `127.0.0.1`，接收不到来自外部的连接。

<a name="7"></a>

## 预测问题

#### Q: 使用 GPU 第一次预测时特别慢，如何调整 RPC 服务的等待时间避免超时？ 

**A:** GPU 第一次预测需要初始化。使用 `set_rpc_timeout_ms` 设置更长的等待时间，单位为毫秒，默认时间为20秒。

示例：

```
from paddle_serving_client import Client

client = Client()
client.load_client_config(sys.argv[1])
client.set_rpc_timeout_ms(100000)
client.connect(["127.0.0.1:9393"])
```
#### Q: 执行 GPU 预测时遇到 `ExternalError: Cudnn error, CUDNN_STATUS_BAD_PARAM at (../batch_norm_op.cu:198)`错误

**A:** 将 cuDNN 的 lib64路径添加到 `LD_LIBRARY_PATH`，安装自 `pypi` 的 Paddle Serving 中 `post9` 版本使用的是 `cuDNN 7.3,post10` 使用的是 `cuDNN 7.5。如果是使用自己编译的 Paddle Serving，可以在 `log/serving.INFO` 日志文件中查看对应的 cuDNN 版本。

#### Q: 执行 GPU 预测时遇到 `Error: Failed to find dynamic library: libcublas.so`

**A:** 将 CUDA 的 lib64路径添加到 `LD_LIBRARY_PATH`, post9 版本的 Paddle Serving 使用的是 `cuda 9.0，post10` 版本使用的 `cuda 10.0`。

#### Q: Client 的 `fetch var`变量名如何设置

**A:** 通过[保存用于 Serving 部署的模型参数](https://github.com/PaddlePaddle/Serving/blob/v0.8.3/doc/Save_EN.md) 生成配置文件 `serving_server_conf.prototxt`，获取需要的变量名。

#### Q: 如何使用多语言客户端

**A:** 多语言客户端要与多语言服务端配套使用。当前版本下（0.8.3）

#### Q: 如何在 Windows 下使用 Paddle Serving

**A:** 在 Windows 上可以运行多语言 RPC 客户端，或使用 HTTP 方式访问。

#### Q: 报错信息 `libnvinfer.so: cannot open shared object file: No such file or directory)`

 **A:** 没有安装 TensorRT，安装 TensorRT 请参考链接: https://blog.csdn.net/hesongzefairy/article/details/105343525

<a name="8"></a>

## 日志排查

#### Q: 部署和预测中的日志信息在哪里查看？

**A:** Server 的日志分为两部分，一部分打印到标准输出，一部分打印到启动服务时的目录下的 `log/serving.INFO` 文件中。
Client 的日志直接打印到标准输出。
通过在部署服务之前 'export  GLOG_v=3'可以输出更为详细的日志信息。

#### Q: C++ Serving 启动成功后，日志文件在哪里，在哪里设置日志级别？

**A:** C++ Serving 服务的所有日志在程序运行的当前目录的`log/`目录下，分为 serving.INFO、serving.WARNING 和 serving.ERROR 文件。
1)警告是 `glog` 组件打印的，告知 `glog` 初始化之前日志打印在 STDERR；
2)一般采用 `GLOG_v` 方式启动服务同时设置日志级别。

例如：
```
GLOG_v=2 python -m paddle_serving_server.serve --model xxx_conf/ --port 9999 
```

#### Q: Python Pipeline 启动成功后，日志文件在哪里，在哪里设置日志级别？

**A:** Python Pipeline 服务的日志信息请阅读[Python Pipeline 设计](./7-1_Python_Pipeline_Design_CN.md) 第三节服务日志。

#### Q: （GLOG_v=2下）Server 日志一切正常，但 Client 始终得不到正确的预测结果

**A:** 可能是配置文件有问题，检查下配置文件（is_load_tensor，fetch_type等有没有问题）

#### Q: 如何给 Server 传递 Logid

**A:** Logid 默认为0，Client 通过在 predict 函数中指定 log_id 参数

#### Q: C++ Serving 出现问题如何调试和定位

**A:** 推荐您使用 GDB 进行定位和调试，如果您使用 Serving 的 Docker，在启动容器时候，需要加上 `docker run --privileged `参数，开启特权模式，这样才能在 docker 容器中使用 GDB 定位和调试
如果 C++ Serving 出现 `core dump`，一般会生成 core 文件，若没有，运行 `ulimit -c unlimited`命令开启core dump。
使用 GDB 调试 core 文件的方法为：`gdb <可执行文件> <core文件>`，进入后输入 `bt` 指令显示栈信息。

注意：可执行文件路径是 C++ bin 文件的路径，而不是 python 命令，一般为类似下面的这种 `/usr/local/lib/python3.6/site-packages/paddle_serving_server/serving-gpu-102-0.7.0/serving`
