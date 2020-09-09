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
