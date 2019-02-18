#ifndef BAIDU_PADDLE_SERVING_PREDICTOR_OP_IMAGE_CLASSIFY_OP_H
#define BAIDU_PADDLE_SERVING_PREDICTOR_OP_IMAGE_CLASSIFY_OP_H

#include "builtin_format.pb.h"
#include "image_class.pb.h"
#include "common/inner_common.h"
#include "op/op.h"
#include "framework/channel.h"
#include "framework/op_repository.h"

namespace baidu {
namespace paddle_serving {
namespace serving {

static const char* IMAGE_CLASSIFICATION_MODEL_NAME
    = "image_classification_resnet";

class ClassifyOp : public baidu::paddle_serving::predictor::OpWithChannel<
      baidu::paddle_serving::predictor::image_classification::ClassifyResponse> {
public: 
    typedef std::vector<paddle::PaddleTensor> TensorVector;

    DECLARE_OP(ClassifyOp);

    int inference();
};

} // serving
} // paddle_serving
} // baidu

#endif
