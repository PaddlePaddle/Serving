# FAQ

## 版本升级问题

#### Q: 从v0.6.x升级到v0.7.0版本时，运行Python Pipeline程序时报错信息如下：
```
Failed to predict: (data_id=1 log_id=0) [det|0] Failed to postprocess: postprocess() takes 4 positional arguments but 5 were given
```
**A:** 在服务端程序（例如 web_service.py)的postprocess函数定义中增加参数data_id，改为 def postprocess(self, input_dicts, fetch_dict, **data_id**, log_id) 即可。

***

## 基础知识

#### Q: Paddle Serving 、Paddle Inference、PaddleHub Serving三者的区别及联系？

**A:** paddle serving是远程服务，即发起预测的设备（手机、浏览器、客户端等）与实际预测的硬件不在一起。   paddle inference是一个library，适合嵌入到一个大系统中保证预测效率，paddle serving调用了paddle       inference做远程服务。paddlehub serving可以认为是一个示例，都会使用paddle serving作为统一预测服务入口。如果在web端交互，一般是调用远程服务的形式，可以使用paddle serving的web service搭建。

#### Q: paddle-serving是否支持Int32支持

**A:** 在protobuf定feed_type和fetch_type编号与数据类型对应如下，完整信息可参考[Serving配置与启动参数说明](./Serving_Configure_CN.md#模型配置文件)

     0-int64
    
     1-float32
    
     2-int32

#### Q: paddle-serving是否支持windows和Linux环境下的多线程调用 

**A:** 客户端可以发起多线程访问调用服务端 

#### Q: paddle-serving如何修改消息大小限制

**A:** 在server端和client但通过FLAGS_max_body_size来扩大数据量限制，单位为字节，默认为64MB

#### Q: paddle-serving客户端目前支持哪些语言

**A:** java c++ python 

#### Q: paddle-serving目前支持哪些协议

**A:** http rpc 

***

## 安装问题

#### Q: pip install安装whl包过程，报错信息如下：

```
Collecting opencv-python
  Using cached opencv-python-4.3.0.38.tar.gz (88.0 MB)
  Installing build dependencies ... done
  Getting requirements to build wheel ... error
  ERROR: Command errored out with exit status 1:
   command: /home/work/Python-2.7.17/build/bin/python /home/work/Python-2.7.17/build/lib/python2.7/site-packages/pip/_vendor/pep517/_in_process.py get_requires_for_build_wheel /tmp/tmpLiweA9
       cwd: /tmp/pip-install-_w6AUI/opencv-python
  Complete output (22 lines):
  Traceback (most recent call last):
    File "/home/work/Python-2.7.17/build/lib/python2.7/site-packages/pip/_vendor/pep517/_in_process.py", line 280, in <module>
      main()
    File "/home/work/Python-2.7.17/build/lib/python2.7/site-packages/pip/_vendor/pep517/_in_process.py", line 263, in main
      json_out['return_val'] = hook(**hook_input['kwargs'])
    File "/home/work/Python-2.7.17/build/lib/python2.7/site-packages/pip/_vendor/pep517/_in_process.py", line 114, in get_requires_for_build_wheel
      return hook(config_settings)
    File "/tmp/pip-build-env-AUCbP4/overlay/lib/python2.7/site-packages/setuptools/build_meta.py", line 146, in get_requires_for_build_wheel
      return self._get_build_requires(config_settings, requirements=['wheel'])
    File "/tmp/pip-build-env-AUCbP4/overlay/lib/python2.7/site-packages/setuptools/build_meta.py", line 127, in _get_build_requires
      self.run_setup()
    File "/tmp/pip-build-env-AUCbP4/overlay/lib/python2.7/site-packages/setuptools/build_meta.py", line 243, in run_setup
      self).run_setup(setup_script=setup_script)
    File "/tmp/pip-build-env-AUCbP4/overlay/lib/python2.7/site-packages/setuptools/build_meta.py", line 142, in run_setup
      exec(compile(code, __file__, 'exec'), locals())
    File "setup.py", line 448, in <module>
      main()
    File "setup.py", line 99, in main
      % {"ext": re.escape(sysconfig.get_config_var("EXT_SUFFIX"))}
    File "/home/work/Python-2.7.17/build/lib/python2.7/re.py", line 210, in escape
      s = list(pattern)
  TypeError: 'NoneType' object is not iterable
```

**A:** 指定opencv-python版本安装，pip install opencv-python==4.2.0.32，再安装whl包

#### Q: pip3 install whl包过程报错信息如下：

```
    Complete output from command python setup.py egg_info:
    Found cython-generated files...
    error in grpcio setup command: 'install_requires' must be a string or list of strings containing valid project/version requirement specifiers; Expected ',' or end-of-list in futures>=2.2.0; python_version<'3.2' at ; python_version<'3.2'

    ----------------------------------------
Command "python setup.py egg_info" failed with error code 1 in /tmp/pip-install-taoxz02y/grpcio/
```

**A:** 需要升级pip3，再重新执行安装命令。

```
pip3 install --upgrade pip
pip3 install --upgrade setuptools
```

#### Q: 运行过程中报错，信息如下：

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

**A:** 有2种方法，第一种通过pip/pip3安装shapely，第二种通过pip/pip3安装所有依赖组件。

```
方法1：
pip install shapely==1.7.0

方法2：
pip install -r python/requirements.txt
```
***

## 编译问题

#### Q: 如何使用自己编译的Paddle Serving进行预测？

**A:** 通过pip命令安装自己编译出的whl包，并设置SERVING_BIN环境变量为编译出的serving二进制文件路径。

#### Q: 使用Java客户端，mvn compile过程出现"No compiler is provided in this environment. Perhaps you are running on a JRE rather than a JDK?"错误

**A:** 没有安装JDK，或者JAVA_HOME路径配置错误（正确配置是JDK路径，常见错误配置成JRE路径，例如正确路径参考JAVA_HOME="/usr/lib/jvm/java-1.8.0-openjdk-1.8.0.262.b10-0.el7_8.x86_64/"）。Java JDK安装参考https://segmentfault.com/a/1190000015389941

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

**A:** 运行命令安装libbz2: apt install libbz2-dev

***
## 环境问题

#### Q:  ImportError: dlopen: cannot load any more object with static TLS 

**A:** 一般是用户使用Linux系统版本比较低或者Python使用的gcc版本比较低导致的，可使用以下命令检查，或者通过使用Serving或Paddle镜像安装
```
strings /lib/libc.so | grep GLIBC
```

#### Q：使用过程中出现CXXABI错误。

这个问题出现的原因是Python使用的gcc版本和Serving所需的gcc版本对不上。对于Docker用户，推荐使用[Docker容器](https://github.com/PaddlePaddle/Serving/blob/develop/doc/Docker_Images_CN.md)，由于Docker容器内的Python版本与Serving在发布前都做过适配，这样就不会出现类似的错误。如果是其他开发环境，首先需要确保开发环境中具备GCC 8.2，如果没有gcc 8.2，参考安装方式

```bash
wget -q https://paddle-ci.gz.bcebos.com/gcc-8.2.0.tar.xz 
tar -xvf gcc-8.2.0.tar.xz && \
cd gcc-8.2.0 && \
unset LIBRARY_PATH CPATH C_INCLUDE_PATH PKG_CONFIG_PATH CPLUS_INCLUDE_PATH INCLUDE && \
./contrib/download_prerequisites && \
cd .. && mkdir temp_gcc82 && cd temp_gcc82 && \
../gcc-8.2.0/configure --prefix=/usr/local/gcc-8.2 --enable-threads=posix --disable-checking --disable-multilib && \
make -j8 && make install
cd .. && rm -rf temp_gcc82
cp ${lib_so_6} ${lib_so_6}.bak  && rm -f ${lib_so_6} && 
ln -s /usr/local/gcc-8.2/lib64/libgfortran.so.5 ${lib_so_5} && \
ln -s /usr/local/gcc-8.2/lib64/libstdc++.so.6 ${lib_so_6} && \
cp /usr/local/gcc-8.2/lib64/libstdc++.so.6.0.25 ${lib_path}
```

假如已经有了GCC 8.2，可以自行安装Python，此外我们也提供了两个GCC 8.2编译的[Python2.7](https://paddle-serving.bj.bcebos.com/others/Python2.7.17-gcc82.tar) 和 [Python3.6](https://paddle-serving.bj.bcebos.com/others/Python3.6.10-gcc82.tar) 。下载解压后，需要将对应的目录设置为`PYTHONROOT`，并设置`PATH`和`LD_LIBRARY_PATH`。

```bash
export PYTHONROOT=/path/of/python # 对应解压后的Python目录
export PATH=$PYTHONROOT/bin:$PATH
export LD_LIBRARY_PATH=$PYTHONROOT/lib:$LD_LIBRARY_PATH
```

#### Q：遇到libstdc++.so.6的版本不够的问题

触发该问题的原因在于，编译Paddle Serving相关可执行程序和动态库，所采用的是GCC 8.2(Cuda 9.0和10.0的Server可执行程序受限Cuda兼容性采用GCC 4.8编译)。Python在调用的过程中，有可能链接到了其他GCC版本的 `libstdc++.so`。 需要做的就是受限确保所在环境具备GCC 8.2，其次将GCC8.2的`libstdc++.so.*`拷贝到某个目录例如`/home/libstdcpp`下。最后`export LD_LIBRARY_PATH=/home/libstdcpp:$LD_LIBRARY_PATH` 即可。

#### Q: 遇到OPENSSL_1.0.1EC 符号找不到的问题。

目前Serving的可执行程序和客户端动态库需要链接1.0.2k版本的openssl动态库。如果环境当中没有，可以执行

```bash
wget https://paddle-serving.bj.bcebos.com/others/centos_ssl.tar && \
    tar xf centos_ssl.tar && rm -rf centos_ssl.tar && \
    mv libcrypto.so.1.0.2k /usr/lib/libcrypto.so.1.0.2k && mv libssl.so.1.0.2k /usr/lib/libssl.so.1.0.2k && \
    ln -sf /usr/lib/libcrypto.so.1.0.2k /usr/lib/libcrypto.so.10 && \
    ln -sf /usr/lib/libssl.so.1.0.2k /usr/lib/libssl.so.10 && \
    ln -sf /usr/lib/libcrypto.so.10 /usr/lib/libcrypto.so && \
    ln -sf /usr/lib/libssl.so.10 /usr/lib/libssl.so
```

其中`/usr/lib` 可以换成其他目录，并确保该目录在`LD_LIBRARY_PATH`下。

### GPU相关环境问题

#### Q：需要做哪些检查确保Serving可以运行在GPU环境

**注：如果是使用Serving提供的镜像不需要做下列检查，如果是其他开发环境可以参考以下指导。**

首先需要确保`nvidia-smi`可用，其次需要确保所需的动态库so文件在`LD_LIBRARY_PATH`所在的目录（包括系统lib库）。

（1）Cuda显卡驱动：文件名通常为 `libcuda.so.$DRIVER_VERSION` 例如驱动版本为440.10.15，文件名就是`libcuda.so.440.10.15`。

（2）Cuda和Cudnn动态库：文件名通常为 `libcudart.so.$CUDA_VERSION`，和 `libcudnn.so.$CUDNN_VERSION`。例如Cuda9就是 `libcudart.so.9.0`，Cudnn7就是 `libcudnn.so.7`。Cuda和Cudnn与Serving的版本匹配参见[Serving所有镜像列表](Docker_Images_CN.md#%E9%99%84%E5%BD%95%E6%89%80%E6%9C%89%E9%95%9C%E5%83%8F%E5%88%97%E8%A1%A8).

  (3) Cuda10.1及更高版本需要TensorRT。安装TensorRT相关文件的脚本参考 [install_trt.sh](../tools/dockerfiles/build_scripts/install_trt.sh).

***

## 模型参数保存问题

#### Q: 找不到'_remove_training_info'属性，详细报错信息如下：
```
python3 -m paddle_serving_client.convert --dirname ./ch_PP-OCRv2_det_infer/ \
                                         --model_filename inference.pdmodel          \
                                         --params_filename inference.pdiparams       \
                                         --serving_server ./ppocrv2_det_serving/ \
                                         --serving_client ./ppocrv2_det_client/ 
 AttributeError: 'Program' object has no attribute '_remove_training_info'
```

**A:** Paddle版本低，升级Paddle版本到2.2.x及以上

***

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

**A:** 目前（0.4.0）仅支持CentOS，具体列表查阅[这里](https://github.com/PaddlePaddle/Serving/blob/develop/doc/Docker_Images_CN.md)

#### Q: python编译的GCC版本与serving的版本不匹配

**A:**:1)使用GPU Dockers, [这里是Docker镜像列表](https://github.com/PaddlePaddle/Serving/blob/develop/doc/Docker_Images_CN.md)解决环境问题；2)修改anaconda的虚拟环境下安装的python的gcc版本[改变python的GCC编译环境](https://www.jianshu.com/p/c498b3d86f77) 

#### Q: paddle-serving是否支持本地离线安装 

**A:** 支持离线部署，需要把一些相关的[依赖包](https://github.com/PaddlePaddle/Serving/blob/develop/doc/Compile_CN.md)提前准备安装好

#### Q: Docker中启动server IP地址 127.0.0.1 与 0.0.0.0 差异
**A:** 您必须将容器的主进程设置为绑定到特殊的 0.0.0.0 “所有接口”地址，否则它将无法从容器外部访问。在Docker中 127.0.0.1 代表“这个容器”，而不是“这台机器”。如果您从容器建立到 127.0.0.1 的出站连接，它将返回到同一个容器；如果您将服务器绑定到 127.0.0.1，接收不到来自外部的连接。

***

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

**A:** 多语言客户端要与多语言服务端配套使用。当前版本下（0.4.0），服务端需要将Server改为MultiLangServer（如果是以命令行启动的话只需要添加--use_multilang参数），Python客户端需要将Client改为MultiLangClient，同时去除load_client_config的过程。[Java客户端参考文档](https://github.com/PaddlePaddle/Serving/blob/develop/doc/Java_SDK_CN.md)

#### Q: 如何在Windows下使用Paddle Serving

**A:** 当前版本（0.4.0）在Windows上可以运行多语言RPC客户端，或使用HTTP方式访问。如果使用多语言RPC客户端，需要在Linux环境（比如本机容器，或远程Linux机器）中运行多语言服务端；如果使用HTTP方式，需要在Linux环境中运行普通服务端

#### Q: libnvinfer.so: cannot open shared object file: No such file or directory)

 **A:** 参考该文档安装TensorRT: https://blog.csdn.net/hesongzefairy/article/details/105343525

***

## 日志排查

#### Q: 部署和预测中的日志信息在哪里查看？

**A:** server端的日志分为两部分，一部分打印到标准输出，一部分打印到启动服务时的目录下的log/serving.INFO文件中。

client端的日志直接打印到标准输出。

通过在部署服务之前 'export  GLOG_v=3'可以输出更为详细的日志信息。

#### Q: paddle-serving启动成功后，相关的日志在哪里设置

**A:** 1)警告是glog组件打印的，告知glog初始化之前日志打印在STDERR

       2)一般采用GLOG_v方式启动服务同时设置日志级别。

例如：

```
GLOG_v=2 python -m paddle_serving_server.serve --model xxx_conf/ --port 9999 
```


#### Q: （GLOG_v=2下）Server端日志一切正常，但Client端始终得不到正确的预测结果

**A:** 可能是配置文件有问题，检查下配置文件（is_load_tensor，fetch_type等有没有问题）

#### Q: 如何给Server传递Logid

**A:** Logid默认为0（后续应该有自动生成Logid的计划，当前版本0.4.0），Client端通过在predict函数中指定log_id参数传递

#### Q: C++Server出现问题如何调试和定位

**A:** 推荐您使用gdb进行定位和调试，如果您使用docker,在启动容器时候，需要加上docker run --privileged参数，开启特权模式，这样才能在docker容器中使用gdb定位和调试

如果您C++端出现coredump，一般而言会生成一个core文件，若没有，则应开启生成core文件选项，使用ulimit -c unlimited命令。

使用gdb调试core文件的方法为：gdb <可执行文件> <core文件>，进入后输入bt指令，一般即可显示出错在哪一行。

注意：可执行文件路径是C++ bin文件的路径，而不是python命令，一般为类似下面的这种/usr/local/lib/python3.6/site-packages/paddle_serving_server/serving-gpu-102-0.7.0/serving


