#include "op/dense_echo_op.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

using baidu::paddle_serving::predictor::format::DensePrediction;
using baidu::paddle_serving::predictor::dense_service::Request;
using baidu::paddle_serving::predictor::dense_service::Response;

int DenseEchoOp::inference() {
    const Request* req =
            dynamic_cast<const Request*>(get_request_message());
    Response* res = mutable_data<Response>();
    LOG(INFO) << "Receive request in dense service:"
            << req->ShortDebugString();
    uint32_t sample_size = req->instances_size();
    for (uint32_t si = 0; si < sample_size; si++) {
        DensePrediction* dense_res = 
                res->mutable_predictions()->Add();
        dense_res->add_categories(100.0 + si * 0.1);
        dense_res->add_categories(200.0 + si * 0.1);
    }
    return 0; 
}

DEFINE_OP(DenseEchoOp);

} // predictor
} // paddle_serving
} // baidu
