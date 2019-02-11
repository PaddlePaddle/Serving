#ifndef BAIDU_PREDICTOR_PREDICTOR_COMMON_ECHO_OP_H
#define BAIDU_PREDICTOR_PREDICTOR_COMMON_ECHO_OP_H

#include "echo_service.pb.h"

#include "common/inner_common.h"
#include "op/op.h"
#include "framework/channel.h"
#include "framework/op_repository.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

class CommonEchoOp : public OpWithChannel<
      baidu::paddle_serving::predictor::echo_service::RequestAndResponse> {
public:

    typedef baidu::paddle_serving::predictor::echo_service::RequestAndResponse
        RequestAndResponse;
    
    DECLARE_OP(CommonEchoOp);

    int inference() {
        const RequestAndResponse* req = dynamic_cast<const RequestAndResponse*>(
                get_request_message());

        RequestAndResponse* data = mutable_data<RequestAndResponse>();

        data->CopyFrom(*req);

        return 0;
    }
};

} // predictor
} // paddle_serving
} // baidu

#endif
