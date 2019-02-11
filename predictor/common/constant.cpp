#include "common/constant.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

DEFINE_bool(use_parallel_infer_service, false, "");
DEFINE_int32(el_log_level, 16, "");
DEFINE_int32(idle_timeout_s, 16, "");
DEFINE_int32(port, 8010, "");
DEFINE_string(workflow_path, "./conf", "");
DEFINE_string(workflow_file, "workflow.conf", "");
DEFINE_string(inferservice_path, "./conf", "");
DEFINE_string(inferservice_file, "service.conf", "");
DEFINE_string(logger_path, "./conf", "");
DEFINE_string(logger_file, "log.conf", "");
DEFINE_string(resource_path, "./conf", "");
DEFINE_string(resource_file, "resource.conf", "");
DEFINE_bool(enable_yacl, false, "enable yacl");
DEFINE_string(yacl_module_name, "predictor", "yacl module name");
DEFINE_string(yacl_param_dump_file, "./data/yacl_param_list.txt", "yacl param dump file path");
DEFINE_bool(enable_mc_cache, false, "enable mc cache");
DEFINE_bool(enable_nshead_protocol, false, "enable nshead protocol in server side");
DEFINE_string(nshead_protocol, "itp", "type of nshead protocol, support itp, nova_pbrpc, public_pbrpc, nshead_mcpack");
DEFINE_int32(max_concurrency, 0, "Limit of request processing in parallel, 0: unlimited");
DEFINE_int32(num_threads, 0, "Number of pthreads that server runs on, not change if this value <= 0");
DEFINE_int32(reload_interval_s, 10, "");
DEFINE_bool(enable_model_toolkit, false, "enable model toolkit");
DEFINE_string(enable_protocol_list, "baidu_std nshead", "set protocol list");

} // predictor
} // paddle_serving
} // baidu

// Baidurpc 
BAIDU_REGISTER_ERRNO(baidu::paddle_serving::predictor::ERR_INTERNAL_FAILURE, 
        "Paddle Serving Framework Internal Error.");
BAIDU_REGISTER_ERRNO(baidu::paddle_serving::predictor::ERR_MEM_ALLOC_FAILURE, 
        "Paddle Serving Memory Alloc Error.");
BAIDU_REGISTER_ERRNO(baidu::paddle_serving::predictor::ERR_OVERFLOW_FAILURE, 
        "Paddle Serving Array Overflow Error.");
BAIDU_REGISTER_ERRNO(baidu::paddle_serving::predictor::ERR_OP_INFER_FAILURE, 
        "Paddle Serving Op Inference Error.");
