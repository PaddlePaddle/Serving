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

#include "core/predictor/common/constant.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

DEFINE_bool(use_parallel_infer_service, false, "");
DEFINE_int32(el_log_level, 16, "");
DEFINE_int32(idle_timeout_s, 80, "");
DEFINE_int32(port, 8010, "");
DEFINE_string(workflow_path, "./conf", "");
DEFINE_string(workflow_file, "workflow.prototxt", "");
DEFINE_string(inferservice_path, "./conf", "");
DEFINE_string(inferservice_file, "infer_service.prototxt", "");
DEFINE_string(logger_path, "./conf", "");
DEFINE_string(logger_file, "log.conf", "");
DEFINE_string(resource_path, "./conf", "");
DEFINE_string(resource_file, "resource.prototxt", "");
DEFINE_int32(max_concurrency,
             0,
             "Limit of request processing in parallel, 0: unlimited");
DEFINE_int32(
    num_threads,
    0,
    "Number of pthreads that server runs on, not change if this value <= 0");
DEFINE_int32(reload_interval_s, 10, "");
DEFINE_bool(enable_model_toolkit, true, "enable model toolkit");
DEFINE_string(enable_protocol_list, "baidu_std", "set protocol list");
DEFINE_bool(enable_cube, false, "enable cube");
DEFINE_string(general_model_path, "./conf", "");
DEFINE_string(general_model_file, "general_model.prototxt", "");
DEFINE_bool(enable_general_model, true, "enable general model");
DEFINE_bool(enable_prometheus, false, "enable prometheus");
DEFINE_int32(prometheus_port, 19393, "");
DEFINE_int64(request_cache_size, 0, "request cache size");

const char* START_OP_NAME = "startup_op";
}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu

// Baidurpc
BAIDU_REGISTER_ERRNO(baidu::paddle_serving::predictor::ERR_INTERNAL_FAILURE,
                     "Paddle Serving Framework Internal Error.");
BAIDU_REGISTER_ERRNO(baidu::paddle_serving::predictor::ERR_MEM_ALLOC_FAILURE,
                     "Paddle Serving Memory Alloc Error.");
BAIDU_REGISTER_ERRNO(baidu::paddle_serving::predictor::ERR_OVERFLOW_FAILURE,
                     "Paddle Serving Array Overflow Error.");
BAIDU_REGISTER_ERRNO(baidu::paddle_serving::predictor::ERR_OP_INFER_FAILURE,
                     "Paddle Serving Op Inference Error.");
