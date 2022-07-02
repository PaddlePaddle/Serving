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
#include "core/predictor/common/inner_common.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

// GFLAGS Variables
DECLARE_bool(use_parallel_infer_service);
DECLARE_int32(el_log_level);
DECLARE_int32(idle_timeout_s);
DECLARE_int32(port);
DECLARE_string(workflow_path);
DECLARE_string(workflow_file);
DECLARE_string(inferservice_path);
DECLARE_string(inferservice_file);
DECLARE_string(logger_path);
DECLARE_string(logger_file);
DECLARE_string(resource_path);
DECLARE_string(resource_file);
DECLARE_string(general_model_path);
DECLARE_string(general_model_file);
DECLARE_bool(enable_mc_cache);
DECLARE_bool(enable_nshead_protocol);
DECLARE_string(nshead_protocol);
DECLARE_int32(max_concurrency);
DECLARE_int32(num_threads);
DECLARE_int32(reload_interval_s);
DECLARE_bool(enable_model_toolkit);
DECLARE_string(enable_protocol_list);
DECLARE_bool(enable_cube);
DECLARE_bool(enable_general_model);
DECLARE_bool(enable_prometheus);
DECLARE_int32(prometheus_port);
DECLARE_int64(request_cache_size);

// STATIC Variables
extern const char* START_OP_NAME;

// ERRORCODE
enum {
  // internal error
  ERR_INTERNAL_FAILURE = -5000,
  ERR_MEM_ALLOC_FAILURE = -5001,
  ERR_OVERFLOW_FAILURE = -5002,
  // op error
  ERR_OP_INFER_FAILURE = -5100,
  // no error
  ERR_OK = 0,
  // internal ignore
  ERR_IGNORE_FAILURE = 5000,
  // op ignore
  ERR_OP_IGNORE_FAILURE = 5100,
};

static const size_t MAX_WORKFLOW_NUM_IN_ONE_SERVICE = 20;
static const uint32_t DEFAULT_CACHE_CAPACITY = 10000;
static const uint32_t DEFAULT_CACHE_UNITSIZE = 8192;

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
