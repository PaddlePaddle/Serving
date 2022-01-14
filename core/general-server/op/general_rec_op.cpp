// Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
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

#include "core/general-server/op/general_rec_op.h"
#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>
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
using baidu::paddle_serving::predictor::general_model::Response;
using baidu::paddle_serving::predictor::general_model::Request;
using baidu::paddle_serving::predictor::InferManager;
using baidu::paddle_serving::predictor::PaddleGeneralModelConfig;

/*
std::vector<std::string> ReadDict(const std::string &path) {
  std::ifstream in(path);
  std::string line;
  std::vector<std::string> m_vec;
  if (in) {
    while (getline(in, line)) {
      m_vec.push_back(line);
    }
  } else {
    std::cout << "no such label file: " << path << ", exit the program..."
              << std::endl;
    exit(1);
  }
  return m_vec;
}
*/

int GeneralRecOp::inference() {
  VLOG(2) << "Going to run inference";
  const std::vector<std::string> pre_node_names = pre_names();
  if (pre_node_names.size() != 1) {
    LOG(ERROR) << "This op(" << op_name()
               << ") can only have one predecessor op, but received "
               << pre_node_names.size();
    return -1;
  }
  const std::string pre_name = pre_node_names[0];

  const GeneralBlob *input_blob = get_depend_argument<GeneralBlob>(pre_name);
  if (!input_blob) {
    LOG(ERROR) << "input_blob is nullptr,error";
    return -1;
  }
  uint64_t log_id = input_blob->GetLogId();
  VLOG(2) << "(logid=" << log_id << ") Get precedent op name: " << pre_name;

  GeneralBlob *output_blob = mutable_data<GeneralBlob>();
  if (!output_blob) {
    LOG(ERROR) << "output_blob is nullptr,error";
    return -1;
  }
  output_blob->SetLogId(log_id);

  if (!input_blob) {
    LOG(ERROR) << "(logid=" << log_id
               << ") Failed mutable depended argument, op:" << pre_name;
    return -1;
  }

  const TensorVector *in = &input_blob->tensor_vector;
  TensorVector *out = &output_blob->tensor_vector;

  int batch_size = input_blob->_batch_size;
  output_blob->_batch_size = batch_size;
  VLOG(2) << "(logid=" << log_id << ") infer batch size: " << batch_size;

  Timer timeline;
  int64_t start = timeline.TimeStampUS();
  timeline.Start();

  if (InferManager::instance().infer(
          engine_name().c_str(), in, out, batch_size)) {
    LOG(ERROR) << "(logid=" << log_id
               << ") Failed do infer in fluid model: " << engine_name().c_str();
    return -1;
  }

  /*
  int index = 0;

  std::string label_path = "ppocr_keys_v1.txt";
  std::vector<std::string> label_list_;
  label_list_ = ReadDict(label_path);
  label_list_.insert(label_list_.begin(),"#"); // blank char for ctc
  label_list_.push_back(" ");

  int box_num = batch_size;
  auto predict_shape = out->at(0).shape;
  std::vector<int64_t> rec_idx;
  auto output_t = out->at(0);
  auto rec_idx_lod = output_t.lod;
  auto shape_out = output_t.shape;

  int out_num = std::accumulate(shape_out.begin(), shape_out.end(), 1,
                                std::multiplies<int>());

  rec_idx.resize(out_num);
  memcpy(rec_idx.data(),output_t.data.data(),output_t.data.length());

  std::vector<int> pred_idx;
  for (int n = int(rec_idx_lod[0][0]); n < int(rec_idx_lod[0][1]); n++) {
    pred_idx.push_back(int(rec_idx[n]));
  }

  if (pred_idx.size() < 1e-3){
    continue;
  }

  index += 1;
  std::cout << index << "\t";
  for (int n = 0; n < pred_idx.size(); n++) {
    std::cout << label_list_[pred_idx[n]];
  }
  */

  auto output_tensor_1 = out->at(1);
  auto predict_lod = output_tensor_1.lod;
  auto predict_shape = output_tensor_1.shape;
  auto predict_name = output_tensor_1.name;
  auto predict_type = output_tensor_1.dtype;
  int out_num_1 = std::accumulate(
      predict_shape.begin(), predict_shape.end(), 1, std::multiplies<int>());
  float *output_tensor_1_data = (float *)output_tensor_1.data.data();
  int score_size = predict_lod[0].size() - 1;
  std::vector<float> score_vector(score_size);

  for (int index = 0; index < score_size; ++index) {
    int count = 0;
    float max_value = 0.0f;
    int blank = predict_shape[1];
    int argmax_idx;
    for (int n = predict_lod[0][index]; n < predict_lod[0][index + 1] - 1;
         n++) {
      argmax_idx = int(std::distance(
          &output_tensor_1_data[n * predict_shape[1]],
          std::max_element(&output_tensor_1_data[n * predict_shape[1]],
                           output_tensor_1_data + (n + 1) * predict_shape[1])));
      max_value = float(
          *std::max_element(&output_tensor_1_data[n * predict_shape[1]],
                            output_tensor_1_data + (n + 1) * predict_shape[1]));
      if (blank - 1 - argmax_idx > 1e-5) {
        score_vector[index] += max_value;
        count += 1;
      }
    }
    score_vector[index] /= count;
  }
  out->erase(out->begin() + 1);
  int databuf_size_out = score_size * sizeof(float);
  void *databuf_data_out = NULL;
  char *databuf_char_out = NULL;
  databuf_data_out = MempoolWrapper::instance().malloc(databuf_size_out);
  if (!databuf_data_out) {
    LOG(ERROR) << "Malloc failed, size: " << databuf_size_out;
    return -1;
  }
  memcpy(databuf_data_out, score_vector.data(), databuf_size_out);
  databuf_char_out = reinterpret_cast<char *>(databuf_data_out);
  paddle::PaddleBuf paddleBuf(databuf_char_out, databuf_size_out);
  paddle::PaddleTensor tensor_out;
  tensor_out.name = predict_name;
  tensor_out.dtype = predict_type;
  tensor_out.shape = {score_size, 1};
  tensor_out.data = paddleBuf;
  out->push_back(tensor_out);

  int64_t end = timeline.TimeStampUS();
  CopyBlobInfo(input_blob, output_blob);
  AddBlobInfo(output_blob, start);
  AddBlobInfo(output_blob, end);
  return 0;
}
DEFINE_OP(GeneralRecOp);

}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
