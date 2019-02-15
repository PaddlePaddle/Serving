#include "framework/factory.h"
#include "fluid_cpu_engine.h"

namespace baidu {
namespace paddle_serving {
namespace fluid_cpu {

REGIST_FACTORY_OBJECT_IMPL_WITH_NAME(
        ::baidu::paddle_serving::predictor::FluidInferEngine<FluidCpuAnalysisCore>,
        ::baidu::paddle_serving::predictor::InferEngine, "FLUID_CPU_ANALYSIS");

REGIST_FACTORY_OBJECT_IMPL_WITH_NAME(
        ::baidu::paddle_serving::predictor::FluidInferEngine<FluidCpuAnalysisDirCore>,
        ::baidu::paddle_serving::predictor::InferEngine, "FLUID_CPU_ANALYSIS_DIR");

REGIST_FACTORY_OBJECT_IMPL_WITH_NAME(
        ::baidu::paddle_serving::predictor::FluidInferEngine<FluidCpuAnalysisDirWithSigmoidCore>,
        ::baidu::paddle_serving::predictor::InferEngine, "FLUID_CPU_ANALYSIS_DIR_SIGMOID");

REGIST_FACTORY_OBJECT_IMPL_WITH_NAME(
        ::baidu::paddle_serving::predictor::FluidInferEngine<FluidCpuNativeCore>,
        ::baidu::paddle_serving::predictor::InferEngine, "FLUID_CPU_NATIVE");

REGIST_FACTORY_OBJECT_IMPL_WITH_NAME(
        ::baidu::paddle_serving::predictor::FluidInferEngine<FluidCpuNativeDirCore>,
        ::baidu::paddle_serving::predictor::InferEngine, "FLUID_CPU_NATIVE_DIR");

REGIST_FACTORY_OBJECT_IMPL_WITH_NAME(
        ::baidu::paddle_serving::predictor::FluidInferEngine<FluidCpuNativeDirWithSigmoidCore>,
        ::baidu::paddle_serving::predictor::InferEngine, "FLUID_CPU_NATIVE_DIR_SIGMOID");

} // namespace fluid_cpu
} // namespace paddle_serving
} // namespace baidu
