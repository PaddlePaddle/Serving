// Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "examples/demo-serving/op/general_model_op.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>
#include "core/predictor/framework/infer.h"
#include "core/predictor/framework/memory.h"
#include "core/predictor/framework/resource.h"

namespace baidu {
namespace paddle_serving {
namespace serving {

using baidu::paddle_serving::predictor::MempoolWrapper;
using baidu::paddle_serving::predictor::general_model::Tensor;
using baidu::paddle_serving::predictor::general_model::Request;
using baidu::paddle_serving::predictor::general_model::FeedInst;
using baidu::paddle_serving::predictor::general_model::Response;
using baidu::paddle_serving::predictor::general_model::FetchInst;
using baidu::paddle_serving::predictor::PaddleGeneralModelConfig;

static std::once_flag g_proto_init_flag;

int GeneralModelOp::inference() {
  // request
  const Request *req = dynamic_cast<const Request *>(get_request_message());

  TensorVector *in = butil::get_object<TensorVector>();

  int batch_size = req->insts_size();
  int input_var_num = 0;

  std::vector<int> elem_type;
  std::vector<int> elem_size;
  std::vector<int> capacity;

  // infer
  if (batch_size > 0) {
    int var_num = req->insts(0).tensor_array_size();
    VLOG(2) << "var num: " << var_num;
    elem_type.resize(var_num);
    elem_size.resize(var_num);
    capacity.resize(var_num);
    paddle::PaddleTensor lod_tensor;
    for (int i = 0; i < var_num; ++i) {
      elem_type[i] = req->insts(0).tensor_array(i).elem_type();
      VLOG(2) << "var[" << i << "] has elem type: " << elem_type[i];
      if (elem_type[i] == 0) {  // int64
        elem_size[i] = sizeof(int64_t);
        lod_tensor.dtype = paddle::PaddleDType::INT64;
      } else {
        elem_size[i] = sizeof(float);
        lod_tensor.dtype = paddle::PaddleDType::FLOAT32;
      }

      if (req->insts(0).tensor_array(i).shape(0) == -1) {
        lod_tensor.lod.resize(1);
        lod_tensor.lod[0].push_back(0);
        VLOG(2) << "var[" << i << "] is lod_tensor";
      } else {
        lod_tensor.shape.push_back(batch_size);
        capacity[i] = 1;
        for (int k = 0; k < req->insts(0).tensor_array(i).shape_size(); ++k) {
          int dim = req->insts(0).tensor_array(i).shape(k);
          VLOG(2) << "shape for var[" << i << "]: " << dim;
          capacity[i] *= dim;
          lod_tensor.shape.push_back(dim);
        }
        VLOG(2) << "var[" << i << "] is tensor, capacity: " << capacity[i];
      }
      if (i == 0) {
        lod_tensor.name = "words";
      } else {
        lod_tensor.name = "label";
      }
      in->push_back(lod_tensor);
    }

    for (int i = 0; i < var_num; ++i) {
      if (in->at(i).lod.size() == 1) {
        for (int j = 0; j < batch_size; ++j) {
          const Tensor &tensor = req->insts(j).tensor_array(i);
          int data_len = tensor.data_size();
          VLOG(2) << "tensor size for var[" << i << "]: " << tensor.data_size();
          int cur_len = in->at(i).lod[0].back();
          VLOG(2) << "current len: " << cur_len;
          in->at(i).lod[0].push_back(cur_len + data_len);
          VLOG(2) << "new len: " << cur_len + data_len;
        }
        in->at(i).data.Resize(in->at(i).lod[0].back() * elem_size[i]);
        in->at(i).shape = {in->at(i).lod[0].back(), 1};
        VLOG(2) << "var[" << i
                << "] is lod_tensor and len=" << in->at(i).lod[0].back();
      } else {
        in->at(i).data.Resize(batch_size * capacity[i] * elem_size[i]);
        VLOG(2) << "var[" << i
                << "] is tensor and capacity=" << batch_size * capacity[i];
      }
    }

    for (int i = 0; i < var_num; ++i) {
      if (elem_type[i] == 0) {
        int64_t *dst_ptr = static_cast<int64_t *>(in->at(i).data.data());
        int offset = 0;
        for (int j = 0; j < batch_size; ++j) {
          for (int k = 0; k < req->insts(j).tensor_array(i).data_size(); ++k) {
            dst_ptr[offset + k] =
                *(const int64_t *)req->insts(j).tensor_array(i).data(k).c_str();
          }
          if (in->at(i).lod.size() == 1) {
            offset = in->at(i).lod[0][j + 1];
          } else {
            offset += capacity[i];
          }
        }
      } else {
        float *dst_ptr = static_cast<float *>(in->at(i).data.data());
        int offset = 0;
        for (int j = 0; j < batch_size; ++j) {
          for (int k = 0; k < req->insts(j).tensor_array(i).data_size(); ++k) {
            dst_ptr[offset + k] =
                *(const float *)req->insts(j).tensor_array(i).data(k).c_str();
          }
          if (in->at(i).lod.size() == 1) {
            offset = in->at(i).lod[0][j + 1];
          } else {
            offset += capacity[i];
          }
        }
      }
    }

    VLOG(2) << "going to infer";
    TensorVector *out = butil::get_object<TensorVector>();
    if (!out) {
      LOG(ERROR) << "Failed get tls output object";
      return -1;
    }

    // print request
    std::ostringstream oss;
    int64_t *example = reinterpret_cast<int64_t *>((*in)[0].data.data());
    for (uint32_t i = 0; i < 10; i++) {
      oss << *(example + i) << " ";
    }
    VLOG(2) << "msg: " << oss.str();

    // infer
    if (predictor::InferManager::instance().infer(
            GENERAL_MODEL_NAME, in, out, batch_size)) {
      LOG(ERROR) << "Failed do infer in fluid model: " << GENERAL_MODEL_NAME;
      return -1;
    }
    // print response
    float *example_1 = reinterpret_cast<float *>((*out)[0].data.data());
    VLOG(2) << "result: " << *example_1;

    Response *res = mutable_data<Response>();

    for (int i = 0; i < batch_size; ++i) {
      FetchInst *fetch_inst = res->add_insts();
      for (int j = 0; j < out->size(); ++j) {
        Tensor *tensor = fetch_inst->add_tensor_array();
        tensor->set_elem_type(1);
        if (out->at(j).lod.size() == 1) {
          tensor->add_shape(-1);
        } else {
          for (int k = 1; k < out->at(j).shape.size(); ++k) {
            tensor->add_shape(out->at(j).shape[k]);
          }
        }
      }
    }

    for (int i = 0; i < out->size(); ++i) {
      float *data_ptr = static_cast<float *>(out->at(i).data.data());
      int cap = 1;
      for (int j = 1; j < out->at(i).shape.size(); ++j) {
        cap *= out->at(i).shape[j];
      }
      if (out->at(i).lod.size() == 1) {
        for (int j = 0; j < batch_size; ++j) {
          for (int k = out->at(i).lod[0][j]; k < out->at(i).lod[0][j + 1];
               k++) {
            res->mutable_insts(j)->mutable_tensor_array(i)->add_data(
                reinterpret_cast<char *>(&(data_ptr[k])), sizeof(float));
          }
        }
      } else {
        for (int j = 0; j < batch_size; ++j) {
          for (int k = j * cap; k < (j + 1) * cap; ++k) {
            res->mutable_insts(j)->mutable_tensor_array(i)->add_data(
                reinterpret_cast<char *>(&(data_ptr[k])), sizeof(float));
          }
        }
      }
    }

    for (size_t i = 0; i < in->size(); ++i) {
      (*in)[i].shape.clear();
    }
    in->clear();
    butil::return_object<TensorVector>(in);

    for (size_t i = 0; i < out->size(); ++i) {
      (*out)[i].shape.clear();
    }
    out->clear();
    butil::return_object<TensorVector>(out);
  }

  return 0;
}

DEFINE_OP(GeneralModelOp);

}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
