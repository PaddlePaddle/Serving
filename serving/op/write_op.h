#ifndef BAIDU_PADDLE_SERVING_PREDICTOR_OP_WRITE_OP_H
#define BAIDU_PADDLE_SERVING_PREDICTOR_OP_WRITE_OP_H

#include "builtin_format.pb.h"
#include "image_class.pb.h"
#include "common/inner_common.h"
#include "op/op.h"
#include "framework/channel.h"
#include "framework/op_repository.h"

namespace baidu {
namespace paddle_serving {
namespace serving {

class WriteOp : public baidu::paddle_serving::predictor::OpWithChannel<
      baidu::paddle_serving::predictor::image_classification::Response> {
public: 

    DECLARE_OP(WriteOp);

    int inference();
};

} // serving
} // paddle_serving
} // baidu

#endif
