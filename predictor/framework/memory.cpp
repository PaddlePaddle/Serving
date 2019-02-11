#include "common/inner_common.h"
#include "framework/memory.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

int MempoolWrapper::initialize() {
    if (THREAD_KEY_CREATE(&_bspec_key, NULL) != 0) {
        LOG(ERROR) << "unable to create thread_key of thrd_data";
        return -1;
    }
    if (THREAD_SETSPECIFIC(_bspec_key, NULL) != 0) {
        LOG(ERROR) << "failed initialize bsepecific key to null";
        return -1;
    }

    return 0;
}

int MempoolWrapper::thread_initialize() {
    _region.init();
    im::Mempool* p_mempool = new (std::nothrow) im::Mempool(&_region);
    if (p_mempool == NULL) {
        LOG(ERROR) << "Failed create thread mempool";
        return -1;
    }

    if (THREAD_SETSPECIFIC(_bspec_key, p_mempool) != 0) {
        LOG(ERROR) << "unable to set the thrd_data";
        delete p_mempool;
        return -1;
    } 

    LOG(WARNING) << "Succ thread initialize mempool wrapper";
    return 0;
}

int MempoolWrapper::thread_clear() {
    im::Mempool* p_mempool = (im::Mempool*) THREAD_GETSPECIFIC(
            _bspec_key);
    if (p_mempool) {
        p_mempool->release_block();
        _region.reset(); 
    }

    return 0;
}

void* MempoolWrapper::malloc(size_t size) {
    im::Mempool* p_mempool = (im::Mempool*) THREAD_GETSPECIFIC(
            _bspec_key);
        if (!p_mempool) {
            LOG(WARNING) << "Cannot malloc memory:" << size
                << ", since mempool is not thread initialized";
            return NULL;
        }
        return p_mempool->malloc(size);
}

} // predictor
} // paddle_serving
} // baidu
