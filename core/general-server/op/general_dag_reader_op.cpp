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

#include "core/general-server/op/general_dag_reader_op.h"
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

// in general dag reader
// input is from rpc request
// output is pre-defined with resource loaded from config files
// each rpc input corresponds to a output tensor
int GeneralDAGReaderOp::inference() {
  // reade request from client
  const Request *req = dynamic_cast<const Request *>(get_request_message());

  int batch_size = req->insts_size();
  int input_var_num = 0;
  std::vector<int64_t> elem_type;
  std::vector<int64_t> elem_size;
  std::vector<int64_t> capacity;

  GeneralBlob *res = mutable_data<GeneralBlob>();
  TensorVector *tmp_out = &res->tensor_vector;
  // TensorVector tmp_out;

  res->SetBatchSize(batch_size);

  if (!res) {
    LOG(ERROR) << "Failed get op tls reader object output";
  }

  Timer timeline;
  int64_t start = timeline.TimeStampUS();
  int var_num = req->insts(0).tensor_array_size();
  VLOG(2) << "var num: " << var_num;

  VLOG(2) << "start to call load general model_conf op";
  std::shared_ptr<PaddleGeneralModelConfig> model_config = get_config();

  VLOG(2) << "print general model config done.";

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
    VLOG(2) << "lod_tensor.name=" << model_config->_feed_name[i];
    lod_tensor.name = model_config->_feed_name[i];
    tmp_out->push_back(lod_tensor);
  }

  // specify the memory needed for output tensor_vector
  for (int i = 0; i < var_num; ++i) {
    if (tmp_out->at(i).lod.size() == 1) {
      for (int j = 0; j < batch_size; ++j) {
        const Tensor &tensor = req->insts(j).tensor_array(i);
        int data_len = 0;
        if (tensor.int64_data_size() > 0) {
          data_len = tensor.int64_data_size();
        } else {
          data_len = tensor.float_data_size();
        }
        VLOG(2) << "tensor size for var[" << i << "]: " << data_len;

        int cur_len = tmp_out->at(i).lod[0].back();
        VLOG(2) << "current len: " << cur_len;

        tmp_out->at(i).lod[0].push_back(cur_len + data_len);
        VLOG(2) << "new len: " << cur_len + data_len;
      }
      tmp_out->at(i).data.Resize(tmp_out->at(i).lod[0].back() * elem_size[i]);
      tmp_out->at(i).shape = {tmp_out->at(i).lod[0].back(), 1};
      VLOG(2) << "var[" << i
              << "] is lod_tensor and len=" << tmp_out->at(i).lod[0].back();
    } else {
      tmp_out->at(i).data.Resize(batch_size * capacity[i] * elem_size[i]);
      VLOG(2) << "var[" << i
              << "] is tensor and capacity=" << batch_size * capacity[i];
    }
  }

  // fill the data into output general_blob
  for (int i = 0; i < var_num; ++i) {
    if (elem_type[i] == 0) {
      int64_t *dst_ptr = static_cast<int64_t *>(tmp_out->at(i).data.data());
      int offset = 0;
      for (int j = 0; j < batch_size; ++j) {
        int elem_num = req->insts(j).tensor_array(i).int64_data_size();
        for (int k = 0; k < elem_num; ++k) {
          dst_ptr[offset + k] =
              req->insts(j).tensor_array(i).int64_data(k);
        }
        if (tmp_out->at(i).lod.size() == 1) {
          offset = tmp_out->at(i).lod[0][j + 1];
        } else {
          offset += capacity[i];
        }
      }
    } else {
      float *dst_ptr = static_cast<float *>(tmp_out->at(i).data.data());
      int offset = 0;
      for (int j = 0; j < batch_size; ++j) {
        int elem_num = req->insts(j).tensor_array(i).float_data_size();
        for (int k = 0; k < elem_num; ++k) {
          dst_ptr[offset + k] =
              req->insts(j).tensor_array(i).float_data(k);
        }
        if (tmp_out->at(i).lod.size() == 1) {
          offset = tmp_out->at(i).lod[0][j + 1];
        } else {
          offset += capacity[i];
        }
      }
    }
  }

  /*
  VLOG(2) << "output size: " << tmp_out->size();
  for (int i = 0; i < tmp_out->size(); ++i) {
    VLOG(2) << "moving " << tmp_out[i].name
            << " index at "
            << model_config->_graph.feed_name_to_idx[tmp_out[i].name];
    out->push_back(std::move(tmp_out[i]));
  }
  */
  VLOG(2) << "output size: " << tmp_out->size();

  timeline.Pause();
  int64_t end = timeline.TimeStampUS();
  res->p_size = 0;
  AddBlobInfo(res, start);
  AddBlobInfo(res, end);

  VLOG(2) << "read data from client success";
  return 0;
}
DEFINE_OP(GeneralDAGReaderOp);
}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
