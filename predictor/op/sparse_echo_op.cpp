#include "op/sparse_echo_op.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

int SparseEchoOp::inference() {
    // Every op can obtain request message by:
    // get_request_message()
    const Request* req =
        dynamic_cast<const Request*>(get_request_message());

    // Each op can obtain self-writable-data by:
    // mutable_data()
    Response* res = mutable_data<Response>();

    // You can get the channel/data of depended ops by:
    // get/mutable_depend_argment()
    // ...

    LOG(DEBUG) 
        << "Receive request in sparse service:"
        << req->ShortDebugString();
    uint32_t sample_size = req->instances_size();
    for (uint32_t si = 0; si < sample_size; si++) {
        SparsePrediction* sparse_res = 
            res->mutable_predictions()->Add();
        sparse_res->add_categories(100.0 + si * 0.1);
        sparse_res->add_categories(200.0 + si * 0.1);
    }
    return 0; 
}

DEFINE_OP(SparseEchoOp);

} // predictor
} // paddle_serving
} // baidu
