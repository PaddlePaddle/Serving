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

#include "paddle_inference/inferencer-fluid-gpu/include/fluid_gpu_engine.h"
#include "core/predictor/framework/factory.h"

DEFINE_int32(gpuid, 0, "GPU device id to use");

namespace baidu {
namespace paddle_serving {
namespace fluid_gpu {

REGIST_FACTORY_OBJECT_IMPL_WITH_NAME(
    ::baidu::paddle_serving::predictor::FluidInferEngine<FluidGpuAnalysisCore>,
    ::baidu::paddle_serving::predictor::InferEngine,
    "FLUID_GPU_ANALYSIS");

REGIST_FACTORY_OBJECT_IMPL_WITH_NAME(
    ::baidu::paddle_serving::predictor::FluidInferEngine<
        FluidGpuAnalysisDirCore>,
    ::baidu::paddle_serving::predictor::InferEngine,
    "FLUID_GPU_ANALYSIS_DIR");

REGIST_FACTORY_OBJECT_IMPL_WITH_NAME(
    ::baidu::paddle_serving::predictor::FluidInferEngine<
        FluidGpuAnalysisDirWithSigmoidCore>,
    ::baidu::paddle_serving::predictor::InferEngine,
    "FLUID_GPU_ANALYSIS_DIR_SIGMOID");

REGIST_FACTORY_OBJECT_IMPL_WITH_NAME(
    ::baidu::paddle_serving::predictor::FluidInferEngine<FluidGpuNativeCore>,
    ::baidu::paddle_serving::predictor::InferEngine,
    "FLUID_GPU_NATIVE");

REGIST_FACTORY_OBJECT_IMPL_WITH_NAME(
    ::baidu::paddle_serving::predictor::FluidInferEngine<FluidGpuNativeDirCore>,
    ::baidu::paddle_serving::predictor::InferEngine,
    "FLUID_GPU_NATIVE_DIR");

REGIST_FACTORY_OBJECT_IMPL_WITH_NAME(
    ::baidu::paddle_serving::predictor::FluidInferEngine<
        FluidGpuNativeDirWithSigmoidCore>,
    ::baidu::paddle_serving::predictor::InferEngine,
    "FLUID_GPU_NATIVE_DIR_SIGMOID");

}  // namespace fluid_gpu
}  // namespace paddle_serving
}  // namespace baidu
