# How to Convert Paddle Inference Model To Paddle Serving Format

([简体中文](./INFERENCE_TO_SERVING_CN.md)|English)

## Example

``` python
from paddle_serving_client.io import inference_model_to_serving
inference_model_dir = "your_inference_model"
serving_client_dir = "serving_client_dir"
serving_server_dir = "serving_server_dir"
feed_var_names, fetch_var_names = inference_model_to_serving(
		inference_model_dir, serving_client_dir, serving_server_dir)
```
