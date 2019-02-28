#ifndef BAIDU_PADDLE_SSERVER_PREDICTOR_OP_DENSE_ECHO_OP_H
#define BAIDU_PADDLE_SSERVER_PREDICTOR_OP_DENSE_ECHO_OP_H

#include "int64tensor_service.pb.h"

#include "common/inner_common.h"
#include "op/op.h"
#include "framework/channel.h"
#include "framework/op_repository.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

class Int64TensorEchoOp : public OpWithChannel<
    baidu::paddle_serving::predictor::int64tensor_service::Response> {
public: 

    DECLARE_OP(Int64TensorEchoOp);

    int inference();
};

} // predictor
} // paddle_serving
} // baidu

#endif
