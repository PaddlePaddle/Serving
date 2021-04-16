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
enum ProtoDataType { P_INT64, P_FLOAT32, P_INT32, P_STRING };
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
    const Tensor &tensor = req->insts(0).tensor_array(i);
    if (model_config->_feed_type[i] !=
        tensor.elem_type()) {
      LOG(ERROR) << "feed type not match.";
      return -1;
    }
    if (model_config->_feed_shape[i].size() ==
        tensor.shape_size()) {
      for (int j = 0; j < model_config->_feed_shape[i].size(); ++j) {
        tensor.shape(j);
        if (model_config->_feed_shape[i][j] !=
            tensor.shape(j)) {
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
  // read request from client
  const Request *req = dynamic_cast<const Request *>(get_request_message());
  uint64_t log_id = req->log_id();
  int input_var_num = 0;
  std::vector<int64_t> elem_type;
  std::vector<int64_t> elem_size;
  std::vector<int64_t> databuf_size;

  GeneralBlob *res = mutable_data<GeneralBlob>();
  TensorVector *out = &(res->tensor_vector);
  
  res->SetLogId(log_id);
  if (!res) {
    LOG(ERROR) << "(logid=" << log_id
               << ") Failed get op tls reader object output";
  }

  Timer timeline;
  int64_t start = timeline.TimeStampUS();
  int var_num = req->insts(0).tensor_array_size();

  VLOG(2) << "(logid=" << log_id << ") var num: " << var_num
          << ") start to call load general model_conf op";

  baidu::paddle_serving::predictor::Resource &resource =
      baidu::paddle_serving::predictor::Resource::instance();

  VLOG(2) << "(logid=" << log_id << ") get resource pointer done.";
  //get the first InferOP's model_config as ReaderOp's model_config by default.
  std::shared_ptr<PaddleGeneralModelConfig> model_config =
      resource.get_general_model_config().front();

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
  databuf_size.resize(var_num);
  // prepare basic information for input
  // specify the memory needed for output tensor_vector
  // fill the data into output general_blob
  int data_len = 0;
  for (int i = 0; i < var_num; ++i) {
    paddle::PaddleTensor lod_tensor;
    const Tensor &tensor = req->insts(0).tensor_array(i);
    data_len = 0;
    elem_type[i] = tensor.elem_type();
    VLOG(2) << "var[" << i << "] has elem type: " << elem_type[i];
    if (elem_type[i] == P_INT64) {  // int64
      elem_size[i] = sizeof(int64_t);
      lod_tensor.dtype = paddle::PaddleDType::INT64;
      data_len = tensor.int64_data_size();
    } else if (elem_type[i] == P_FLOAT32) {
      elem_size[i] = sizeof(float);
      lod_tensor.dtype = paddle::PaddleDType::FLOAT32;
      data_len = tensor.float_data_size();
    } else if (elem_type[i] == P_INT32) {
      elem_size[i] = sizeof(int32_t);
      lod_tensor.dtype = paddle::PaddleDType::INT32;
      data_len = tensor.int_data_size();
    } else if (elem_type[i] == P_STRING) {
      //use paddle::PaddleDType::UINT8 as for String.
      elem_size[i] = sizeof(uint8_t);
      lod_tensor.dtype = paddle::PaddleDType::UINT8;
      //this is for vector<String>, cause the databuf_size != vector<String>.size()*sizeof(char);
      for (int idx = 0; idx < tensor.data_size(); idx++) {
        data_len += tensor.data()[idx].length();
      }
    }
    // implement lod tensor here
    // only support 1-D lod
    // TODO:support 2-D lod
    if (tensor.lod_size() > 0) {
      VLOG(2) << "(logid=" << log_id << ") var[" << i << "] is lod_tensor";
      lod_tensor.lod.resize(1);
      for (int k = 0; k < tensor.lod_size(); ++k) {
        lod_tensor.lod[0].push_back(tensor.lod(k));
      }
    }

    for (int k = 0; k < tensor.shape_size(); ++k) {
      int dim = tensor.shape(k);
      VLOG(2) << "(logid=" << log_id << ") shape for var[" << i
              << "]: " << dim;
      lod_tensor.shape.push_back(dim);
    }
    lod_tensor.name = model_config->_feed_name[i];
    out->push_back(lod_tensor);

    
    VLOG(2) << "(logid=" << log_id << ") tensor size for var[" << i
            << "]: " << data_len;
    databuf_size[i] = data_len * elem_size[i];
    out->at(i).data.Resize(data_len * elem_size[i]);
    VLOG(2) << "(logid=" << log_id << ") var[" << i
            << "] is lod_tensor and len=" << out->at(i).lod[0].back();
    
    if (elem_type[i] == P_INT64) {
      int64_t *dst_ptr = static_cast<int64_t *>(out->at(i).data.data());
      VLOG(2) << "(logid=" << log_id << ") first element data in var[" << i
              << "] is " << tensor.int64_data(0);
      if (!dst_ptr) {
        LOG(ERROR) << "dst_ptr is nullptr";
            return -1;
      }
      memcpy(dst_ptr, tensor.int64_data().data(),databuf_size[i]);
      /*
      int elem_num = tensor.int64_data_size();
      for (int k = 0; k < elem_num; ++k) {
        dst_ptr[k] = tensor.int64_data(k);
      }
      */
    } else if (elem_type[i] == P_FLOAT32) {
      float *dst_ptr = static_cast<float *>(out->at(i).data.data());
      VLOG(2) << "(logid=" << log_id << ") first element data in var[" << i
              << "] is " << tensor.float_data(0);
      if (!dst_ptr) {
        LOG(ERROR) << "dst_ptr is nullptr";
            return -1;
      }
      memcpy(dst_ptr, tensor.float_data().data(),databuf_size[i]);
      /*int elem_num = tensor.float_data_size();
      for (int k = 0; k < elem_num; ++k) {
        dst_ptr[k] = tensor.float_data(k);
      }*/
    } else if (elem_type[i] == P_INT32) {
      int32_t *dst_ptr = static_cast<int32_t *>(out->at(i).data.data());
      VLOG(2) << "(logid=" << log_id << ") first element data in var[" << i
              << "] is " << tensor.int_data(0);
      if (!dst_ptr) {
        LOG(ERROR) << "dst_ptr is nullptr";
            return -1;
      }
      memcpy(dst_ptr, tensor.int_data().data(),databuf_size[i]);
      /*
      int elem_num = tensor.int_data_size();
      for (int k = 0; k < elem_num; ++k) {
        dst_ptr[k] = tensor.int_data(k);
      }
      */
    } else if (elem_type[i] == P_STRING) {
      std::string *dst_ptr = static_cast<std::string *>(out->at(i).data.data());
      VLOG(2) << "(logid=" << log_id << ") first element data in var[" << i
              << "] is " << tensor.data(0);
      if (!dst_ptr) {
        LOG(ERROR) << "dst_ptr is nullptr";
            return -1;
      }
      int elem_num = tensor.data_size();
      for (int k = 0; k < elem_num; ++k) {
        dst_ptr[k] = tensor.data(k);
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
