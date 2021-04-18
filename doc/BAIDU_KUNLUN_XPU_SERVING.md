# Paddle Serving Using Baidu Kunlun Chips
(English|[简体中文](./BAIDU_KUNLUN_XPU_SERVING_CN.md))

Paddle serving supports deployment using Baidu Kunlun chips. At present, the pilot support is deployed on the ARM server with Baidu Kunlun chips
 (such as Phytium FT-2000+/64). We will improve
 the deployment capability on various heterogeneous hardware servers in the future. 

# Compilation and installation
Refer to [compile](COMPILE.md) document to setup the compilation environment。
## Compilatiton
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
You can run `make install` to produce the target in `./output` directory. Add `-DCMAKE_INSTALL_PREFIX=./output` to specify the output path to CMake command shown above。
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
## Install the wheel package
After the compilations stages above, the whl package will be generated in ```python/dist/``` under the specific temporary directories.
For example, after the Server Compiation step，the whl package will be produced under the server-build-arm/python/dist directory, and you can run ```pip install -u python/dist/*.whl``` to install the package。

# Request parameters description
In order to deploy serving
 service on the arm server with Baidu Kunlun xpu chips and use the acceleration capability of Paddle-Lite，please specify the following parameters during deployment。
| param    | param description                | about                                                              |
| :------- | :------------------------------- | :----------------------------------------------------------------- |
| use_lite | using Paddle-Lite Engine         | use the inference capability of Paddle-Lite                        |
| use_xpu  | using Baidu Kunlun for inference | need to be used with the use_lite option                           |
| ir_optim | open the graph optimization      | refer to[Paddle-Lite](https://github.com/PaddlePaddle/Paddle-Lite) |
# Deplyment examples
## Download the model
```
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/uci_housing.tar.gz
tar -xzf uci_housing.tar.gz
```
## Start RPC service
There are mainly three deployment methods：
* deploy on the ARM server with Baidu xpu using the acceleration capability of Paddle-Lite and xpu；
* deploy on the ARM server standalone with Paddle-Lite；
* deploy on the ARM server standalone without Paddle-Lite。
    
The first two deployment methods are recommended。

Start the rpc service, deploying on ARM server with Baidu Kunlun chips，and accelerate with Paddle-Lite and Baidu Kunlun xpu.
```
python3 -m paddle_serving_server_gpu.serve --model uci_housing_model --thread 6 --port 9292 --use_lite --use_xpu --ir_optim
```
Start the rpc service, deploying on ARM server，and accelerate with Paddle-Lite.
```
python3 -m paddle_serving_server_gpu.serve --model uci_housing_model --thread 6 --port 9292 --use_lite --ir_optim
```
Start the rpc service, deploying on ARM server.
```
python3 -m paddle_serving_server_gpu.serve --model uci_housing_model --thread 6 --port 9292
```
## 
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
Some examples are provided below, and other models can be modifed with reference to these examples。
| sample name | sample links                                                |
| :---------- | :---------------------------------------------------------- |
| fit_a_line  | [fit_a_line_xpu](../python/examples/xpu/fit_a_line_xpu)     |
| resnet      | [resnet_v2_50_xpu](../python/examples/xpu/resnet_v2_50_xpu) |
