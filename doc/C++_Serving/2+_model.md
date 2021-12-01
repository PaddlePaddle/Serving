# 如何使用C++定义模型组合

如果您的模型处理过程包含2+的模型推理环节（例如OCR一般需要det+rec两个环节），此时有两种做法可以满足您的需求。

1. 启动两个Serving服务（例如Serving-det, Serving-rec），在您的Client中，读入数据——det前处理——调用Serving-det预测——det后处理——rec前处理——调用Serving-rec预测——rec后处理——输出结果。
    - 优点：无须改动Paddle Serving代码
    - 缺点：需要两次请求服务，请求数据量越大，效率稍差。
2. 通过修改代码，自定义模型预测行为（自定义OP），自定义服务处理的流程（自定义DAG），将多个模型的组合处理过程(上述的det前处理——调用Serving-det预测——det后处理——rec前处理——调用Serving-rec预测——rec后处理)集成在一个Serving服务中。此时，在您的Client中，读入数据——调用集成后的Serving——输出结果。
    - 优点：只需要一次请求服务，效率高。
    - 缺点：需要改动代码，且需要重新编译。

本文主要介绍第二种效率高的方法，该方法的基本步骤如下：
1. 自定义OP
2. 自定义DAG
3. 编译
4. 服务启动与调用

# 1. 自定义OP
OP是Paddle Serving服务端的处理流程（即DAG图）的基本组成，参考[从0开始自定义OP](./OP_CN.md)，该文档只是讲述了如何自定义一个调用预测的OP节点，您可以在此基础上加上前处理，后处理。


首先获取前置OP的输出，作为本OP的输入，并可以根据自己的需求，通过修改TensorVector* in指向的内存的数据，进行数据的前处理。
``` c++
  const GeneralBlob *input_blob = get_depend_argument<GeneralBlob>(pre_name());
  const TensorVector *in = &input_blob->tensor_vector;
```

声明本OP的输出
``` c++
  GeneralBlob *output_blob = mutable_data<GeneralBlob>();
  TensorVector *out = &output_blob->tensor_vector;
  int batch_size = input_blob->GetBatchSize();
  output_blob->SetBatchSize(batch_size);
```

完成前处理和定义输出变了后，核心调用预测引擎的一句话如下：
``` c++
if (InferManager::instance().infer(engine_name().c_str(), in, out, batch_size)) {
    LOG(ERROR) << "Failed do infer in fluid model: " << engine_name().c_str();
    return -1;
}
```

在此之后，模型预测的输出已经写入与OP绑定的TensorVector* out指针变量所指向的内存空间，此时`可以通过修改TensorVector* out指向的内存的数据，进行数据的后处理`，下一个后置OP获取该OP的输出。

最后如果您使用Python API的方式启动Server端，在服务器端为Paddle Serving定义C++运算符后，最后一步是在Python API中为Paddle Serving服务器API添加注册， `python/paddle_serving_server/dag.py`文件里有关于API注册的代码如下

``` python
self.op_dict = {
            "general_infer": "GeneralInferOp",
            "general_reader": "GeneralReaderOp",
            "general_response": "GeneralResponseOp",
            "general_text_reader": "GeneralTextReaderOp",
            "general_text_response": "GeneralTextResponseOp",
            "general_single_kv": "GeneralSingleKVOp",
            "general_dist_kv_infer": "GeneralDistKVInferOp",
            "general_dist_kv": "GeneralDistKVOp",
            "general_copy": "GeneralCopyOp",
            "general_detection":"GeneralDetectionOp",
        }
```
其中左侧的`”general_infer“名字为自定义（下文有用）`，右侧的`"GeneralInferOp"为自定义的C++OP类的类名`。

在`python/paddle_serving_server/server.py`文件中仅添加`需要加载模型，执行推理预测的自定义的C++OP类的类名`。例如`general_reader`由于只是做一些简单的数据处理而不加载模型调用预测，故在👆的代码中需要添加，而不添加在👇的代码中。
``` python
default_engine_types = [
                'GeneralInferOp',
                'GeneralDistKVInferOp',
                'GeneralDistKVQuantInferOp',
                'GeneralDetectionOp',
            ]
```

# 2. 自定义DAG
DAG图是Server端处理流程的基本定义，在完成上述OP定义的基础上，参考[自定义DAG图](./DAG_CN.md)，您可以自行构建Server端多模型（即多个OP）之间的处理逻辑关系。

框架一般需要在开头加上一个`general_reader`，在结尾加上一个`general_response`，中间添加实际需要调用预测的自定义OP，例如`general_infer`就是一个框架定义好的默认OP,它只调用预测，没有前后处理。

例如，对于OCR模型来说，实际是串联det和rec两个模型，我们可以使用一个`自定义的"general_detection"`和`"general_infer"(注意，此处名字必须与上述Python API中严格对应)`构建DAG图，代码(`python/paddle_serving_server/serve.py`）原理如下图所示。

``` python
import paddle_serving_server as serving
from paddle_serving_server import OpMaker
from paddle_serving_server import OpSeqMaker

op_maker = serving.OpMaker()
read_op = op_maker.create('general_reader')
general_detection_op = op_maker.create('general_detection')
general_infer_op = op_maker.create('general_infer')
general_response_op = op_maker.create('general_response')

op_seq_maker = serving.OpSeqMaker()
op_seq_maker.add_op(read_op)
op_seq_maker.add_op(general_detection_op)
op_seq_maker.add_op(general_infer_op)
op_seq_maker.add_op(general_response_op)
```

# 3. 编译
此时，需要您重新编译生成serving，并通过`export SERVING_BIN`设置环境变量来指定使用您编译生成的serving二进制文件，并通过`pip3 install`的方式安装相关python包，细节请参考[如何编译Serving](../Compile_CN.md)

# 4. 服务启动与调用
## 4.1 Server端启动
仍然以OCR模型为例，分别单独启动det单模型和的脚本代码如下：
```python
#分别单独启动模型
python3 -m paddle_serving_server.serve --model ocr_det_model --port 9293#det模型
python3 -m paddle_serving_server.serve --model ocr_rec_model --port 9294#rec模型
```
在前面三个小节工作做好的基础上，一个服务启动两个模型串联，只需要在`--model后依次按顺序传入模型文件夹的相对路径`即可，脚本代码如下：
```python
#一个服务启动多模型串联
python3 -m paddle_serving_server.serve --model ocr_det_model ocr_rec_model --port 9295#多模型串联
```

## 4.2 Client端调用
此时，Client端的调用，也需要传入两个Client端的[proto定义](./Serving_Configure_CN.md)，python脚本代码如下：
```python
#一个服务启动多模型串联
python3 ocr_cpp_client.py ocr_det_client ocr_rec_client
#ocr_det_client为第一个模型的Client端proto文件夹相对路径
#ocr_rec_client为第二个模型的Client端proto文件夹相对路径
```
此时，对于Server端而言，`'general_reader'`会检查输入的数据的格式是否与第一个模型的Client端proto格式定义的一致，`'general_response'`会保证输出的数据格式与第二个模型的Client端proto文件一致。
