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

#include "demo-serving/op/bert_service_op.h"
#include <cstdio>
#include <string>
#include "predictor/framework/infer.h"
#include "predictor/framework/memory.h"
namespace baidu {
namespace paddle_serving {
namespace serving {

using baidu::paddle_serving::predictor::MempoolWrapper;
using baidu::paddle_serving::predictor::bert_service::BertResInstance;
using baidu::paddle_serving::predictor::bert_service::Response;
using baidu::paddle_serving::predictor::bert_service::BertReqInstance;
using baidu::paddle_serving::predictor::bert_service::Request;
using baidu::paddle_serving::predictor::bert_service::Embedding_values;

extern int64_t MAX_SEQ_LEN = 128;
const bool POOLING = true;
const int LAYER_NUM = 12;
const int EMB_SIZE = 768;

int BertServiceOp::inference() {
  timeval op_start;
  gettimeofday(&op_start, NULL);

  const Request *req = dynamic_cast<const Request *>(get_request_message());

  TensorVector *in = butil::get_object<TensorVector>();
  Response *res = mutable_data<Response>();

  uint32_t batch_size = req->instances_size();
  if (batch_size <= 0) {
    LOG(WARNING) << "No instances need to inference!";
    return 0;
  }

  MAX_SEQ_LEN = req->instances(0).max_seq_len();

  paddle::PaddleTensor src_ids;
  paddle::PaddleTensor pos_ids;
  paddle::PaddleTensor seg_ids;
  paddle::PaddleTensor input_masks;
  src_ids.name = std::string("src_ids");
  pos_ids.name = std::string("pos_ids");
  seg_ids.name = std::string("sent_ids");
  input_masks.name = std::string("input_mask");

  src_ids.dtype = paddle::PaddleDType::INT64;
  src_ids.shape = {batch_size, MAX_SEQ_LEN, 1};
  src_ids.data.Resize(batch_size * MAX_SEQ_LEN * sizeof(int64_t));

  pos_ids.dtype = paddle::PaddleDType::INT64;
  pos_ids.shape = {batch_size, MAX_SEQ_LEN, 1};
  pos_ids.data.Resize(batch_size * MAX_SEQ_LEN * sizeof(int64_t));

  seg_ids.dtype = paddle::PaddleDType::INT64;
  seg_ids.shape = {batch_size, MAX_SEQ_LEN, 1};
  seg_ids.data.Resize(batch_size * MAX_SEQ_LEN * sizeof(int64_t));

  input_masks.dtype = paddle::PaddleDType::FLOAT32;
  input_masks.shape = {batch_size, MAX_SEQ_LEN, 1};
  input_masks.data.Resize(batch_size * MAX_SEQ_LEN * sizeof(float));

  std::vector<std::vector<size_t>> lod_set;
  lod_set.resize(1);
  for (uint32_t i = 0; i < batch_size; i++) {
    lod_set[0].push_back(i * MAX_SEQ_LEN);
  }
  // src_ids.lod = lod_set;
  // pos_ids.lod = lod_set;
  // seg_ids.lod = lod_set;
  // input_masks.lod = lod_set;

  uint32_t index = 0;
  for (uint32_t i = 0; i < batch_size; i++) {
    int64_t *src_data = static_cast<int64_t *>(src_ids.data.data()) + index;
    int64_t *pos_data = static_cast<int64_t *>(pos_ids.data.data()) + index;
    int64_t *seg_data = static_cast<int64_t *>(seg_ids.data.data()) + index;
    float *input_masks_data =
        static_cast<float *>(input_masks.data.data()) + index;

    const BertReqInstance &req_instance = req->instances(i);

    memcpy(src_data,
           req_instance.token_ids().data(),
           sizeof(int64_t) * MAX_SEQ_LEN);
#if 1
    memcpy(pos_data,
           req_instance.position_ids().data(),
           sizeof(int64_t) * MAX_SEQ_LEN);
    memcpy(seg_data,
           req_instance.sentence_type_ids().data(),
           sizeof(int64_t) * MAX_SEQ_LEN);
    memcpy(input_masks_data,
           req_instance.input_masks().data(),
           sizeof(float) * MAX_SEQ_LEN);
#endif
    index += MAX_SEQ_LEN;
  }

  in->push_back(src_ids);
  in->push_back(pos_ids);
  in->push_back(seg_ids);
  in->push_back(input_masks);

  TensorVector *out = butil::get_object<TensorVector>();
  if (!out) {
    LOG(ERROR) << "Failed get tls output object";
    return -1;
  }

#if 0  // print request
  std::ostringstream oss;
  for (int j = 0; j < 3; j++) {
    int64_t* example = reinterpret_cast<int64_t*>(*in)[j].data.data();
    for (uint32_t i = 0; i < MAX_SEQ_LEN; i++) {
        oss << *(example + i) << " ";
    }
    oss << ";";
  }
  float* example = reinterpret_cast<float*>(*in)[3].data.data();
  for (int i = 0; i < MAX_SEQ_LEN; i++) {
    oss << *(example + i) << " ";
  }
  LOG(INFO) << "msg: " << oss.str();
#endif
  timeval infer_start;
  gettimeofday(&infer_start, NULL);
  if (predictor::InferManager::instance().infer(
          BERT_MODEL_NAME, in, out, batch_size)) {
    LOG(ERROR) << "Failed do infer in fluid model: " << BERT_MODEL_NAME;
    return -1;
  }
  timeval infer_end;
  gettimeofday(&infer_end, NULL);
  uint64_t infer_time =
      (infer_end.tv_sec * 1000 + infer_end.tv_usec / 1000 -
       (infer_start.tv_sec * 1000 + infer_start.tv_usec / 1000));
#if 0
    LOG(INFO) << "batch_size : " << out->at(0).shape[0]
        << " seq_len : " << out->at(0).shape[1]
        << " emb_size : " << out->at(0).shape[2];

    float *out_data = reinterpret_cast<float *>(out->at(0).data.data());
    for (uint32_t bi = 0; bi < batch_size; bi++) {
      BertResInstance *res_instance = res->add_instances();
      for (uint32_t si = 0; si < MAX_SEQ_LEN; si++) {
        Embedding_values *emb_instance = res_instance->add_instances();
        for (uint32_t ei = 0; ei < EMB_SIZE; ei++) {
          uint32_t index = bi * MAX_SEQ_LEN * EMB_SIZE + si * EMB_SIZE + ei;
          emb_instance->add_values(out_data[index]);
        }
      }
    }
#else
  LOG(INFO) << "batch_size : " << out->at(0).shape[0]
            << " emb_size : " << out->at(0).shape[1];
  float *out_data = reinterpret_cast<float *>(out->at(0).data.data());
  for (uint32_t bi = 0; bi < batch_size; bi++) {
    BertResInstance *res_instance = res->add_instances();
    for (uint32_t si = 0; si < 1; si++) {
      Embedding_values *emb_instance = res_instance->add_instances();
      for (uint32_t ei = 0; ei < EMB_SIZE; ei++) {
        uint32_t index = bi * EMB_SIZE + ei;
        emb_instance->add_values(out_data[index]);
      }
    }
  }

  timeval op_end;
  gettimeofday(&op_end, NULL);
  uint64_t op_time = (op_end.tv_sec * 1000 + op_end.tv_usec / 1000 -
                      (op_start.tv_sec * 1000 + op_start.tv_usec / 1000));

  res->set_op_time(op_time);
  res->set_infer_time(infer_time);
#endif
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

  return 0;
}

DEFINE_OP(BertServiceOp);

}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
