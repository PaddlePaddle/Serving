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
  const Request *req = dynamic_cast<const Request *>(get_request_message());
  uint64_t log_id = req->log_id();
  int input_var_num = 0;
  std::vector<int64_t> elem_type;
  std::vector<int64_t> elem_size;
  std::vector<int64_t> capacity;

  GeneralBlob *res = mutable_data<GeneralBlob>();
  TensorVector *out = &res->tensor_vector;

  res->SetLogId(log_id);

  if (!res) {
    LOG(ERROR) << "(logid=" << log_id
               << ") Failed get op tls reader object output";
  }

  Timer timeline;
  int64_t start = timeline.TimeStampUS();
  int var_num = req->insts(0).tensor_array_size();
  VLOG(2) << "(logid=" << log_id << ") var num: " << var_num;

  VLOG(2) << "(logid=" << log_id
          << ") start to call load general model_conf op";

  baidu::paddle_serving::predictor::Resource &resource =
      baidu::paddle_serving::predictor::Resource::instance();

  VLOG(2) << "(logid=" << log_id << ") get resource pointer done.";
  std::shared_ptr<PaddleGeneralModelConfig> model_config =
      resource.get_general_model_config();

  VLOG(2) << "(logid=" << log_id << ") print general model config done.";

  // TODO(guru4elephant): how to do conditional check?
  /*
  int ret = conf_check(req, model_config);
  if (ret != 0) {
    LOG(ERROR) << "model conf of server:";
    resource.print_general_model_config(model_config);
    return 0;
  }
  */
  // package tensor

  elem_type.resize(var_num);
  elem_size.resize(var_num);
  capacity.resize(var_num);
  // prepare basic information for input
  for (int i = 0; i < var_num; ++i) {
    paddle::PaddleTensor lod_tensor;
    elem_type[i] = req->insts(0).tensor_array(i).elem_type();
    VLOG(2) << "var[" << i << "] has elem type: " << elem_type[i];
    if (elem_type[i] == 0) {  // int64
      elem_size[i] = sizeof(int64_t);
      lod_tensor.dtype = paddle::PaddleDType::INT64;
    } else if (elem_type[i] == 1) {
      elem_size[i] = sizeof(float);
      lod_tensor.dtype = paddle::PaddleDType::FLOAT32;
    } else if (elem_type[i] == 2) {
      elem_size[i] = sizeof(int32_t);
      lod_tensor.dtype = paddle::PaddleDType::INT32;
    }
    // implement lod tensor here
    if (req->insts(0).tensor_array(i).lod_size() > 0) {
      VLOG(2) << "(logid=" << log_id << ") var[" << i << "] is lod_tensor";
      lod_tensor.lod.resize(1);
      for (int k = 0; k < req->insts(0).tensor_array(i).lod_size(); ++k) {
        lod_tensor.lod[0].push_back(req->insts(0).tensor_array(i).lod(k));
      }
      capacity[i] = 1;
      for (int k = 0; k < req->insts(0).tensor_array(i).shape_size(); ++k) {
        int dim = req->insts(0).tensor_array(i).shape(k);
        VLOG(2) << "(logid=" << log_id << ") shape for var[" << i
                << "]: " << dim;
        capacity[i] *= dim;
        lod_tensor.shape.push_back(dim);
      }
      VLOG(2) << "(logid=" << log_id << ") var[" << i
              << "] is tensor, capacity: " << capacity[i];
    } else {
      capacity[i] = 1;
      for (int k = 0; k < req->insts(0).tensor_array(i).shape_size(); ++k) {
        int dim = req->insts(0).tensor_array(i).shape(k);
        VLOG(2) << "(logid=" << log_id << ") shape for var[" << i
                << "]: " << dim;
        capacity[i] *= dim;
        lod_tensor.shape.push_back(dim);
      }
      VLOG(2) << "(logid=" << log_id << ") var[" << i
              << "] is tensor, capacity: " << capacity[i];
    }
    lod_tensor.name = model_config->_feed_name[i];
    out->push_back(lod_tensor);
  }
  // specify the memory needed for output tensor_vector
  for (int i = 0; i < var_num; ++i) {
    if (out->at(i).lod.size() == 1) {
      int tensor_size = 0;
      const Tensor &tensor = req->insts(0).tensor_array(i);
      int data_len = 0;
      if (tensor.int64_data_size() > 0) {
        data_len = tensor.int64_data_size();
      } else if (tensor.float_data_size() > 0) {
        data_len = tensor.float_data_size();
      } else if (tensor.int_data_size() > 0) {
        data_len = tensor.int_data_size();
      }
      VLOG(2) << "(logid=" << log_id << ") tensor size for var[" << i
              << "]: " << data_len;
      tensor_size += data_len;

      int cur_len = out->at(i).lod[0].back();
      VLOG(2) << "(logid=" << log_id << ") current len: " << cur_len;

      int sample_len = 0;
      if (tensor.shape_size() == 1) {
        sample_len = data_len;
      } else {
        sample_len = tensor.shape(0);
      }
      VLOG(2) << "(logid=" << log_id << ") new len: " << cur_len + sample_len;
      out->at(i).data.Resize(tensor_size * elem_size[i]);
      VLOG(2) << "(logid=" << log_id << ") var[" << i
              << "] is lod_tensor and len=" << out->at(i).lod[0].back();
    } else {
      out->at(i).data.Resize(capacity[i] * elem_size[i]);
      VLOG(2) << "(logid=" << log_id << ") var[" << i
              << "] is tensor and capacity=" << capacity[i];
    }
  }

  // fill the data into output general_blob
  for (int i = 0; i < var_num; ++i) {
    if (elem_type[i] == 0) {
      int64_t *dst_ptr = static_cast<int64_t *>(out->at(i).data.data());
      VLOG(2) << "(logid=" << log_id << ") first element data in var[" << i
              << "] is " << req->insts(0).tensor_array(i).int64_data(0);
      int offset = 0;
      int elem_num = req->insts(0).tensor_array(i).int64_data_size();
      for (int k = 0; k < elem_num; ++k) {
        dst_ptr[offset + k] = req->insts(0).tensor_array(i).int64_data(k);
      }
    } else if (elem_type[i] == 1) {
      float *dst_ptr = static_cast<float *>(out->at(i).data.data());
      VLOG(2) << "(logid=" << log_id << ") first element data in var[" << i
              << "] is " << req->insts(0).tensor_array(i).float_data(0);
      int offset = 0;
      int elem_num = req->insts(0).tensor_array(i).float_data_size();
      for (int k = 0; k < elem_num; ++k) {
        dst_ptr[offset + k] = req->insts(0).tensor_array(i).float_data(k);
      }
    } else if (elem_type[i] == 2) {
      int32_t *dst_ptr = static_cast<int32_t *>(out->at(i).data.data());
      VLOG(2) << "(logid=" << log_id << ") first element data in var[" << i
              << "] is " << req->insts(0).tensor_array(i).int_data(0);
      int offset = 0;
      int elem_num = req->insts(0).tensor_array(i).int_data_size();
      for (int k = 0; k < elem_num; ++k) {
        dst_ptr[offset + k] = req->insts(0).tensor_array(i).int_data(k);
      }
    }
  }

  VLOG(2) << "(logid=" << log_id << ") output size: " << out->size();
  timeline.Pause();
  int64_t end = timeline.TimeStampUS();
  res->p_size = 0;
  res->_batch_size = 1;
  AddBlobInfo(res, start);
  AddBlobInfo(res, end);

  VLOG(2) << "(logid=" << log_id << ") read data from client success";
  return 0;
}
DEFINE_OP(GeneralReaderOp);
}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
