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

#include "core/predictor/unittest/test_server_manager.h"  // TestServerManager
#include <gflags/gflags.h>                                // FLAGS
#include <string>
#include "core/predictor/framework/server.h"  // ServerManager

namespace baidu {
namespace paddle_serving {
namespace unittest {

using baidu::paddle_serving::predictor::ServerManager;
using baidu::paddle_serving::predictor::FLAGS_enable_nshead_protocol;
using baidu::paddle_serving::predictor::FLAGS_nshead_protocol;

TEST_F(TestServerManager, test_nshead_protocol) {
  ASSERT_EQ(FLAGS_enable_nshead_protocol, false);
  ServerManager server_manager1;
  EXPECT_EQ(server_manager1._options.nshead_service, NULL);

  google::SetCommandLineOption("enable_nshead_protocol", "true");
  ASSERT_EQ(FLAGS_enable_nshead_protocol, true);
  ASSERT_STREQ(FLAGS_nshead_protocol.c_str(), "itp");
  ServerManager server_manager2;
  EXPECT_NE(server_manager2._options.nshead_service, NULL);

  std::string protocol = "nova_pbrpc";
  google::SetCommandLineOption("enable_nshead_protocol", "true");
  google::SetCommandLineOption("nshead_protocol", protocol.c_str());
  ASSERT_EQ(FLAGS_enable_nshead_protocol, true);
  ASSERT_STREQ(FLAGS_nshead_protocol.c_str(), protocol.c_str());
  ServerManager server_manager3;
  EXPECT_NE(server_manager3._options.nshead_service, NULL);

  protocol = "public_pbrpc";
  google::SetCommandLineOption("enable_nshead_protocol", "true");
  google::SetCommandLineOption("nshead_protocol", protocol.c_str());
  ASSERT_EQ(FLAGS_enable_nshead_protocol, true);
  ASSERT_STREQ(FLAGS_nshead_protocol.c_str(), protocol.c_str());
  ServerManager server_manager4;
  EXPECT_NE(server_manager4._options.nshead_service, NULL);

  protocol = "nshead_mcpack";
  google::SetCommandLineOption("enable_nshead_protocol", "true");
  google::SetCommandLineOption("nshead_protocol", protocol.c_str());
  ASSERT_EQ(FLAGS_enable_nshead_protocol, true);
  ASSERT_STREQ(FLAGS_nshead_protocol.c_str(), protocol.c_str());
  ServerManager server_manager5;
  EXPECT_NE(server_manager5._options.nshead_service, NULL);

  protocol = "nshead_wrong_protocol";
  google::SetCommandLineOption("enable_nshead_protocol", "true");
  google::SetCommandLineOption("nshead_protocol", protocol.c_str());
  ASSERT_EQ(FLAGS_enable_nshead_protocol, true);
  ASSERT_STREQ(FLAGS_nshead_protocol.c_str(), protocol.c_str());
  ServerManager server_manager6;
  EXPECT_EQ(server_manager6._options.nshead_service, NULL);
}

}  // namespace unittest
}  // namespace paddle_serving
}  // namespace baidu
