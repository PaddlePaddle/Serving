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

#include "paddle_inference/inferencer-fluid-cpu/include/fluid_cpu_engine.h"
#include "core/predictor/framework/factory.h"

namespace baidu {
namespace paddle_serving {
namespace fluid_cpu {

REGIST_FACTORY_OBJECT_IMPL_WITH_NAME(
    ::baidu::paddle_serving::predictor::FluidInferEngine<FluidCpuAnalysisCore>,
    ::baidu::paddle_serving::predictor::InferEngine,
    "FLUID_CPU_ANALYSIS");

REGIST_FACTORY_OBJECT_IMPL_WITH_NAME(
    ::baidu::paddle_serving::predictor::FluidInferEngine<
        FluidCpuAnalysisDirCore>,
    ::baidu::paddle_serving::predictor::InferEngine,
    "FLUID_CPU_ANALYSIS_DIR");

}  // namespace fluid_cpu
}  // namespace paddle_serving
}  // namespace baidu
