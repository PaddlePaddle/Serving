# 搭建预测服务集群

从[客户端配置](CLIENT_CONFIGURE.md)中我们已经知道，通过在客户端SDK的配置文件predictors.prototxt适当配置，可以搭建多副本和多Variant的预测集群。以下以图像分类任务为例，在单机上模拟搭建单Variant的多副本、和多Variant的预测集群

## 1. 单Variant多副本的预测集群

### 1.1 在本机创建一个serving副本

首先复制一个sering目录

```shell
$ cd /path/to/paddle-serving/build/output/demo
$ cp -r serving/ serving_new/
$ cd serving_new/

```

在serving_new目录中，在conf/gflags.conf中增加如下一行，修改其启动端口为8011，这是为了让该副本监听不同端口

```shell
--port=8011
```

然后启动新副本

```shell
$ bin/serving&
```

### 1.2 修改client端配置，将新副本地址加入ip列表：

```shell
$ cd /path/to/paddle-serving/build/output/demo/client/image_classification
```

修改conf/predictors.prototxt ImageClassifyService部分如下所示

```JSON
predictors {
  name: "ximage"
  service_name: "baidu.paddle_serving.predictor.image_classification.ImageClassifyService"
  endpoint_router: "WeightedRandomRender"
  weighted_random_render_conf {
    variant_weight_list: "50"
  }
  variants {
    tag: "var1"
    naming_conf {
      cluster: "list://127.0.0.1:8010, 127.0.0.1:8011"  # 在这里增加一个新的副本地址
    }
  }
}
```

重启client端

```shell
$ bin/ximage&
```

查看2个serving副本目录下是否均有收到请求：

```shell
$ cd /path/to/paddle-serving/build/output/demo/serving
$ tail -f log/serving.INFO

$ cd /path/to/paddle-serving/build/output/demo/serving_new
$ tail -f log/serving.INFO
```

## 2. 多Variant

### 2.1 本机创建新的serving副本

步骤同1.1节，略过

### 2.2 修改client配置，增加一个Variant

```shell
$ cd /path/to/paddle-serving/build/output/demo/client/image_classification
```

修改conf/predictors.prototxt ImageClassifyService部分如下所示

```JSON
predictors {
  name: "ximage"
  service_name: "baidu.paddle_serving.predictor.image_classification.ImageClassifyService"
  endpoint_router: "WeightedRandomRender"
  weighted_random_render_conf {
    variant_weight_list: "50 | 50"      # 一共2个variant，代表模型的2个版本。这里的权重代表调度的流量比例关系
  }
  variants {
    tag: "var1"
    naming_conf {
      cluster: "list://127.0.0.1:8010"
    }
  }
  variants {                            # 增加一个variant
    tag: "var2"
    naming_conf {
      cluster: "list://127.0.0.1:8011"
    }
  }
}
```

重启client端

```shell
$ bin/ximage&
```

查看2个serving副本目录下是否均有收到请求：

```shell
$ cd /path/to/paddle-serving/build/output/demo/serving
$ tail -f log/serving.INFO

$ cd /path/to/paddle-serving/build/output/demo/serving_new
$ tail -f log/serving.INFO
```

查看client端是否有收到来自Variant1和Variant2的响应

```shell
$ cd /path/to/paddle-serving/build/output/demo/client/image_classification
$ tail -f log/ximage.INFO

```

以下是正常的输出

```
I0307 17:54:22.862087 24719 ximage.cpp:172] Debug string: 
I0307 17:54:22.862650 24719 ximage.cpp:110] sample-0's classify result: n02112018,博美犬, prop: 0.522815
I0307 17:54:22.862666 24719 ximage.cpp:114] Succ call predictor[ximage], the tag is: var1, elapse_ms: 333

I0307 17:54:23.194780 24719 ximage.cpp:172] Debug string: 
I0307 17:54:23.195322 24719 ximage.cpp:110] sample-0's classify result: n02112018,博美犬, prop: 0.522815
I0307 17:54:23.195334 24719 ximage.cpp:114] Succ call predictor[ximage], the tag is: var2, elapse_ms: 332
```
