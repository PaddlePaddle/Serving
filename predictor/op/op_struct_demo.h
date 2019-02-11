#ifndef BAIDU_PADDLE_SEVING_PREDICTOR_OP_STRUCT_DEMO_H
#define BAIDU_PADDLE_SEVING_PREDICTOR_OP_STRUCT_DEMO_H

#include "common/inner_common.h"
#include "op/op.h"
#include "framework/channel.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

struct DemoData {
    boost::unordered_map<std::string, int> name_id;
    int data;
};

class StructOp : public OpWithChannel<DemoData> {
public:

    DECLARE_OP(StructOp);

    int inference() {
        DemoData* data = mutable_data<DemoData>();
        data.data = 1;

        return 0;
    }
};

DEFINE_OP(StructOp);

} // predictor
} // paddle_serving
} // baidu

#endif
