Ernie 3.0 百亿参数模型部署案例

- [1.任务介绍](#1)
- [2.运行环境说明](#2)
- [3.部署步骤](#3)
    - [3.1 使用 Docker 镜像](#3.1)
    - [3.2 下载 Serving 离线 Wheel 部署包](#3.2)
    - [3.3 下载 output.tar.gz 部署包](#3.3)
    - [3.4 下载百亿参数问答模型](#3.4)
    - [3.5 启动服务](#3.5)
    - [3.6 发送客户端请求](#3.6)
    - [3.7 关闭服务](#3.7)
- [4.编译方法](#4)
    - [4.1 编译 Paddle](#4.1)
    - [4.2 编译 Serving](#4.2)

<a name="1"></a>

## 1.任务介绍

问答任务（Question-Answering）介绍：是自然语言处理 (NLP) 中一种业务场景，问答任务模型可构建自动回答以自然语言提出的问题。通常，通过查询知识或信息的结构化数据库来构建答案，也可以从非结构化的自然语言文档集合中提取答案。

<a name="2"></a>

## 2.运行环境说明

要求：
-- 模型大小：20 GB
-- 显卡要求：有单机4卡或多机4卡
-- 显存要求：12 GB 级以上
-- 计算环境：CUDA >= 11.0, cuDNN > 8
-- Paddle版本：基于 Paddle 2.3 分支开启多个编译选项，详见 ##4.联编方法
-- Serving版本：使用Serving v0.9.0 或 develop 分支联编 Paddle 2.3，详见 ##4.联编方法

<a name="3"></a>

## 3.部署步骤：
部署百亿参数模型与部署常规模型相比，大部分步骤是相同的，在启动服务和Client使用上有差异，具体步骤如下：
1. 使用 Docker 镜像
2. 下载 Serving 离线 Wheel 部署包
3. 下载 output.tar.gz 部署包
4. 下载百亿模型
5. 启动服务
6. 发送Client请求
7. 关闭服务

<a name="3.1"></a>

### 3.1 使用 Docker 镜像
首先，拉取 CUDA 11.2  serving 镜像，启动并进入镜像，创建work目录并进入目录。
```shell
docker pull registry.baidubce.com/paddlepaddle/serving:0.9.0-cuda11.2-cudnn8-devel
nvidia-docker run --name serving_dist_demo --network=host -it registry.baidubce.com/paddlepaddle/serving:0.9.0-cuda11.2-cudnn8-devel bash
λ yq01-inf-hic-k8s-a100-aa24-0087 /home mkdir -p work && cd work
```

<a name="3.2"></a>

### 3.2 下载 Serving 离线 Wheel 部署包
安装特定版本 paddle_serving_server 和 paddle_serving_client。特别提示，此版本与 v0.9.0 发布版不同，差异点是集成联编特定选项的 Paddle 推理库，联编方法详见 ##4.联编方法。
```shell
wget https://paddle-serving.bj.bcebos.com/distributed_demos/py37_offline_whl_packages.tar
tar xvf py37_offline_whl_packages.tar
cd py37_offline_whl_packages

## 安装 特定版本 Serving
python3.7 install_local_wheels.py --whl_path paddle_serving_server_gpu-0.0.0.post112-py3-none-any.whl --dep_whl_path py37_offline_whls/serving_dependent_wheels/
python3.7 install_local_wheels.py --whl_path paddle_serving_client-0.0.0-cp37-none-any.whl --dep_whl_path py37_offline_whls/serving_dependent_wheels

## 安装 特定版本 Paddle 2.3 
pip3.7 install paddlepaddle-gpu==2.3.0.post112 -f https://www.paddlepaddle.org.cn/whl/linux/mkl/avx/stable.html

## 安装 依赖库
pip3.7 install sentencepiece
```

<a name="3.3"></a>

### 3.3 下载 output.tar.gz 部署包
output.tar.gz 部署包包含所有依赖库、代码和运行脚本。进入 output 目录后，运行 get_models.sh 下载百亿模型
```shell
wget https://paddle-serving.bj.bcebos.com/distributed_demos/output.tar.gz
tar zxvf output.tar.gz
cd output 
```

目录结构如下：
```
drwxr-xr-x 10 root root 4.0K May 27 08:46 ./
drwxr-xr-x  4 root root 4.0K May 27 08:51 ../
-rw-r--r--  1 root root 3.0K May 18 03:36 batching.py
drwxr-xr-x  2 root root 4.0K May 17 03:23 bin/
-rw-r--r--  1 root root   64 May 17 08:38 config_pp1mp4.csv
drwxr-xr-x  2 root root 4.0K May 27 07:14 data/
-rw-r--r--  1 root root   66 May 17 14:25 dev.txt
drwxr-xr-x  2 root root 4.0K May 27 06:59 dict/
-rw-r--r--  1 root root  16K May 18 04:06 finetune_args.py
-rwxrwxrwx  1 root root  556 May 27 08:23 get_models.sh*
drwxr-xr-x  2 root root 4.0K May 23 06:25 lib/
-rw-r--r--  1 root root 2.3M May 20 12:07 libcrypto.so.1.0.0
-rw-r--r--  1 root root 417K May 20 12:07 libssl.so.1.0.0
-rw-r--r--  1 root root  277 May 23 06:44 model_conf
drwxr-xr-x  4 root root 4.0K May 17 03:40 nccl_2.3.5/
-rw-r--r--  1 root root  584 May 27 08:43 ProcessInfo.json
-rw-r--r--  1 root root    0 May 18 03:37 ps_usr_print_log
drwxr-xr-x  2 root root 4.0K May 18 04:07 __pycache__/
drwxr-xr-x  3 root root 4.0K May 27 07:28 reader/
-rwxr-xr-x  1 root root 1.4K May 17 14:12 start_master_serving.sh*
-rwxrwxrwx  1 root root  195 May 18 02:52 start.sh*
-rwxr-xr-x  1 root root 1.9K May 23 06:43 start_slave_serving.sh*
-rwxrwxrwx  1 root root  281 May 17 13:19 stop.sh*
-rw-r--r--  1 root root 7.2K May 27 07:29 test_client_while.py
-rw-r--r--  1 root root  17K May 18 03:36 tokenization.py
drwxr-xr-x  3 root root 4.0K May 17 14:18 utils/
```

<a name="3.4"></a>

### 3.4 下载百亿参数问答模型

运行命令下载模型，建议提前关闭 HTTP 代理。
```
unset http_proxy
unset https_proxy
sh get_models.sh
```
下载模型到 serving_model_pp1mp4 目录，共有12个目录，28个文件。模型已保存出带有feed/fetch 信息的prototxt文件。
```
serving_model_pp1mp4/
├── rank_0
│   ├── serving_client
│   │   ├── serving_client_conf.prototxt
│   │   └── serving_client_conf.stream.prototxt
│   └── serving_server
│       ├── fluid_time_file
│       ├── serving_server_conf.prototxt
│       ├── serving_server_conf.stream.prototxt
│       ├── step_0.pdiparams
│       └── step_0.pdmodel
├── rank_1
│   ├── serving_client
│   │   ├── serving_client_conf.prototxt
│   │   └── serving_client_conf.stream.prototxt
│   └── serving_server
│       ├── fluid_time_file
│       ├── serving_server_conf.prototxt
│       ├── serving_server_conf.stream.prototxt
│       ├── step_0.pdiparams
│       └── step_0.pdmodel
├── rank_2
│   ├── serving_client
│   │   ├── serving_client_conf.prototxt
│   │   └── serving_client_conf.stream.prototxt
│   └── serving_server
│       ├── fluid_time_file
│       ├── serving_server_conf.prototxt
│       ├── serving_server_conf.stream.prototxt
│       ├── step_0.pdiparams
│       └── step_0.pdmodel
└── rank_3
    ├── serving_client
    │   ├── serving_client_conf.prototxt
    │   └── serving_client_conf.stream.prototxt
    └── serving_server
        ├── fluid_time_file
        ├── serving_server_conf.prototxt
        ├── serving_server_conf.stream.prototxt
        ├── step_0.pdiparams
        └── step_0.pdmodel

12 directories, 28 files
```

<a name="3.5"></a>

### 3.5 启动服务

运行如下命令后台启动 1个 Master Serving 进程 和 4 个 Worker Serving 进程。
```shell
bash start.sh
```

每个 Worker Serving 的启动目录在 work_dir/worker_0/1/2/3 目录下，启动信息输出到 slave_start.log 日志中。Master Serving 在当前目录启动，启动日志信息在 log/master_start.log 日志中。

```
-------------------------------------------
cvs_file: ../../config_pp1mp4.csv
model_path: ../../serving_model_pp1mp4/rank_0/serving_server/
dist_subgraph_idx: 0
serving_port: 9001
work dir: work_dir/worker_0
Starting 0 slave serving backgroud...
Starting done. start log is work_dir/worker_0/slave_start.log
------------------------------------------------------------
cvs_file: ../../config_pp1mp4.csv
model_path: ../../serving_model_pp1mp4/rank_1/serving_server/
dist_subgraph_idx: 1
serving_port: 9002
work dir: work_dir/worker_1
Starting 1 slave serving backgroud...
Starting done. start log is work_dir/worker_1/slave_start.log
------------------------------------------------------------
cvs_file: ../../config_pp1mp4.csv
model_path: ../../serving_model_pp1mp4/rank_2/serving_server/
dist_subgraph_idx: 2
serving_port: 9003
work dir: work_dir/worker_2
Starting 2 slave serving backgroud...
Starting done. start log is work_dir/worker_2/slave_start.log
------------------------------------------------------------
cvs_file: ../../config_pp1mp4.csv
model_path: ../../serving_model_pp1mp4/rank_3/serving_server/
dist_subgraph_idx: 3
serving_port: 9004
work dir: work_dir/worker_3
Starting 3 slave serving backgroud...
Starting done. start log is work_dir/worker_3/slave_start.log
------------------------------------------------------------
-------------------------------------------
SERVING_MASTER_PORT: 2003
SERVING_SLAVE_PORTS: 4
slave address: 127.0.0.1:9001,127.0.0.1:9002,127.0.0.1:9003,127.0.0.1:9004
-------------------------------------------------------------

Starting master serving backgroud...
Starting done. start log is log/master_start.log
```

以上打印信息表示 Master Serving 和 4个 Worker Serving 启动配置信息，服务已处于启动状态中，需要等待。

由于模型较大加载时间较长，并且 Worker Serving 之间还需要通讯初始化，当所有 Worker 服务启动完成时才能对外提供服务。

当 Worker Serving 的 `serving.Info`日志中出现`C++ Serving service started successfully!`日志表示 Worker Serving 启动成功，可以对外提供服务。
```
I0527 09:06:55.883172  1155 analysis_predictor.cc:1007] ======= optimize end =======
I0527 09:06:55.892191  1155 gen_comm_id_helper.cc:205] Server listening on: 127.0.0.1:9993 successful.
W0527 09:07:39.848902  1155 gpu_context.cc:278] Please NOTE: device: 2, GPU Compute Capability: 8.0, Driver API Version: 11.6, Runtime API Version: 11.2
W0527 09:07:39.856563  1155 gpu_context.cc:306] device: 2, cuDNN Version: 8.1.
I0527 09:07:50.847107  1155 collective_helper.cc:104] nccl communicator of rank 2 in ring 0 has been created on device 2
I0527 09:07:50.847275  1155 task_node.cc:45] Constructing TaskNode for DistModelInf. The TaskNode's id is: 2. And the TaskNode's max_run_time and max_slot_num will be set to 1.
I0527 09:07:50.894994  1155 message_bus.cc:194] Message bus's listen port thread starts successful.
C++ Serving service started successfully!
```

<a name="3.6"></a>

### 3.6 发送客户端请求
运行以下命令启动客户端：
```
python3.7 test_client_while.py --model_prototxt_path serving_model_pp1mp4/rank_0/serving_client/ --serving_port 2003
```

运行结果如下：
```
serving port : 2003
model prototxt path: serving_model_pp1mp4/rank_0/serving_client/
prompt_str: ["%s%s"] multi_choice_prompt:
total-num: 1
examples: [Example(qid='1', text_a='How are you today?', text_b=''), Example(qid='2', text_a='The capital of China is', text_b='')]
WARNING: Logging before InitGoogleLogging() is written to STDERR
I0527 07:29:35.225399 146811 naming_service_thread.cpp:202] brpc::policy::ListNamingService("127.0.0.1:2003"): added 1
[DEBUG]  qid: 0 , tokens: ['how', 'are', 'you', 'today', '?'] token_ids: [1, 2248, 1875, 1631, 4014, 35] pos_ids: [0, 1, 2, 3, 4, 5]
[DEBUG]  qid: 0 , tokens: ['the', 'capital', 'of', 'china', 'is'] token_ids: [1, 1595, 4460, 1605, 1906, 1706] pos_ids: [0, 1, 2, 3, 4, 5]
running 0 text...
score: -14.7109375 current_tokens: ['[<S>]', 'how', 'are', 'you', 'today', '?', 'how', 'are', 'you', 'today', '?', '[<S>]', '[UNK]', '[UNK]', '[UNK]', '[UNK]', '[UNK]', '[UNK]', '[UNK]']
---------------------------------------------------------------------

running 1 text...
score: -27.109375 current_tokens: ['a', 'country', 'with', 'a', 'long', 'history', 'and', 'a', 'long', 'history', 'of', 'history', '.', '[<S>]', '，', '《', '》', '，', '《']
---------------------------------------------------------------------
```

此时，可以看到服务端计算结果：
```
Q: How are you today?
A: [<S>] how are you, today? how are you today ?[<S>]

Q: The capital of China is
A: a country with a long history and a long history of history.[<S>]，《》，《
```

<a name="3.7"></a>

### 3.7 关闭服务
运行 stop.sh 关闭所有进程。
```
bash stop.sh
```
所有进程关闭。
```
start kill serving
grep: warning: GREP_OPTIONS is deprecated; please use an alias or script
grep: warning: GREP_OPTIONS is deprecated; please use an alias or script
760 762 764 766 772 1124 1128 1130 1133 1135 1137 1138 1139 1140 1141
pid:760
pid:762
pid:764
pid:766
pid:772
pid:1124
pid:1128
pid:1130
pid:1133
pid:1135
pid:1137
pid:1138
pid:1139
pid:1140
pid:1141
end kill serving
```
<a name="4"></a>

## 4. 编译方法
由于大模型推理不仅要使用推理技术，要需要分布式技术，因此需要开启特定编译选项的 Paddle 和 Serving 才能满足需求。

<a name="4.1"></a>

### 4.1 编译 Paddle

首先，参考飞桨官网[使用Docker编译](https://www.paddlepaddle.org.cn/documentation/docs/zh/install/compile/linux-compile.html#compile_from_docker)步骤完成以下5步：
1.选择您希望储存PaddlePaddle的路径
2.进入Paddle目录下
3.拉取 PaddlePaddle 镜像
4.创建并进入已配置好编译环境的Docker容器
5.进入Docker后进入paddle目录下
6.切换到 `release/2.3` 进行编译
7.创建并进入/paddle/build路径下
8.使用以下命令安装相关依赖

9.执行 cmake 步骤使用以下命令，开启了多个编译选项，如分布式选项`-DWITH_DISTRIBUTE`
```
cmake .. -DWITH_MKLML=ON -DWITH_MKLDNN=ON -DWITH_GPU=ON -DWITH_TENSORRT=OFF -DWITH_C_API=OFF -DWITH_DISTRIBUTE=ON -DCMAKE_BUILD_TYPE=Release -DWITH_TESTING=OFF -DWITH_STYLE_CHECK=OFF -DWITH_CONTRIB=OFF -DWITH_GRPC=ON -DWITH_BRPC_RDMA=OFF -DWITH_FLUID_ONLY=ON -DWITH_INFERENCE_API_TEST=ON  -DWITH_DOC=ON -DCUDA_ARCH_NAME=Auto -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DWITH_PYTHON=ON -DPY_VERSION=3.7 -DWITH_PSCORE=ON
```

10. make -j10 执行编译

编译结果如下：
```
-rw-r--r--  1 root root   96 Apr 20 07:28 cblas_dummy.c
-rw-r--r--  1 root root 633K May 26 03:35 CMakeCache.txt
drwxr-xr-x 36 root root 4.0K May 26 03:49 CMakeFiles/
-rw-r--r--  1 root root 1.8K Apr 20 07:28 cmake_install.cmake
-rw-r--r--  1 root root 9.5M May 26 03:36 compile_commands.json
-rw-r--r--  1 root root  387 Apr 20 07:27 detect_cuda_archs.cu
-rw-r--r--  1 root root  159 May 26 03:35 detect_thrust.cu
-rw-r--r--  1 root root   58 May 26 03:35 FindNumpyPath.py
-rw-r--r--  1 root root 2.4K May 26 03:35 includes.txt
-rw-r--r--  1 root root 1.5K May 26 03:36 libcblas.a
-rw-r--r--  1 root root 1.5K May 26 03:36 libmkldnn.a
-rw-r--r--  1 root root 798K May 26 03:35 Makefile
-rw-r--r--  1 root root   99 Apr 20 07:28 mkldnn_dummy.c
drwxr-xr-x  9 root root 4.0K May 26 03:35 paddle/
drwxr-xr-x  7 root root 4.0K May 26 03:35 python/
drwxr-xr-x 30 root root 4.0K Apr 29 03:10 third_party/
```
动态和静态库保存在 `./paddle/fluid/inference/` 目录下：
```
drwxr-xr-x    5 root root 4.0K May 26 03:42 analysis/
drwxr-xr-x    4 root root 4.0K May 26 03:42 api/
drwxr-xr-x    3 root root 4.0K May 26 03:46 capi_exp/
-rw-r--r--    1 root root  302 May 26 03:35 check_symbol.cmake
drwxr-xr-x    6 root root 4.0K May 26 03:35 CMakeFiles/
-rw-r--r--    1 root root 1.8K May 26 03:35 cmake_install.cmake
-rw-r--r--    1 root root 1.4G May 26 03:46 libpaddle_inference.a
-rw-r--r--    1 root root 382K May 26 03:41 libpaddle_inference_io.a
-rwxr-xr-x    1 root root 624M May 26 03:47 libpaddle_inference.so*
-rw-r--r--    1 root root  24K Apr 20 07:28 Makefile
drwxr-xr-x 1420 root root 140K May 25 04:56 paddle_inference.dir/
-rw-r--r--    1 root root  138 May 26 03:46 paddle_inference_dummy.c
-rw-r--r--    1 root root 130K Apr 29 06:14 paddle_inference.mri
drwxr-xr-x    5 root root 4.0K May 26 03:40 tensorrt/
drwxr-xr-x    3 root root 4.0K May 26 03:38 utils/
```

<a name="4.2"></a>

### 4.2 编译 Serving
参考 [Paddle Serving源码编译](https://github.com/PaddlePaddle/Serving/blob/v0.9.0/doc/Compile_CN.md) 文档完成编译环境准备、下载代码库和环境变量准备 3个步骤。

下面介绍差异化编译步骤：

**1.Paddle Serving 采用动态库方式联编 Paddle**

首先修改 `Serving/cmake/paddlepaddle.cmake`，联编静态库`libpaddle_inference.a`变更为动态库`libpaddle_inference.so`。
```
ADD_LIBRARY(paddle_inference STATIC IMPORTED GLOBAL)
#SET_PROPERTY(TARGET paddle_inference PROPERTY IMPORTED_LOCATION ${PADDLE_INSTALL_DIR}/lib/libpaddle_inference.a)
SET_PROPERTY(TARGET paddle_inference PROPERTY IMPORTED_LOCATION ${PADDLE_INSTALL_DIR}/lib/libpaddle_inference.so)

if (WITH_ASCEND_CL OR WITH_XPU)
    SET_PROPERTY(TARGET paddle_inference PROPERTY IMPORTED_LOCATION ${PADDLE_INSTALL_DIR}/lib/libpaddle_inference.so)
endif()
```

**2.make编译**
设置完成环境变量后，编译paddle-serving-server，其 cmake 和 make 命令与编译示例是一致的。
```
mkdir build_server
cd build_server
cmake -DPYTHON_INCLUDE_DIR=$PYTHON_INCLUDE_DIR \
    -DPYTHON_LIBRARIES=$PYTHON_LIBRARIES \
    -DPYTHON_EXECUTABLE=$PYTHON_EXECUTABLE \
    -DCUDA_TOOLKIT_ROOT_DIR=${CUDA_PATH} \
    -DCUDNN_LIBRARY=${CUDNN_LIBRARY} \
    -DCUDA_CUDART_LIBRARY=${CUDA_CUDART_LIBRARY} \
    -DTENSORRT_ROOT=${TENSORRT_LIBRARY_PATH} \
    -DSERVER=ON \
    -DWITH_GPU=ON ..
make -j20
cd ..
```
编译目录 build_server 信息如下：
```
-rw-r--r--  1 root root   85 May  9 05:23 boost_dummy.c
-rw-r--r--  1 root root  42K Apr 25 08:08 CMakeCache.txt
drwxr-xr-x 19 root root 4.0K May 26 04:09 CMakeFiles/
-rw-r--r--  1 root root 1.9K Apr 25 08:08 cmake_install.cmake
-rw-r--r--  1 root root 215K May  9 05:23 compile_commands.json
drwxr-xr-x 10 root root 4.0K May  9 05:23 core/
-rw-r--r--  1 root root 1.5K May  9 05:23 libboost.a
-rw-r--r--  1 root root  27K May  9 05:23 Makefile
drwxr-xr-x  4 root root 4.0K May  9 05:23 paddle_inference/
drwxr-xr-x  8 root root 4.0K May  9 05:23 python/
drwxr-xr-x 16 root root 4.0K Apr 25 08:08 third_party/
```

在编译过程中会报错，需要用本地编译生成的Paddle动态库替换下载的Paddle动态库。其中`paddle_build`和`paddle_build`要替换为实际路径。
```
copy -R paddle_build/paddle/fluid/inference/*.so serving_build/third_party/Paddle/src/extern_paddle/paddle/lib/
copy -R paddle_build/paddle/fluid/inference/*.so serving_build/third_party/install/Paddle/lib/
```

再次运行 `make -j10` 继续编译。编译成功后生成二进制文件`serving`:
```
serving_build/core/general-server/serving
```
导出到环境变量中：
```
export SERVING_BIN=${PWD}/core/general-server/serving 
```