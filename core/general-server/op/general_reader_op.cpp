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
#include "core/general-server/op/general_infer_helper.h"
#include "core/general-server/op/general_reader_op.h"
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

int conf_check(const Request *req,
               const std::shared_ptr<PaddleGeneralModelConfig> &model_config) {
  int var_num = req->insts(0).tensor_array_size();
  if (var_num != model_config->_feed_type.size()) {
    VLOG(2) << "var num: " << var_num;
    VLOG(2) << "model config var num: " << model_config->_feed_type.size();
    LOG(ERROR) << "feed var number not match.";
    return -1;
  }

  VLOG(2) << "fetch var num in reader op: " << req->fetch_var_names_size();

  for (int i = 0; i < var_num; ++i) {
    if (model_config->_feed_type[i] !=
        req->insts(0).tensor_array(i).elem_type()) {
      LOG(ERROR) << "feed type not match.";
      return -1;
    }
    if (model_config->_feed_shape[i].size() ==
        req->insts(0).tensor_array(i).shape_size()) {
      for (int j = 0; j < model_config->_feed_shape[i].size(); ++j) {
        req->insts(0).tensor_array(i).shape(j);
        if (model_config->_feed_shape[i][j] !=
            req->insts(0).tensor_array(i).shape(j)) {
          LOG(ERROR) << "feed shape not match.";
          return -1;
        }
      }
    } else {
      LOG(ERROR) << "feed shape not match.";
      return -1;
    }
  }
  return 0;
}

int GeneralReaderOp::inference() {
  // reade request from client
  const Request *req = dynamic_cast<const Request *>(get_request_message());

  int batch_size = req->insts_size();
  int input_var_num = 0;

  std::vector<int64_t> elem_type;
  std::vector<int64_t> elem_size;
  std::vector<int64_t> capacity;

  GeneralBlob *res = mutable_data<GeneralBlob>();
  TensorVector *out = &res->tensor_vector;

  if (!res) {
    LOG(ERROR) << "Failed get op tls reader object output";
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

  // TODO(guru4elephant): how to do conditional check?
  res->reader_status = conf_check(req, model_config);
  if (res->reader_status != 0) {
    LOG(INFO) << "model conf of server:";
    resource.print_general_model_config(model_config);
    return 0;
  }
  // package tensor

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
    out->push_back(lod_tensor);
  }

  for (int i = 0; i < var_num; ++i) {
    if (out->at(i).lod.size() == 1) {
      for (int j = 0; j < batch_size; ++j) {
        const Tensor &tensor = req->insts(j).tensor_array(i);
        int data_len = tensor.data_size();
        VLOG(2) << "tensor size for var[" << i << "]: " << tensor.data_size();
        int cur_len = out->at(i).lod[0].back();
        VLOG(2) << "current len: " << cur_len;
        out->at(i).lod[0].push_back(cur_len + data_len);
        VLOG(2) << "new len: " << cur_len + data_len;
      }
      out->at(i).data.Resize(out->at(i).lod[0].back() * elem_size[i]);
      out->at(i).shape = {out->at(i).lod[0].back(), 1};
      VLOG(2) << "var[" << i
              << "] is lod_tensor and len=" << out->at(i).lod[0].back();
    } else {
      out->at(i).data.Resize(batch_size * capacity[i] * elem_size[i]);
      VLOG(2) << "var[" << i
              << "] is tensor and capacity=" << batch_size * capacity[i];
    }
  }

  for (int i = 0; i < var_num; ++i) {
    if (elem_type[i] == 0) {
      int64_t *dst_ptr = static_cast<int64_t *>(out->at(i).data.data());
      int offset = 0;
      for (int j = 0; j < batch_size; ++j) {
        for (int k = 0; k < req->insts(j).tensor_array(i).data_size(); ++k) {
          dst_ptr[offset + k] =
              *(const int64_t *)req->insts(j).tensor_array(i).data(k).c_str();
        }
        if (out->at(i).lod.size() == 1) {
          offset = out->at(i).lod[0][j + 1];
        } else {
          offset += capacity[i];
        }
      }
    } else {
      float *dst_ptr = static_cast<float *>(out->at(i).data.data());
      int offset = 0;
      for (int j = 0; j < batch_size; ++j) {
        for (int k = 0; k < req->insts(j).tensor_array(i).data_size(); ++k) {
          dst_ptr[offset + k] =
              *(const float *)req->insts(j).tensor_array(i).data(k).c_str();
        }
        if (out->at(i).lod.size() == 1) {
          offset = out->at(i).lod[0][j + 1];
        } else {
          offset += capacity[i];
        }
      }
    }
  }

  VLOG(2) << "read data from client success";
  return 0;
}
DEFINE_OP(GeneralReaderOp);
}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
