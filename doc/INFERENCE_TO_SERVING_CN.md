# 如何从Paddle保存的预测模型转为Paddle Serving格式可部署的模型

([English](./INFERENCE_TO_SERVING.md)|简体中文)

## 示例

在下列代码中，我们需要知道以下信息。

**模型文件夹**：这个文件夹就是Paddle的inference_model所在的文件夹

**serving_client_dir**: 这个文件夹是inference_model转换成Serving模型后，服务端配置的保存路径

**serving_client_dir**: 这个文件夹是inference_model转换成Serving模型后，客户端配置的保存路径

**模型描述文件**: 模型描述文件也就是`model_filename`默认值为`__model__`,是一个pb2文本文件，如果是别的文件名需要显式指定

**模型参数文件**: 在`save_inference_model`阶段，默认方式是每一个Variable保存一个二进制文件，如果是这种情况就不需要做指定。如果所有参数用压缩成一个文件的形式保存，则需要显式指定`params_filename`


``` python
from paddle_serving_client.io import inference_model_to_serving
inference_model_dir = "your_inference_model"
serving_client_dir = "serving_client_dir"
serving_server_dir = "serving_server_dir"
feed_var_names, fetch_var_names = inference_model_to_serving(
		inference_model_dir, serving_client_dir, serving_server_dir)
```
如果模型中有模型描述文件`model_filename` 和 模型参数文件`params_filename`，那么请用
```
feed_var_names, fetch_var_names = inference_model_to_serving(
		inference_model_dir, serving_client_dir, serving_server_dir,
		 model_filename="model", params_filename="params")
```
