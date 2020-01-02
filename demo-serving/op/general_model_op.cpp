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

#include "demo-serving/op/general_model_op.h"
#include <algorithm>
#include <sstream>
#include <iostream>
#include "predictor/framework/infer.h"
#include "predictor/framework/memory.h"

namespace baidu {
namespace paddle_serving {
namespace serving {

using baidu::paddle_serving::predictor::MempoolWrapper;
using baidu::paddle_serving::predictor::general_model::Tensor;
using baidu::paddle_serving::predictor::general_model::Request;
using baidu::paddle_serving::predictor::general_model::FeedInst;
using baidu::paddle_serving::predictor::general_model::Response;
using baidu::paddle_serving::predictor::general_model::FetchInst;

static std::once_flag g_proto_init_flag;

int GeneralModelOp::inference() {
  const Request *req = dynamic_cast<const Request *>(get_request_message());

  TensorVector *in = butil::get_object<TensorVector>();

  int batch_size = req->insts_size();
  int input_var_num = 0;

  std::vector<int> elem_type;
  std::vector<int> elem_size;
  std::vector<int> capacity;
  if (batch_size > 0) {
    int var_num = req->insts(0).tensor_array_size();
    elem_type.resize(var_num);
    elem_size.resize(var_num);
    capacity.resize(var_num);
    paddle::PaddleTensor lod_tensor;
    for (int i = 0; i < var_num; ++i) {
      elem_type[i] = req->insts(0).tensor_array(i).elem_type();
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
      } else {
        lod_tensor.shape.push_back(batch_size);
        capacity[i] = 1;
        for (int k = 0;
             k < req->insts(0).tensor_array(i).shape_size();
             ++k) {
          int dim = req->insts(0).tensor_array(i).shape(k);
          capacity[i] *= dim;
          lod_tensor.shape.push_back(dim);
        }
      }
      in->push_back(lod_tensor);
    }

    for (int i = 0; i < var_num; ++i) {
      if ((*in)[i].lod.size() > 0) {
        for (int j = 0; j < batch_size; ++j) {
          const Tensor & tensor = req->insts(j).tensor_array(i);
          int data_len = tensor.data_size() / elem_size[i];
          int cur_len = (*in)[i].lod[0].back();
          (*in)[i].lod[0].push_back(cur_len + data_len);
        }
        (*in)[i].data.Resize((*in)[i].lod[0].back());
      } else {
        (*in)[i].data.Resize(batch_size * capacity[i]);
      }
    }

    for (int i = 0; i < var_num; ++i) {
      void * dst_ptr = (*in)[i].data.data();
      int offset = 0;
      for (int j = 0; j < batch_size; ++j) {
        memcpy(dst_ptr + offset,
               (void *)(req->insts(j).tensor_array(i).data().data()),
               req->insts(j).tensor_array(i).data_size() * elem_size[i]);
        if ((*in)[i].lod.size() > 0) {
          offset += (*in)[i].lod[0][j + 1] * elem_size[i];
        } else {
          offset += capacity[i] * elem_size[i];
        }
      }
    }

    TensorVector *out = butil::get_object<TensorVector>();
    if (!out) {
      LOG(ERROR) << "Failed get tls output object";
      return -1;
    }

    if (predictor::InferManager::instance().infer(
            GENERAL_MODEL_NAME, in, out, batch_size)) {
      LOG(ERROR) << "Failed do infer in fluid model: "
                  << GENERAL_MODEL_NAME;
      return -1;
    }

    Response * res = mutable_data<Response>();
    
    // we suppose the dtype of all fetch variables is float
    for (int i = 0; i < batch_size; ++i) {
      FetchInst * fetch_inst = res->add_insts();
      for (int j = 0; j < out->size(); ++j) {
        Tensor * tensor = fetch_inst->add_tensor_array();
        tensor->set_elem_type(1);
        if ((*out)[j].lod.size() > 0) {
          tensor->add_shape(-1);
          tensor->mutable_data()->Reserve(
              (*out)[j].lod[0].back() * sizeof(float));
        } else {
          int cap = 1;
          for (int k = 1; k < (*out)[j].shape.size(); ++k) {
            cap *= (*out)[j].shape[k];
            tensor->add_shape((*out)[j].shape[k]);
          }
          tensor->mutable_data()->Reserve(cap * sizeof(float));
        }
      }
    }

    for (int i = 0; i < out->size(); ++i) {
      if ((*out)[i].lod.size() > 0) {
        for (int j = 0; j < batch_size; ++j) {
          Tensor * tensor = res->mutable_insts(j)->mutable_tensor_array(i);
          void * dst_ptr = tensor->mutable_data()->mutable_data();
          memcpy(dst_ptr,
                 (*out)[i].data.data() + (*out)[i].lod[0][j] * elem_size[i],
                 ((*out)[i].lod[0][j + 1] - (*out)[i].lod[0][j])
                 * elem_size[i]);
        }
      } else {
        for (int j = 0; j < batch_size; ++j) {
          Tensor * tensor = res->mutable_insts(j)->mutable_tensor_array(i);
          void * dst_ptr = tensor->mutable_data()->mutable_data();
          memcpy(dst_ptr,
                 (*out)[i].data.data() + j * capacity[i] * elem_size[i],
                 capacity[i] * elem_size[i]);
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
