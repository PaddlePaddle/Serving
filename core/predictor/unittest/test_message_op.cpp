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

#include "core/predictor/unittest/test_message_op.h"
#include <string>
#include "core/predictor/framework/dag.h"
#include "core/predictor/framework/manager.h"
#include "core/predictor/framework/service.h"
#include "core/predictor/unittest/test_tool.h"

namespace baidu {
namespace paddle_serving {
namespace unittest {

using baidu::paddle_serving::predictor::Manager;
using baidu::paddle_serving::predictor::InferService;
using baidu::paddle_serving::predictor::ParallelInferService;
using baidu::paddle_serving::predictor::FLAGS_use_parallel_infer_service;
using baidu::paddle_serving::predictor::InferServiceManager;
using baidu::paddle_serving::predictor::Bus;
using baidu::paddle_serving::predictor::Dag;
using baidu::paddle_serving::predictor::Channel;
using pds::ut::OpMessageData;

TEST_F(TestMSGOP, test_init) {
  Bus* bus = new Bus();
  ASSERT_NE(bus, NULL);
  Dag* dag = NULL;

  MsgOP op;
  std::string op_name = "TestMSGOp";
  std::string op_type = "TestMSGOp";
  EXPECT_EQ(0, op.init(bus, dag, (uint32_t)9999, op_name, op_type, NULL));
  EXPECT_FALSE(op.has_calc());
  EXPECT_EQ(9999, op.id());
  EXPECT_STREQ("TestMSGOp", op.name());
  EXPECT_STREQ("", op.debug_string().c_str());
  EXPECT_NE(op._timer, NULL);
  EXPECT_EQ(bus, op._bus);

  OpMessageData* ab = op.mutable_data<OpMessageData>();
  EXPECT_EQ(33, ab->a());
  EXPECT_FLOAT_EQ(4.4, ab->b());

  Channel* chn = op.mutable_channel();
  EXPECT_EQ(chn->id(), 9999);
  EXPECT_STREQ(chn->op().c_str(), "TestMSGOp");

  EXPECT_EQ(ab, chn->param());

  EXPECT_EQ(0, bus->size());
  Channel* chn2 = bus->channel_by_name("TestOp");
  EXPECT_EQ(NULL, chn2);

  // Message OP can obtain data via message()
  EXPECT_EQ(ab, chn->message());
}
}  // namespace unittest
}  // namespace paddle_serving
}  // namespace baidu
