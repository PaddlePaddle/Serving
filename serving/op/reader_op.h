#ifndef BAIDU_PADDLE_SERVING_SERVING_OP_READER_OP_H
#define BAIDU_PADDLE_SERVING_SERVING_OP_READER_OP_H

#include "builtin_format.pb.h"
#include "image_class.pb.h"
#include "common/inner_common.h"
#include "op/op.h"
#include "framework/channel.h"
#include "framework/op_repository.h"

// opencv
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv/cxcore.h"
#include "opencv/cv.hpp"

#include "paddle/fluid/inference/paddle_inference_api.h"

namespace baidu {
namespace paddle_serving {
namespace serving {

struct ReaderOutput {
    std::vector<paddle::PaddleTensor> tensors;

    void Clear() {
        size_t tensor_count = tensors.size();
        for (size_t ti = 0; ti < tensor_count; ++ti) {
            tensors[ti].shape.clear();
        }
        tensors.clear();
    }

    std::string ShortDebugString() const {
        return "Not implemented!";
    }
};

class ReaderOp : public baidu::paddle_serving::predictor::OpWithChannel<
                 ReaderOutput> {
public: 
    typedef std::vector<paddle::PaddleTensor> TensorVector;

    DECLARE_OP(ReaderOp);

    int inference();

private:
    cv::Mat _image_8u_tmp;
    cv::Mat _image_8u_rgb;
    std::vector<char> _image_vec_tmp;
};

} // serving
} // paddle_serving
} // baidu

#endif
