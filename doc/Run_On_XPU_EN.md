## Paddle Serving Using Baidu Kunlun Chips

(English|[简体中文](./Run_On_XPU_CN.md))

Paddle serving supports deployment using Baidu Kunlun chips. Currently, it supports deployment on the ARM CPU server with Baidu Kunlun chips
 (such as Phytium FT-2000+/64), or Intel CPU with Baidu Kunlun chips. We will improve
 the deployment capability on various heterogeneous hardware servers in the future. 
 
## Install docker images
We recommend using the docker deployment service. In the xpu environment, you can refer to the [Docker image document](Docker_Images_EN.md) to install the xpu image, and further complete tasks such as construction, installation, and deployment.

## Compilation and installation
Refer to [compile](./Compile_EN.md) document to setup the compilation environment. The following is based on FeiTeng FT-2000 +/64 platform.
### Compilatiton
* Compile the Serving Server
```
cd Serving
mkdir -p server-build-arm && cd server-build-arm

cmake -DPYTHON_INCLUDE_DIR=/usr/include/python3.7m/ \
    -DPYTHON_LIBRARIES=/usr/lib64/libpython3.7m.so \
    -DPYTHON_EXECUTABLE=/usr/bin/python \
    -DWITH_PYTHON=ON \
    -DWITH_LITE=ON \
    -DWITH_XPU=ON \
    -DSERVER=ON ..
make -j10
```
You can run `make install` to produce the target in `./output` directory. Add `-DCMAKE_INSTALL_PREFIX=./output` to specify the output path to CMake command shown above. Please specify `-DWITH_MKL=ON` on Intel CPU platform with AVX2 support.  
* Compile the Serving Client
```
mkdir -p client-build-arm && cd client-build-arm

cmake -DPYTHON_INCLUDE_DIR=/usr/include/python3.7m/ \
    -DPYTHON_LIBRARIES=/usr/lib64/libpython3.7m.so \
    -DPYTHON_EXECUTABLE=/usr/bin/python \
    -DWITH_PYTHON=ON \
    -DWITH_LITE=ON \
    -DWITH_XPU=ON \
    -DCLIENT=ON ..

make -j10
```
* Compile the App
```
cd Serving 
mkdir -p app-build-arm && cd app-build-arm

cmake -DPYTHON_INCLUDE_DIR=/usr/include/python3.7m/ \
    -DPYTHON_LIBRARIES=/usr/lib64/libpython3.7m.so \
    -DPYTHON_EXECUTABLE=/usr/bin/python \
    -DWITH_PYTHON=ON \
    -DWITH_LITE=ON \
    -DWITH_XPU=ON \
    -DAPP=ON ..

make -j10
```
### Install the wheel package
After the compilations stages above, the whl package will be generated in ```python/dist/``` under the specific temporary directories.
For example, after the Server Compiation step，the whl package will be produced under the server-build-arm/python/dist directory, and you can run ```pip install -u python/dist/*.whl``` to install the package.

## Request parameters description
In order to deploy serving
 service on the arm server with Baidu Kunlun xpu chips and use the acceleration capability of Paddle-Lite，please specify the following parameters during deployment.
| param    | param description                | about                                                              |
| :------- | :------------------------------- | :----------------------------------------------------------------- |
| use_lite | using Paddle-Lite Engine         | use the inference capability of Paddle-Lite                        |
| use_xpu  | using Baidu Kunlun for inference | need to be used with the use_lite option                           |
| ir_optim | open the graph optimization      | refer to[Paddle-Lite](https://github.com/PaddlePaddle/Paddle-Lite) |
## Deplyment examples
### Download the model
```
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/uci_housing.tar.gz
tar -xzf uci_housing.tar.gz
```
### Start RPC service
There are mainly three deployment methods：
* deploy on the cpu server with Baidu xpu using the acceleration capability of Paddle-Lite and xpu；
* deploy on the cpu server standalone with Paddle-Lite；
* deploy on the cpu server standalone without Paddle-Lite.
    
The first two deployment methods are recommended.

Start the rpc service, deploying on cpu server with Baidu Kunlun chips，and accelerate with Paddle-Lite and Baidu Kunlun xpu.
```
python3 -m paddle_serving_server.serve --model uci_housing_model --thread 6 --port 9292 --use_lite --use_xpu --ir_optim
```
Start the rpc service, deploying on cpu server，and accelerate with Paddle-Lite.
```
python3 -m paddle_serving_server.serve --model uci_housing_model --thread 6 --port 9292 --use_lite --ir_optim
```
Start the rpc service, deploying on cpu server.
```
python3 -m paddle_serving_server.serve --model uci_housing_model --thread 6 --port 9292
```
### 
```
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
## Others
### Model example and explanation

Some examples are provided below, and other models can be modifed with reference to these examples.
| sample name | sample links                                                |
| :---------- | :---------------------------------------------------------- |
| fit_a_line  | [fit_a_line_xpu](../examples/C++/xpu/fit_a_line_xpu)     |
| resnet      | [resnet_v2_50_xpu](../examples/C++/xpu/resnet_v2_50_xpu) |

Note：Supported model lists refer to [doc](https://paddlelite.paddlepaddle.org.cn/introduction/support_model_list.html). There are differences in the adaptation of different models, and there may be some unsupported cases. If you have any problem，please submit [Github issue](https://github.com/PaddlePaddle/Serving/issues), and we will follow up in real time.

### Kunlun chip related reference materials
* [PaddlePaddle on Baidu Kunlun xpu chips](https://www.paddlepaddle.org.cn/documentation/docs/zh/develop/guides/xpu_docs/index_cn.html)
* [Deployment on Baidu Kunlun xpu chips using PaddleLite](https://paddlelite.paddlepaddle.org.cn/demo_guides/baidu_xpu.html)
