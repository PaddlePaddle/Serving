## Paddle Serving for Windows Users

(English|[简体中文](./Windows_Tutorial_CN.md))

### Summary

This document guides users how to build Paddle Serving service on the Windows platform. Due to the limited support of third-party libraries, the Windows platform currently only supports the use of web services to build local predictor prediction services. If you want to experience all the services, you need to use Docker for Windows to simulate the operating environment of Linux.

### Running Paddle Serving on Native Windows System

**Configure Python environment variables to PATH**: **We only support Python 3.6+ on Native Windows System.**. First, you need to add the directory where the Python executable program is located to the PATH. Usually in **System Properties/My Computer Properties**-**Advanced**-**Environment Variables**, click Path and add the path at the beginning. For example, `C:\Users\$USER\AppData\Local\Programs\Python\Python36`, and finally click **OK** continuously. If you enter python on Powershell, you can enter the python interactive interface, indicating that the environment variable configuration is successful.

**Install wget**: Because all the downloads in the tutorial and the built-in model download function in `paddle_serving_app` all use the wget tool, download the binary package at the [link](http://gnuwin32.sourceforge.net/packages/wget.htm), unzip and copy it to `C:\Windows\System32`, if there is a security prompt, you need to pass it.

**Install Git**: For details, see [Git official website](https://git-scm.com/downloads)

**Install the necessary C++ library (optional)**: Some users may encounter the problem that the dll cannot be linked during the `import paddle` stage. It is recommended to [Install Visual Studio Community Edition](https://visualstudio.microsoft.com/), and install the relevant components of C++.

**Install Paddle and Serving**: In Powershell, execute

```
python -m pip install -U paddle_serving_server paddle_serving_client paddle_serving_app paddlepaddle`
```

for GPU users,

```
python -m pip install -U paddle_serving_server_gpu paddle_serving_client paddle_serving_app paddlepaddle-gpu
```

**Git clone Serving Project:**

```
git clone https://github.com/paddlepaddle/Serving
pip install -r python/requirements_win.txt
```

**Run OCR example**:

```
cd Serving/examples/C++/PaddleOCR/ocr/
python -m paddle_serving_app.package --get_model ocr_rec
tar -xzvf ocr_rec.tar.gz
python -m paddle_serving_app.package --get_model ocr_det
tar -xzvf ocr_det.tar.gz
python ocr_debugger_server.py cpu &
python ocr_web_client.py
```

### Create a new Paddle Serving Web Service on Windows

Currently Windows supports the Local Predictor of the Web Service framework. The server code framework is as follows

```
# filename:your_webservice.py
from paddle_serving_server.web_service import WebService
# If it is the GPU version, please use from paddle_serving_server.web_service import WebService
class YourWebService(WebService):
    def preprocess(self, feed=[], fetch=[]):
        #Implement pre-processing here
        #feed_dict is key: var names, value: numpy array input
        #fetch_names is a list of fetch variable names
        The meaning of #is_batch is whether the numpy array in the value of feed_dict contains the batch dimension
        return feed_dict, fetch_names, is_batch
    def postprocess(self, feed={}, fetch=[], fetch_map=None):
        #fetch map is the returned dictionary after prediction, the key is the fetch names given when the process returns, and the value is the var specific value corresponding to the fetch names
        #After processing here, the result needs to be converted into a dictionary again, and the type of values should be a list, so that it can be serialized in JSON to facilitate web return
        return response

your_service = YourService(name="XXX")
your_service.load_model_config("your_model_path")
your_service.prepare_server(workdir="workdir", port=9292)
# If you are a GPU user, you can refer to the python example under Serving/examples/Pipeline/PaddleOCR/ocr
your_service.run_debugger_service()
# Windows platform cannot use run_rpc_service() interface
your_service.run_web_service()
```

Client code example

```
# filename:your_client.py
import requests
import json
import base64
import os, sys
import time
import cv2 # If you need to upload pictures
# Used for image reading, the principle is to use base64 encoding file content
def cv2_to_base64(image):
    return base64.b64encode(image).decode(
        'utf8') #data.tostring()).decode('utf8')

headers = {"Content-type": "application/json"}
url = "http://127.0.0.1:9292/XXX/prediction" # XXX depends on the initial name parameter of the server YourService
r = requests.post(url=url, headers=headers, data=json.dumps(data))
print(r.json())
```

The user only needs to follow the above instructions and implement the relevant content in the corresponding function. For more information, please refer to [How to develop a new Web Service? ](./C++_Serving/Http_Service_CN.md)

Execute after development

```
python your_webservice.py &
python your_client.py
```

Because the port needs to be occupied, there may be a security prompt during the startup process. Please click through and an IP address will be generated. It should be noted that when the Windows platform starts the service, the local IP address may not be 127.0.0.1. You need to confirm the IP address and then see how the Client should set the access IP.

### Docker for Windows User Guide

The above content is used for native Windows. If users want to experience complete functions, they need to use Docker tools to model Linux systems.

Please refer to [Docker Desktop](https://www.docker.com/products/docker-desktop) to install Docker

After installation, start the docker linux engine and download the relevant image. In the Serving directory

```
docker pull registry.baidubce.com/paddlepaddle/serving:latest-devel
# There is no expose port here, users can set -p to perform port mapping as needed
docker run --rm -dit --name serving_devel -v $PWD:/Serving registry.baidubce.com/paddlepaddle/serving:latest-devel
docker exec -it serving_devel bash
cd /Serving
```

The rest of the operations are exactly the same as the Linux version.
