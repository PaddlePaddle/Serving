#ifndef BAIDU_PADDLE_SERVING_PREDICTOR_TEST_MESSAGE_OP_H
#define BAIDU_PADDLE_SERVING_PREDICTOR_TEST_MESSAGE_OP_H

#include <gtest/gtest.h>
#include "op/op.h"
#include "msg_data.pb.h"
#include "framework/channel.h"

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
#define DEFINE_UP_DOWN  \
    void SetUp() {}     \
    void TearDown() {}  \

class TestMSGOP : public ::testing::Test {
public:
    TestMSGOP() {}
    virtual ~TestMSGOP() {}

    DEFINE_UP_DOWN
};

#undef DEFINE_UP_DOWN
#endif

}
}
}

#endif
