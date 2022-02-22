# 如何使用C++定义模型组合

如果您的模型处理过程包含2+的模型推理环节（例如OCR一般需要det+rec两个环节），此时有两种做法可以满足您的需求。

1. 启动两个Serving服务（例如Serving-det, Serving-rec），在您的Client中，读入数据——det前处理——调用Serving-det预测——det后处理——rec前处理——调用Serving-rec预测——rec后处理——输出结果。
    - 优点：无须改动Paddle Serving代码
    - 缺点：需要两次请求服务，请求数据量越大，效率稍差。
2. 通过修改代码，自定义模型预测行为（自定义OP），自定义服务处理的流程（自定义DAG），将多个模型的组合处理过程(上述的det前处理——调用Serving-det预测——det后处理——rec前处理——调用Serving-rec预测——rec后处理)集成在一个Serving服务中。此时，在您的Client中，读入数据——调用集成后的Serving——输出结果。
    - 优点：只需要一次请求服务，效率高。
    - 缺点：需要改动代码，且需要重新编译。

本文主要介绍第二种效率高的方法，该方法的基本步骤如下：
1. 自定义OP（即定义单个模型的前处理-模型预测-模型后处理）
2. 编译
3. 服务启动与调用

# 1. 自定义OP
一个OP定义了单个模型的前处理-模型预测-模型后处理，定义OP需要以下2步：
1. 定义C++ .h头文件
2. 定义C++ .cpp源文件

## 1.1 定义C++ .h头文件
复制👇的代码，将其中`/*自定义Class名称*/`更换为自定义的类名即可，如`GeneralDetectionOp`

放置于`core/general-server/op/`路径下，文件名自定义即可，如`general_detection_op.h`
``` C++
#pragma once
#include <string>
#include <vector>
#include "core/general-server/general_model_service.pb.h"
#include "core/general-server/op/general_infer_helper.h"
#include "paddle_inference_api.h"  // NOLINT

namespace baidu {
namespace paddle_serving {
namespace serving {

class /*自定义Class名称*/
    : public baidu::paddle_serving::predictor::OpWithChannel<GeneralBlob> {
 public:
  typedef std::vector<paddle::PaddleTensor> TensorVector;

  DECLARE_OP(/*自定义Class名称*/);

  int inference();
};

}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
```
## 1.2 定义C++ .cpp源文件
复制👇的代码，将其中`/*自定义Class名称*/`更换为自定义的类名，如`GeneralDetectionOp`

将前处理和后处理的代码添加在👇的代码中注释的前处理和后处理的位置。

放置于`core/general-server/op/`路径下，文件名自定义即可，如`general_detection_op.cpp`

``` C++
#include "core/general-server/op/自定义的头文件名"
#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>
#include "core/predictor/framework/infer.h"
#include "core/predictor/framework/memory.h"
#include "core/predictor/framework/resource.h"
#include "core/util/include/timer.h"

namespace baidu {
namespace paddle_serving {
namespace serving {

using baidu::paddle_serving::Timer;
using baidu::paddle_serving::predictor::MempoolWrapper;
using baidu::paddle_serving::predictor::general_model::Tensor;
using baidu::paddle_serving::predictor::general_model::Response;
using baidu::paddle_serving::predictor::general_model::Request;
using baidu::paddle_serving::predictor::InferManager;
using baidu::paddle_serving::predictor::PaddleGeneralModelConfig;

int /*自定义Class名称*/::inference() {
  //获取前置OP节点
  const std::vector<std::string> pre_node_names = pre_names();
  if (pre_node_names.size() != 1) {
    LOG(ERROR) << "This op(" << op_name()
               << ") can only have one predecessor op, but received "
               << pre_node_names.size();
    return -1;
  }
  const std::string pre_name = pre_node_names[0];

  //将前置OP的输出，作为本OP的输入。
  GeneralBlob *input_blob = mutable_depend_argument<GeneralBlob>(pre_name);
  if (!input_blob) {
    LOG(ERROR) << "input_blob is nullptr,error";
    return -1;
  }
  TensorVector *in = &input_blob->tensor_vector;
  uint64_t log_id = input_blob->GetLogId();
  int batch_size = input_blob->_batch_size;

  //初始化本OP的输出。
  GeneralBlob *output_blob = mutable_data<GeneralBlob>();
  output_blob->SetLogId(log_id);
  output_blob->_batch_size = batch_size;
  VLOG(2) << "(logid=" << log_id << ") infer batch size: " << batch_size;
  TensorVector *out = &output_blob->tensor_vector;

  //前处理的代码添加在此处，前处理直接修改上文的TensorVector* in
  //注意in里面的数据是前置节点的输出经过后处理后的out中的数据

  Timer timeline;
  int64_t start = timeline.TimeStampUS();
  timeline.Start();
  // 将前处理后的in，初始化的out传入，进行模型预测，模型预测的输出会直接修改out指向的内存中的数据
  // 如果您想定义一个不需要模型调用，只进行数据处理的OP，删除下面这一部分的代码即可。
  if (InferManager::instance().infer(
          engine_name().c_str(), in, out, batch_size)) {
    LOG(ERROR) << "(logid=" << log_id
               << ") Failed do infer in fluid model: " << engine_name().c_str();
    return -1;
  }

  //后处理的代码添加在此处，后处理直接修改上文的TensorVector* out
  //后处理后的out会被传递给后续的节点

  int64_t end = timeline.TimeStampUS();
  CopyBlobInfo(input_blob, output_blob);
  AddBlobInfo(output_blob, start);
  AddBlobInfo(output_blob, end);
  return 0;
}
DEFINE_OP(/*自定义Class名称*/);

}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
```
### TensorVector数据结构
TensorVector* in和out都是一个TensorVector类型的指指针，其使用方法跟Paddle C++API中的Tensor几乎一样，相关的数据结构如下所示

``` C++
//TensorVector
typedef std::vector<paddle::PaddleTensor> TensorVector;

//paddle::PaddleTensor
struct PD_INFER_DECL PaddleTensor {
  PaddleTensor() = default;
  std::string name;  ///<  variable name.
  std::vector<int> shape;
  PaddleBuf data;  ///<  blob of data.
  PaddleDType dtype;
  std::vector<std::vector<size_t>> lod;  ///<  Tensor+LoD equals LoDTensor
};

//PaddleBuf
class PD_INFER_DECL PaddleBuf {
 public:

 explicit PaddleBuf(size_t length)
      : data_(new char[length]), length_(length), memory_owned_(true) {}

  PaddleBuf(void* data, size_t length)
      : data_(data), length_(length), memory_owned_{false} {}

  explicit PaddleBuf(const PaddleBuf& other);

  void Resize(size_t length);
  void Reset(void* data, size_t length);
  bool empty() const { return length_ == 0; }
  void* data() const { return data_; }
  size_t length() const { return length_; }
  ~PaddleBuf() { Free(); }
  PaddleBuf& operator=(const PaddleBuf&);
  PaddleBuf& operator=(PaddleBuf&&);
  PaddleBuf() = default;
  PaddleBuf(PaddleBuf&& other);
 private:
  void Free();
  void* data_{nullptr};  ///< pointer to the data memory.
  size_t length_{0};     ///< number of memory bytes.
  bool memory_owned_{true};
};
```

### TensorVector代码示例
```C++
/*例如，你想访问输入数据中的第1个Tensor*/
paddle::PaddleTensor& tensor_1 = in->at(0);
/*例如，你想修改输入数据中的第1个Tensor的名称*/
tensor_1.name = "new name";
/*例如，你想获取输入数据中的第1个Tensor的shape信息*/
std::vector<int> tensor_1_shape = tensor_1.shape;
/*例如，你想修改输入数据中的第1个Tensor中的数据*/
void* data_1 = tensor_1.data.data();
//后续直接修改data_1指向的内存即可
//比如，当您的数据是int类型，将void*转换为int*进行处理即可
```


# 2. 编译
此时，需要您重新编译生成serving，并通过`export SERVING_BIN`设置环境变量来指定使用您编译生成的serving二进制文件，并通过`pip3 install`的方式安装相关python包，细节请参考[如何编译Serving](../Compile_CN.md)

# 3. 服务启动与调用
## 3.1 Server端启动

在前面两个小节工作做好的基础上，一个服务启动两个模型串联，只需要在`--model后依次按顺序传入模型文件夹的相对路径`，且需要在`--op后依次传入自定义C++OP类名称`，其中--model后面的模型与--op后面的类名称的顺序需要对应，`这里假设我们已经定义好了两个OP分别为GeneralDetectionOp和GeneralRecOp`，则脚本代码如下：
```python
#一个服务启动多模型串联
python3 -m paddle_serving_server.serve --model ocr_det_model ocr_rec_model --op GeneralDetectionOp GeneralRecOp --port 9292
#多模型串联 ocr_det_model对应GeneralDetectionOp  ocr_rec_model对应GeneralRecOp
```

## 3.2 Client端调用
此时，Client端的调用，也需要传入两个Client端的proto文件或文件夹的路径，以OCR为例，可以参考[ocr_cpp_client.py](../../examples/C++/PaddleOCR/ocr/ocr_cpp_client.py)来自行编写您的脚本，此时Client调用如下：
```python
#一个服务启动多模型串联
python3 自定义.py ocr_det_client ocr_rec_client
#ocr_det_client为第一个模型的Client端proto文件夹的相对路径
#ocr_rec_client为第二个模型的Client端proto文件夹的相对路径
```
此时，对于Server端而言，输入的数据的格式与`第一个模型的Client端proto格式`定义的一致，输出的数据格式与`最后一个模型的Client端proto`文件一致。一般情况下您无须关注此事，当您需要了解详细的[proto的定义，请参考此处](../Serving_Configure_CN.md)。
