# How to Convert Paddle Inference Model To Paddle Serving Format

([简体中文](./INFERENCE_TO_SERVING_CN.md)|English)

We should know something before converting to serving model

**inference_model_dir**：the directory of Paddle inference model

**serving_client_dir**: the directory of server side configuration

**serving_client_dir**: the directory of client side configuration

**model_filename**: this is model description file whose default value is `__model__`, if it's not default name, set `model_filename` explicitly

**params_filename**: during `save_inference_model` every Variable will be save as a single file. If we have the inference model whose params are compressed into one file, please set `params_filename` explicitly



## Example

``` python
from paddle_serving_client.io import inference_model_to_serving
inference_model_dir = "your_inference_model"
serving_client_dir = "serving_client_dir"
serving_server_dir = "serving_server_dir"
feed_var_names, fetch_var_names = inference_model_to_serving(
		inference_model_dir, serving_client_dir, serving_server_dir)
```

if your model file and params file are both standalone, please use the following api.

```
feed_var_names, fetch_var_names = inference_model_to_serving(
		inference_model_dir, serving_client_dir, serving_server_dir,
		model_filename="model", params_filename="params")
```
