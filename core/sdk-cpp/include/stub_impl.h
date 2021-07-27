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
#include <vector>
#include "core/sdk-cpp/include/common.h"
#include "core/sdk-cpp/include/endpoint_config.h"
#include "core/sdk-cpp/include/macros.h"
#include "core/sdk-cpp/include/predictor.h"
#include "core/sdk-cpp/include/stub.h"

namespace baidu {
namespace paddle_serving {
namespace sdk_cpp {

static const char* AVG_PREFIX = "avg_";
static const char* LTC_PREFIX = "ltc_";

class Predictor;
template <typename T>
class PredictorImpl;

static const char* INFERENCE_METHOD_NAME = "inference";
static const char* DEBUG_METHOD_NAME = "debug";

class MetricScope {
 public:
  MetricScope(Stub* stub, const char* routine)
      : _stub(stub), _tt(butil::Timer::STARTED), _routine(routine) {
    TRACEPRINTF("enter %s", routine);
  }

  ~MetricScope() {
    TRACEPRINTF("exit %s", _routine.c_str());
    _tt.stop();
    _stub->update_latency(_tt.u_elapsed(), _routine.c_str());
  }

 private:
  Stub* _stub;
  butil::Timer _tt;
  std::string _routine;
};

class TracePackScope {
 public:
  explicit TracePackScope(const char* routine) : _routine(routine), _index(-1) {
    TRACEPRINTF("start pack: %s", routine);
  }

  TracePackScope(const char* routine, int index)
      : _routine(routine), _index(index) {
    TRACEPRINTF("start pack: %s, index: %d", routine, index);
  }

  ~TracePackScope() {
    if (_index >= 0) {
      TRACEPRINTF("finish pack: %s, index: %d", _routine.c_str(), _index);
    } else {
      TRACEPRINTF("finish pack: %s", _routine.c_str());
    }
  }

 private:
  std::string _routine;
  int _index;
};

class TagFilter : public brpc::NamingServiceFilter {
 public:
  class TagHelper {
   public:
    explicit TagHelper(const std::string& kv_str) {
      if (kv_str.compare("") == 0) {
        return;
      }

      static const char TAG_DELIM = ',';
      static const char KV_DELIM = ':';

      std::string::size_type start_pos = 0;
      std::string::size_type end_pos;

      do {
        end_pos = kv_str.find(TAG_DELIM, start_pos);
        std::string kv_pair_str;
        if (end_pos == std::string::npos) {
          kv_pair_str = kv_str.substr(start_pos);
        } else {
          kv_pair_str = kv_str.substr(start_pos, end_pos - start_pos);
          start_pos = end_pos + 1;
        }

        std::string::size_type kv_delim_pos = kv_pair_str.find(KV_DELIM, 0);
        if (kv_delim_pos == std::string::npos) {
          LOG(ERROR) << "invalid kv pair: " << kv_pair_str.c_str();
          continue;
        }

        std::string key = kv_pair_str.substr(0, kv_delim_pos);
        std::string value = kv_pair_str.substr(kv_delim_pos + 1);
        _kv_map.insert(std::pair<std::string, std::string>(key, value));
      } while (end_pos != std::string::npos);
    }

    bool container(const std::string& k, const std::string& v) const {
      std::map<std::string, std::string>::const_iterator found =
          _kv_map.find(k);
      if (found == _kv_map.end()) {
        // key not found
        return false;
      }

      if (v.compare(found->second) != 0) {
        // value not equals
        return false;
      }
      return true;
    }

   private:
    std::map<std::string, std::string> _kv_map;
  };

  TagFilter(const std::string& key, const std::string& val) {
    _key = key;
    _value = val;
  }

  bool Accept(const brpc::ServerNode& server) const {
    TagHelper helper(server.tag);
    return helper.container(_key, _value);
  }

 private:
  std::string _key;
  std::string _value;
};

class BvarWrapper {
 public:
  virtual void update_latency(int64_t acc) = 0;
  virtual void update_average(int64_t acc) = 0;
};

class LatencyWrapper : public BvarWrapper {
 public:
  explicit LatencyWrapper(const std::string& name) : _ltc(name + "_ltc") {}

  void update_latency(int64_t acc) { _ltc << acc; }

  void update_average(int64_t acc) {
    LOG(ERROR) << "Cannot update average to a LatencyRecorder";
  }

 private:
  bvar::LatencyRecorder _ltc;
};

class AverageWrapper : public BvarWrapper {
 public:
  explicit AverageWrapper(const std::string& name)
      : _win(name + "_avg", &_avg, ::bvar::FLAGS_bvar_dump_interval) {}

  void update_latency(int64_t acc) {
    LOG(ERROR) << "Cannot update latency to a AverageWrapper";
  }

  void update_average(int64_t acc) { _avg << acc; }

 private:
  bvar::IntRecorder _avg;
  bvar::Window<bvar::IntRecorder> _win;
};

struct StubTLS {
  StubTLS() {
    predictor_pools.clear();
    request_pools.clear();
    response_pools.clear();
  }

  std::vector<Predictor*> predictor_pools;
  std::vector<google::protobuf::Message*> request_pools;
  std::vector<google::protobuf::Message*> response_pools;
};

template <typename T, typename C, typename R, typename I, typename O>
class StubImpl : public Stub {
 public:
  typedef google::protobuf::Message Message;

  StubImpl()
      : _channel(NULL),
        _pchannel(NULL),
        _gchannel(NULL),
        _service_stub(NULL),
        _infer(NULL),
        _debug(NULL) {}
  ~StubImpl() {}

  int initialize(const VariantInfo& var,
                 const std::string& ep,
                 const std::string* tag,
                 const std::string* tag_value);

  Predictor* fetch_predictor();
  int return_predictor(Predictor* predictor);
  int return_predictor(Predictor* predictor) const;

  Message* fetch_request();
  int return_request(Message* request);
  int return_request(Message* request) const;

  Message* fetch_response();
  int return_response(Message* response);
  int return_response(Message* response) const;

  int thrd_initialize();
  int thrd_clear();
  int thrd_finalize();

  const std::string& which_endpoint() const { return _endpoint; }

 private:
  google::protobuf::RpcChannel* init_channel(
      const VariantInfo& var, brpc::NamingServiceFilter* filter = NULL);

  brpc::ParallelChannel* init_pchannel(brpc::Channel* sub_channel,
                                       uint32_t channel_count,
                                       uint32_t package_size,
                                       const brpc::ChannelOptions& options);

  StubTLS* get_tls() {
    return static_cast<StubTLS*>(THREAD_GETSPECIFIC(_bthread_key));
  }

 private:
  brpc::Channel* _channel;
  brpc::ParallelChannel* _pchannel;
  google::protobuf::RpcChannel* _gchannel;
  T* _service_stub;
  const google::protobuf::MethodDescriptor* _infer;
  const google::protobuf::MethodDescriptor* _debug;
  std::string _endpoint;
  RpcParameters _options;
  std::string _tag;
  uint32_t _max_channel;
  uint32_t _package_size;

  // tls handlers
  // bthread_key_t _bthread_key;
  THREAD_KEY_T _bthread_key;

  // bvar variables
  std::map<std::string, BvarWrapper*> _ltc_bvars;
  std::map<std::string, BvarWrapper*> _avg_bvars;
  mutable butil::Mutex _bvar_mutex;

#ifndef DECLARE_LATENCY
#define DECLARE_LATENCY(item) LatencyWrapper* _ltc_##item;
#endif

  DECLARE_LATENCY(infer_sync);    // 同步请求
  DECLARE_LATENCY(infer_async);   // 异步请求
  DECLARE_LATENCY(infer_send);    // 半同步send
  DECLARE_LATENCY(infer_recv);    // 半同步recv
  DECLARE_LATENCY(infer_cancel);  // 半同步cancel
  DECLARE_LATENCY(debug);         // 调试请求
  DECLARE_LATENCY(rpc_init);      // rpc reset
  DECLARE_LATENCY(thrd_clear);    // thrd clear
  DECLARE_LATENCY(pack_map);      // thrd clear
  DECLARE_LATENCY(pack_merge);    // thrd clear

#undef DECLARE_LATENCY

#ifndef DECLARE_AVERAGE
#define DECLARE_AVERAGE(item) AverageWrapper* _avg_##item;
#endif

  DECLARE_AVERAGE(failure);    // 失败请求数
  DECLARE_AVERAGE(item_size);  // 单次请求item数
  DECLARE_AVERAGE(pack);       // 单次请求分包数
  DECLARE_AVERAGE(pack_fail);  // 单次请求分包失败数

#undef DECLARE_AVERAGE

 public:
  void update_average(int64_t acc, const char* name) {
    std::map<std::string, BvarWrapper*>::iterator iter =
        _avg_bvars.find(std::string(AVG_PREFIX) + name);
    if (iter == _avg_bvars.end()) {
      LOG(ERROR) << "Not found average record:avg_" << name;
      return;
    }

    iter->second->update_average(acc);
  }

  void update_latency(int64_t acc, const char* name) {
    std::map<std::string, BvarWrapper*>::iterator iter =
        _ltc_bvars.find(std::string(LTC_PREFIX) + name);
    if (iter == _ltc_bvars.end()) {
      LOG(ERROR) << "Not found latency record:ltc_" << name;
      return;
    }

    iter->second->update_latency(acc);
  }
};

}  // namespace sdk_cpp
}  // namespace paddle_serving
}  // namespace baidu

#include "stub_impl.hpp"
