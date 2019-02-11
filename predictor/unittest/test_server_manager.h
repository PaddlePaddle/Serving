#ifndef BAIDU_PADDLE_SERVING_PREDICTOR_TEST_SERVER_MANAGER_H
#define BAIDU_PADDLE_SERVING_PREDICTOR_TEST_SERVER_MANAGER_H

#include <gtest/gtest.h>

namespace baidu {
namespace paddle_serving {
namespace unittest {

class TestServerManager : public ::testing::Test {
public:
    void SetUp() { }
    void TearDown() { }
};

} // namespace unittest
} // namespace paddle_serving
} // namespace baidu

#endif // BAIDU_PADDLE_SERVING_PREDICTOR_TEST_SERVER_MANAGER_H
