#ifndef BAIDU_PADDLE_SERVING_PREDICTOR_RESOURCE_H
#define BAIDU_PADDLE_SERVING_PREDICTOR_RESOURCE_H

#include "common/inner_common.h"
#include "framework/memory.h"
#include "framework/mc_cache.h" // McCache

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

    Resource() :
        _mc_cache(NULL) {
    }

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

    McCache* get_mc_cache() {
        return _mc_cache;
    }

private:
    int thread_finalize() {
        return 0; 
    }

    THREAD_KEY_T _tls_bspec_key;
    
    McCache* _mc_cache;
};

} // predictor
} // paddle_serving
} // baidu

#endif
