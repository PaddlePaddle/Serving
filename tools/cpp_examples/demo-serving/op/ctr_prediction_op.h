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
#include "examples/demo-serving/ctr_prediction.pb.h"
#include "paddle_inference_api.h"  // NOLINT

namespace baidu {
namespace paddle_serving {
namespace serving {

static const char* CTR_PREDICTION_MODEL_NAME = "ctr_prediction";

/**
 * CTRPredictionOp: Serve CTR prediction requests.
 *
 * Original model can be found here:
 * https://github.com/PaddlePaddle/models/tree/develop/PaddleRec/ctr
 *
 * NOTE:
 *
 * The main purpose of this OP is to demonstrate usage of large-scale sparse
 * parameter service (RocksDB for local, mCube for distributed service). To
 * achieve this, we modified the orginal model slightly:
 * 1) Function ctr_dnn_model() returns feed_vars and fetch_vars
 * 2) Use fluid.io.save_inference_model using feed_vars and fetch_vars
 * returned from ctr_dnn_model(), instead of fluid.io.save_persistables
 * 3) Further, feed_vars were specified to be inputs of concat layer. Then in
 * the process of save_inference_model(), the generated inference program will
 * have the inputs of concat layer as feed targets.
 * 4) Weight values for the embedding layer will be fetched from sparse param
 * server for each sample
 *
 * Please refer to doc/CTR_PREDICTION.md for details on the original model
 * and modifications we made
 *
 */

class CTRPredictionOp
    : public baidu::paddle_serving::predictor::OpWithChannel<
          baidu::paddle_serving::predictor::ctr_prediction::Response> {
 public:
  typedef std::vector<paddle::PaddleTensor> TensorVector;

  DECLARE_OP(CTRPredictionOp);

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
