#include "test_server_manager.h" // TestServerManager
#include <gflags/gflags.h> // FLAGS
#include "framework/server.h" // ServerManager

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

} // namespace unittest
} // namespace paddle_serving
} // namespace baidu
