#ifndef BAIDU_PADDLE_SERVING_PREDICTOR_RESOURCE_H
#define BAIDU_PADDLE_SERVING_PREDICTOR_RESOURCE_H

#include "common/inner_common.h"
#include "framework/memory.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

class BaseRdDict;
struct DynamicResource {
    DynamicResource();
    
    ~DynamicResource();
    
    int initialize();
    
    int clear();
};

class Resource {
public:

    Resource() {}

    ~Resource() { finalize(); }

    static Resource& instance() {
        static Resource ins;
        return ins;
    }

    int initialize(const std::string& path, const std::string& file);

    int thread_initialize();

    int thread_clear();

    int reload();

    int finalize();

    DynamicResource* get_dynamic_resource() {
        return (DynamicResource*) THREAD_GETSPECIFIC(_tls_bspec_key);
    }

private:
    int thread_finalize() {
        return 0; 
    }

    THREAD_KEY_T _tls_bspec_key;
    
};

} // predictor
} // paddle_serving
} // baidu

#endif
