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

#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>
#include "core/general-server/op/general_text_reader_op.h"
#include "core/general-server/op/general_infer_helper.h"
#include "core/predictor/framework/infer.h"
#include "core/predictor/framework/memory.h"

namespace baidu {
namespace paddle_serving {
namespace serving {

using baidu::paddle_serving::predictor::MempoolWrapper;
using baidu::paddle_serving::predictor::general_model::Tensor;
using baidu::paddle_serving::predictor::general_model::Request;
using baidu::paddle_serving::predictor::general_model::FeedInst;
using baidu::paddle_serving::predictor::PaddleGeneralModelConfig;


int GeneralTextReaderOp::inference() {
  // reade request from client
  const Request *req = dynamic_cast<const Request *>(get_request_message());

  int batch_size = req->insts_size();
  int input_var_num = 0;

  std::vector<int64_t> elem_type;
  std::vector<int64_t> elem_size;
  std::vector<int64_t> capacity;

  GeneralBlob *res = mutable_data<GeneralBlob>();
  TensorVector *in = &res->tensor_vector;

  if (!res) {
    LOG(ERROR) << "Failed get op tls reader object output";
  }

  if (batch_size <= 0) {
    res->reader_status = -1;
    return 0;
  }

  int var_num = req->insts(0).tensor_array_size();
  VLOG(2) << "var num: " << var_num;
  // read config

  VLOG(2) << "start to call load general model_conf op";
  baidu::paddle_serving::predictor::Resource &resource =
      baidu::paddle_serving::predictor::Resource::instance();

  VLOG(2) << "get resource pointer done.";
  std::shared_ptr<PaddleGeneralModelConfig> model_config =
      resource.get_general_model_config();

  VLOG(2) << "print general model config done.";

  elem_type.resize(var_num);
  elem_size.resize(var_num);
  capacity.resize(var_num);
  for (int i = 0; i < var_num; ++i) {
    paddle::PaddleTensor lod_tensor;
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
    lod_tensor.name = model_config->_feed_name[i];
    in->push_back(lod_tensor);
  }

  for (int i = 0; i < var_num; ++i) {
    if (in->at(i).lod.size() == 1) {
      for (int j = 0; j < batch_size; ++j) {
        const Tensor &tensor = req->insts(j).tensor_array(i);
        int data_len = tensor.int_data_size();
        int cur_len = in->at(i).lod[0].back();
        in->at(i).lod[0].push_back(cur_len + data_len);
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
        for (int k = 0;
             k < req->insts(j).tensor_array(i).int_data_size();
             ++k) {
          dst_ptr[offset + k] =
              req->insts(j).tensor_array(i).int_data(k);
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
        for (int k = 0;
             k < req->insts(j).tensor_array(i).int_data_size();
             ++k) {
          dst_ptr[offset + k] =
              req->insts(j).tensor_array(i).int_data(k);
        }
        if (in->at(i).lod.size() == 1) {
          offset = in->at(i).lod[0][j + 1];
        } else {
          offset += capacity[i];
        }
      }
    }
  }

  VLOG(2) << "read data from client success";
  return 0;
}
DEFINE_OP(GeneralTextReaderOp);
}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
