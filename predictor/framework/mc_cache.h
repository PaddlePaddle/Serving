#ifndef BAIDU_PADDLE_SERVING_PREDICTOR_FRAMEWORK_MC_CACHE_H
#define BAIDU_PADDLE_SERVING_PREDICTOR_FRAMEWORK_MC_CACHE_H

#include <stdint.h>
#include <mc_cache.h> // mc_creat_cache
#include <base/scoped_lock.h> // BAIDU_SCOPED_LOCK

namespace baidu {
namespace paddle_serving {
namespace predictor {

class McCache {
public:
    McCache();
    
    int initialize(uint32_t cache_capacity, uint32_t unitsize);

    int finalize();

    int add_item(uint32_t* sign, void* pdata);

    int add_item(uint32_t* sign, void* pdata, uint32_t unitsize);

    int seek_item(uint32_t* sign, void* pdata) const;

    int remove_item(uint32_t* sign);

    const uint32_t get_cache_unitsize();

private:
    mc_cache* _pcache;  // cache对象指针
    uint32_t _cache_unitsize; // cache单元大小
    mutable base::Mutex _mutex;
};

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu

#endif  // BAIDU_PADDLE_SERVING_PREDICTOR_FRAMEWORK_MC_CACHE_H

