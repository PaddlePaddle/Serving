# How to Convert Paddle Inference Model To Paddle Serving Format

([简体中文](./INFERENCE_TO_SERVING_CN.md)|English)

you can use a build-in python module called `paddle_serving_client.convert` to convert it.
```python
python -m paddle_serving_client.convert --dirname ./your_inference_model_dir
```
Arguments are the same as `inference_model_to_serving` API.
| Argument | Type | Default | Description |
|--------------|------|-----------|--------------------------------|
| `dirname` | str | - | Path of saved model files. Program file and parameter files are saved in this directory. |
| `serving_server` | str | `"serving_server"` | The path of model files and configuration files for server. |
| `serving_client` | str | `"serving_client"` | The path of configuration files for client. |
| `model_filename` | str | None | The name of file to load the inference program. If it is None, the default filename `__model__` will be used. |
| `params_filename` | str | None | The name of file to load all parameters. It is only used for the case that all parameters were saved in a single binary file. If parameters were saved in separate files, set it as None. |
