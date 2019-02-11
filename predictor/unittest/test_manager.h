#ifndef BAIDU_PADDLE_SERVING_PREDICTOR_TEST_MANAGER_H
#define BAIDU_PADDLE_SERVING_PREDICTOR_TEST_MANAGER_H

#include <gtest/gtest.h>

namespace baidu {
namespace paddle_serving {
namespace unittest {

#ifndef DEFINE_UP_DOWN  
#define DEFINE_UP_DOWN  \
    void SetUp() {}     \
    void TearDown() {}  \

class TestManager : public ::testing::Test {
public:
    TestManager() {}
    virtual ~TestManager() {}

    DEFINE_UP_DOWN
};

#undef DEFINE_UP_DOWN
#endif

}
}
}

#endif
