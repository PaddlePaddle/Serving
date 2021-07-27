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

#include <brpc/channel.h>
#include "gtest/gtest.h"

#include "core/cube/cube-server/include/cube/control.h"

namespace rec {
namespace mcube {
namespace unittest {
struct DoNothing : public google::protobuf::Closure {
  void Run() {}
};

class ControlTest : public ::testing::Test {
 protected:
  ControlTest() {}
  virtual ~ControlTest() {}
  virtual void SetUp() {}
  virtual void TearDown() {}
};  // class ControlTest

TEST_F(ControlTest, control_cmd) {
  brpc::Controller cntl;
  DoNothing do_nothing;
  Control control;

  control.cmd(&cntl, NULL, NULL, &do_nothing);
  ASSERT_EQ(brpc::HTTP_STATUS_BAD_REQUEST, cntl.http_response().status_code());
  cntl.Reset();
}

int run(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

}  // namespace unittest
}  // namespace mcube
}  // namespace rec

int main(int argc, char** argv) {
  return ::rec::mcube::unittest::run(argc, argv);
}
