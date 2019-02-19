/***************************************************************************
 * 
 * Copyright (c) 2018 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
/**
 * @file include/factory.h
 * @author wanlijin01(wanlijin01@baidu.com)
 * @date 2018/07/10 22:09:57
 * @brief 
 *  
 **/

#ifndef BAIDU_PADDLE_SERVING_SDK_CPP_FACTORY_H
#define BAIDU_PADDLE_SERVING_SDK_CPP_FACTORY_H

#include "common.h"
#include "stub_impl.h"
#include "glog/raw_logging.h"

namespace baidu {
namespace paddle_serving {
namespace sdk_cpp {

#define INLINE_REGIST_OBJECT(D, B, E)           \
do {                                            \
    Factory<D, B>* factory =                    \
            new (std::nothrow) Factory<D, B>(); \
    if (factory == NULL                         \
            || FactoryPool<B>::instance().register_factory(\
                #D, factory) != 0) {            \
        RAW_LOG_FATAL("Failed regist factory: %s->%s in macro!", #D, #B);  \
        return E;                               \
    }                                           \
} while (0)

#define DECLARE_FACTORY_OBJECT(D, B)            \
    static int regist(const std::string& tag) { \
        Factory<D, B>* factory =                \
                new (std::nothrow) Factory<D, B>();\
        if (factory == NULL                     \
                || FactoryPool<B>::instance().register_factory(\
                    tag, factory) != 0) {       \
            RAW_LOG_FATAL("Failed regist factory: %s in macro!", #D);\
            return -1;                          \
        }                                       \
        return 0;                               \
    }

#define PDS_STR_CAT(a, b) PDS_STR_CAT_I(a, b)
#define PDS_STR_CAT_I(a, b) a ## b

#define DEFINE_FACTORY_OBJECT(D)                \
__attribute__((constructor)) static void PDS_STR_CAT(GlobalRegistObject, __LINE__)(void)    \
{                                               \
    D::regist(#D);                              \
}

#define REGIST_FACTORY_OBJECT_IMPL(D, B)        \
__attribute__((constructor)) static void PDS_STR_CAT(GlobalRegistObject, __LINE__)(void)    \
{                                               \
    ::baidu::paddle_serving::sdk_cpp::Factory<D, B>* factory =\
            new (::std::nothrow) ::baidu::paddle_serving::sdk_cpp::Factory<D, B>();\
    if (factory == NULL                         \
            || ::baidu::paddle_serving::sdk_cpp::FactoryPool<B>::instance().register_factory(\
                #D, factory) != 0) {            \
        RAW_LOG_FATAL("Failed regist factory: %s->%s in macro!", #D, #B); \
        return ;                                \
    }                                           \
    return ;                                    \
}

#define REGIST_FACTORY_OBJECT_IMPL_WITH_TAG(D, B, T)\
__attribute__((constructor)) static void PDS_STR_CAT(GlobalRegistObject, __LINE__)(void)    \
{                                               \
    ::baidu::paddle_serving::sdk_cpp::Factory<D, B>* factory =\
            new (::std::nothrow) ::baidu::paddle_serving::sdk_cpp::Factory<D, B>();\
    if (factory == NULL                         \
            || ::baidu::paddle_serving::sdk_cpp::FactoryPool<B>::instance().register_factory(\
                T, factory) != 0) {             \
        RAW_LOG_FATAL("Failed regist factory: %s->%s, tag %s in macro!", #D, #B, T);  \
        return ;                                \
    }                                           \
    return ;                                    \
}

#define REGIST_ABTEST_OBJECT(D)                 \
    REGIST_FACTORY_OBJECT_IMPL(                 \
            D,                                  \
            ::baidu::paddle_serving::sdk_cpp::ABTestRouterBase)

#define REGIST_ABTEST_OBJECT_WITH_TAG(D, T)     \
    REGIST_FACTORY_OBJECT_IMPL_WITH_TAG(        \
            D,                                  \
            ::baidu::paddle_serving::sdk_cpp::ABTestRouterBase,\
            T)

#define REGIST_STUB_OBJECT_WITH_TAG(D, C, R, I, O, T)                   \
__attribute__((constructor)) static void PDS_STR_CAT(GlobalRegistObject, __LINE__)(void)    \
{                                                                       \
    ::baidu::paddle_serving::sdk_cpp::Factory<                          \
                ::baidu::paddle_serving::sdk_cpp::StubImpl<D, C, R, I, O>,\
                ::baidu::paddle_serving::sdk_cpp::Stub>* factory =      \
            new (::std::nothrow) ::baidu::paddle_serving::sdk_cpp::Factory< \
                    ::baidu::paddle_serving::sdk_cpp::StubImpl<D, C, R, I, O>,\
                    ::baidu::paddle_serving::sdk_cpp::Stub>();          \
    if (factory == NULL                                                 \
            || ::baidu::paddle_serving::sdk_cpp::FactoryPool<           \
                    ::baidu::paddle_serving::sdk_cpp::Stub>::instance().register_factory(\
                T, factory) != 0) {                                     \
        RAW_LOG_FATAL("Failed regist factory: %s->Stub, tag: %s in macro!", #D, T); \
        return ;                                                        \
    }                                                                   \
    return ;                                                            \
}

class Stub;
class EndpointRouterBase;
class VariantRouterBase;

template<typename B>
class FactoryBase {
public:
    virtual B* gen() = 0;
    virtual void del(B* obj) = 0;
};

template<typename D, typename B>
class Factory : public FactoryBase<B> {
public:
    B* gen() {
        return new(std::nothrow) D();
    }

    void del(B* obj) {
        delete dynamic_cast<D*>(obj);
    }
};

template<typename B>
class FactoryPool {
public:
    static FactoryPool<B>& instance() {
        static FactoryPool<B> singleton;
        return singleton;
    }

    int register_factory(const std::string& tag,
            FactoryBase<B>* factory) {
        typename std::map<std::string, FactoryBase<B>*>::iterator it 
            = _pool.find(tag);
        if (it != _pool.end()) {
            RAW_LOG_FATAL("Insert duplicate with tag: %s", tag.c_str());
            return -1;
        }

        std::pair<
            typename std::map<std::string, FactoryBase<B>*>::iterator, 
            bool> r = _pool.insert(std::make_pair(tag, factory));
        if (!r.second) {
            RAW_LOG_FATAL("Failed insert new factory with: %s", tag.c_str());
            return -1;
        }

        RAW_LOG_INFO("Succ insert one factory, tag: %s, base type %s", tag.c_str(), typeid(B).name()); 

        return 0;
    }

    B* generate_object(const std::string& tag) {
        typename std::map<std::string, FactoryBase<B>*>::iterator it 
            = _pool.find(tag);
        if (it == _pool.end() || it->second == NULL) {
            RAW_LOG_FATAL("Not found factory pool, tag: %s, pool size: %u", tag.c_str(), _pool.size()); 
            return NULL;
        }

        return it->second->gen();
    }

    template<typename D>
    void return_object(B* object) {
        Factory<D, B> factory;
        factory->del(object);
    }

private:
    std::map<std::string, FactoryBase<B>*> _pool;
};

typedef FactoryPool<Stub> StubFactory;
typedef FactoryPool<brpc::CallMapper> CallMapperFactory;
typedef FactoryPool<brpc::ResponseMerger> ResponseMergerFactory;
typedef FactoryPool<EndpointRouterBase> EndpointRouterFactory;
typedef FactoryPool<VariantRouterBase> VariantRouterFactory;

} // sdk_cpp
} // paddle_serving
} // baidu

#endif  //BAIDU_PADDLE_SERVING_SDK_CPP_FACTORY_H

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
