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

#include "examples/demo-serving/op/ctr_prediction_op.h"
#include <algorithm>
#include <string>
#if 0
#include <iomanip>
#endif
#include "core/cube/cube-api/include/cube_api.h"
#include "core/predictor/framework/infer.h"
#include "core/predictor/framework/kv_manager.h"
#include "core/predictor/framework/memory.h"

// Flag where enable profiling mode
DECLARE_bool(enable_ctr_profiling);

namespace baidu {
namespace paddle_serving {
namespace serving {

using baidu::paddle_serving::predictor::MempoolWrapper;
using baidu::paddle_serving::predictor::ctr_prediction::CTRResInstance;
using baidu::paddle_serving::predictor::ctr_prediction::Response;
using baidu::paddle_serving::predictor::ctr_prediction::CTRReqInstance;
using baidu::paddle_serving::predictor::ctr_prediction::Request;

const int VARIABLE_NAME_LEN = 256;

// Total 26 sparse input + 1 dense input
const int CTR_PREDICTION_INPUT_SLOTS = 27;

// First 26: sparse input
const int CTR_PREDICTION_SPARSE_SLOTS = 26;

// Last 1: dense input
const int CTR_PREDICTION_DENSE_SLOT_ID = 26;
const int CTR_PREDICTION_DENSE_DIM = 13;
const int CTR_PREDICTION_EMBEDDING_SIZE = 10;

bthread::Mutex CTRPredictionOp::mutex_;
int64_t CTRPredictionOp::cube_time_us_ = 0;
int32_t CTRPredictionOp::cube_req_num_ = 0;
int32_t CTRPredictionOp::cube_req_key_num_ = 0;

void fill_response_with_message(Response *response,
                                int err_code,
                                std::string err_msg) {
  if (response == NULL) {
    LOG(ERROR) << "response is NULL";
    return;
  }

  response->set_err_code(err_code);
  response->set_err_msg(err_msg);
  return;
}

int CTRPredictionOp::inference() {
  const Request *req = dynamic_cast<const Request *>(get_request_message());

  TensorVector *in = butil::get_object<TensorVector>();
  Response *res = mutable_data<Response>();

  uint32_t sample_size = req->instances_size();
  if (sample_size <= 0) {
    LOG(WARNING) << "No instances need to inference!";
    fill_response_with_message(res, -1, "Sample size invalid");
    return 0;
  }

  paddle::PaddleTensor lod_tensors[CTR_PREDICTION_INPUT_SLOTS];
  for (int i = 0; i < CTR_PREDICTION_INPUT_SLOTS; ++i) {
    lod_tensors[i].dtype = paddle::PaddleDType::FLOAT32;
    std::vector<std::vector<size_t>> &lod = lod_tensors[i].lod;
    lod.resize(1);
    lod[0].push_back(0);
  }

  // Query cube API for sparse embeddings
  std::vector<uint64_t> keys;
  std::vector<rec::mcube::CubeValue> values;

  for (uint32_t si = 0; si < sample_size; ++si) {
    const CTRReqInstance &req_instance = req->instances(si);
    if (req_instance.sparse_ids_size() != CTR_PREDICTION_SPARSE_SLOTS) {
      std::ostringstream iss;
      iss << "Sparse input size != " << CTR_PREDICTION_SPARSE_SLOTS;
      fill_response_with_message(res, -1, iss.str());
      return 0;
    }

    for (int i = 0; i < req_instance.sparse_ids_size(); ++i) {
      keys.push_back(req_instance.sparse_ids(i));
    }
  }

  rec::mcube::CubeAPI *cube = rec::mcube::CubeAPI::instance();
  predictor::KVManager &kv_manager = predictor::KVManager::instance();
  const predictor::KVInfo *kvinfo =
      kv_manager.get_kv_info(CTR_PREDICTION_MODEL_NAME);
  if (kvinfo == NULL) {
    LOG(ERROR) << "Sparse param service info not found for model "
               << CTR_PREDICTION_MODEL_NAME
               << ". Maybe forgot to specify sparse_param_service_type and "
               << "sparse_param_service_table_name in "
               << "conf/model_toolkit.prototxt";
    fill_response_with_message(res, -1, "Sparse param service info not found");
    return 0;
  }

  std::string table_name;
  if (kvinfo->sparse_param_service_type != configure::EngineDesc::NONE) {
    table_name = kvinfo->sparse_param_service_table_name;
    if (table_name.empty()) {
      LOG(ERROR) << "sparse_param_service_table_name not specified. "
                 << "Please specify it in conf/model_toolkit.protxt for model "
                 << CTR_PREDICTION_MODEL_NAME;
      fill_response_with_message(
          res, -1, "sparse_param_service_table_name not specified");
      return 0;
    }
  }

  if (kvinfo->sparse_param_service_type == configure::EngineDesc::LOCAL) {
    // Query local KV service
    LOG(ERROR) << "Local kv service not supported for model "
               << CTR_PREDICTION_MODEL_NAME;

    fill_response_with_message(
        res, -1, "Local kv service not supported for this model");
    return 0;
  } else if (kvinfo->sparse_param_service_type ==
             configure::EngineDesc::REMOTE) {
    struct timeval start;
    struct timeval end;

    int ret;

    gettimeofday(&start, NULL);
    ret = cube->seek(table_name, keys, &values);
    gettimeofday(&end, NULL);
    uint64_t usec =
        end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec;

    // Statistics
    mutex_.lock();
    cube_time_us_ += usec;
    ++cube_req_num_;
    cube_req_key_num_ += keys.size();

    if (cube_req_num_ >= 1000) {
      LOG(INFO) << "Cube request count: " << cube_req_num_;
      LOG(INFO) << "Cube request key count: " << cube_req_key_num_;
      LOG(INFO) << "Cube request total time: " << cube_time_us_ << "us";
      LOG(INFO) << "Average "
                << static_cast<float>(cube_time_us_) / cube_req_num_
                << "us/req";
      LOG(INFO) << "Average "
                << static_cast<float>(cube_time_us_) / cube_req_key_num_
                << "us/key";

      cube_time_us_ = 0;
      cube_req_num_ = 0;
      cube_req_key_num_ = 0;
    }
    mutex_.unlock();
    // Statistics end

    if (ret != 0) {
      fill_response_with_message(res, -1, "Query cube for embeddings error");
      LOG(ERROR) << "Query cube for embeddings error";
      return 0;
    }
  }

  if (values.size() != keys.size()) {
    LOG(ERROR) << "Sparse embeddings not ready; "
               << "maybe forgot to set sparse_param_service_type and "
               << "sparse_param_sevice_table_name for "
               << CTR_PREDICTION_MODEL_NAME
               << " in conf/model_toolkit.prototxt";
    fill_response_with_message(
        res, -1, "Sparse param service not configured properly");
    return 0;
  }

  for (int i = 0; i < keys.size(); ++i) {
    std::ostringstream oss;
    oss << keys[i] << ": ";
    const char *value = (values[i].buff.data());
    if (values[i].buff.size() !=
        sizeof(float) * CTR_PREDICTION_EMBEDDING_SIZE) {
      LOG(WARNING) << "Key " << keys[i] << " has values less than "
                   << CTR_PREDICTION_EMBEDDING_SIZE;
    }

#if 0
      for (int j = 0; j < values[i].buff.size(); ++j) {
        oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
            << (static_cast<int>(value[j]) & 0xff);
      }

      LOG(INFO) << oss.str().c_str();
#endif
  }

  // Sparse embeddings
  for (int i = 0; i < CTR_PREDICTION_SPARSE_SLOTS; ++i) {
    paddle::PaddleTensor &lod_tensor = lod_tensors[i];
    std::vector<std::vector<size_t>> &lod = lod_tensor.lod;

    char name[VARIABLE_NAME_LEN];
    snprintf(name, VARIABLE_NAME_LEN, "embedding_%d.tmp_0", i);
    lod_tensor.name = std::string(name);

    for (uint32_t si = 0; si < sample_size; ++si) {
      const CTRReqInstance &req_instance = req->instances(si);
      lod[0].push_back(lod[0].back() + 1);
    }

    lod_tensor.shape = {lod[0].back(), CTR_PREDICTION_EMBEDDING_SIZE};
    lod_tensor.data.Resize(lod[0].back() * sizeof(float) *
                           CTR_PREDICTION_EMBEDDING_SIZE);

    int offset = 0;
    for (uint32_t si = 0; si < sample_size; ++si) {
      float *data_ptr = static_cast<float *>(lod_tensor.data.data()) + offset;
      const CTRReqInstance &req_instance = req->instances(si);

      int idx = si * CTR_PREDICTION_SPARSE_SLOTS + i;
      if (values[idx].buff.size() !=
          sizeof(float) * CTR_PREDICTION_EMBEDDING_SIZE) {
        LOG(ERROR) << "Embedding vector size not expected";
        fill_response_with_message(
            res, -1, "Embedding vector size not expected");
        return 0;
      }

      memcpy(data_ptr, values[idx].buff.data(), values[idx].buff.size());
      offset += CTR_PREDICTION_EMBEDDING_SIZE;
    }

    in->push_back(lod_tensor);
  }

  // Dense features
  paddle::PaddleTensor &lod_tensor = lod_tensors[CTR_PREDICTION_DENSE_SLOT_ID];
  lod_tensor.dtype = paddle::PaddleDType::FLOAT32;
  std::vector<std::vector<size_t>> &lod = lod_tensor.lod;
  lod_tensor.name = std::string("dense_input");

  for (uint32_t si = 0; si < sample_size; ++si) {
    const CTRReqInstance &req_instance = req->instances(si);
    if (req_instance.dense_ids_size() != CTR_PREDICTION_DENSE_DIM) {
      std::ostringstream iss;
      iss << "dense input size != " << CTR_PREDICTION_DENSE_DIM;
      fill_response_with_message(res, -1, iss.str());
      return 0;
    }
    lod[0].push_back(lod[0].back() + req_instance.dense_ids_size());
  }

  lod_tensor.shape = {lod[0].back() / CTR_PREDICTION_DENSE_DIM,
                      CTR_PREDICTION_DENSE_DIM};
  lod_tensor.data.Resize(lod[0].back() * sizeof(float));

  int offset = 0;
  for (uint32_t si = 0; si < sample_size; ++si) {
    float *data_ptr = static_cast<float *>(lod_tensor.data.data()) + offset;
    const CTRReqInstance &req_instance = req->instances(si);
    int id_count = req_instance.dense_ids_size();
    memcpy(data_ptr,
           req_instance.dense_ids().data(),
           sizeof(float) * req_instance.dense_ids_size());
    offset += req_instance.dense_ids_size();
  }

  in->push_back(lod_tensor);

  TensorVector *out = butil::get_object<TensorVector>();
  if (!out) {
    LOG(ERROR) << "Failed get tls output object";
    fill_response_with_message(res, -1, "Failed get thread local resource");
    return 0;
  }

  // call paddle fluid model for inferencing
  if (predictor::InferManager::instance().infer(
          CTR_PREDICTION_MODEL_NAME, in, out, sample_size)) {
    LOG(ERROR) << "Failed do infer in fluid model: "
               << CTR_PREDICTION_MODEL_NAME;
    fill_response_with_message(res, -1, "Failed do infer in fluid model");
    return 0;
  }

  if (out->size() != 1) {
    LOG(ERROR) << "Model returned number of fetch tensor more than 1";
    fill_response_with_message(
        res, -1, "Model returned number of fetch tensor more than 1");
    return 0;
  }

  int output_shape_dim = out->at(0).shape.size();
  if (output_shape_dim != 2) {
    LOG(ERROR) << "Fetch LoDTensor should be shape of [sample_size, 2]";
    fill_response_with_message(
        res, -1, "Fetch LoDTensor should be shape of [sample_size, 2]");
    return 0;
  }

  if (out->at(0).dtype != paddle::PaddleDType::FLOAT32) {
    LOG(ERROR) << "Fetch LoDTensor data type should be FLOAT32";
    fill_response_with_message(
        res, -1, "Fetch LoDTensor data type should be FLOAT32");
    return 0;
  }

  int dim1 = out->at(0).shape[0];
  int dim2 = out->at(0).shape[1];

  if (dim1 != sample_size) {
    LOG(ERROR) << "Returned result count not equal to sample_size";
    fill_response_with_message(
        res, -1, "Returned result count not equal to sample size");
    return 0;
  }

  if (dim2 != 2) {
    LOG(ERROR) << "Returned result is not expected, should be 2 floats for "
                  "each sample";
    fill_response_with_message(
        res, -1, "Retunred result is not 2 floats for each sample");
    return 0;
  }

  float *data = static_cast<float *>(out->at(0).data.data());
  for (int i = 0; i < dim1; ++i) {
    CTRResInstance *res_instance = res->add_predictions();
    res_instance->set_prob0(data[i * dim2]);
    res_instance->set_prob1(data[i * dim2 + 1]);
  }

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

  res->set_err_code(0);
  res->set_err_msg(std::string(""));
  return 0;
}

DEFINE_OP(CTRPredictionOp);

}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
