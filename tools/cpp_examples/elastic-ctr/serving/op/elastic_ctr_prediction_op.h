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

#pragma once
#include <vector>
#include "elastic-ctr/serving/elastic_ctr_prediction.pb.h"
#include "paddle_inference_api.h"  // NOLINT

namespace baidu {
namespace paddle_serving {
namespace serving {

static const char* CTR_PREDICTION_MODEL_NAME = "elastic_ctr_prediction";

/**
 * ElasticCTRPredictionOp: Serve CTR prediction requests.
 *
 */

class ElasticCTRPredictionOp
    : public baidu::paddle_serving::predictor::OpWithChannel<
          baidu::paddle_serving::predictor::elastic_ctr::Response> {
 public:
  typedef std::vector<paddle::PaddleTensor> TensorVector;
  typedef std::map<int, std::vector<int64_t>> Sample;  // NOLINT
  typedef std::vector<Sample> Samples;

  DECLARE_OP(ElasticCTRPredictionOp);

  int inference();

 private:
  static bthread::Mutex mutex_;
  static int64_t cube_time_us_;
  static int32_t cube_req_num_;
  static int32_t cube_req_key_num_;
};

}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
