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

#include "core/general-server/op/general_reader_op.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>
#include "core/general-server/op/general_infer_helper.h"
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
using baidu::paddle_serving::predictor::general_model::Request;
using baidu::paddle_serving::predictor::general_model::FeedInst;
using baidu::paddle_serving::predictor::PaddleGeneralModelConfig;
using baidu::paddle_serving::predictor::InferManager;

int conf_check(const Request *req,
               const std::shared_ptr<PaddleGeneralModelConfig> &model_config) {
  int var_num = req->insts(0).tensor_array_size();
  if (var_num != model_config->_feed_type.size()) {
    LOG(ERROR) << "feed var number not match: model config["
               << model_config->_feed_type.size() << "] vs. actual[" << var_num
               << "]";
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
  // TODO: only support one engine here
  std::string engine_name = "general_infer_0";
  const Request *req = dynamic_cast<const Request*>(get_request_message());
  uint64_t log_id = req->log_id();
  int input_var_num = 0;
  std::vector<int64_t> elem_type;
  std::vector<int64_t> elem_size;
  std::vector<int64_t> capacity;
  int var_num = req->insts(0).tensor_array_size();
  baidu::paddle_serving::predictor::Resource &resource =
      baidu::paddle_serving::predictor::Resource::instance();
  std::shared_ptr<PaddleGeneralModelConfig> model_config =
      resource.get_general_model_config();
  elem_type.resize(var_num);
  elem_size.resize(var_num);
  capacity.resize(var_num);
  for (int i = 0; i < var_num; ++i) {
    std::string tensor_name = model_config->_feed_name[i];
    std::cout << "START Tensor Name: " <<tensor_name << std::endl;
    auto lod_tensor = InferManager::instance().GetInputHandle(engine_name.c_str(), tensor_name.c_str());
    std::cout << "PICK lod tensor. " << std::endl;
    std::vector<std::vector<size_t>> lod;
    std::vector<int> shape;
    // get lod info here
    if (req->insts(0).tensor_array(i).lod_size() > 0) {
      lod.resize(1);
      for (int k = 0; k < req->insts(0).tensor_array(i).lod_size(); ++k) {
        lod[0].push_back(req->insts(0).tensor_array(i).lod(k));
      }
      capacity[i] = 1;
      for (int k = 0; k < req->insts(0).tensor_array(i).shape_size(); ++k) {
        int dim = req->insts(0).tensor_array(i).shape(k);
        VLOG(2) << "(logid=" << log_id << ") shape for var[" << i
                << "]: " << dim;
        capacity[i] *= dim;
        shape.push_back(dim);
      }
      VLOG(2) << "(logid=" << log_id << ") var[" << i
              << "] is tensor, capacity: " << capacity[i];
    }
    else {
      capacity[i] = 1;
      for (int k = 0; k < req->insts(0).tensor_array(i).shape_size(); ++k) {
        int dim = req->insts(0).tensor_array(i).shape(k);
        VLOG(2) << "(logid=" << log_id << ") shape for var[" << i
                << "]: " << dim;
        capacity[i] *= dim;
        shape.push_back(dim);
      }
      VLOG(2) << "(logid=" << log_id << ") var[" << i
              << "] is tensor, capacity: " << capacity[i];
    }
    lod_tensor->SetLoD(lod);
    lod_tensor->Reshape(shape);
    std::cout << "FINI Set Lod and Reshape, and elem type: "<< elem_type[i] << std::endl;
    // insert data here
    if (req->insts(0).tensor_array(i).elem_type() == 0) {
      // TODO: Copy twice here, can optimize
      int elem_num = req->insts(0).tensor_array(i).int64_data_size();
      std::vector<int64_t> data(elem_num);
      int64_t* dst_ptr = data.data();
      for (int k = 0; k < elem_num; ++k) {
        dst_ptr[k] = req->insts(0).tensor_array(i).int64_data(k);
      }
      lod_tensor->CopyFromCpu(dst_ptr);
    } else if(req->insts(0).tensor_array(i).elem_type() == 1) {
      int elem_num = req->insts(0).tensor_array(i).float_data_size();
      std::vector<float> data(elem_num);
      float* dst_ptr = data.data();
      for (int k = 0; k < elem_num; ++k) {
        dst_ptr[k] = req->insts(0).tensor_array(i).float_data(k);
      }
      lod_tensor->CopyFromCpu(dst_ptr);
    } else if(req->insts(0).tensor_array(i).elem_type()== 2) {
      int elem_num = req->insts(0).tensor_array(i).int_data_size();
      std::vector<int32_t> data(elem_num);
      int32_t* dst_ptr = data.data();
      for (int k = 0; k < elem_num; ++k) {
        dst_ptr[k] = req->insts(0).tensor_array(i).int_data(k);
      }
      lod_tensor->CopyFromCpu(dst_ptr);
    }
    std::cout << "FINISH Tensor Name: " <<tensor_name << std::endl;
  }
  return 0;
}
DEFINE_OP(GeneralReaderOp);
}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
