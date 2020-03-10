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
#include <exception>

#include "gflags/gflags.h"
#include "google/protobuf/message.h"

#ifdef BCLOUD
#include "baidu/rpc/channel.h"
#include "baidu/rpc/parallel_channel.h"
#include "baidu/rpc/traceprintf.h"
#include "base/comlog_sink.h"
#include "base/logging.h"
#include "base/object_pool.h"
#include "base/time.h"
#include "bthread.h"  // NOLINT
#else
#include "brpc/channel.h"
#include "brpc/parallel_channel.h"
#include "brpc/traceprintf.h"
#include "bthread/bthread.h"
#include "butil/logging.h"
#include "butil/object_pool.h"
#include "butil/time.h"
#endif

#include "bvar/bvar.h"

#ifdef BCLOUD
#include "json_to_pb.h"  // NOLINT
#else
#include "json2pb/json_to_pb.h"
#endif

#include "core/configure/general_model_config.pb.h"
#include "core/configure/include/configure_parser.h"
#include "core/configure/sdk_configure.pb.h"

#include "core/sdk-cpp/include/utils.h"

#ifdef BCLOUD
namespace brpc = baidu::rpc;
namespace butil = base;
#endif
