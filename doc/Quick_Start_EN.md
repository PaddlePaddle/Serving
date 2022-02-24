## Paddle Serving Quick Start Examples

(English|[简体中文](./Quick_Start_CN.md))

This quick start example is mainly for those users who already have a model to deploy, and we also provide a model that can be used for deployment. in case if you want to know how to complete the process from offline training to online service, please refer to the AiStudio tutorial above.

### Boston House Price Prediction model

get into the Serving git directory, and change dir to `fit_a_line`
``` shell
cd Serving/examples/C++/fit_a_line
sh get_data.sh
```

Paddle Serving provides HTTP and RPC based service for users to access

### RPC service

A user can also start a RPC service with `paddle_serving_server.serve`. RPC service is usually faster than HTTP service, although a user needs to do some coding based on Paddle Serving's python client API. Note that we do not specify `--name` here. 
``` shell
python3 -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9292
```

For a complete list of parameters, see the document [Serving Configuration](Serving_Configure_CN.md#c-serving)

```python
# A user can visit rpc service through paddle_serving_client API
from paddle_serving_client import Client
import numpy as np
client = Client()
client.load_client_config("uci_housing_client/serving_client_conf.prototxt")
client.connect(["127.0.0.1:9292"])
data = [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727,
        -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332]
fetch_map = client.predict(feed={"x": np.array(data).reshape(1,13,1)}, fetch=["price"])
print(fetch_map)
```
Here, `client.predict` function has two arguments. `feed` is a `python dict` with model input variable alias name and values. `fetch` assigns the prediction variables to be returned from servers. In the example, the name of `"x"` and `"price"` are assigned when the servable model is saved during training.


### WEB service
Users can also put the data format processing logic on the server side, so that they can directly use curl to access the service, refer to the following case whose path is `Serving/examples/C++/fit_a_line`

```
python3 -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9292
```
for client side,
```
curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"x": [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332]}], "fetch":["price"]}' http://127.0.0.1:9292/uci/prediction
```
the response is
```
{"result":{"price":[[18.901151657104492]]}}
```
<h3 align="center">Pipeline Service</h3>

Paddle Serving provides industry-leading multi-model tandem services, which strongly supports the actual operating business scenarios of major companies, please refer to [OCR word recognition](../examples/Pipeline/PaddleOCR/ocr/).

we get two models
```
python3 -m paddle_serving_app.package --get_model ocr_rec
tar -xzvf ocr_rec.tar.gz
python3 -m paddle_serving_app.package --get_model ocr_det
tar -xzvf ocr_det.tar.gz
```
then we start server side, launch two models as one standalone web service
```
python3 web_service.py
```
http request
```
python3 pipeline_http_client.py
```
grpc request
```
python3 pipeline_rpc_client.py
```
output
```
{'err_no': 0, 'err_msg': '', 'key': ['res'], 'value': ["['土地整治与土壤修复研究中心', '华南农业大学1素图']"]}
```

<h3 align="center">Stop Serving/Pipeline service</h3>

**Method one** ：Ctrl+C to quit

**Method Two** ：In the path where starting the Serving/Pipeline service or the path which environment variable SERVING_HOME set (the file named ProcessInfo.json exists in this path)

```
python3 -m paddle_serving_server.serve stop
```
