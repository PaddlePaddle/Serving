# paddle serving 大规模稀疏参数流程化部署
* [环境配置](#环境配置)
* [分布式训练](#分布式训练)
    * [1、集群配置](#1、集群配置)
        * [1.1 创建集群](#1.1 创建集群)
        * [1.2 配置集群环境](# 1.2 配置集群环境)

## 环境配置

**环境要求** ：helm、kubectl、go

## 分布式训练

分布式训练采用[volcano](https://github.com/volcano-sh/volcano)开源框架以及云平台实现，文档中以[百度智能云](https://cloud.baidu.com/?from=console)以及CTR预估模型为例，演示如何实现大规模稀疏参数模型的分布式训练。

### 1、集群配置

#### 1.1 创建集群

登录百度智能云官网，参考[帮助文档](https://cloud.baidu.com/doc/CCE/s/zjxpoqohb)创建容器引擎。

#### 1.2 配置集群环境

进入“产品服务>容器引擎CCE”，点击“集群管理>集群列表”，可看到用户已创建的集群列表。从集群列表中查看创建的集群信息。

![img](./deploy/cluster-info.png)

点击左侧的"Helm>Helm实例"，点击安装链接为集群一键安装helm。百度智能云为集群安装的helm版本为2.12.3，kubectl版本为1.13.4

为了能够从外部登录集群节点，需要为集群中安装了tiller的节点申请弹性公网。点击"更多操作>控制台"。

![concole](./deploy/concole.png)

点击"命名空间"选择kube-system，点击"容器组"，查看tiller开头的节点。

![tiller](./deploy/tiller.png)

点击"产品服务>网络>弹性公网"

![eip](./deploy/eip.png)

创建弹性公网实例，完成后选择创建的实例，点击"更多操作>绑定到BCC"，填写tiller开头的节点信息进行绑定。

### 2、 配置开发机环境

配置过程需要开发机的root权限。

#### 2.1 安装KubeCtl

KubeCtl可以实现在本地开发机上连接百度智能云的Kubernets集群，建议参考百度云操作指南文档中[通过KubeCtl连接集群](https://cloud.baidu.com/doc/CCE/s/6jxpotcn5)部分进行安装。

#### 2.2 安装Helm

建议参考[Helm官方安装文档](https://helm.sh/docs/using_helm/#installing-helm)进行安装。

**注意事项：**

开发机上的kubectl与helm的版本需要与集群上的版本相一致，目前百度智能云为集群安装的helm版本为2.12.3，kubectl版本为1.13.4。

#### 2.3 配置文件

点击"集群列表"界面的"配置文件下载"，下载配置文件。

![conf download](./deploy/conf-download.png)

将下载的配置文件移动到~/.kube文件夹下，文件名修改为config。

通过之前创建的弹性公网ip登录运行tiller的节点，账户密码为创建集群时设置的账户和密码，默认账户为root。

将节点上的以下三个文件

> /etc/kubernetes/pki/ca.pem
>
> /etc/kubernetes/pki/admin.pem
>
> /etc/kubernetes/pki/admin-key.pem

下载至开发机并放在相同的路径，添加四个环境变量

```bash
export HELM_TLS_ENABLE=true

export HELM_TLS_CA_CERT=/etc/kubernetes/pki/ca.pem

export HELM_TLS_CERT=/etc/kubernetes/pki/admin.pem

export HELM_TLS_KEY=/etc/kubernetes/pki/admin-key.pem
```

分别执行`kubectl version`与`helm version`，如果返回client端与server端信息，则证明配置成功。

示例：

![kubectl version](./deploy/kubectl-version.png)

![helm version](./deploy/helm-version.png)

如果只返回client端信息，server端信息显示"Forbidden"，检查开发机是否使用了代理，若有可以尝试关闭代理再次执行命令检查。

#### 2.4 安装Go

推荐安装Go 1.12

下载安装包

```bash
wget https://studygolang.com/dl/golang/go1.12.7.linux-amd64.tar.gz --no-check-certificate
```

解压到 /usr/local/路径下

```bash
tar zxvf go1.12.7.linux-amd64.tar.gz -C /usr/local/
```

设置环境变量

```bash
export GOPATH=/usr/local/go
```

### 3、安装volcano

参考[volcano官方文档](https://github.com/volcano-sh/volcano#quick-start-guide)。

通过yaml文件安装

```bash
kubectl apply -f https://raw.githubusercontent.com/volcano-sh/volcano/master/installer/volcano-development.yaml
```

安装完成后执行`kubectl get pods --namespace volcano-system`

若出现以下信息则证明安装成功：

![volcano](./deploy/volcano.png)

### 4、执行训练

创建cluster role和service account，[defaultserviceaccountclusterrole.yaml](./resource/defaultserviceaccountclusterrole.yaml) 文件示例如下：

```yaml
kind: ClusterRole
apiVersion: rbac.authorization.k8s.io/v1
metadata:
  name: default
  namespace: default
rules:
  - apiGroups: [""]
    resources: ["pods"]
    verbs: ["get", "list", "watch"]

---
kind: ClusterRoleBinding
apiVersion: rbac.authorization.k8s.io/v1
metadata:
  name: default
  namespace: default
subjects:
  - kind: ServiceAccount
    name: default
    namespace: default
roleRef:
  kind: ClusterRole
  name: default
  apiGroup: rbac.authorization.k8s.io
```

执行

```bash
kubectl create -f defaultserviceaccountclusterrole.yaml 
```

CTR模型的训练镜像存放在[dockerhub](https://hub.docker.com/)网站，通过kubectl加载yaml文件启动训练任务，CTR预估模型训练任务的yaml文件为[volcano-ctr-demo-baiduyun.yaml](./resource/volcano-ctr-demo-baiduyun.yaml)。

执行

```bash
kubectl apply -f volcano-ctr-demo-baiduyun.yaml
```

通过`kubectl get pods`命令可以查看训练任务的运行情况

![ctr running](./deploy/ctr-running.png)

通过`kubectl logs $POD_NAME`可以查看对应的日志，例如`kubectl logs edl-demo-trainer-0`

![trainer log](./deploy/trainer-log.png)

也可以通过百度云平台提供的web页面观察集群的工作负载

![工作负载](./deploy/workload.png)

### 5、模型产出

CTR预估模型包含了embedding部分以及dense神经网络两部分，其中embedding部分包含的稀疏参数较多，在某些场景下单机的资源难以加载整个模型，因此需要将这两部分分割开来，稀疏参数部分放在分布式的稀疏参数服务器，dense网络部分加载到serving服务中。

#### 5.1 模型裁剪

产出用于paddle serving预测服务的dense模型需要对保存的原始模型进行裁剪操作，修改模型的输入以及内部结构。具体操作请参考文档[模型裁剪]([https://github.com/PaddlePaddle/Serving/blob/develop/doc/CTR_PREDICTION.md#2-%E6%A8%A1%E5%9E%8B%E8%A3%81%E5%89%AA](https://github.com/PaddlePaddle/Serving/blob/develop/doc/CTR_PREDICTION.md#2-模型裁剪))。

#### 5.2 稀疏参数产出

分布式稀疏参数服务器由paddle serving的cube模块实现。cube服务器中加载的数据格式为seqfile格式，因此需要对paddle保存出的模型文件进行格式转换。

可以通过[格式转换脚本](http://icode.baidu.com/repos/baidu/personal-code/wangguibao/blob/master:ctr-embedding-to-sequencefile/dumper.py)

使用方法：

```bash
python dumper.py --model_path=xxx --output_data_path=xxx
```

**注意事项：**文档中使用的CTR模型训练镜像中已经包含了模型裁剪以及稀疏参数产出的脚本，并且搭建了一个http服务用于从外部获取产出的dense模型以及稀疏参数文件。

## 预测服务部署

### 1、Server端

通过wget命令从集群获取dense部分模型用于Server端。

```bash
wget "${公网ip}:/path/to/models"
```

K8s集群上CTR预估任务训练完成后，模型参数分成2部分：一是embedding数据，经过dumper.py已经转成hadoop SequenceFile格式，传输给cube建库流程构建索引和灌cube；二是除embedding之外的参数文件，连同save_program.py裁剪后的program，一起配合传输给Serving加载。save_program.py裁剪原始模型的具体背景和详细步骤请参考文档[Paddle Serving CTR预估模型说明](https://github.com/PaddlePaddle/Serving/blob/develop/doc/CTR_PREDICTION.md)。

本文介绍Serving使用上述模型参数和program加载模型提供预测服务的流程。

#### 1.1 Cube服务

假设Cube服务已经成功部署，用于cube客户端API的配置文件如下所示：

```json
[{
    "dict_name": "dict",
    "shard": 2,
    "dup": 1,
    "timeout": 200,
    "retry": 3,
    "backup_request": 100,
    "type": "ipport_list",
    "load_balancer": "rr",
    "nodes": [{
        "ipport_list": "list://192.168.1.1:8000"
    },{
        "ipport_list": "list://192.168.1.2:8000"
    }]
}]
```

上述例子中，cube提供外部访问的表名是`dict`，有2个物理分片，分别在192.168.1.1:8000和192.168.1.2:8000

#### 1.2 Serving编译

截至写本文时，Serving develop分支已经提供了CTR预估服务相关OP，参考[ctr_prediction_op.cpp](https://github.com/PaddlePaddle/Serving/blob/develop/demo-serving/op/ctr_prediction_op.cpp)，该OP从client端接收请求后会将每个请求的26个sparse feature id发给cube服务，获得对应的embedding向量，然后填充到模型feed variable对应的LoDTensor，执行预测计算。只要按常规步骤编译Serving即可。

```bash
$ git clone https://github.com/PaddlePaddle/Serving.git
$ cd Serving
$ makedir build
$ cd build
$ cmake -DWITH_GPU=OFF .. # 不需要GPU
$ make -jN                # 这里可修改并发编译线程数
$ make install
$ cd output/demo/serving
```

#### 1.3 配置修改

##### 1.3.1 conf/gflags.conf

将--enable_cube改为true:

```json
--enable_cube=true
```

##### 1.3.2 conf/model_toolkit.prototxt

Paddle Serving自带的model_toolkit.prototxt如下所示，如有必要可只保留ctr_prediction一个：

```
engines {
  name: "image_classification_resnet"
  type: "FLUID_CPU_NATIVE_DIR"
  reloadable_meta: "./data/model/paddle/fluid_time_file"
  reloadable_type: "timestamp_ne"
  model_data_path: "./data/model/paddle/fluid/SE_ResNeXt50_32x4d"
  runtime_thread_num: 0
  batch_infer_size: 0
  enable_batch_align: 0
}
engines {
  name: "text_classification_bow"
  type: "FLUID_CPU_ANALYSIS_DIR"
  reloadable_meta: "./data/model/paddle/fluid_time_file"
  reloadable_type: "timestamp_ne"
  model_data_path: "./data/model/paddle/fluid/text_classification_lstm"
  runtime_thread_num: 0
  batch_infer_size: 0
  enable_batch_align: 0
}

engines {
  name: "ctr_prediction"
  type: "FLUID_CPU_ANALYSIS_DIR"
  reloadable_meta: "./data/model/paddle/fluid_time_file"
  reloadable_type: "timestamp_ne"
  model_data_path: "./data/model/paddle/fluid/ctr_prediction"
  runtime_thread_num: 0
  batch_infer_size: 0
  enable_batch_align: 0
  sparse_param_service_type: REMOTE
  sparse_param_service_table_name: "dict"
}
```

注意ctr_prediction model有如下2行配置：

```json
  sparse_param_service_type: REMOTE
  sparse_param_service_table_name: "dict"
```

##### 1.3.3 conf/cube.conf

conf/cube.conf是一个完整的cube配置文件模板，其中只要修改nodes列表为真实的物理节点IP:port列表即可。例如 （与第1节cube配置文件内容一致）：

```json
[{
    "dict_name": "dict",
    "shard": 2,
    "dup": 1,
    "timeout": 200,
    "retry": 3,
    "backup_request": 100,
    "type": "ipport_list",
    "load_balancer": "rr",
    "nodes": [{
        "ipport_list": "list://192.168.1.1:8000"
    },{
        "ipport_list": "list://192.168.1.2:8000"
    }]
}]
```

**注意事项：**如果修改了`dict_name`，需要同步修改1.3.2节中`sparse_param_service_table_name`字段

##### 1.3.4 模型文件

Paddle Serving自带了一个可以工作的CTR预估模型，是从BCE上下载下来的，其制作方法为： 1) 分布式训练CTR预估任务，保存模型program和参数文件 2) 用save_program.py保存一份用于预测的program (文件名为**model**)。save_program.py随trainer docker image发布 3) 第2步中保存的program (**model**) 覆盖到第1)步保存的模型文件夹中**model**文件，打包成.tar.gz上传到BCE

如果只是为了验证demo流程，serving此时已经可以用自带的CTR模型加载模型并提供预测服务能力。

为了应用重新训练的模型，只需要从k8s集群暴露的ftp服务下载新的.tar.gz，解压到data/model/paddle/fluid下，覆盖原来的ctr_prediction目录即可。从K8S集群暴露的ftp服务下载训练模型，请参考文档[PaddlePaddle分布式训练和Serving流程化部署](http://icode.baidu.com/repos/baidu/personal-code/wangguibao/blob/master:ctr-embedding-to-sequencefile/path/to/doc/DISTRIBUTED_TRANING_AND_SERVING.md)

#### 1.4 启动Serving

执行`./bin/serving  `启动serving服务，在./log路径下可以查看serving日志。

### 2、Client端

参考[从零开始写一个预测服务：client端]([https://github.com/PaddlePaddle/Serving/blob/develop/doc/CREATING.md#3-client%E7%AB%AF](https://github.com/PaddlePaddle/Serving/blob/develop/doc/CREATING.md#3-client端))文档，实现client端代码。

文档中使用的CTR预估任务client端代码存放在Serving代码库demo-client路径下，链接[ctr_prediction.cpp](https://github.com/PaddlePaddle/Serving/blob/develop/demo-client/src/ctr_prediction.cpp)。

#### 2.1 测试数据

CTR预估任务样例使用的数据来自于[原始模型](https://github.com/PaddlePaddle/models/tree/develop/PaddleRec/ctr)的测试数据，在样例中提供了1000个测试样本，如果需要更多样本可以参照原始模型下载数据的[脚本](https://github.com/PaddlePaddle/models/blob/develop/PaddleRec/ctr/data/download.sh)。

#### 2.2 Client编译与部署

按照1.2 Serving编译部分完成编译后，client端文件在output/demo/client/ctr_prediction路径下。

##### 2.2.1 配置修改

修改conf/predictors.prototxt文件ctr_prediction_service部分

```
predictors {
  name: "ctr_prediction_service"
  service_name: "baidu.paddle_serving.predictor.ctr_prediction.CTRPredictionService"
  endpoint_router: "WeightedRandomRender"
  weighted_random_render_conf {
    variant_weight_list: "50"
  }
  variants {
    tag: "var1"
    naming_conf {
        cluster: "list://127.0.0.1:8010"
    }
  }
}
```

配置Server端ip与端口号，默认为本机ip、8010端口。

##### 2.2.2 运行服务

执行`./bin/ctr_predictoin`启动client端，在./log路径下可以看到client端执行的日志。
