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

#include "elastic-ctr/serving/op/elastic_ctr_prediction_op.h"
#include <algorithm>
#include <iomanip>
#include <string>
#include "cube/cube-api/include/cube_api.h"
#include "predictor/framework/infer.h"
#include "predictor/framework/kv_manager.h"
#include "predictor/framework/memory.h"

// Flag where enable profiling mode
DECLARE_bool(enable_ctr_profiling);

namespace baidu {
namespace paddle_serving {
namespace serving {

using baidu::paddle_serving::predictor::MempoolWrapper;
using baidu::paddle_serving::predictor::elastic_ctr::Slot;
using baidu::paddle_serving::predictor::elastic_ctr::ResInstance;
using baidu::paddle_serving::predictor::elastic_ctr::Response;
using baidu::paddle_serving::predictor::elastic_ctr::ReqInstance;
using baidu::paddle_serving::predictor::elastic_ctr::Request;

const int VARIABLE_NAME_LEN = 256;

const int CTR_PREDICTION_DENSE_DIM = 13;
const int CTR_PREDICTION_EMBEDDING_SIZE = 9;

bthread::Mutex ElasticCTRPredictionOp::mutex_;
int64_t ElasticCTRPredictionOp::cube_time_us_ = 0;
int32_t ElasticCTRPredictionOp::cube_req_num_ = 0;
int32_t ElasticCTRPredictionOp::cube_req_key_num_ = 0;

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

int ElasticCTRPredictionOp::inference() {
  const Request *req = dynamic_cast<const Request *>(get_request_message());

  TensorVector *in = butil::get_object<TensorVector>();
  Response *res = mutable_data<Response>();

  uint32_t sample_size = req->instances_size();
  if (sample_size <= 0) {
    LOG(WARNING) << "No instances need to inference!";
    fill_response_with_message(res, -1, "Sample size invalid");
    return 0;
  }

  Samples samples;
  samples.resize(req->instances_size());

  for (int i = 0; i < req->instances_size(); ++i) {
    const ReqInstance &req_instance = req->instances(i);
    for (int j = 0; j < req_instance.slots_size(); ++j) {
      const Slot &slot = req_instance.slots(j);
      for (int k = 0; k < slot.feasigns().size(); ++k) {
        int slot_id = strtol(slot.slot_name().c_str(), NULL, 10);
        samples[i][slot_id].push_back(slot.feasigns(k));
      }
    }
  }

  // Verify all request instances have same slots
  std::vector<int> slot_ids;
  for (auto x : samples[0]) {
    slot_ids.push_back(x.first);
  }
  std::sort(slot_ids.begin(), slot_ids.end());

  // use of slot_map:
  //
  // Example:
  // slot_ids: 1, 20, 50, 100
  //
  // Then
  // slot_map[1] = 0
  // slot_map[20] = 1
  // slot_map[50] = 2
  // slot_map[100] = 3
  //
  // Later we use slot_map to index into lod_tenor array
  //
  std::map<int, int> slot_map;  // NOLINT
  int index = 0;
  for (auto slot_id : slot_ids) {
    slot_map[slot_id] = index;
    ++index;
  }

  for (size_t i = 1; i < samples.size(); ++i) {
    if (samples[i].size() != slot_ids.size()) {
      LOG(WARNING) << "Req " << i
                   << " has different slot num with that of req 0";
      fill_response_with_message(
          res, -1, "Req intance has varying slot numbers");
      return 0;
    }

    for (auto slot : samples[i]) {
      int id = slot.first;
      auto x = std::find(slot_ids.begin(), slot_ids.end(), id);
      if (x == slot_ids.end()) {
        std::ostringstream oss;
        oss << "Req instance " << i << " has an outlier slot id: " << id;
        fill_response_with_message(res, -1, oss.str().c_str());
        return 0;
      }
    }
  }

  // Query cube API for sparse embeddings
  std::vector<uint64_t> keys;
  std::vector<rec::mcube::CubeValue> values;

  // How to organize feasigns in the above `keys` vector:
  //
  // Assuming N instances, each instance having M feature slots:
  //
  // ins1:
  // slot_1: ins1_slot1_1|ins1_slot1_2   slot2: ins1_slot2_1|ins1_slot2_2
  //
  // ins2:
  // slot_1: ins2_slot1_1                slot2: ins2_slot2_1|ins2_slot2_2
  //
  // ...
  //
  // insN:
  // slot_1: insN_slot1_1|insN_slot1_2   slot2: insN_slot2_1
  //
  // We organize the features in such a way that all slot_1 features are before
  // slot_2 features:
  //
  // ins1_slot1_1|ins1_slot1_2|ins2_slot1_1|...|insN_slot1_1|insN_slot1_2
  // ins1_slot2_1|ins1_slot2_2|ins2_slot2_1|ins2_slot2_2|...|insN_slot2_1
  //
  // With this placement, after querying KV service, we can retrieve the
  // embeddings for each feature slot from the returned `values` vector easily,
  // as they are grouped togegher.

  // Level of details of each feature slot
  std::vector<std::vector<size_t>> feature_slot_lods;
  feature_slot_lods.resize(slot_ids.size());

  // Number of feature signs in each slot
  std::vector<int> feature_slot_sizes;
  feature_slot_sizes.resize(slot_ids.size());

  // Iterate over each feature slot
  for (auto slot_id : slot_ids) {
    feature_slot_lods[slot_map[slot_id]].push_back(0);
    feature_slot_sizes[slot_map[slot_id]] = 0;

    // Extract feature i values from each instance si
    for (size_t si = 0; si < samples.size(); ++si) {
      Sample &sample = samples[si];
      std::vector<int64_t> &slot = sample[slot_id];
      feature_slot_lods[slot_map[slot_id]].push_back(
          feature_slot_lods[slot_map[slot_id]].back() + slot.size());
      feature_slot_sizes[slot_map[slot_id]] += slot.size();

      for (size_t j = 0; j < slot.size(); ++j) {
        keys.push_back(slot[j]);
      }
    }
  }

#if 1
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

  for (size_t i = 0; i < keys.size(); ++i) {
    std::ostringstream oss;
    oss << keys[i] << ": ";
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

  // Fill feature embedding into feed tensors
  std::vector<paddle::PaddleTensor> lod_tensors;
  lod_tensors.resize(slot_ids.size());

  for (auto slot_id : slot_ids) {
    paddle::PaddleTensor &lod_tensor = lod_tensors[slot_map[slot_id]];

    char name[VARIABLE_NAME_LEN];
    snprintf(name, VARIABLE_NAME_LEN, "embedding_%d.tmp_0", slot_id);
    lod_tensor.name = std::string(name);

    lod_tensor.dtype = paddle::PaddleDType::FLOAT32;
    std::vector<std::vector<size_t>> &lod = lod_tensor.lod;
    lod.resize(1);
    lod[0].push_back(0);
  }

  int base = 0;

  // Iterate over all slots
  for (auto slot_id : slot_ids) {
    paddle::PaddleTensor &lod_tensor = lod_tensors[slot_map[slot_id]];
    std::vector<std::vector<size_t>> &lod = lod_tensor.lod;

    lod[0] = feature_slot_lods[slot_map[slot_id]];

    lod_tensor.shape = {lod[0].back(), CTR_PREDICTION_EMBEDDING_SIZE};
    lod_tensor.data.Resize(lod[0].back() * sizeof(float) *
                           CTR_PREDICTION_EMBEDDING_SIZE);

    int offset = 0;
    // Copy all slot i feature embeddings to lod_tensor[i]
    for (uint32_t j = 0; j < feature_slot_sizes[slot_map[slot_id]]; ++j) {
      float *data_ptr = static_cast<float *>(lod_tensor.data.data()) + offset;

      int idx = base + j;
      if (values[idx].buff.size() !=
          sizeof(float) * CTR_PREDICTION_EMBEDDING_SIZE) {
#if 0
        LOG(ERROR) << "Embedding vector size not expected";
        fill_response_with_message(
            res, -1, "Embedding vector size not expected");
        return 0;
#else
// sizeof(float) * CTR_PREDICTION_EMBEDDING_SIZE = 36
#if 1
        LOG(INFO) << "values[" << idx << "].buff.size != 36";
#endif
        values[idx].buff.append(36, '0');
#endif
      }

      memcpy(data_ptr, values[idx].buff.data(), values[idx].buff.size());

      offset += CTR_PREDICTION_EMBEDDING_SIZE;
    }

    in->push_back(lod_tensor);

    // Bump base counter
    base += feature_slot_sizes[slot_map[slot_id]];
  }

#else

  // Fill all tensors
  std::vector<paddle::PaddleTensor> lod_tensors;
  lod_tensors.resize(slot_num);

  const ReqInstance &instance = req->instances(0);
  for (int i = 0; i < slot_num; ++i) {
    paddle::PaddleTensor &lod_tensor = lod_tensors[i];

    lod_tensor.name = instance.slots(i).slot_name();

    LOG(INFO) << "slot " << i << "name: " << lod_tensor.name.c_str();
    lod_tensors[i].dtype = paddle::PaddleDType::INT64;
  }

  // Iterate over all slots
  for (int i = 0; i < slot_num; ++i) {
    paddle::PaddleTensor &lod_tensor = lod_tensors[i];
    std::vector<std::vector<size_t>> &lod = lod_tensor.lod;

    lod.push_back(feature_slot_lods[i]);

    lod_tensor.shape = {lod[0].back(), 1};
    lod_tensor.data.Resize(lod[0].back() * sizeof(uint64_t));

    int offset = 0;
    // Copy all slot i features to lod_tensor[i]
    uint64_t *keys_block = keys.data();
    uint64_t *data_ptr = static_cast<uint64_t *>(lod_tensor.data.data());

    memcpy(data_ptr,
           keys_block + offset,
           feature_slot_sizes[i] * sizeof(uint64_t));
    offset += feature_slot_sizes[i];

    in->push_back(lod_tensor);

    // Bump base counter
    offset += feature_slot_sizes[i];
  }
#endif
  TensorVector *out = butil::get_object<TensorVector>();
  if (!out) {
    LOG(ERROR) << "Failed get tls output object";
    fill_response_with_message(res, -1, "Failed get thread local resource");
    return 0;
  }

  // call paddle fluid model for inference
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
    ResInstance *res_instance = res->add_predictions();
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

DEFINE_OP(ElasticCTRPredictionOp);

}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
