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
#include <google/protobuf/message.h>
#include <string>
#include <vector>
#include "core/sdk-cpp/include/common.h"
#include "core/sdk-cpp/include/factory.h"
#include "core/sdk-cpp/include/stub.h"

namespace baidu {
namespace paddle_serving {
namespace sdk_cpp {

class Stub;
class Variant;

static const char* WEIGHT_SEPERATOR = "|";

class EndpointRouterBase {
 public:
  typedef std::vector<Variant*> VariantList;

  virtual ~EndpointRouterBase() {}

  virtual int initialize(const google::protobuf::Message& conf) = 0;

  virtual Variant* route(const VariantList&) = 0;

  virtual Variant* route(const VariantList&, const void*) = 0;
};

class WeightedRandomRender : public EndpointRouterBase {
 public:
  static int register_self() {
    // INLINE_REGIST_OBJECT(WeightedRandomRender, EndpointRouterBase, -1);

    Factory<WeightedRandomRender, EndpointRouterBase>* factory =
        new (std::nothrow) Factory<WeightedRandomRender, EndpointRouterBase>();
    if (factory == NULL) {
      RAW_LOG(ERROR,
              "Failed regist factory: WeightedRandomRender->EndpointRouterBase "
              "in macro!");
      return -1;
    }

    // When two clients are created in the same process, two
    // "WeightedRandomRender" factory objects are registered.
    // But in fact, the two clients can use one factory object
    // together.
    if (FactoryPool<EndpointRouterBase>::instance().register_factory(
            "WeightedRandomRender", factory) != 0) {
      RAW_LOG(INFO,
              "Factory has been registed: "
              "WeightedRandomRender->EndpointRouterBase.");
    }

    return 0;
  }

  WeightedRandomRender() : _normalized_sum(0) {}

  ~WeightedRandomRender() {}

  int initialize(const google::protobuf::Message& conf);

  Variant* route(const VariantList&);

  Variant* route(const VariantList&, const void*);

 private:
  std::vector<uint32_t> _variant_weight_list;
  uint32_t _normalized_sum;
};

}  // namespace sdk_cpp
}  // namespace paddle_serving
}  // namespace baidu
