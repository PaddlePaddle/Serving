#ifndef BAIDU_PADDLE_SERVING_PREDICTOR_OP_WRITE_JSON_OP_H
#define BAIDU_PADDLE_SERVING_PREDICTOR_OP_WRITE_JSON_OP_H

#include "builtin_format.pb.h"
#include "image_class.pb.h"
#include "common/inner_common.h"
#include "op/op.h"
#include "framework/channel.h"
#include "framework/op_repository.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

class WriteJsonOp : public OpWithChannel<
      baidu::paddle_serving::predictor::image_classification::Response> {
public: 

    DECLARE_OP(WriteJsonOp);

    int inference();
};

} // predictor
} // paddle_serving
} // baidu

#endif
