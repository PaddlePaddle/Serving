# Client side configuration

Paddle Serving C++ client SDK的配置文件格式用protobuf定义，全部在configure/proto/sdk_configure.proto中。如果要增加配置字段，需要先在该protobuf文件中增加相应字段，才能被Serving SDK读取和解析。

Paddle Serving主配置文件为conf/predictors.prototxt。其中一个示例如下：

## 1. Sample conf

```shell
default_variant_conf {
  tag: "default"
  connection_conf {
    connect_timeout_ms: 2000
    rpc_timeout_ms: 20000
    connect_retry_count: 2
    max_connection_per_host: 100
    hedge_request_timeout_ms: -1
    hedge_fetch_retry_count: 2
    connection_type: "pooled"
  }
  naming_conf {
    cluster_filter_strategy: "Default"
    load_balance_strategy: "la"
  }
  rpc_parameter {
    compress_type: 0
    package_size: 20
    protocol: "baidu_std"
    max_channel_per_request: 3
  }
}
predictors {
  name: "ximage"
  service_name: "baidu.paddle_serving.predictor.image_classification.ImageClassifyService"
  endpoint_router: "WeightedRandomRender"
  weighted_random_render_conf {
    variant_weight_list: "50|50"
  }
  variants {
    tag: "var1"
    naming_conf {
      cluster: "list://127.0.0.1:8010"
    }
  }
  variants {
    tag: "var2"
    naming_conf {
      cluster: "list://127.0.0.1:8011"
    }
  }
}

predictors {
  name: "echo_service"
  service_name: "baidu.paddle_serving.predictor.echo_service.BuiltinTestEchoService"
  endpoint_router: "WeightedRandomRender"
  weighted_random_render_conf {
    variant_weight_list: "50"
  }
  variants {
    tag: "var1"
    naming_conf {
      cluster: "list://127.0.0.1:8010,127.0.0.1:8011"
    }
  }
}

```

## 2. 名词解释
- 预测服务 (Predictor)：对一个Paddle预测服务的封装
- 端点（Endpoit）：对一个预测需求的逻辑抽象，通常包含一到多个服务变体，以方便多版本模型管理；
- 变体（Variant）：一套同质化的Paddle Serving集群服务，每个实例起一个Paddle Serving进程；

## 3. 配置项解释

### 3.1 default_variant_conf

```shell
default_variant_conf {
  tag: "default"
  connection_conf {
    connect_timeout_ms: 2000
    rpc_timeout_ms: 20000
    connect_retry_count: 2
    max_connection_per_host: 100
    hedge_request_timeout_ms: -1
    hedge_fetch_retry_count: 2
    connection_type: "pooled"
  }
  naming_conf {
    cluster_filter_strategy: "Default"  # Not used for now
    load_balance_strategy: "la"
  }
  rpc_parameter {
    compress_type: 0 
    package_size: 20
    protocol: "baidu_std"
    max_channel_per_request: 3
  }
}
```
其中：

connection_type: Maybe single/short/pooled, see [BRPC DOC: connection_type](https://github.com/apache/incubator-brpc/blob/master/docs/cn/client.md#%E8%BF%9E%E6%8E%A5%E6%96%B9%E5%BC%8F)

cluster_filter_strategy: 暂时未用

load_balance_strategy: Maybe rr/wrr/random/la/c_murmurhash/c_md5, see [BRPC DOC: load_balance](https://github.com/apache/incubator-brpc/blob/master/docs/cn/client.md#%E8%B4%9F%E8%BD%BD%E5%9D%87%E8%A1%A1)

compress_type: 0-None, 1-Snappy, 2-gzip, 3-zlib, 4-lz4, see [BRPC DOC: compress_type](https://github.com/apache/incubator-brpc/blob/master/docs/cn/client.md#%E5%8E%8B%E7%BC%A9)

protocol: Maybe baidu_std/http/h2/h2:grpc/thrift/memcache/redis... see [BRPC DOC: protocol](https://github.com/apache/incubator-brpc/blob/master/docs/cn/client.md#%E5%8D%8F%E8%AE%AE) 

### 3.2 Predictors

可以为客户端配置多个predictor，每个predictor代表一个要访问的预测服务

```shell
predictors {
  name: "ximage"
  service_name: "baidu.paddle_serving.predictor.image_classification.ImageClassifyService"
  endpoint_router: "WeightedRandomRender"
  weighted_random_render_conf {
    variant_weight_list: "50|50"
  }
  variants {
    tag: "var1"
    naming_conf {
    cluster: "list://127.0.0.1:8010, 127.0.0.1:8011"
    }
  }
  variants {
    tag: "var2"
    naming_conf {
      cluster: "list://127.0.0.1:8011"
    }
  }
}

predictors {
  name: "echo_service"
  service_name: "baidu.paddle_serving.predictor.echo_service.BuiltinTestEchoService"
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
其中：

service_name: 写sdk-cpp/proto/xx.proto的package name

endpoint_router: 目前只支持WeightedRandomRender

variant_weight_list: 与接下来的variants列表共用，用于表示variants之间相对权重；通过修改此数值可以调整variants调度的比重

cluster: Cluster支持的格式见 [BRPC DOC: naming service](https://github.com/apache/incubator-brpc/blob/master/docs/cn/client.md#%E5%91%BD%E5%90%8D%E6%9C%8D%E5%8A%A1)
