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

#pragma once

#include <string.h>
#include <vector>
#include <memory>
#ifdef BCLOUD
#ifdef WITH_GPU
#include "paddle/paddle_inference_api.h"
#else
#include "paddle/fluid/inference/api/paddle_inference_api.h"
#endif
#else
#include "paddle_inference_api.h"  // NOLINT
#endif
#include "core/predictor/framework/resource.h"
#include <string>

namespace baidu {
namespace paddle_serving {
namespace serving {

using baidu::paddle_serving::predictor::PaddleGeneralModelConfig;
static const char* GENERAL_MODEL_NAME = "general_model";

struct GeneralConfig {
  std::vector<paddle::PaddleTensor> inputs;
};

struct GeneralBlob {
  std::vector<paddle::PaddleTensor> tensor_vector;
  int64_t time_stamp[20];
  int p_size = 0;

  int _batch_size;

  void Clear() {
    size_t tensor_count = tensor_vector.size();
    for (size_t ti = 0; ti < tensor_count; ++ti) {
      tensor_vector[ti].shape.clear();
    }
    tensor_vector.clear();
  }

  int SetBatchSize(int batch_size) { _batch_size = batch_size; }

  int GetBatchSize() const { return _batch_size; }
  std::string ShortDebugString() const { return "Not implemented!"; }
};

static std::shared_ptr<PaddleGeneralModelConfig> get_config() {
  baidu::paddle_serving::predictor::Resource &resource =
      baidu::paddle_serving::predictor::Resource::instance();
  std::shared_ptr<PaddleGeneralModelConfig> config =
      resource.get_general_model_config();
  return config;
}

static void AddBlobInfo(GeneralBlob* blob, int64_t init_value) {
  blob->time_stamp[blob->p_size] = init_value;
  blob->p_size++;
}

static void CopyBlobInfo(const GeneralBlob* src, GeneralBlob* tgt) {
  memcpy(&(tgt->time_stamp[0]),
         &(src->time_stamp[0]),
         src->p_size * sizeof(int64_t));
}

static void CopyLod(const paddle::PaddleTensor* src,
                    paddle::PaddleTensor* tgt) {
  VLOG(2) << "copy lod done.";
  tgt->lod.resize(src->lod.size());
  VLOG(2) << "src lod size: " << src->lod.size();
  for (int i = 0; i < src->lod.size(); ++i) {
    tgt->lod[i].resize(src->lod[i].size());
    for (int j = 0; j < src->lod[i].size(); ++j) {
      tgt->lod[i][j] = src->lod[i][j];
    }
  }
}

}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
