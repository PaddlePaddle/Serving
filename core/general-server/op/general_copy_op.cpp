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

#include "core/general-server/op/general_copy_op.h"
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

int GeneralCopyOp::inference() {
  // reade request from client
  const std::vector<std::string> pre_node_names = pre_names();
  if (pre_node_names.size() != 1) {
    LOG(ERROR) << "This op(" << op_name()
               << ") can only have one predecessor op, but received "
               << pre_node_names.size();
    return -1;
  }
  const std::string pre_name = pre_node_names[0];

  const GeneralBlob *input_blob = get_depend_argument<GeneralBlob>(pre_name);
  uint64_t log_id = input_blob->GetLogId();

  VLOG(2) << "(logid=" << log_id << ") precedent name: " << pre_name;
  const TensorVector *in = &input_blob->tensor_vector;
  VLOG(2) << "(logid=" << log_id << ") input size: " << in->size();
  int batch_size = input_blob->GetBatchSize();
  int input_var_num = 0;

  GeneralBlob *res = mutable_data<GeneralBlob>();
  res->SetLogId(log_id);
  TensorVector *out = &res->tensor_vector;

  VLOG(2) << "(logid=" << log_id << ") input batch size: " << batch_size;
  res->SetBatchSize(batch_size);

  if (!res) {
    LOG(ERROR) << "(logid=" << log_id
               << ") Failed get op tls reader object output";
  }

  Timer timeline;
  int64_t start = timeline.TimeStampUS();

  VLOG(2) << "(logid=" << log_id << ") Going to init lod tensor";
  for (int i = 0; i < in->size(); ++i) {
    paddle::PaddleTensor lod_tensor;
    CopyLod(&in->at(i), &lod_tensor);
    lod_tensor.dtype = in->at(i).dtype;
    lod_tensor.name = in->at(i).name;
    VLOG(2) << "(logid=" << log_id << ") lod tensor [" << i
            << "].name = " << lod_tensor.name;
    out->push_back(lod_tensor);
  }

  VLOG(2) << "(logid=" << log_id << ") pack done.";

  for (int i = 0; i < out->size(); ++i) {
    int64_t *src_ptr = static_cast<int64_t *>(in->at(i).data.data());
    out->at(i).data.Resize(out->at(i).lod[0].back() * sizeof(int64_t));
    out->at(i).shape = {out->at(i).lod[0].back(), 1};
    int64_t *tgt_ptr = static_cast<int64_t *>(out->at(i).data.data());
    for (int j = 0; j < out->at(i).lod[0].back(); ++j) {
      tgt_ptr[j] = src_ptr[j];
    }
  }

  VLOG(2) << "(logid=" << log_id << ") output done.";

  timeline.Pause();
  int64_t end = timeline.TimeStampUS();
  CopyBlobInfo(input_blob, res);
  AddBlobInfo(res, start);
  AddBlobInfo(res, end);

  VLOG(2) << "(logid=" << log_id << ") read data from client success";
  return 0;
}

DEFINE_OP(GeneralCopyOp);
}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
