## 在Kubenetes集群上部署Paddle Serving

Paddle Serving在0.6.0版本开始支持在Kubenetes集群上部署，并提供反向代理和安全网关支持。与Paddle Serving在Docker镜像中开发类似，Paddle Serving 模型在Kubenetes集群部署需要制作轻量化的运行镜像，并使用kubectl工具在集群上部署。

### 1.集群准备

如果您还没有Kubenetes集群，我们推荐[购买并使用百度智能云CCE集群](https://cloud.baidu.com/doc/CCE/index.html). 如果是其他云服务商提供的集群，或者自行安装Kubenetes集群，请遵照对应的教程。

您还需要准备一个用于Kubenetes集群部署使用的镜像仓库，通常与云服务提供商绑定，如果您使用的是百度智能云的CCE集群，可以参照[百度智能云CCR镜像仓库使用方式](https://cloud.baidu.com/doc/CCR/index.html)。当然Docker Hub也可以作为镜像仓库，但是可能在部署时会出现下载速度慢的情况。

### 2.环境准备

需要在Kubenetes集群上安装网关工具KONG。

```
kubectl apply -f https://bit.ly/kong-ingress-dbless
```

### 选择Serving开发镜像 （可选）
您可以直接选择已生成的Serving [DOCKER开发镜像列表](./Docker_Images_CN.md)作为Kubernetes部署的首选，携带了开发工具，可用于调试和编译代码。

### 制作Serving运行镜像（可选）

与[DOCKER开发镜像列表](./Docker_Images_CN.md)文档相比，开发镜像用于调试、编译代码，携带了大量的开发工具，因此镜像体积较大。运行镜像通常容器体积更小的轻量级容器，可在边缘端设备上部署。如您不需要轻量级运行容器，请直接跳过这一部分。

我们提供了运行镜像的生成脚本在Serving代码库下`tools/generate_runtime_docker.sh`文件，通过以下命令可生成代码。

```bash
bash tools/generate_runtime_docker.sh --env cuda10.1 --python 3.7 --image_name serving_runtime:cuda10.1-py37 --paddle 2.2.0 --serving 0.8.0
```

会生成 cuda10.1，python 3.7，serving版本0.8.0 还有 paddle版本2.2.2的运行镜像。如果有其他疑问，可以执行下列语句得到帮助信息。强烈建议您使用最新的paddle和serving的版本（2个版本是对应的如paddle 2.2.0 与serving 0.7.x对应，paddle 2.2.2 与 serving 0.8.x对应），因为更早的版本上出现的错误只在最新版本修复，无法在历史版本中修复。

```
bash tools/generate_runtime_docker.sh --help
```

运行镜像会携带以下组建在运行镜像中

- paddle-serving-server， paddle-serving-client，paddle-serving-app，paddlepaddle，具体版本可以在tools/runtime.dockerfile当中查看，同时，如果有定制化的需求，也可以在该文件中进行定制化。
- paddle-serving-server 二进制可执行程序

也就是说，运行镜像在生成之后，我们只需要将我们运行的代码（如果有）和模型搬运到镜像中就可以。生成后的镜像名为`paddle_serving:cuda10.2-py37`

### 添加您的代码和模型 

在刚才镜像的基础上，我们需要先收集好运行文件。这取决于您是如何使用PaddleServing的

#### Pipeline模式：

对于pipeline模式，我们需要确保模型和程序文件、配置文件等各种依赖都能够在镜像中运行。因此可以在`/home/project`下存放我们的执行文件时，我们以`Serving/examples/Pipeline/PaddleOCR/ocr`为例，这是OCR文字识别任务。

```bash
#假设您已经拥有Serving运行镜像，假设镜像名为paddle_serving:cuda10.2-py36
docker run --rm -dit --name pipeline_serving_demo paddle_serving:cuda10.2-py36 bash
cd Serving/examples/Pipeline/PaddleOCR/ocr
# get models
python -m paddle_serving_app.package --get_model ocr_rec
tar -xzvf ocr_rec.tar.gz
python -m paddle_serving_app.package --get_model ocr_det
tar -xzvf ocr_det.tar.gz
cd ..

docker cp ocr pipeline_serving_demo:/home/
docker commit pipeline_serving_demo ocr_serving:latest
```

其中容器名`paddle_serving_demo`和最终的镜像名`ocr_serving:latest`都可以自行定义，最终通过`docker push`来推到云端的镜像仓库。至此，部署前的最后一步工作已完成。

**提示：如果您对runtime镜像是否可运行需要验证，可以执行**

```
docker exec -it pipeline_serving_demo bash
cd /home/ocr
python3.6 web_service.py 
```

进入容器到工程目录之后，剩下的操作和调试代码的工作是类似的。

**为了方便您对照，我们也提供了示例镜像registry.baidubce.com/paddlepaddle/serving:k8s-pipeline-demo**

#### WebService模式：

web service模式本质上和pipeline模式类似，因此我们以`Serving/examples/C++/PaddleNLP/bert`为例

```bash
#假设您已经拥有Serving运行镜像，假设镜像名为registry.baidubce.com/paddlepaddle/serving:0.8.0-cpu-py36
docker run --rm -dit --name webservice_serving_demo registry.baidubce.com/paddlepaddle/serving:0.8.0-cpu-py36 bash
cd Serving/examples/C++/PaddleNLP/bert
### download model 
wget https://paddle-serving.bj.bcebos.com/paddle_hub_models/text/SemanticModel/bert_chinese_L-12_H-768_A-12.tar.gz
tar -xzf bert_chinese_L-12_H-768_A-12.tar.gz
mv bert_chinese_L-12_H-768_A-12_model bert_seq128_model
mv bert_chinese_L-12_H-768_A-12_client bert_seq128_client
sh get_data.sh
cd ..
docker cp bert webservice_serving_demo:/home/
docker commit webservice_serving_demo bert_serving:latest
```

**提示：如果您对runtime镜像是否可运行需要验证，可以执行**

```bash
docker exec -it webservice_serving_demo bash
cd /home/bert
python3.6 bert_web_service.py bert_seq128_model 9292
```

进入容器到工程目录之后，剩下的操作和调试代码的工作是类似的。

**为了方便您对照，我们也提供了示例镜像registry.baidubce.com/paddlepaddle/serving:k8s-web-demo**



### 在Kubenetes集群上部署 

kubenetes集群操作需要`kubectl`去操纵yaml文件。我们这里给出了三个部署的例子，他们分别是

- pipeline ocr示例 

```bash
sh tools/generate_k8s_yamls.sh  --app_name ocr --image_name registry.baidubce.com/paddlepaddle/serving:k8s-pipeline-demo --workdir /home/ocr --command "python3.6 web_service.py" --port 9999
```

- web service bert示例

```bash
sh tools/generate_k8s_yamls.sh  --app_name bert --image_name registry.baidubce.com/paddlepaddle/serving:k8s-web-demo --workdir /home/bert --command "python3.6 bert_web_service.py bert_seq128_model 9292" --port 9292
```
**需要注意的是，app_name需要同URL的函数名相同。例如示例中bert的访问URL是`https://127.0.0.1:9292/bert/prediction`，那么app_name应为bert。**

接下来我们会看到有两个yaml文件，分别是`k8s_serving.yaml`和 k8s_ingress.yaml`.

为减少大家的阅读时间，我们只选择以pipeline为例。

```yaml
#k8s_serving.yaml
apiVersion: v1
kind: Service
metadata:
  labels:
    app: ocr
  name: ocr
spec:
  ports:
  - port: 18080
    name: http
    protocol: TCP
    targetPort: 18080
  selector:
    app: ocr
---
apiVersion: apps/v1
kind: Deployment
metadata:
  labels:
    app: ocr
  name: ocr
spec:
  replicas: 1
  selector:
    matchLabels:
      app: ocr
  strategy: {}
  template:
    metadata:
      creationTimestamp: null
      labels:
        app: ocr
    spec:
      containers:
      - image: registry.baidubce.com/paddlepaddle/serving:k8s-pipeline-demo
        name: ocr
        ports:
        - containerPort: 18080
        workingDir: /home/ocr
        name: ocr
        command: ['/bin/bash', '-c']
        args: ["python3.6 bert_web_service.py bert_seq128_model 9292"]
        env:
          - name: NODE_NAME
            valueFrom:
              fieldRef:
                fieldPath: spec.nodeName
          - name: POD_NAME
            valueFrom:
              fieldRef:
                fieldPath: metadata.name
          - name: POD_NAMESPACE
            valueFrom:
              fieldRef:
                fieldPath: metadata.namespace
          - name: POD_IP
            valueFrom:
              fieldRef:
                fieldPath: status.podIP
        resources: {}
```

```yaml
#kong_api.yaml
apiVersion: extensions/v1beta1
kind: Ingress
metadata:
  name: ocr
  annotations:
    kubernetes.io/ingress.class: kong
spec:
  rules:
  - http:
      paths:
      - path: /ocr
        backend:
          serviceName: ocr
          servicePort: 18080
```

最终我们执行就可以启动相关容器和API网关。

```
kubectl apply -f k8s_serving.yaml
kubectl apply -f k8s_ingress.yaml
```

输入

```
kubectl get deploy
```

可见

```
NAME   READY   UP-TO-DATE   AVAILABLE   AGE
ocr    1/1     1            1           2d20h
```

我们使用

```
kubectl get service --all-namespaces
```

可以看到

```
NAMESPACE     NAME                      TYPE           CLUSTER-IP       EXTERNAL-IP   PORT(S)                    AGE
default       bert                      ClusterIP      172.16.86.12     <none>        9292/TCP                   20m
default       kubernetes                ClusterIP      172.16.0.1       <none>        443/TCP                    28d
default       ocr                       ClusterIP      172.16.152.43    <none>        9999/TCP                   50m
kong          kong-proxy                LoadBalancer   172.16.88.132    <pending>     80:8893/TCP,443:8805/TCP   25d
kong          kong-validation-webhook   ClusterIP      172.16.38.100    <none>        443/TCP                    25d
kube-system   heapster                  ClusterIP      172.16.240.64    <none>        80/TCP                     28d
kube-system   kube-dns                  ClusterIP      172.16.0.10      <none>        53/UDP,53/TCP,9153/TCP     28d
kube-system   metrics-server            ClusterIP      172.16.34.157    <none>        443/TCP                    28d
```

访问的方式就在

```:
http://${KONG_IP}:80/${APP_NAME}/prediction
```

例如Bert

```
curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"words": "hello"}], "fetch":["pooled_output"]}' http://172.16.88.132:80/bert/prediction
```

就会从KONG的网关转发给bert服务。同理，OCR服务也可以把对应的IP地址换成`http://172.16.88.132:80/ocr/prediction`
