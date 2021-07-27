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
#include <gtest/gtest.h>
#include <string>
#include "core/predictor/framework/channel.h"
#include "core/predictor/op/op.h"

namespace baidu {
namespace paddle_serving {
namespace unittest {

struct AB {
  int a;
  float b;

  int Clear() {
    a = 3;
    b = 4.0;
    return 0;
  }

  std::string ShortDebugString() const {
    std::ostringstream oss;
    oss << "{\"a\": ";
    oss << a;
    oss << ", \"b\": ";
    oss << b;
    oss << "}";
    return oss.str();
  }
};

class ABOP : public baidu::paddle_serving::predictor::OpWithChannel<AB> {
 public:
  int inference() {
    AB* ab = mutable_data<AB>();
    ab->a = 1;
    ab->b = 2.2;
    return 0;
  }
  DECLARE_OP(ABOP);
};

DEFINE_OP(ABOP);

struct OpConf {
  std::string name_in_conf;
};

struct OpOutput {
  std::string name_for_output;
  void Clear() { name_for_output.clear(); }
  std::string ShortDebugString() const { return name_for_output; }
};

class OpWithConf
    : public baidu::paddle_serving::predictor::OpWithChannelAndConf<OpOutput,
                                                                    OpConf> {
 public:
  DECLARE_OP(OpWithConf);
  void* create_config(const comcfg::ConfigUnit& conf) {
    OpConf* op_conf = new (std::nothrow) OpConf();
    int err = 0;
    op_conf->name_in_conf = conf["name_in_conf"].to_cstr(&err);
    if (err != 0) {
      return NULL;
    }
    return op_conf;
  }

  void delete_config(void* conf) { delete static_cast<OpConf*>(conf); }

  int inference() {
    OpConf* op_conf = get_self_config();
    OpOutput* op_output = mutable_data<OpOutput>();
    op_output->name_for_output = op_conf->name_in_conf;
    return 0;
  }
};

DEFINE_OP(OpWithConf);

#ifndef DEFINE_UP_DOWN
#define DEFINE_UP_DOWN \
  void SetUp() {}      \
  void TearDown() {}

class TestOP : public ::testing::Test {
 public:
  TestOP() {}
  virtual ~TestOP() {}

  DEFINE_UP_DOWN
};

#undef DEFINE_UP_DOWN
#endif
}  // namespace unittest
}  // namespace paddle_serving
}  // namespace baidu
