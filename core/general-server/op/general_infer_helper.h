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
#ifdef BCLOUD
#ifdef WITH_GPU
#include "paddle/paddle_inference_api.h"
#else
#include "paddle/fluid/inference/api/paddle_inference_api.h"
#endif
#else
#include "paddle_inference_api.h"  // NOLINT
#endif
#include <string>

namespace baidu {
namespace paddle_serving {
namespace serving {

static const char* GENERAL_MODEL_NAME = "general_model";

struct GeneralBlob {
  std::vector<paddle::PaddleTensor> tensor_vector;
  int64_t time_stamp[20];
  int p_size = 0;

  void Clear() {
    size_t tensor_count = tensor_vector.size();
    for (size_t ti = 0; ti < tensor_count; ++ti) {
      tensor_vector[ti].shape.clear();
    }
    tensor_vector.clear();
  }
  
  int GetBatchSize() const {
    if (tensor_vector.size() > 0) {
      if (tensor_vector[0].lod.size() == 1) {
        return tensor_vector[0].lod[0].size() - 1;
      } else {
        return tensor_vector[0].shape[0];
      }
    } else {
      return -1;
    }
  }

  std::string ShortDebugString() const { return "Not implemented!"; }
};

static void AddBlobInfo(GeneralBlob * blob,
                        int64_t init_value) {
  blob->time_stamp[blob->p_size] = init_value;
  blob->p_size++;
}

static void CopyBlobInfo(const GeneralBlob * src,
                         GeneralBlob * tgt) {
  memcpy(&(tgt->time_stamp[0]), &(src->time_stamp[0]),
         src->p_size * sizeof(int64_t));
}

}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
