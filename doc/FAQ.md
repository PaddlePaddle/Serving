# FAQ

- Q: 如何调整RPC服务的等待时间，避免超时？ 

  A: 使用set_rpc_timeout_ms设置更长的等待时间，单位为毫秒，默认时间为20秒。
  
  示例：
  ```
  from paddle_serving_client import Client

  client = Client()
  client.load_client_config(sys.argv[1])
  client.set_rpc_timeout_ms(100000)
  client.connect(["127.0.0.1:9393"])
  ```

- Q: 如何使用自己编译的Paddle Serving进行预测？

  A: 通过pip命令安装自己编译出的whl包，并设置SERVING_BIN环境变量为编译出的serving二进制文件路径。

- Q: 执行GPU预测时遇到InvalidArgumentError: Device id must be less than GPU count, but received id is: 0. GPU count is: 0.

  A: 将显卡驱动对应的libcuda.so的目录添加到LD_LIBRARY_PATH环境变量中

- Q: 执行GPU预测时遇到ExternalError: Cudnn error, CUDNN_STATUS_BAD_PARAM at (/home/scmbuild/workspaces_cluster.dev/baidu.lib.paddlepaddle/baidu/lib/paddlepaddle/Paddle/paddle/fluid/operators/batch_norm_op.cu:198)

  A: 将cudnn的lib64路径添加到LD_LIBRARY_PATH，安装自pypi的Paddle Serving中post9版使用的是cudnn 7.3,post10使用的是cudnn 7.5。如果是使用自己编译的Paddle Serving，可以在log/serving.INFO日志文件中查看对应的cudnn版本。

- Q: 执行GPU预测时遇到Error: Failed to find dynamic library: libcublas.so

  A: 将cuda的lib64路径添加到LD_LIBRARY_PATH, post9版本的Paddle Serving使用的是cuda 9.0，post10版本使用的cuda 10.0。

- Q: 部署和预测中的日志信息在哪里查看？

- A: server端的日志分为两部分，一部分打印到标准输出，一部分打印到启动服务时的目录下的log/serving.INFO文件中。

    client端的日志直接打印到标准输出。

    通过在部署服务之前 'export  GLOG_v=3'可以输出更为详细的日志信息。
    
- Q: GPU环境运行Serving报错，GPU count is: 0。
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
  A: libcuda.so没有链接成功。首先在机器上找到libcuda.so，ldd检查libnvidia版本与nvidia-smi中版本一致（libnvidia-fatbinaryloader.so.418.39，与NVIDIA-SMI 418.39 Driver Version: 418.39）,然后用export导出libcuda.so的路径即可（例如libcuda.so在/usr/lib64/，export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib64/）
  
- Q: 遇到 GPU not found, please check your environment or use cpu version by "pip install paddle_serving_server"

    A: 检查环境中是否有N卡：ls /dev/ | grep nvidia

