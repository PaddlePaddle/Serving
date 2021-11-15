## Windows平台使用Paddle Serving指导

([English](./Windows_Tutorial_EN.md)|简体中文）

### 综述

本文档指导用户如何在Windows平台手把手搭建Paddle Serving服务。由于受限第三方库的支持，Windows平台目前只支持用web service的方式搭建local predictor预测服务。如果想要体验全部的服务，需要使用Docker for Windows，来模拟Linux的运行环境。

### 原生Windows系统运行Paddle Serving

**配置Python环境变量到PATH**：**目前原生Windows仅支持Python 3.6或更高版本**。首先需要将Python的可执行程序所在目录加入到PATH当中。通常在**系统属性/我的电脑属性**-**高级**-**环境变量** ，点选Path，并在开头加上路径。例如`C:\Users\$USER\AppData\Local\Programs\Python\Python36`，最后连续点击**确定** 。在Powershell上如果输入python可以进入python交互界面，说明环境变量配置成功。

**安装wget工具**：由于教程当中所有的下载，以及`paddle_serving_app`当中内嵌的模型下载功能，都是用到wget工具，在链接[下载wget](http://gnuwin32.sourceforge.net/packages/wget.htm)，解压后复制到`C:\Windows\System32`下，如有安全提示需要通过。

**安装Git工具**： 详情参见[Git官网](https://git-scm.com/downloads)

**安装必要的C++库（可选）**：部分用户可能会在`import paddle`阶段遇见dll无法链接的问题，建议[安装Visual Studio社区版本](https://visualstudio.microsoft.com/) ，并且安装C++的相关组件。

**安装Paddle和Serving**：在Powershell，执行

```
python -m pip install -U paddle_serving_server paddle_serving_client paddle_serving_app paddlepaddle`
```

如果是GPU用户

```
python -m pip install -U paddle_serving_server_gpu paddle_serving_client paddle_serving_app paddlepaddle-gpu
```

**下载Serving库**：

```
git clone https://github.com/paddlepaddle/Serving
pip install -r python/requirements_win.txt
```

**运行OCR示例**：

```
cd Serving/examples/C++/PaddleOCR/ocr/
python -m paddle_serving_app.package --get_model ocr_rec
tar -xzvf ocr_rec.tar.gz
python -m paddle_serving_app.package --get_model ocr_det
tar -xzvf ocr_det.tar.gz
python ocr_debugger_server.py cpu &
python ocr_web_client.py
```

### 创建新的Windows支持的Paddle Serving服务

目前Windows支持Web Service框架的Local Predictor。服务端代码框架如下

```
# filename:your_webservice.py
from paddle_serving_server.web_service import WebService
# 如果是GPU版本，请使用 from paddle_serving_server.web_service import WebService
class YourWebService(WebService):
    def preprocess(self, feed=[], fetch=[]):
        #在这里实现前处理
        #feed_dict是 key: var names, value: numpy array input
        #fetch_names 是fetch变量名列表
        #is_batch的含义是feed_dict的value里的numpy array是否包含了batch维度
        return feed_dict, fetch_names, is_batch
    def postprocess(self, feed={}, fetch=[], fetch_map=None):
        #fetch map是经过预测之后的返回字典，key是process返回时给定的fetch names，value是对应fetch names的var具体值
        #在这里做处理之后，结果需重新转换成字典，并且values的类型应是列表list，这样可以JSON序列化方便web返回
        return response

your_service = YourService(name="XXX")
your_service.load_model_config("your_model_path")
your_service.prepare_server(workdir="workdir", port=9292)
# 如果是GPU用户，可以参照Serving/examples/Pipeline/PaddleOCR/ocr下的python示例
your_service.run_debugger_service()
# Windows平台不可以使用 run_rpc_service()接口
your_service.run_web_service()
```

客户端代码示例

```
# filename：your_client.py
import requests
import json
import base64
import os, sys
import time
import cv2 # 如果需要上传图片
# 用于图片读取，原理是采用base64编码文件内容
def cv2_to_base64(image):
    return base64.b64encode(image).decode(
        'utf8')  #data.tostring()).decode('utf8')

headers = {"Content-type": "application/json"}
url = "http://127.0.0.1:9292/XXX/prediction" # XXX取决于服务端YourService的初始化name参数
r = requests.post(url=url, headers=headers, data=json.dumps(data))
print(r.json())
```

用户只需要按照如上指示，在对应函数中实现相关内容即可。更多信息请参见[如何开发一个新的Web Service？](./C++_Serving/Http_Service_CN.md)

开发完成后执行

```
python your_webservice.py &
python your_client.py
```

因为需要占用端口，因此启动过程可能会有安全提示，请点选通过，就会有IP地址生成。需要注意的是，Windows平台启动服务时，本地IP地址可能不是127.0.0.1，需要确认好IP地址再看Client应该如何设定访问IP。

### Docker for Windows 使用指南

以上内容用于原生的Windows，如果用户想要体验完整的功能，需要使用Docker工具，来模拟Linux系统。

安装Docker请参考[Docker Desktop](https://www.docker.com/products/docker-desktop)

安装之后启动docker的linux engine，下载相关镜像。在Serving目录下

```
docker pull registry.baidubce.com/paddlepaddle/serving:latest-devel
# 此处没有expose端口，用户可根据需要设置-p来进行端口映射
docker run --rm -dit --name serving_devel -v $PWD:/Serving registry.baidubce.com/paddlepaddle/serving:latest-devel 
docker exec -it serving_devel bash
cd /Serving
```

其余操作与Linux版本完全一致。
