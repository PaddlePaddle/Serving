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

#include <error.h>
#include <getopt.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <unistd.h>

#include <typeinfo>

#include "boost/algorithm/string.hpp"  // for boost::split&trim
#include "boost/function.hpp"
#include "boost/unordered_map.hpp"
#include "google/protobuf/text_format.h"

#include "gflags/gflags.h"

#ifdef BCLOUD
#include "baidu/rpc/channel.h"
#include "baidu/rpc/policy/giano_authenticator.h"
#include "baidu/rpc/server.h"
#include "base/comlog_sink.h"
#include "base/logging.h"
#include "base/object_pool.h"
#include "base/time.h"
#include "bthread.h"  // NOLINT
#else
#include "brpc/channel.h"
#include "brpc/policy/giano_authenticator.h"
#include "brpc/server.h"
#include "bthread/bthread.h"
#include "butil/logging.h"
#include "butil/object_pool.h"
#include "butil/time.h"
#endif

#define ERROR_STRING_LEN 10240

#include "core/configure/general_model_config.pb.h"
#include "core/configure/include/configure_parser.h"
#include "core/configure/server_configure.pb.h"

#include "core/predictor/common/constant.h"
#include "core/predictor/common/types.h"
#include "core/predictor/common/utils.h"

#include "core/predictor/framework/prometheus_metric.h"
#include "core/predictor/framework/request_cache.h"

#ifdef BCLOUD
namespace brpc = baidu::rpc;
namespace butil = base;
#endif
