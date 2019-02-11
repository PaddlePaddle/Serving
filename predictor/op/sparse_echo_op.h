#ifndef BAIDU_PADDLE_SERVING_PREDICTOR_OP_SPARSE_ECHO_OP_H
#define BAIDU_PADDLE_SERVING_PREDICTOR_OP_SPARSE_ECHO_OP_H

#include "sparse_service.pb.h"

#include "common/inner_common.h"
#include "op/op.h"
#include "framework/channel.h"
#include "framework/op_repository.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

class SparseEchoOp : public OpWithChannel<
    baidu::paddle_serving::predictor::sparse_service::Response> {
public:

    DECLARE_OP(SparseEchoOp);

    typedef baidu::paddle_serving::predictor::sparse_service::Request Request;
    typedef baidu::paddle_serving::predictor::sparse_service::Response Response;
    typedef baidu::paddle_serving::predictor::format::SparsePrediction
        SparsePrediction;

    int inference();
};

} // predictor
} // paddle_serving
} // baidu

#endif
