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
using baidu::paddle_serving::predictor::PaddleGeneralModelConfig;
// support: FLOAT32, INT64, INT32, UINT8, INT8, FLOAT16
enum ProtoDataType {
  P_INT64 = 0,
  P_FLOAT32,
  P_INT32,
  P_FP64,
  P_INT16,
  P_FP16,
  P_BF16,
  P_UINT8,
  P_INT8,
  P_BOOL,
  P_COMPLEX64,
  P_COMPLEX128,
  P_STRING = 20,
};

int GeneralReaderOp::inference() {
  // read request from client
  const Request *req = dynamic_cast<const Request *>(get_request_message());
  if (!req) {
    LOG(ERROR) << "Failed get request message";
    return -1;
  }

  uint64_t log_id = req->log_id();
  int input_var_num = 0;

  GeneralBlob *res = mutable_data<GeneralBlob>();
  if (!res) {
    LOG(ERROR) << "(logid=" << log_id << ") Failed get GeneralBlob";
    return -1;
  }

  TensorVector *out = &(res->tensor_vector);
  if (!out) {
    LOG(ERROR) << "(logid=" << log_id << ") Failed get tensor_vector of res";
    return -1;
  }

  res->SetLogId(log_id);
  Timer timeline;
  int64_t start = timeline.TimeStampUS();
  // var_num means the number of feed_var.
  int var_num = req->tensor_size();

  VLOG(2) << "(logid=" << log_id << ") var num: " << var_num
          << ") start to call load general model_conf op";
  if (var_num < 1) {
    LOG(ERROR) << "(logid=" << log_id
               << ") Failed get feed_var, var_num=" << var_num;
    return -1;
  }

  baidu::paddle_serving::predictor::Resource &resource =
      baidu::paddle_serving::predictor::Resource::instance();

  VLOG(2) << "(logid=" << log_id << ") get resource pointer done.";

  // package tensor
  // prepare basic information for input
  // specify the memory needed for output tensor_vector
  // fill the data into output general_blob
  int data_len = 0;
  int64_t elem_type = 0;
  int64_t elem_size = 0;
  int64_t databuf_size = 0;
  const void *src_ptr = nullptr;
  for (int i = 0; i < var_num; ++i) {
    paddle::PaddleTensor paddleTensor;
    const Tensor &tensor = req->tensor(i);
    data_len = 0;
    elem_type = 0;
    elem_size = 0;
    databuf_size = 0;
    elem_type = tensor.elem_type();
    src_ptr = nullptr;
    if (elem_type == P_INT64) {  // int64
      elem_size = sizeof(int64_t);
      paddleTensor.dtype = paddle::PaddleDType::INT64;
      data_len = tensor.int64_data_size();
      src_ptr = tensor.int64_data().data();
    } else if (elem_type == P_FLOAT32) {
      elem_size = sizeof(float);
      paddleTensor.dtype = paddle::PaddleDType::FLOAT32;
      data_len = tensor.float_data_size();
      src_ptr = tensor.float_data().data();
    } else if (elem_type == P_INT32) {
      elem_size = sizeof(int32_t);
      paddleTensor.dtype = paddle::PaddleDType::INT32;
      data_len = tensor.int_data_size();
      src_ptr = tensor.int_data().data();
    } else if (elem_type == P_UINT8) {
      elem_size = sizeof(uint8_t);
      paddleTensor.dtype = paddle::PaddleDType::UINT8;
      data_len = tensor.tensor_content().size();
      src_ptr = tensor.tensor_content().data();
    } else if (elem_type == P_INT8) {
      elem_size = sizeof(int8_t);
      paddleTensor.dtype = paddle::PaddleDType::INT8;
      data_len = tensor.tensor_content().size();
      src_ptr = tensor.tensor_content().data();
    } else if (elem_type == P_FP16) {
      // copy bytes from tensor content to TensorVector
      elem_size = 1;
      paddleTensor.dtype = paddle::PaddleDType::FLOAT16;
      data_len = tensor.tensor_content().size();
      src_ptr = tensor.tensor_content().data();
    } else if (elem_type == P_STRING) {
      // use paddle::PaddleDType::UINT8 as for String.
      elem_size = sizeof(char);
      paddleTensor.dtype = paddle::PaddleDType::UINT8;
      // this is for vector<String>, cause the databuf_size !=
      // vector<String>.size()*sizeof(char);
      // data_len should be +1 cause '\0'
      // now only support single string
      for (int idx = 0; idx < tensor.data_size(); idx++) {
        data_len += tensor.data()[idx].length() + 1;
        src_ptr = tensor.data()[idx].data();
      }
    }
    VLOG(2) << "var[" << i << "] has elem type: " << elem_type << ";"
            << "elem_size=" << elem_size << ";"
            << "dtype=" << paddleTensor.dtype << ";"
            << "data_len=" << data_len;
    if (src_ptr == nullptr) {
      LOG(ERROR) << "Not support var[" << i << "] with elem_type[" << elem_type
                 << "]";
      continue;
    }
    // implement lod tensor here
    // only support 1-D lod
    // TODO(HexToString): support 2-D lod
    if (tensor.lod_size() > 0) {
      VLOG(2) << "(logid=" << log_id << ") var[" << i << "] is lod_tensor";
      int lod_index = -1;
      for (int k = 0; k < tensor.lod_size(); ++k) {
        if (tensor.lod(k) == 0) {
          lod_index++;
          paddleTensor.lod.resize(lod_index + 1);
        }
        paddleTensor.lod[lod_index].push_back(tensor.lod(k));
        VLOG(2) << "(logid=" << log_id << ") lod[" << lod_index
                << "]=" << tensor.lod(k);
      }
    }

    for (int k = 0; k < tensor.shape_size(); ++k) {
      int dim = tensor.shape(k);
      VLOG(2) << "(logid=" << log_id << ") shape for var[" << i << "]: " << dim;
      paddleTensor.shape.push_back(dim);
    }
    paddleTensor.name = tensor.name();
    out->push_back(paddleTensor);

    VLOG(2) << "(logid=" << log_id << ") tensor size for var[" << i
            << "]: " << data_len;
    databuf_size = data_len * elem_size;
    void *databuf_char = MempoolWrapper::instance().malloc(databuf_size);
    paddle::PaddleBuf paddleBuf(databuf_char, databuf_size);
    out->at(i).data = paddleBuf;
    // out->at(i).data.Resize(databuf_size);
    if (out->at(i).lod.size() > 0) {
      VLOG(2) << "(logid=" << log_id << ") var[" << i
              << "] has lod_tensor and len=" << out->at(i).lod[0].back();
    }
    void *dst_ptr = out->at(i).data.data();
    if (!dst_ptr) {
      LOG(ERROR) << "dst_ptr is nullptr";
      return -1;
    }

    // For common data, we just copy from src to dst
    // For string data, we need to iterate through all str
    if (elem_type != P_STRING) {
      memcpy(dst_ptr, src_ptr, databuf_size);
    } else {
      char *dst_ptr = static_cast<char *>(out->at(i).data.data());
      VLOG(2) << "(logid=" << log_id << ") first element data in var[" << i
              << "] is " << tensor.data(0);
      if (!dst_ptr) {
        LOG(ERROR) << "dst_ptr is nullptr";
        return -1;
      }
      int elem_num = tensor.data_size();
      int offset = 0;
      for (int k = 0; k < elem_num; ++k) {
        memcpy(dst_ptr + offset,
               tensor.data(k).c_str(),
               strlen(tensor.data(k).c_str()) + 1);
        offset += strlen(tensor.data(k).c_str()) + 1;
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
