// Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#include <map>
#include <string>
#include <utility>
#include "core/predictor/common/inner_common.h"
#include "glog/raw_logging.h"
namespace baidu {
namespace paddle_serving {
namespace predictor {

//////////////// DECLARE INTERFACE ////////////////
#define DECLARE_FACTORY_OBJECT(D, B)                                         \
  static int regist(const std::string& tag) {                                \
    FactoryDerive<D, B>* factory = new (std::nothrow) FactoryDerive<D, B>(); \
    if (factory == NULL ||                                                   \
        FactoryPool<B>::instance().register_factory(tag, factory) != 0) {    \
      RAW_LOG_FATAL("Failed regist factory: %s in macro!", #D);              \
      return -1;                                                             \
    }                                                                        \
    return 0;                                                                \
  }

#define PDS_STR_CAT(a, b) PDS_STR_CAT_I(a, b)
#define PDS_STR_CAT_I(a, b) a##b

#define DEFINE_FACTORY_OBJECT(D)                                           \
  __attribute__((constructor)) static void PDS_STR_CAT(GlobalRegistObject, \
                                                       __LINE__)(void) {   \
    D::regist(#D);                                                         \
  }

//////////////// REGISTER INTERFACE ////////////////

#define REGIST_FACTORY_OBJECT_IMPL(D, B)                                       \
  __attribute__((constructor)) static void PDS_STR_CAT(GlobalRegistObject,     \
                                                       __LINE__)(void) {       \
    ::baidu::paddle_serving::predictor::FactoryDerive<D, B>* factory = new (   \
        ::std::nothrow)::baidu::paddle_serving::predictor::FactoryDerive<D,    \
                                                                         B>(); \
    if (factory == NULL ||                                                     \
        ::baidu::paddle_serving::predictor::FactoryPool<B>::instance()         \
                .register_factory(#D, factory) != 0) {                         \
      RAW_LOG_FATAL("Failed regist factory: %s->%s in macro!", #D, #B);        \
      return;                                                                  \
    }                                                                          \
    return;                                                                    \
  }

#define REGIST_FACTORY_OBJECT_IMPL_WITH_NAME(D, B, N)                          \
  __attribute__((constructor)) static void PDS_STR_CAT(GlobalRegistObject,     \
                                                       __LINE__)(void) {       \
    ::baidu::paddle_serving::predictor::FactoryDerive<D, B>* factory = new (   \
        ::std::nothrow)::baidu::paddle_serving::predictor::FactoryDerive<D,    \
                                                                         B>(); \
    if (factory == NULL ||                                                     \
        ::baidu::paddle_serving::predictor::FactoryPool<B>::instance()         \
                .register_factory(N, factory) != 0) {                          \
      RAW_LOG_FATAL(                                                           \
          "Failed regist factory: %s->%s, tag: %s in macro!", #D, #B, N);      \
      return;                                                                  \
    }                                                                          \
    RAW_LOG_WARNING(                                                           \
        "Succ regist factory: %s->%s, tag: %s in macro!", #D, #B, N);          \
    return;                                                                    \
  }

template <typename B>
class FactoryBase {
 public:
  virtual B* gen() = 0;
  virtual void del(B* obj) = 0;
};

template <typename D, typename B>
class FactoryDerive : public FactoryBase<B> {
 public:
  B* gen() { return new (std::nothrow) D(); }

  void del(B* obj) { delete dynamic_cast<D*>(obj); }
};

template <typename B>
class FactoryPool {
 public:
  static FactoryPool<B>& instance() {
    static FactoryPool<B> singleton;
    return singleton;
  }

  int register_factory(const std::string& tag, FactoryBase<B>* factory) {
    typename std::map<std::string, FactoryBase<B>*>::iterator it =
        _pool.find(tag);
    if (it != _pool.end()) {
      RAW_LOG_FATAL("Insert duplicate with tag: %s", tag.c_str());
      return -1;
    }

    std::pair<typename std::map<std::string, FactoryBase<B>*>::iterator, bool>
        r = _pool.insert(std::make_pair(tag, factory));
    if (!r.second) {
      RAW_LOG_FATAL("Failed insert new factory with: %s", tag.c_str());
      return -1;
    }

    RAW_LOG_INFO("Succ insert one factory, tag: %s, base type %s",
                 tag.c_str(),
                 typeid(B).name());

    return 0;
  }

  B* generate_object(const std::string& tag) {
    typename std::map<std::string, FactoryBase<B>*>::iterator it =
        _pool.find(tag);
    if (it == _pool.end() || it->second == NULL) {
      RAW_LOG_FATAL("Not found factory pool, tag: %s, pool size %u",
                    tag.c_str(),
                    _pool.size());
      return NULL;
    }

    return it->second->gen();
  }

  template <typename D>
  void return_object(B* object) {
    FactoryDerive<D, B> factory;
    factory.del(object);
  }

 private:
  std::map<std::string, FactoryBase<B>*> _pool;
};

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu

/* vim: set expandtab ts=2 sw=2 sts=2 tw=100: */
