#include "mc_cache.h"
#include <bvar/bvar.h> // bvar

namespace baidu {
namespace paddle_serving {
namespace predictor {

::bvar::Adder<int> g_mc_cache_seek_error_count("mc_cache_seek_error_count"); // 失败查询次数
::bvar::Window<::bvar::Adder<int> > g_mc_cache_seek_error_window(
        "mc_cache_seek_error_window", &g_mc_cache_seek_error_count,
        ::bvar::FLAGS_bvar_dump_interval);

::bvar::Adder<int> g_mc_cache_seek_count("mc_cache_seek_count"); // 总查询次数
::bvar::Window<::bvar::Adder<int> > g_mc_cache_seek_window(
        "mc_cache_seek_window", &g_mc_cache_seek_count,
        ::bvar::FLAGS_bvar_dump_interval);

float get_mc_cache_seek_error_percent(void*) {
    if (g_mc_cache_seek_window.get_value() <= 0) {
        return 0;
    }
    return g_mc_cache_seek_error_window.get_value()
            / (float) g_mc_cache_seek_window.get_value();
}

::bvar::PassiveStatus<float> g_mc_cache_seek_error_percent("mc_cache_seek_error_percent",
        get_mc_cache_seek_error_percent, NULL);

McCache::McCache() {
    _pcache = NULL;
    _cache_unitsize = 0;
}

int McCache::initialize(uint32_t cache_capacity, uint32_t unitsize) {
    _pcache = mc_creat_cache(cache_capacity, unitsize);
    if (_pcache == NULL) {
        LOG(ERROR) << "create mc_cache capacity[" << cache_capacity
                << "], unitsize[" << unitsize << "] failed.";
        return -1;
    }
    _cache_unitsize = unitsize;
    return 0;
}

int McCache::finalize() {
    // 销毁cache结构
    if (mc_destroy_cache(_pcache) == RT_NOTICE_NONE_PROCESSED) {
        LOG(ERROR) << "input pcache[" << _pcache << "] destroy failed";
        return -1;
    }
    return 0;
}

int McCache::add_item(uint32_t* sign, void* pdata) {
    int ret = 0;
    {
        BAIDU_SCOPED_LOCK(_mutex);
        ret = mc_additem(_pcache, sign, pdata, _cache_unitsize);
    }
    return (ret - 1);
}

int McCache::add_item(uint32_t* sign, void* pdata, uint32_t unitsize) {
    CHECK_GT(_cache_unitsize, unitsize) << "input unitsize should < _cache_unitsize"; 
    int ret = 0;
    {
        BAIDU_SCOPED_LOCK(_mutex);
        ret = mc_additem(_pcache, sign, pdata, unitsize);
    }
    return (ret - 1);
}

int McCache::seek_item(uint32_t* sign, void* pdata) const {
    int ret = 0;
    {
        BAIDU_SCOPED_LOCK(_mutex);
        ret = mc_seekitem(_pcache, sign, pdata, _cache_unitsize);
    }

    g_mc_cache_seek_count << 1;
    if (ret != RT_NOTICE_PROCESSED) {
        g_mc_cache_seek_error_count << 1; 
    }
    return (ret - 1);
}

int McCache::remove_item(uint32_t* sign) {
    int ret = 0;
    {
        BAIDU_SCOPED_LOCK(_mutex);
        ret = mc_removeitem(_pcache, sign);
    }
    if (ret != RT_NOTICE_PROCESSED) {
        LOG(WARNING) << "remove item from cache failed, errno[" << ret
                << "], sign[" << *sign << "].";
        return -1;
    }
    return 0;
}

const uint32_t McCache::get_cache_unitsize() {
    return _cache_unitsize;
}

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
