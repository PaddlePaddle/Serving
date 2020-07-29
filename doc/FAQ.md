# FAQ

- Q：如何调整RPC服务的等待时间，避免超时？ 

  A：使用set_rpc_timeout_ms设置更长的等待时间，单位为毫秒，默认时间为20秒。
  
  示例：
  ```
  from paddle_serving_client import Client

  client = Client()
  client.load_client_config(sys.argv[1])
  client.set_rpc_timeout_ms(100000)
  client.connect(["127.0.0.1:9393"])
   ```

- Q: 如何使用自己编译的Paddle Serving进行预测？
  A：通过pip命令安装自己编译出的whl包，并设置SERVING_BIN环境变量为编译出的serving二进制文件路径。
