#include "pb_to_json.h"
#include <google/protobuf/text_format.h>

#include "op/write_json_op.h"
#include "framework/memory.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

using baidu::paddle_serving::predictor::format::XImageResInstance;
using baidu::paddle_serving::predictor::image_classification::ClassifyResponse;
using baidu::paddle_serving::predictor::image_classification::Response;

int WriteJsonOp::inference() {
    const ClassifyResponse* classify_out =
            get_depend_argument<ClassifyResponse>("image_classify_op");
    if (!classify_out) {
        LOG(ERROR) << "Failed mutable depended argument, op:"
            << "image_classify_op";
        return -1;
    }

    Response* res = mutable_data<Response>();
    if (!res) {
        LOG(ERROR) << "Failed mutable output response in op:" 
            << "WriteJsonOp";
        return -1;
    }

    // transfer classify output message into json format
    std::string err_string;
    uint32_t sample_size = classify_out->predictions_size();
    for (uint32_t si = 0; si < sample_size; si++) {
        XImageResInstance* ins = res->add_predictions();
        if (!ins) {
            LOG(ERROR) << "Failed add one prediction ins"; 
            return -1;
        }
        std::string* text = ins->mutable_response_json();
        if (!ProtoMessageToJson(classify_out->predictions(si), 
                    text, &err_string)) {
            LOG(ERROR) << "Failed convert message[" 
                << classify_out->predictions(si).ShortDebugString()
                << "], err: " << err_string;
            return -1;
        }
    }

    LOG(TRACE) << "Succ write json:"
        << classify_out->ShortDebugString();

    return 0; 
}

DEFINE_OP(WriteJsonOp);

} // predictor
} // paddle_serving
} // baidu
