# 如何从Paddle保存的预测模型转为Paddle Serving格式可部署的模型

([English](./INFERENCE_TO_SERVING.md)|简体中文)

你可以使用Paddle Serving提供的名为`paddle_serving_client.convert`的内置模块进行转换。
```python
python -m paddle_serving_client.convert --dirname ./your_inference_model_dir
```
模块参数与`inference_model_to_serving`接口参数相同。
| 参数 | 类型 | 默认值 | 描述 |
|--------------|------|-----------|--------------------------------|
| `dirname` | str | - | 需要转换的模型文件存储路径，Program结构文件和参数文件均保存在此目录。|
| `serving_server` | str | `"serving_server"` | 转换后的模型文件和配置文件的存储路径。默认值为serving_server |
| `serving_client` | str | `"serving_client"` | 转换后的客户端配置文件存储路径。默认值为serving_client |
| `model_filename` | str | None | 存储需要转换的模型Inference Program结构的文件名称。如果设置为None，则使用 `__model__` 作为默认的文件名 |
| `params_filename` | str | None | 存储需要转换的模型所有参数的文件名称。当且仅当所有模型参数被保存在一个单独的>二进制文件中，它才需要被指定。如果模型参数是存储在各自分离的文件中，设置它的值为None |
