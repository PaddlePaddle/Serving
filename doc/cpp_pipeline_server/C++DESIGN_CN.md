# C++ Serving设计方案

(简体中文|[English](./C++DESIGN.md))

注意本页内容有已经过期，请查看：[设计文档](DESIGN_DOC_CN.md)

## 1. 项目背景

PaddlePaddle是百度开源的机器学习框架，广泛支持各种深度学习模型的定制化开发; Paddle Serving是Paddle的在线预测部分，与Paddle模型训练环节无缝衔接，提供机器学习预测云服务。本文将从模型、服务、接入等层面，自底向上描述Paddle Serving设计方案。

1. 模型是Paddle Serving预测的核心，包括模型数据和推理计算的管理；
2. 预测框架封装模型推理计算，对外提供RPC接口，对接不同上游；
3. 预测服务SDK提供一套接入框架

最终形成一套完整的serving解决方案。

## 2. 名词解释

- **baidu-rpc**: 百度官方开源RPC框架，支持多种常见通信协议，提供基于protobuf的自定义接口体验
- **Variant**: Paddle Serving架构对一个最小预测集群的抽象，其特点是内部所有实例（副本）完全同质，逻辑上对应一个model的一个固定版本
- **Endpoint**: 多个Variant组成一个Endpoint，逻辑上看，Endpoint代表一个model，Endpoint内部的Variant代表不同的版本
- **OP**: PaddlePaddle用来封装一种数值计算的算子，Paddle Serving用来表示一种基础的业务操作算子，核心接口是inference。OP通过配置其依赖的上游OP，将多个OP串联成一个workflow
- **Channel**: 一个OP所有请求级中间数据的抽象；OP之间通过Channel进行数据交互
- **Bus**: 对一个线程中所有channel的管理，以及根据DAG之间的DAG依赖图对OP和Channel两个集合间的访问关系进行调度
- **Stage**: Workflow按照DAG描述的拓扑图中，属于同一个环节且可并行执行的OP集合
- **Node**: 由某个OP算子类结合参数配置组成的OP算子实例，也是Workflow中的一个执行单元
- **Workflow**: 按照DAG描述的拓扑，有序执行每个OP的inference接口
- **DAG/Workflow**: 由若干个相互依赖的Node组成，每个Node均可通过特定接口获得Request对象，节点OP通过依赖关系获得其前置OP的输出对象，最后一个Node的输出默认就是Response对象
- **Service**: 对一次PV的请求封装，可配置若干条Workflow，彼此之间复用当前PV的Request对象，然后各自并行/串行执行，最后将Response写入对应的输出slot中；一个Paddle-serving进程可配置多套Service接口，上游根据ServiceName决定当前访问的Service接口。

## 3. Python Interface设计

### 3.1 核心目标：

完成一整套Paddle Serving的动态库，支持Paddle保存的通用模型的远程预估服务，通过Python Interface调用PaddleServing底层的各种功能。

### 3.2 通用模型：

能够使用Paddle Inference Library进行预测的模型，在训练过程中保存的模型，包含Feed Variable和Fetch Variable

### 3.3 整体设计：

- 用户通过Python Client启动Client和Server，Python API有检查互联和待访问模型是否匹配的功能
- Python API背后调用的是Paddle Serving实现的client和server对应功能的pybind，互传的信息通过RPC实现
- Client Python API当前有两个简单的功能，load_inference_conf和predict，分别用来执行加载待预测的模型和预测
- Server Python API主要负责加载预估模型，以及生成Paddle Serving需要的各种配置，包括engines，workflow，resource等

### 3.4 Server Inferface

![Server Interface](images/server_interface.png)

### 3.5 Client Interface

<img src='images/client_inferface.png' width = "600" height = "200">

### 3.6 训练过程中使用的Client io

PaddleServing设计可以在训练过程中使用的保存模型接口，与Paddle保存inference model的接口基本一致，feed_var_dict与fetch_var_dict
可以为输入和输出变量起别名，serving启动需要读取的配置会保存在client端和server端的保存目录中。

``` python
def save_model(server_model_folder,
               client_config_folder,
               feed_var_dict,
               fetch_var_dict,
               main_program=None)
```

## 4. Paddle Serving底层框架

![Paddle-Serging总体框图](images/framework.png)

**模型管理框架**：对接多种机器学习平台的模型文件，向上提供统一的inference接口
**业务调度框架**：对各种不同预测模型的计算逻辑进行抽象，提供通用的DAG调度框架，通过DAG图串联不同的算子，共同完成一次预测服务。该抽象模型使用户可以方便的实现自己的计算逻辑，同时便于算子共用。（用户搭建自己的预测服务，很大一部分工作是搭建DAG和提供算子的实现）
**PredictService**:对外部提供的预测服务接口封装。通过protobuf定义与客户端的通信字段。

### 4.1 模型管理框架

模型管理框架负责管理机器学习框架训练出来的模型，总体可抽象成模型加载、模型数据和模型推理等3个层次。

#### 模型加载

将模型从磁盘加载到内存，支持多版本、热加载、增量更新等功能

#### 模型数据

模型在内存中的数据结构，集成fluid预测lib

#### inferencer

向上为预测服务提供统一的预测接口

```C++
class FluidFamilyCore {
  virtual bool Run(const void* in_data, void* out_data);
  virtual int create(const std::string& data_path);
  virtual int clone(void* origin_core);
};
```

### 4.2 业务调度框架

#### 4.2.1 预测服务Service

参考TF框架的模型计算的抽象思想，将业务逻辑抽象成DAG图，由配置驱动，生成workflow，跳过C++代码编译。业务的每个具体步骤，对应一个具体的OP，OP可配置自己依赖的上游OP。OP之间消息传递统一由线程级Bus和channel机制实现。例如，一个简单的预测服务的服务过程，可以抽象成读请求数据->调用预测接口->写回预测结果等3个步骤，相应的实现到3个OP: ReaderOp->ClassifyOp->WriteOp

![预测服务Service](images/predict-service.png)

关于OP之间的依赖关系，以及通过OP组建workflow，可以参考[从零开始写一个预测服务](CREATING.md)的相关章节

服务端实例透视图

![服务端实例透视图](images/server-side.png)


#### 4.2.2 Paddle Serving的多服务机制

![Paddle Serving的多服务机制](images/multi-service.png)

Paddle Serving实例可以同时加载多个模型，每个模型用一个Service（以及其所配置的workflow）承接服务。可以参考[Demo例子中的service配置文件](../tools/cpp_examples/demo-serving/conf/service.prototxt)了解如何为serving实例配置多个service

#### 4.2.3 业务调度层级关系

从客户端看，一个Paddle Serving service从顶向下可分为Service, Endpoint, Variant等3个层级

![调用层级关系](images/multi-variants.png)

一个Service对应一个预测模型，模型下有1个endpoint。模型的不同版本，通过endpoint下多个variant概念实现：
同一个模型预测服务，可以配置多个variant，每个variant有自己的下游IP列表。客户端代码可以对各个variant配置相对权重，以达到调节流量比例的关系（参考[客户端配置](CLIENT_CONFIGURE.md)第3.2节中关于variant_weight_list的说明）。

![Client端proxy功能](images/client-side-proxy.png)

## 5. 用户接口

在满足一定的接口规范前提下，服务框架不对用户数据字段做任何约束，以应对各种预测服务的不同业务接口。Baidu-rpc继承了Protobuf serice的接口，用户按照Protobuf语法规范描述Request和Response业务接口。Paddle Serving基于Baidu-rpc框架搭建，默认支持该特性。

无论通信协议如何变化，框架只需确保Client和Server间通信协议和业务数据两种信息的格式同步，即可保证正常通信。这些信息又可细分如下：

- 协议：Server和Client之间事先约定的、确保相互识别数据格式的包头信息。Paddle Serving用Protobuf作为基础通信格式
- 数据：用来描述Request和Response的接口，例如待预测样本数据，和预测返回的打分。包括：
  - 数据字段：请求包Request和返回包Response两种数据结构包含的字段定义
  - 描述接口：跟协议接口类似，默认支持Protobuf

### 5.1 数据压缩方法

Baidu-rpc内置了snappy, gzip, zlib等数据压缩方法，可在配置文件中配置（参考[客户端配置](CLIENT_CONFIGURE.md)第3.1节关于compress_type的介绍）

### 5.2 C++ SDK API接口

```C++
class PredictorApi {
 public:
  int create(const char* path, const char* file);
  int thrd_initialize();
  int thrd_clear();
  int thrd_finalize();
  void destroy();

  Predictor* fetch_predictor(std::string ep_name);
  int free_predictor(Predictor* predictor);
};

class Predictor {
 public:
  // synchronize interface
  virtual int inference(google::protobuf::Message* req,
                        google::protobuf::Message* res) = 0;

  // asynchronize interface
  virtual int inference(google::protobuf::Message* req,
                        google::protobuf::Message* res,
                        DoneType done,
                        brpc::CallId* cid = NULL) = 0;

  // synchronize interface
  virtual int debug(google::protobuf::Message* req,
                    google::protobuf::Message* res,
                    butil::IOBufBuilder* debug_os) = 0;
};

```

### 5.3 OP相关接口

```C++
class Op {
  // ------Getters for Channel/Data/Message of dependent OP-----

  // Get the Channel object of dependent OP
  Channel* mutable_depend_channel(const std::string& op);

  // Get the Channel object of dependent OP
  const Channel* get_depend_channel(const std::string& op) const;

  template <typename T>
  T* mutable_depend_argument(const std::string& op);

  template <typename T>
  const T* get_depend_argument(const std::string& op) const;

  // -----Getters for Channel/Data/Message of current OP----

  // Get pointer to the progobuf message of current OP
  google::protobuf::Message* mutable_message();

  // Get pointer to the protobuf message of current OP
  const google::protobuf::Message* get_message() const;

  // Get the template class data object of current OP
  template <typename T>
  T* mutable_data();

  // Get the template class data object of current OP
  template <typename T>
  const T* get_data() const;

  // ---------------- Other base class members ----------------

  int init(Bus* bus,
           Dag* dag,
           uint32_t id,
           const std::string& name,
           const std::string& type,
           void* conf);

  int deinit();


  int process(bool debug);

  // Get the input object
  const google::protobuf::Message* get_request_message();

  const std::string& type() const;

  uint32_t id() const;

  // ------------------ OP Interface -------------------

  // Get the derived Channel object of current OP
  virtual Channel* mutable_channel() = 0;

  // Get the derived Channel object of current OP
  virtual const Channel* get_channel() const = 0;

  // Release the derived Channel object of current OP
  virtual int release_channel() = 0;

  // Inference interface
  virtual int inference() = 0;

  // ------------------ Conf Interface -------------------
  virtual void* create_config(const configure::DAGNode& conf) { return NULL; }

  virtual void delete_config(void* conf) {}

  virtual void set_config(void* conf) { return; }

  // ------------------ Metric Interface -------------------
  virtual void regist_metric() { return; }
};

```

### 5.4 框架相关接口

Service

```C++
class InferService {
 public:
  static const char* tag() { return "service"; }
  int init(const configure::InferService& conf);
  int deinit() { return 0; }
  int reload();
  const std::string& name() const;
  const std::string& full_name() const { return _infer_service_format; }

  // Execute each workflow serially
  virtual int inference(const google::protobuf::Message* request,
                        google::protobuf::Message* response,
                        butil::IOBufBuilder* debug_os = NULL);

  int debug(const google::protobuf::Message* request,
            google::protobuf::Message* response,
            butil::IOBufBuilder* debug_os);

};

class ParallelInferService : public InferService {
 public:
  // Execute workflows in parallel
  int inference(const google::protobuf::Message* request,
                google::protobuf::Message* response,
                butil::IOBufBuilder* debug_os) {
    return 0;
  }
};
```
ServerManager

```C++
class ServerManager {
 public:
  typedef google::protobuf::Service Service;
  ServerManager();

  static ServerManager& instance() {
    static ServerManager server;
    return server;
  }
  static bool reload_starting() { return _s_reload_starting; }
  static void stop_reloader() { _s_reload_starting = false; }
  int add_service_by_format(const std::string& format);
  int start_and_wait();
};
```

DAG

```C++
class Dag {
 public:
  EdgeMode parse_mode(std::string& mode);  // NOLINT

  int init(const char* path, const char* file, const std::string& name);

  int init(const configure::Workflow& conf, const std::string& name);

  int deinit();

  uint32_t nodes_size();

  const DagNode* node_by_id(uint32_t id);

  const DagNode* node_by_id(uint32_t id) const;

  const DagNode* node_by_name(std::string& name);  // NOLINT

  const DagNode* node_by_name(const std::string& name) const;

  uint32_t stage_size();

  const DagStage* stage_by_index(uint32_t index);

  const std::string& name() const { return _dag_name; }

  const std::string& full_name() const { return _dag_name; }

  void regist_metric(const std::string& service_name);
};
```

Workflow

```C++
class Workflow {
 public:
  Workflow() {}
  static const char* tag() { return "workflow"; }

  // Each workflow object corresponds to an independent
  // configure file, so you can share the object between
  // different apps.
  int init(const configure::Workflow& conf);

  DagView* fetch_dag_view(const std::string& service_name);

  int deinit() { return 0; }

  void return_dag_view(DagView* view);

  int reload();

  const std::string& name() { return _name; }

  const std::string& full_name() { return _name; }
};
```
