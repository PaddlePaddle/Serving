# How to save a servable model of Paddle Serving?

([简体中文](./SAVE_CN.md)|English)

## Save from training or prediction script 
Currently, paddle serving provides a save_model interface for users to access, the interface is similar with `save_inference_model` of Paddle.
``` python
import paddle_serving_client.io as serving_io
serving_io.save_model("imdb_model", "imdb_client_conf",
                      {"words": data}, {"prediction": prediction},
                      fluid.default_main_program())
```
`imdb_model` is the server side model with serving configurations. `imdb_client_conf` is the client rpc configurations. 

Serving has a dictionary for `Feed` and `Fetch` variables for client to assign. In the example, `{"words": data}` is the feed dict that specify the input of saved inference model. `{"prediction": prediction}` is the fetch dic that specify the output of saved inference model. An alias name can be defined for feed and fetch variables. An example of how to use alias name
 is as follows:
 ``` python
 from paddle_serving_client import Client
import sys

client = Client()
client.load_client_config(sys.argv[1])
client.connect(["127.0.0.1:9393"])

for line in sys.stdin:
    group = line.strip().split()
    words = [int(x) for x in group[1:int(group[0]) + 1]]
    label = [int(group[-1])]
    feed = {"words": words, "label": label}
    fetch = ["acc", "cost", "prediction"]
    fetch_map = client.predict(feed=feed, fetch=fetch)
    print("{} {}".format(fetch_map["prediction"][1], label[0]))
 ```

## Export from saved model files
If you have saved model files using Paddle's `save_inference_model` API, you can use Paddle Serving's` inference_model_to_serving` API to convert it into a model file that can be used for Paddle Serving.
```python
import paddle_serving_client.io as serving_io
serving_io.inference_model_to_serving(dirname, serving_server="serving_server", serving_client="serving_client", model_filename=None, params_filename=None )
```
Or you can use a build-in python module called `paddle_serving_client.convert` to convert it.
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
| `paras_filename` | str | None | The name of file to load all parameters. It is only used for the case that all parameters were saved in a single binary file. If parameters were saved in separate files, set it as None. |
