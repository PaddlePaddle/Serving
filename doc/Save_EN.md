# How to save a servable model of Paddle Serving?

([简体中文](./Save_CN.md)|English)

## Export from saved model files

you can use a build-in python module called `paddle_serving_client.convert` to convert it.
```python
python -m paddle_serving_client.convert --dirname ./your_inference_model_dir
```

If you have saved model files using Paddle's `save_inference_model` API, you can use Paddle Serving's` inference_model_to_serving` API to convert it into a model file that can be used for Paddle Serving.
```python
import paddle_serving_client.io as serving_io
serving_io.inference_model_to_serving(dirname, serving_server="serving_server", serving_client="serving_client", model_filename=None, params_filename=None )
```
Arguments are the same as `inference_model_to_serving` API.
| Argument | Type | Default | Description |
|--------------|------|-----------|--------------------------------|
| `dirname` | str | - | Path of saved model files. Program file and parameter files are saved in this directory. |
| `serving_server` | str | `"serving_server"` | The path of model files and configuration files for server. |
| `serving_client` | str | `"serving_client"` | The path of configuration files for client. |
| `model_filename` | str | None | The name of file to load the inference program. If it is None, the default filename `__model__` will be used. |
| `params_filename` | str | None | The name of file to load all parameters. It is only used for the case that all parameters were saved in a single binary file. If parameters were saved in separate files, set it as None. |

### Convert From Dynamic Graph

PaddlePaddle 2.0 provides a new dynamic graph mode, so here we use imagenet ResNet50 dynamic graph as an example to teach how to export from a saved model and use it for real online inference scenarios.

```
wget https://paddle-serving.bj.bcebos.com/others/dygraph_res50.tar #模型
wget https://paddle-serving.bj.bcebos.com/imagenet-example/daisy.jpg #示例输入（向日葵）
tar xf dygraph_res50.tar
python -m paddle_serving_client.convert --dirname . --model_filename dygraph_model.pdmodel --params_filename dygraph_model.pdiparams --serving_server serving_server --serving_client serving_client```

We can see that the `serving_server` and `serving_client` folders hold the server and client configuration of the model respectively

Start the server (GPU)

```
python -m paddle_serving_server.serve --model serving_server --port 9393 --gpu_id 0
```

Client (`test_client.py`)
```
from paddle_serving_client import Client
from paddle_serving_app.reader import Sequential, File2Image, Resize, CenterCrop
from paddle_serving_app.reader import RGB2BGR, Transpose, Div, Normalize

client = Client()
client.load_client_config(
    "serving_client/serving_client_conf.prototxt")
client.connect(["127.0.0.1:9393"])

seq = Sequential([
    File2Image(), Resize(256), CenterCrop(224), RGB2BGR(), Transpose((2, 0, 1)),
    Div(255), Normalize([0.485, 0.456, 0.406], [0.229, 0.224, 0.225], True)
])

image_file = "daisy.jpg"
img = seq(image_file)
fetch_map = client.predict(feed={"inputs": img}, fetch=["save_infer_model/scale_0.tmp_0"])
print(fetch_map["save_infer_model/scale_0.tmp_0"].reshape(-1))
```
Run
```
python test_client.py
```

You can see that the prediction has been successfully executed. The above is the content predicted by the dynamic graph ResNet50 model on Serving. The use of other dynamic graph models is similar.

## Save from training or prediction script (Static Graph Mode) 
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
