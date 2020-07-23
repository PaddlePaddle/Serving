# 如何从Paddle保存的预测模型转为Paddle Serving格式可部署的模型

([English](./INFERENCE_TO_SERVING.md)|简体中文)

## 示例

``` python
from paddle_serving_client.io import inference_model_to_serving
inference_model_dir = "your_inference_model"
serving_client_dir = "serving_client_dir"
serving_server_dir = "serving_server_dir"
feed_var_names, fetch_var_names = inference_model_to_serving(
		inference_model_dir, serving_client_dir, serving_server_dir)
```
