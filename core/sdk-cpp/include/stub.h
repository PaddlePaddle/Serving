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
#include <string>
#include "core/sdk-cpp/include/common.h"

namespace baidu {
namespace paddle_serving {
namespace sdk_cpp {

class Predictor;
struct VariantInfo;

class Stub {
 public:
  typedef google::protobuf::Message Message;

  virtual ~Stub() {}

  virtual int initialize(const VariantInfo& var,
                         const std::string& ep,
                         const std::string* tag,
                         const std::string* tag_value) = 0;

  // predictor
  virtual Predictor* fetch_predictor() = 0;
  virtual int return_predictor(Predictor* predictor) = 0;
  virtual int return_predictor(Predictor* predictor) const = 0;

  // request
  virtual Message* fetch_request() = 0;
  virtual int return_request(Message* request) = 0;
  virtual int return_request(Message* request) const = 0;

  // response
  virtual Message* fetch_response() = 0;
  virtual int return_response(Message* response) = 0;
  virtual int return_response(Message* response) const = 0;

  virtual const std::string& which_endpoint() const = 0;

  // control logic for tls
  virtual int thrd_initialize() = 0;

  virtual int thrd_clear() = 0;

  virtual int thrd_finalize() = 0;

  virtual void update_average(int64_t acc, const char* name) = 0;
  virtual void update_latency(int64_t acc, const char* name) = 0;
};

}  // namespace sdk_cpp
}  // namespace paddle_serving
}  // namespace baidu
