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
#include "core/predictor/framework/channel.h"
#include "core/predictor/msg_data.pb.h"
#include "core/predictor/op/op.h"

namespace baidu {
namespace paddle_serving {
namespace unittest {

class MsgOP : public baidu::paddle_serving::predictor::OpWithChannel<
                  pds::ut::OpMessageData> {
 public:
  int inference() {
    pds::ut::OpMessageData* msg = mutable_data<pds::ut::OpMessageData>();
    msg->set_a(11);
    msg->set_b(22.2);
    return 0;
  }
};

#ifndef DEFINE_UP_DOWN
#define DEFINE_UP_DOWN \
  void SetUp() {}      \
  void TearDown() {}

class TestMSGOP : public ::testing::Test {
 public:
  TestMSGOP() {}
  virtual ~TestMSGOP() {}

  DEFINE_UP_DOWN
};

#undef DEFINE_UP_DOWN
#endif
}  // namespace unittest
}  // namespace paddle_serving
}  // namespace baidu
