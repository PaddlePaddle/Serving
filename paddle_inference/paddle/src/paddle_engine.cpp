// Copyright (c) 2021 PaddlePaddle Authors. All Rights Reserved.
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

#include "paddle_inference/paddle/include/paddle_engine.h"
#include "core/predictor/framework/factory.h"

namespace baidu {
namespace paddle_serving {
namespace inference {

DEFINE_int32(gpuid, 0, "GPU device id to use");
DEFINE_string(precision, "fp32", "precision to deploy, default is fp32");
DEFINE_bool(use_calib, false, "calibration mode, default is false");
DEFINE_string(nnadapter_device_names, "", "Names of nnadapter device");
DEFINE_string(nnadapter_context_properties,
              "",
              "Properties of nnadapter context");
DEFINE_string(nnadapter_model_cache_dir, "", "Cache dir of nnadapter model");

REGIST_FACTORY_OBJECT_IMPL_WITH_NAME(
    ::baidu::paddle_serving::predictor::FluidInferEngine<PaddleInferenceEngine>,
    ::baidu::paddle_serving::predictor::InferEngine,
    "PADDLE_INFER");

}  // namespace inference
}  // namespace paddle_serving
}  // namespace baidu
