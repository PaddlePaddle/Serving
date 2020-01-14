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

#include <butil/files/temp_file.h>
#include <string>
#include <vector>
#include "core/predictor/framework/dag.h"
#include "core/predictor/framework/dag_view.h"
#include "core/predictor/framework/manager.h"
#include "core/predictor/framework/service.h"

#include "core/predictor/unittest/test_message_op.h"
#include "core/predictor/unittest/test_op.h"
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
using baidu::paddle_serving::predictor::Dag;
using baidu::paddle_serving::predictor::DagView;
using baidu::paddle_serving::predictor::ViewStage;
using baidu::paddle_serving::predictor::ViewNode;
using pds::ut::OpMessageData;

TEST_F(TestOP, test_init) {
  Bus* bus = new Bus();
  ASSERT_NE(bus, NULL);
  Dag* dag = NULL;

  ABOP op;
  std::string op_name = "TestOp";
  std::string op_type = "TestOp";
  EXPECT_EQ(0, op.init(bus, dag, (uint32_t)999, op_name, op_type, NULL));
  EXPECT_FALSE(op.has_calc());
  EXPECT_EQ(999, op.id());
  EXPECT_STREQ("TestOp", op.name());
  EXPECT_STREQ("{\"a\": 3, \"b\": 4}", op.debug_string().c_str());
  EXPECT_NE(op._timer, NULL);
  EXPECT_EQ(bus, op._bus);

  AB* ab = op.mutable_data<AB>();
  EXPECT_EQ(3, ab->a);
  EXPECT_FLOAT_EQ(4.0, ab->b);

  Channel* chn = op.mutable_channel();
  EXPECT_EQ(chn->id(), 999);
  EXPECT_STREQ(chn->op().c_str(), "TestOp");

  EXPECT_EQ(ab, chn->param());
  EXPECT_EQ(NULL, chn->message());

  EXPECT_EQ(0, bus->size());
  Channel* chn2 = bus->channel_by_name("TestOp");
  EXPECT_EQ(NULL, chn2);
}

TEST_F(TestOP, test_depend_argment) {
  Bus bus;
  Dag dag;

  AutoTempFile file(
      "[@Node]\n"
      "name: node1\n"
      "type: ABOP\n"
      "[@Node]\n"
      "name: node2\n"
      "type: ABOP\n"
      "[.@Depend]\n"
      "name: node1\n"
      "mode: RO\n"
      "[@Node]\n"
      "name: node3\n"
      "type: ABOP\n"
      "[.@Depend]\n"
      "name: node1\n"
      "mode: RO\n"
      "[@Node]\n"
      "name: node4\n"
      "type: ABOP\n"
      "[.@Depend]\n"
      "name: node2\n"
      "mode: RW\n"
      "[.@Depend]\n"
      "name: node3\n"
      "mode: RO");

  std::string dag_name = "DagTest";
  EXPECT_EQ(0, dag.init("./", file.name(), dag_name));

  ABOP op;
  std::string op_name = "node4";
  std::string op_type = "ABOP";
  EXPECT_EQ(0, op.init(&bus, &dag, (uint32_t)888, op_name, op_type, NULL));

  EXPECT_FALSE(op.is_readable("node1"));
  EXPECT_FALSE(op.is_mutable("node1"));
  EXPECT_TRUE(op.is_readable("node2"));
  EXPECT_TRUE(op.is_mutable("node2"));
  EXPECT_TRUE(op.is_readable("node3"));
  EXPECT_FALSE(op.is_mutable("node3"));

  // process() is not called, channel has not been
  // committed to bus yet!
  EXPECT_TRUE(NULL == op.get_depend_channel("node1"));
  EXPECT_TRUE(NULL == op.get_depend_channel("node2"));
  EXPECT_TRUE(NULL == op.get_depend_channel("node3"));
  EXPECT_TRUE(NULL == op.mutable_depend_channel("node1"));
  EXPECT_TRUE(NULL == op.mutable_depend_channel("node2"));
  EXPECT_TRUE(NULL == op.mutable_depend_channel("node3"));
}

TEST_F(TestOP, test_inference) {
  Bus bus;
  Dag dag;

  AutoTempFile file(
      "[@Node]\n"
      "name: node1\n"
      "type: ABOP\n"
      "[@Node]\n"
      "name: node2\n"
      "type: ABOP\n"
      "[.@Depend]\n"
      "name: node1\n"
      "mode: RO\n"
      "[@Node]\n"
      "name: node3\n"
      "type: ABOP\n"
      "[.@Depend]\n"
      "name: node1\n"
      "mode: RO\n"
      "[@Node]\n"
      "name: node4\n"
      "type: ABOP\n"
      "[.@Depend]\n"
      "name: node2\n"
      "mode: RW\n"
      "[.@Depend]\n"
      "name: node3\n"
      "mode: RO");

  std::string dag_name = "DagTest";
  EXPECT_EQ(0, dag.init("./", file.name(), dag_name));

  ABOP op1;
  std::string op1_name = "node1";
  std::string op_type = "ABOP";
  EXPECT_EQ(0, op1.init(&bus, &dag, (uint32_t)888, op1_name, op_type, NULL));

  ABOP op2;
  std::string op2_name = "node2";
  EXPECT_EQ(0, op2.init(&bus, &dag, (uint32_t)888, op2_name, op_type, NULL));

  MsgOP op3;
  std::string op3_name = "node3";
  EXPECT_EQ(0, op3.init(&bus, &dag, (uint32_t)888, op3_name, op_type, NULL));

  ABOP op4;
  std::string op4_name = "node4";
  EXPECT_EQ(0, op4.init(&bus, &dag, (uint32_t)888, op4_name, op_type, NULL));

  EXPECT_TRUE(op2.is_readable("node1"));
  EXPECT_FALSE(op2.is_mutable("node1"));

  EXPECT_FALSE(op2.is_readable("node3"));
  EXPECT_FALSE(op2.is_mutable("node3"));

  EXPECT_FALSE(op2.is_readable("node4"));
  EXPECT_FALSE(op2.is_mutable("node4"));

  EXPECT_TRUE(op3.is_readable("node1"));
  EXPECT_FALSE(op3.is_mutable("node1"));

  EXPECT_FALSE(op3.is_readable("node2"));
  EXPECT_FALSE(op3.is_mutable("node2"));

  EXPECT_FALSE(op3.is_readable("node4"));
  EXPECT_FALSE(op3.is_mutable("node4"));

  EXPECT_FALSE(op4.is_readable("node1"));
  EXPECT_FALSE(op4.is_mutable("node1"));

  EXPECT_TRUE(op4.is_readable("node2"));
  EXPECT_TRUE(op4.is_mutable("node2"));

  EXPECT_TRUE(op4.is_readable("node3"));
  EXPECT_FALSE(op4.is_mutable("node3"));

  EXPECT_EQ(0, op1.process(false));
  EXPECT_EQ(0, op2.process(false));
  EXPECT_EQ(0, op3.process(false));
  EXPECT_EQ(0, op4.process(true));

  EXPECT_TRUE(NULL == op4.get_depend_channel("node1"));
  EXPECT_FALSE(NULL == op4.get_depend_channel("node2"));
  EXPECT_FALSE(NULL == op4.get_depend_channel("node3"));
  EXPECT_TRUE(NULL == op4.mutable_depend_channel("node1"));
  EXPECT_FALSE(NULL == op4.mutable_depend_channel("node2"));
  EXPECT_TRUE(NULL == op4.mutable_depend_channel("node3"));

  const AB* dop1 = op4.get_depend_argument<AB>("node1");

  const AB* dop21 = op4.get_depend_argument<AB>("node2");
  const google::protobuf::Message* dop22 =
      op4.get_depend_channel("node2")->message();
  const google::protobuf::Message* dop23 =
      op4.get_depend_argument<google::protobuf::Message>("node2");

  const OpMessageData* dop31 = op4.get_depend_argument<OpMessageData>("node3");
  const google::protobuf::Message* dop32 =
      op4.get_depend_channel("node3")->message();
  const google::protobuf::Message* dop33 =
      op4.get_depend_argument<google::protobuf::Message>("node3");

  EXPECT_EQ(NULL, dop1);

  EXPECT_NE(NULL, dop21);
  EXPECT_EQ(NULL, dop22);
  EXPECT_EQ(NULL, dop23);

  EXPECT_NE(NULL, dop31);
  EXPECT_NE(NULL, dop32);
  EXPECT_EQ(NULL, dop33);
  EXPECT_EQ(dop31, dop32);

  const OpMessageData* dop322 = dynamic_cast<const OpMessageData*>(dop32);

  EXPECT_EQ(1, dop21->a);
  EXPECT_FLOAT_EQ(2.2, dop21->b);

  EXPECT_EQ(11, dop31->a());
  EXPECT_FLOAT_EQ(22.2, dop31->b());

  EXPECT_EQ(11, dop322->a());
  EXPECT_FLOAT_EQ(22.2, dop322->b());
}

TEST_F(TestOP, test_op_with_channel_and_conf) {
  Dag dag;
  std::string op_name = "test_op";
  std::string name_in_conf = "test_name_in_conf";
  butil::TempFile dag_conf;
  dag_conf.save_format(
      "[@Node]\n"
      "name: %s\n"
      "type: OpWithConf\n"
      "name_in_conf: %s\n",
      op_name.c_str(),
      name_in_conf.c_str());

  std::string dag_name = "DagTest";
  ASSERT_EQ(0, dag.init("./", dag_conf.fname(), dag_name));

  DagView view;
  view.init(&dag, "service_name");
  ASSERT_EQ(0, view.execute(NULL));

  const std::vector<ViewStage*>& view_stage_vec = view._view;
  uint32_t stage_size = view_stage_vec.size();
  for (uint32_t si = 0; si < stage_size; si++) {
    ViewStage* vstage = view_stage_vec[si];
    uint32_t node_size = vstage->nodes.size();
    for (uint32_t ni = 0; ni < node_size; ni++) {
      ViewNode* vnode = vstage->nodes[ni];
      OpWithConf* op = dynamic_cast<OpWithConf*>(vnode->op);
      ASSERT_NE(NULL, op);
      EXPECT_STREQ(op->name(), op_name.c_str());
      EXPECT_STREQ(op->get_self_config()->name_in_conf.c_str(),
                   name_in_conf.c_str());
      EXPECT_STREQ(op->mutable_data<OpOutput>()->name_for_output.c_str(),
                   name_in_conf.c_str());
    }
  }
}
}  // namespace unittest
}  // namespace paddle_serving
}  // namespace baidu
