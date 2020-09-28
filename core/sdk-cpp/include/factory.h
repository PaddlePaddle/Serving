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
#include "core/sdk-cpp/include/common.h"
#include "core/sdk-cpp/include/stub_impl.h"

namespace baidu {
namespace paddle_serving {
namespace sdk_cpp {

#ifdef BCLOUD
namespace brpc = baidu::rpc;
#endif

#define ERROR_STRING_LEN 10240

#define INLINE_REGIST_OBJECT(D, B, E)                                    \
  do {                                                                   \
    Factory<D, B>* factory = new (std::nothrow) Factory<D, B>();         \
    if (factory == NULL ||                                               \
        FactoryPool<B>::instance().register_factory(#D, factory) != 0) { \
      char err_str[ERROR_STRING_LEN];                                    \
      snprintf(err_str,                                                  \
               ERROR_STRING_LEN - 1,                                     \
               "Failed regist factory: %s->%s in macro!",                \
               #D,                                                       \
               #B);                                                      \
      RAW_LOG(ERROR, err_str);                                           \
      return E;                                                          \
    }                                                                    \
  } while (0)

#define DECLARE_FACTORY_OBJECT(D, B)                                      \
  static int regist(const std::string& tag) {                             \
    Factory<D, B>* factory = new (std::nothrow) Factory<D, B>();          \
    if (factory == NULL ||                                                \
        FactoryPool<B>::instance().register_factory(tag, factory) != 0) { \
      char err_str[ERROR_STRING_LEN];                                     \
      snprintf(err_str,                                                   \
               ERROR_STRING_LEN - 1,                                      \
               "Failed regist factory: %s in macro!",                     \
               #D);                                                       \
      RAW_LOG(ERROR, err_str);                                            \
      return -1;                                                          \
    }                                                                     \
    return 0;                                                             \
  }

#define PDS_STR_CAT(a, b) PDS_STR_CAT_I(a, b)
#define PDS_STR_CAT_I(a, b) a##b

#define DEFINE_FACTORY_OBJECT(D)                                           \
  __attribute__((constructor)) static void PDS_STR_CAT(GlobalRegistObject, \
                                                       __LINE__)(void) {   \
    D::regist(#D);                                                         \
  }

#define REGIST_FACTORY_OBJECT_IMPL(D, B)                                       \
  __attribute__((constructor)) static void PDS_STR_CAT(GlobalRegistObject,     \
                                                       __LINE__)(void) {       \
    ::baidu::paddle_serving::sdk_cpp::Factory<D, B>* factory =                 \
        new (::std::nothrow)::baidu::paddle_serving::sdk_cpp::Factory<D, B>(); \
    if (factory == NULL ||                                                     \
        ::baidu::paddle_serving::sdk_cpp::FactoryPool<B>::instance()           \
                .register_factory(#D, factory) != 0) {                         \
      char err_str[ERROR_STRING_LEN];                                          \
      snprintf(err_str,                                                        \
               ERROR_STRING_LEN - 1,                                           \
               "Failed regist factory: %s->%s in macro!",                      \
               #D,                                                             \
               #B);                                                            \
      RAW_LOG(ERROR, err_str);                                                 \
      return;                                                                  \
    }                                                                          \
    return;                                                                    \
  }

#define REGIST_FACTORY_OBJECT_IMPL_WITH_TAG(D, B, T)                           \
  __attribute__((constructor)) static void PDS_STR_CAT(GlobalRegistObject,     \
                                                       __LINE__)(void) {       \
    ::baidu::paddle_serving::sdk_cpp::Factory<D, B>* factory =                 \
        new (::std::nothrow)::baidu::paddle_serving::sdk_cpp::Factory<D, B>(); \
    if (factory == NULL ||                                                     \
        ::baidu::paddle_serving::sdk_cpp::FactoryPool<B>::instance()           \
                .register_factory(T, factory) != 0) {                          \
      char err_str[ERROR_STRING_LEN];                                          \
      snprintf(err_str,                                                        \
               ERROR_STRING_LEN - 1,                                           \
               "Failed regist factory: %s->%s, tag %s in macro!",              \
               #D,                                                             \
               #B,                                                             \
               T);                                                             \
      RAW_LOG(ERROR, err_str);                                                 \
      return;                                                                  \
    }                                                                          \
    return;                                                                    \
  }

#define REGIST_ABTEST_OBJECT(D) \
  REGIST_FACTORY_OBJECT_IMPL(   \
      D, ::baidu::paddle_serving::sdk_cpp::ABTestRouterBase)

#define REGIST_ABTEST_OBJECT_WITH_TAG(D, T) \
  REGIST_FACTORY_OBJECT_IMPL_WITH_TAG(      \
      D, ::baidu::paddle_serving::sdk_cpp::ABTestRouterBase, T)

#define REGIST_STUB_OBJECT_WITH_TAG(D, C, R, I, O, T)                      \
  __attribute__((constructor)) static void PDS_STR_CAT(GlobalRegistObject, \
                                                       __LINE__)(void) {   \
    ::baidu::paddle_serving::sdk_cpp::Factory<                             \
        ::baidu::paddle_serving::sdk_cpp::StubImpl<D, C, R, I, O>,         \
        ::baidu::paddle_serving::sdk_cpp::Stub>* factory =                 \
        new (::std::nothrow)::baidu::paddle_serving::sdk_cpp::Factory<     \
            ::baidu::paddle_serving::sdk_cpp::StubImpl<D, C, R, I, O>,     \
            ::baidu::paddle_serving::sdk_cpp::Stub>();                     \
    if (factory == NULL ||                                                 \
        ::baidu::paddle_serving::sdk_cpp::FactoryPool<                     \
            ::baidu::paddle_serving::sdk_cpp::Stub>::instance()            \
                .register_factory(T, factory) != 0) {                      \
      char err_str[ERROR_STRING_LEN];                                      \
      snprintf(err_str,                                                    \
               ERROR_STRING_LEN - 1,                                       \
               "Failed regist factory: %s->Stub, tag: %s in macro!",       \
               #D,                                                         \
               T);                                                         \
      RAW_LOG(ERROR, err_str);                                             \
      return;                                                              \
    }                                                                      \
    return;                                                                \
  }

class Stub;
class EndpointRouterBase;
class VariantRouterBase;

template <typename B>
class FactoryBase {
 public:
  virtual B* gen() = 0;
  virtual void del(B* obj) = 0;
};

template <typename D, typename B>
class Factory : public FactoryBase<B> {
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
      char err_str[ERROR_STRING_LEN];
      snprintf(err_str,
               ERROR_STRING_LEN - 1,
               "Insert duplicate with tag: %s",
               tag.c_str());
      RAW_LOG(ERROR, err_str);
      return -1;
    }

    std::pair<typename std::map<std::string, FactoryBase<B>*>::iterator, bool>
        r = _pool.insert(std::make_pair(tag, factory));
    if (!r.second) {
      char err_str[ERROR_STRING_LEN];
      snprintf(err_str,
               ERROR_STRING_LEN - 1,
               "Failed insert new factory with: %s",
               tag.c_str());
      RAW_LOG(ERROR, err_str);
      return -1;
    }

    return 0;
  }

  B* generate_object(const std::string& tag) {
    typename std::map<std::string, FactoryBase<B>*>::iterator it =
        _pool.find(tag);
    if (it == _pool.end() || it->second == NULL) {
      char err_str[ERROR_STRING_LEN];
      snprintf(err_str,
               ERROR_STRING_LEN - 1,
               "Not found factory pool, tag: %s, pool size: %u",
               tag.c_str(),
               _pool.size());
      RAW_LOG(ERROR, err_str);
      return NULL;
    }

    return it->second->gen();
  }

  template <typename D>
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

}  // namespace sdk_cpp
}  // namespace paddle_serving
}  // namespace baidu
