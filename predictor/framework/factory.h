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

#ifndef BAIDU_PADDLE_SERVING_PREDICTOR_FACTORY_H
#define BAIDU_PADDLE_SERVING_PREDICTOR_FACTORY_H

#include "common/inner_common.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

//////////////// DECLARE INTERFACE ////////////////
#define DECLARE_FACTORY_OBJECT(D, B)            \
    static int regist(const std::string& tag) { \
        FactoryDerive<D, B>* factory =                \
                new (std::nothrow) FactoryDerive<D, B>();\
        if (factory == NULL                     \
                || FactoryPool<B>::instance().register_factory(\
                    tag, factory) != 0) {       \
            LOG(FATAL) << "Failed regist factory:"\
                << #D << " in macro!";          \
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

//////////////// REGISTER INTERFACE ////////////////

#define REGIST_FACTORY_OBJECT_IMPL(D, B)        \
__attribute__((constructor)) static void PDS_STR_CAT(GlobalRegistObject, __LINE__)(void)    \
{                                               \
    ::baidu::paddle_serving::predictor::FactoryDerive<D, B>* factory =\
            new (::std::nothrow) ::baidu::paddle_serving::predictor::FactoryDerive<D, B>();\
    if (factory == NULL                         \
            || ::baidu::paddle_serving::predictor::FactoryPool<B>::instance().register_factory(\
                #D, factory) != 0) {            \
        LOG(FATAL) << "Failed regist factory:"  \
            << #D << "->" << #B << " in macro!";\
        return ;                                \
    }                                           \
    return ;                                    \
}

#define REGIST_FACTORY_OBJECT_IMPL_WITH_NAME(D, B, N) \
__attribute__((constructor)) static void PDS_STR_CAT(GlobalRegistObject, __LINE__)(void)    \
{                                               \
    ::baidu::paddle_serving::predictor::FactoryDerive<D, B>* factory =\
            new (::std::nothrow) ::baidu::paddle_serving::predictor::FactoryDerive<D, B>();\
    if (factory == NULL                         \
            || ::baidu::paddle_serving::predictor::FactoryPool<B>::instance().register_factory(\
                N, factory) != 0) {             \
        LOG(FATAL) << "Failed regist factory:"  \
            << #D << "->" << #B << ", tag: "    \
            << N << " in macro!";               \
        return ;                                \
    }                                           \
    LOG(WARNING) << "Succ regist factory:"      \
        << #D << "->" << #B << ", tag: "        \
        << N << " in macro!";                   \
    return ;                                    \
}

template<typename B>
class FactoryBase {
public:
    virtual B* gen() = 0;
    virtual void del(B* obj) = 0;
};

template<typename D, typename B>
class FactoryDerive : public FactoryBase<B> {
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
            LOG(FATAL) << "Insert duplicate with tag: "
                << tag;
            return -1;
        }

        std::pair<
            typename std::map<std::string, FactoryBase<B>*>::iterator, 
            bool> r = _pool.insert(std::make_pair(tag, factory));
        if (!r.second) {
            LOG(FATAL) << "Failed insert new factory with:"
                << tag;
            return -1;
        }

        LOG(INFO) << "Succ insert one factory, tag: " << tag
            << ", base type: " << typeid(B).name();

        return 0;
    }

    B* generate_object(const std::string& tag) {
        typename std::map<std::string, FactoryBase<B>*>::iterator it 
            = _pool.find(tag);
        if (it == _pool.end() || it->second == NULL) {
            LOG(FATAL) << "Not found factory pool, tag:" 
                << tag << ", pool size: " << _pool.size();
            return NULL;
        }

        return it->second->gen();
    }

    template<typename D>
    void return_object(B* object) {
        FactoryDerive<D, B> factory;
        factory.del(object);
    }

private:
    std::map<std::string, FactoryBase<B>*> _pool;
};

} // predictor
} // paddle_serving
} // baidu

#endif  //BAIDU_PADDLE_SERVING_PREDICTOR_FACTORY_H

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
