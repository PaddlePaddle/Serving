#ifndef BAIDU_PADDLE_SERVING_PREDICTOR_MEMORY_H
#define BAIDU_PADDLE_SERVING_PREDICTOR_MEMORY_H

#include "common/inner_common.h"
#include "mempool.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

class MempoolWrapper {
public:
    MempoolWrapper() {}

    static MempoolWrapper& instance() {
        static MempoolWrapper mempool;
        return mempool;
    }

    int initialize();

    int thread_initialize();

    int thread_clear();

    void* malloc(size_t size);

private:
    im::fugue::memory::Region _region;
    THREAD_KEY_T _bspec_key;
};

} // predictor
} // paddle_serving
} // baidu

#endif
