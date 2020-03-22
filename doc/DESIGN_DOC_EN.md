# Paddle Serving Design Doc

## 1. Design Objectives

- Long Term Vision: Online deployment of deep learning models will be a user-facing application in the future. Any AI developer will face the problem of deploying an online service for his or her trained model.
Paddle Serving is the official open source online deployment framework. The long term goal of Paddle Serving is to provide professional, reliable and easy-to-use online service to the last mile of AI application.

- Easy-To-Use: For algorithmic developers to quickly deploy their models online, Paddle Serving designs APIs that can be used with Paddle's training process seamlessly, most Paddle models can be deployed as a service with one line command.

- Industrial Oriented: To meet industrial deployment requirements, Paddle Serving supports lots of large-scale deployment functions: 1) Distributed Sparse Embedding Indexing. 2) Highly concurrent underlying communications. 3) Model Management, online A/B test, model online loading.

- Extensibility: Paddle Serving supports C++, Python and Golang client, and will support more clients with different languages. It is very easy to extend Paddle Serving to support other machine learning inference library, although currently Paddle inference library is the only official supported inference backend.


## 2. Module design and implementation

### 2.1 Python API interface design

#### 2.1.1 save a servable model
The inference phase of Paddle model focuses on 1) input variables of the model. 2) output variables of the model. 3) model structure and model parameters. Paddle Serving Python API provides a `save_model` interface for trained model, and save necessary information for Paddle Serving to use during deployment phase. An example is as follows:

``` python
import paddle_serving_client.io as serving_io
serving_io.save_model("serving_model", "client_conf",
                      {"words": data}, {"prediction": prediction},
                      fluid.default_main_program())
```
In the example, `{"words": data}` and `{"prediction": prediction}` assign the inputs and outputs of a model. `"words"` and `"prediction"` are alias names of inputs and outputs. The design of alias name is to help developers to memorize model inputs and model outputs. `data` and `prediction` are Paddle `[Variable](https://www.paddlepaddle.org.cn/documentation/docs/zh/api_cn/fluid_cn/Variable_cn.html#variable)` in training phase that often represents ([Tensor](https://www.paddlepaddle.org.cn/documentation/docs/zh/api_cn/fluid_cn/Tensor_cn.html#tensor)) or ([LodTensor](https://www.paddlepaddle.org.cn/documentation/docs/zh/beginners_guide/basic_concept/lod_tensor.html#lodtensor)). When the `save_model` API is called, two directories called `"serving_model"` and `"client_conf"` will be generated. The content of the saved model is as follows:

``` shell
.
├── client_conf
│   ├── serving_client_conf.prototxt
│   └── serving_client_conf.stream.prototxt
└── serving_model
    ├── embedding_0.w_0
    ├── fc_0.b_0
    ├── fc_0.w_0
    ├── fc_1.b_0
    ├── fc_1.w_0
    ├── fc_2.b_0
    ├── fc_2.w_0
    ├── lstm_0.b_0
    ├── lstm_0.w_0
    ├── __model__
    ├── serving_server_conf.prototxt
    └── serving_server_conf.stream.prototxt
```
`"serving_client_conf.prototxt"` and `"serving_server_conf.prototxt"` are the client side and the server side configurations of Paddle Serving, and `"serving_client_conf.stream.prototxt"` and `"serving_server_conf.stream.prototxt"` are the corresponding parts. Other contents saved in the directory are the same as Paddle saved inference model. We are considering to support `save_model` interface in Paddle training framework so that a user is not aware of the servable configurations. 

#### 2.1.2 Model loading on the server side

Prediction logics on the server side can be defined through Paddle Serving Server API with a few lines of code, an example is as follows:
``` python
import paddle_serving_server as serving
op_maker = serving.OpMaker()
read_op = op_maker.create('general_reader')
dist_kv_op = op_maker.create('general_dist_kv')
general_infer_op = op_maker.create('general_infer')
general_response_op = op_maker.create('general_response')

op_seq_maker = serving.OpSeqMaker()
op_seq_maker.add_op(read_op)
op_seq_maker.add_op(dist_kv_op)
op_seq_maker.add_op(general_infer_op)
op_seq_maker.add_op(general_response_op)
```
Current Paddle Serving supports operator list on the server side as follows:

<center>

| Op Name | Description |
|--------------|------|
| `general_reader` | General Data Reading Operator |
| `genreal_infer` | General Data Inference with Paddle Operator |
| `general_response` | General Data Response Operator |
| `general_dist_kv` | Distributed Sparse Embedding Indexing |

</center>

Paddle Serving supports inference engine on multiple devices. Current supports are CPU and GPU engine. Docker Images of CPU and GPU are provided officially. User can use one line command to start an inference service either on CPU or on GPU. 

``` shell
python -m paddle_serving_server.serve --model your_servable_model --thread 10 --port 9292
```
``` shell
python -m paddle_serving_server_gpu.serve --model your_servable_model --thread 10 --port 9292
```

Options of startup command are listed below: 
<center>

| Arguments | Types | Defaults | Descriptions |
|--------------|------|-----------|--------------------------------|
| `thread` | int | `4` | Concurrency on server side, usually equal to the number of CPU core |
| `port` | int | `9292` | Port exposed to users |
| `name` | str | `""` | Service name that if a user specifies, the name of HTTP service is allocated |
| `model` | str | `""` | Servable models for Paddle Serving |
| `gpu_ids` | str | `""` | Supported only in paddle_serving_server_gpu, similar to the usage of CUDA_VISIBLE_DEVICES |

</center>

For example, `python -m paddle_serving_server.serve --model your_servable_model --thread 10 --port 9292` is the same as the following code as user can define: 
``` python
from paddle_serving_server import OpMaker, OpSeqMaker, Server

op_maker = OpMaker()
read_op = op_maker.create('general_reader')
general_infer_op = op_maker.create('general_infer')
general_response_op = op_maker.create('general_response')
op_seq_maker = OpSeqMaker()
op_seq_maker.add_op(read_op)
op_seq_maker.add_op(general_infer_op)
op_seq_maker.add_op(general_response_op)
server = Server()
server.set_op_sequence(op_seq_maker.get_op_sequence())
server.set_num_threads(10)
server.load_model_config(”your_servable_model“)
server.prepare_server(port=9292, device="cpu")
server.run_server()
```

#### 2.1.3 客户端访问API
Paddle Serving支持远程服务访问的协议一种是基于RPC，另一种是HTTP。用户通过RPC访问，可以使用Paddle Serving提供的Python Client API，通过定制输入数据的格式来实现服务访问。下面的例子解释Paddle Serving Client如何定义输入数据。保存可部署模型时需要指定每个输入的别名，例如`sparse`和`dense`，对应的数据可以是离散的ID序列`[1, 1001, 100001]`，也可以是稠密的向量`[0.2, 0.5, 0.1, 0.4, 0.11, 0.22]`。当前Client的设计，对于离散的ID序列，支持Paddle中的`lod_level=0`和`lod_level=1`的情况，即张量以及一维变长张量。对于稠密的向量，支持`N-D Tensor`。用户不想要显式指定输入数据的形状，Paddle Serving的Client API会通过保存配置时记录的输入形状进行对应的检查。
``` python
feed_dict["sparse"] = [1, 1001, 100001]
feed_dict["dense"] = [0.2, 0.5, 0.1, 0.4, 0.11, 0.22]
fetch_map = client.predict(feed=feed_dict, fetch=["prob"])
```
Client链接Server的代码，通常只需要加载保存模型时保存的Client端配置，以及指定要去访问的服务端点即可。为了保持内部访问进行数据并行的扩展能力，Paddle Serving Client允许定义多个服务端点。
``` python
client = Client()
client.load_client_config('servable_client_configs')
client.connect(["127.0.0.1:9292"])
```


### 2.2 底层通信机制
Paddle Serving采用[baidu-rpc](https://github.com/apache/incubator-brpc)进行底层的通信。baidu-rpc是百度开源的一款PRC通信库，具有高并发、低延时等特点，已经支持了包括百度在内上百万在线预估实例、上千个在线预估服务，稳定可靠。Paddle Serving底层采用baidu-rpc的另一个原因是深度学习模型的远程调用服务通常对延时比较敏感，需要采用一款延时较低的rpc。

### 2.3 核心执行引擎
Paddle Serving的核心执行引擎是一个有向无环图，图中的每个节点代表预估服务的一个环节，例如计算模型预测打分就是其中一个环节。有向无环图有利于可并发节点充分利用部署实例内的计算资源，缩短延时。一个例子，当同一份输入需要送入两个不同的模型进行预估，并将两个模型预估的打分进行加权求和时，两个模型的打分过程即可以通过有向无环图的拓扑关系并发。
<p align="center">
    <br>
<img src='design_doc.png'">
    <br>
<p>

### 2.4 微服务插件模式
由于Paddle Serving底层采用基于C++的通信组件，并且核心框架也是基于C/C++编写，当用户想要在服务端定义复杂的前处理与后处理逻辑时，一种办法是修改Paddle Serving底层框架，重新编译源码。另一种方式可以通过在服务端嵌入轻量级的Web服务，通过在Web服务中实现更复杂的预处理逻辑，从而搭建一套逻辑完整的服务。当访问量超过了Web服务能够接受的范围，开发者有足够的理由开发一些高性能的C++预处理逻辑，并嵌入到Serving的原生服务库中。Web服务和RPC服务的关系以及他们的组合方式可以参考下文`用户类型`中的说明。

## 3. 工业级特性

### 3.1 分布式稀疏参数索引

分布式稀疏参数索引通常在广告推荐中出现，并与分布式训练配合形成完整的离线-在线一体化部署。下图解释了其中的流程，产品的在线服务接受用户请求后将请求发送给预估服务，同时系统会记录用户的请求以进行相应的训练日志处理和拼接。离线分布式训练系统会针对流式产出的训练日志进行模型增量训练，而增量产生的模型会配送至分布式稀疏参数索引服务，同时对应的稠密的模型参数也会配送至在线的预估服务。在线服务由两部分组成，一部分是针对用户的请求提取特征后，将需要进行模型的稀疏参数索引的特征发送请求给分布式稀疏参数索引服务，针对分布式稀疏参数索引服务返回的稀疏参数再进行后续深度学习模型的计算流程，从而完成预估。

<p align="center">
    <br>
<img src='cube_eng.png' width = "450" height = "230">
    <br>
<p>
                    
为什么要使用Paddle Serving提供的分布式稀疏参数索引服务？1）在一些推荐场景中，模型的输入特征规模通常可以达到上千亿，单台机器无法支撑T级别模型在内存的保存，因此需要进行分布式存储。2）Paddle Serving提供的分布式稀疏参数索引服务，具有并发请求多个节点的能力，从而以较低的延时完成预估服务。
                          
### 3.2 模型管理、在线A/B流量测试、模型热加载

Paddle Serving的C++引擎支持模型管理、在线A/B流量测试、模型热加载等功能，当前在Python API还有没完全开放这部分功能的配置，敬请期待。

## 4. 用户类型
Paddle Serving面向的用户提供RPC和HTTP两种访问协议。对于HTTP协议，我们更倾向于流量中小型的服务使用，并且对延时没有严格要求的AI服务开发者。对于RPC协议，我们面向流量较大，对延时要求更高的用户，此外RPC的客户端可能也处在一个大系统的服务中，这种情况下非常适合使用Paddle Serving提供的RPC服务。对于使用分布式稀疏参数索引服务而言，Paddle Serving的用户不需要关心底层的细节，其调用本质也是通过RPC服务再调用RPC服务。下图给出了当前设计的Paddle Serving可能会使用Serving服务的几种场景。

<p align="center">
    <br>
<img src='user_groups.png' width = "700" height = "470">
    <br>
<p>

对于普通的模型而言（具体指通过Serving提供的IO保存的模型，并且没有对模型进行后处理），用户使用RPC服务不需要额外的开发即可实现服务启动，但需要开发一些Client端的代码来使用服务。对于Web服务的开发，需要用户现在Paddle Serving提供的Web Service框架中进行前后处理的开发，从而实现整个HTTP服务。

### 4.1 Web服务开发

Web服务有很多开源的框架，Paddle Serving当前集成了Flask框架，但这部分对用户不可见，在未来可能会提供性能更好的Web框架作为底层HTTP服务集成引擎。用户需要继承WebService，从而实现对rpc服务的输入输出进行加工的目的。

``` python
from paddle_serving_server.web_service import WebService
from imdb_reader import IMDBDataset
import sys


class IMDBService(WebService):
    def prepare_dict(self, args={}):
        if len(args) == 0:
            exit(-1)
        self.dataset = IMDBDataset()
        self.dataset.load_resource(args["dict_file_path"])

    def preprocess(self, feed={}, fetch=[]):
        if "words" not in feed:
            exit(-1)
        res_feed = {}
        res_feed["words"] = self.dataset.get_words_only(feed["words"])[0]
        return res_feed, fetch


imdb_service = IMDBService(name="imdb")
imdb_service.load_model_config(sys.argv[1])
imdb_service.prepare_server(
    workdir=sys.argv[2], port=int(sys.argv[3]), device="cpu")
imdb_service.prepare_dict({"dict_file_path": sys.argv[4]})
imdb_service.run_server()
```

`WebService`作为基类，提供将用户接受的HTTP请求转化为RPC输入的接口`preprocess`，同时提供对RPC请求返回的结果进行后处理的接口`postprocess`，继承`WebService`的子类，可以定义各种类型的成员函数。`WebService`的启动命令和普通RPC服务提供的启动API一致。

## 5. 未来计划

### 5.1 有向无环图结构定义开放
当前版本开放的python API仅支持用户定义Sequential类型的执行流，如果想要进行Server进程内复杂的计算，需要增加对应的用户API。

### 5.2 云端自动部署能力
为了方便用户更容易将Paddle的预测模型部署到线上，Paddle Serving在接下来的版本会提供Kubernetes生态下任务编排的工具。

### 5.3 向量检索、树结构检索
在推荐与广告场景的召回系统中，通常需要采用基于向量的快速检索或者基于树结构的快速检索，Paddle Serving会对这方面的检索引擎进行集成或扩展。
