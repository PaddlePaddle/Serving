#ifndef BAIDU_PADDLE_SERVING_PREDICTOR_SERVICE_H
#define BAIDU_PADDLE_SERVING_PREDICTOR_SERVICE_H

#include "common/inner_common.h"
#include "framework/workflow.h"
#include "framework/merger.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

class InferService {
public:
    typedef OpChannel<google::protobuf::Message> BuiltinChannel;

    static const char* tag() {
        return "Service";
    }

    InferService() :
            _last_change_timestamp(0),
            _enable_map_request_to_workflow(false),
            _request_field_key(""),
            _merger(NULL) {
        _flows.clear();
        _request_to_workflow_map.clear();
    }

    int init(const comcfg::ConfigUnit& conf);

    int deinit() { return 0; }

    int reload();

    const std::string& name() const;
    
    const std::string& full_name() const {
        return _infer_service_format;
    }

    // 串行执行每个workflow
    virtual int inference(
            const google::protobuf::Message* request,
            google::protobuf::Message* response,
            butil::IOBufBuilder* debug_os = NULL);

    int debug(
            const google::protobuf::Message* request,
            google::protobuf::Message* response,
            butil::IOBufBuilder* debug_os);

    int execute_one_workflow(
            uint32_t index,
            const google::protobuf::Message* request, 
            google::protobuf::Message* response,
            butil::IOBufBuilder* debug_os);
private:
    int _execute_workflow(
            Workflow* workflow,
            const google::protobuf::Message* request, 
            google::protobuf::Message* response,
            butil::IOBufBuilder* debug_os);

    std::vector<Workflow*>* _map_request_to_workflow(const google::protobuf::Message* request);

private:
    std::vector<Workflow*> _flows;
    std::string _infer_service_format;
    uint64_t _last_change_timestamp;
    bool _enable_map_request_to_workflow;
    std::string _request_field_key;
    ::butil::FlatMap<std::string, std::vector<Workflow*> > _request_to_workflow_map;
    IMerger* _merger;
};

class ParallelInferService : public InferService {
public:
    // 并行执行每个workflow
    int inference(
            const google::protobuf::Message* request,
            google::protobuf::Message* response,
            butil::IOBufBuilder* debug_os) {
        return 0;
    }
};

} // predictor
} // paddle_serving
} // baidu

#endif // BAIDU_PADDLE_SERVING_PREDICTOR_INFERSERVICE_H
