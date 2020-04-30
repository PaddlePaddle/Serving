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
#include <string>
namespace baidu {
namespace paddle_serving {
namespace sdk_cpp {
#ifdef BCLOUD
namespace butil = base;
#endif

template <typename T, typename C, typename R, typename I, typename O>
int StubImpl<T, C, R, I, O>::initialize(const VariantInfo& var,
                                        const std::string& ep,
                                        const std::string* tag,
                                        const std::string* tag_value) {
  if (tag != NULL && tag_value != NULL) {
    TagFilter* filter = new (std::nothrow) TagFilter(*tag, *tag_value);
    if (!filter) {
      LOG(FATAL) << "Failed create tag filter, key: " << tag
                 << ", value: " << tag_value;
      return -1;
    }

    _gchannel = init_channel(var, filter);
    VLOG(2) << "Create stub with tag: " << *tag << ", " << *tag_value
            << ", ep: " << ep;
  } else {
    _gchannel = init_channel(var, NULL);
    VLOG(2) << "Create stub without tag, ep " << ep;
  }

  if (!_gchannel) {
    LOG(FATAL) << "Failed init channel via var_info";
    return -1;
  }

  _service_stub = new (std::nothrow) T(_gchannel);
  if (!_service_stub) {
    LOG(FATAL) << "Failed create stub with channel";
    return -1;
  }

  _infer =
      _service_stub->GetDescriptor()->FindMethodByName(INFERENCE_METHOD_NAME);
  if (!_infer) {
    LOG(FATAL) << "Failed get inference method, "
               << "method name: " << INFERENCE_METHOD_NAME;
    return -1;
  }

  _debug = _service_stub->GetDescriptor()->FindMethodByName(DEBUG_METHOD_NAME);
  if (!_debug) {
    LOG(FATAL) << "Failed get debug method, "
               << "method name: " << DEBUG_METHOD_NAME;
    return -1;
  }

  _endpoint = ep;

  if (THREAD_KEY_CREATE(&_bthread_key, NULL) != 0) {
    LOG(FATAL) << "Failed create key for stub tls";
    return -1;
  }

  const std::string& name = _endpoint + "_" +
                            _service_stub->GetDescriptor()->full_name() + "_" +
                            _tag;

  _ltc_bvars.clear();
  _avg_bvars.clear();
  BAIDU_SCOPED_LOCK(_bvar_mutex);

#ifndef DEFINE_LATENCY
#define DEFINE_LATENCY(item)                                               \
  do {                                                                     \
    _ltc_##item = new (std::nothrow) LatencyWrapper(name + "_" #item);     \
    if (!_ltc_##item) {                                                    \
      LOG(FATAL) << "Failed create latency recorder:" << name + "_" #item; \
      return -1;                                                           \
    }                                                                      \
    _ltc_bvars["ltc_" #item] = _ltc_##item;                                \
  } while (0)
#endif

  DEFINE_LATENCY(infer_sync);
  DEFINE_LATENCY(infer_async);
  DEFINE_LATENCY(infer_send);
  DEFINE_LATENCY(infer_recv);
  DEFINE_LATENCY(infer_cancel);
  DEFINE_LATENCY(debug);
  DEFINE_LATENCY(rpc_init);
  DEFINE_LATENCY(thrd_clear);
  DEFINE_LATENCY(pack_map);
  DEFINE_LATENCY(pack_merge);

#undef DEFINE_LATENCY

#ifndef DEFINE_AVERAGE
#define DEFINE_AVERAGE(item)                                               \
  do {                                                                     \
    _avg_##item = new (std::nothrow) AverageWrapper(name + "_" #item);     \
    if (!_avg_##item) {                                                    \
      LOG(FATAL) << "Failed create average recorder:" << name + "_" #item; \
      return -1;                                                           \
    }                                                                      \
    _avg_bvars["avg_" #item] = _avg_##item;                                \
  } while (0)
#endif

  DEFINE_AVERAGE(failure);
  DEFINE_AVERAGE(pack);
  DEFINE_AVERAGE(item_size);
  DEFINE_AVERAGE(pack_fail);

#undef DEFINE_AVERAGE

  return 0;
}

template <typename T, typename C, typename R, typename I, typename O>
int StubImpl<T, C, R, I, O>::thrd_initialize() {
  if (THREAD_GETSPECIFIC(_bthread_key) != NULL) {
    // Because gPRC is uesd, this function may be called
    // multiple times, so the warning will be commented out
    // temporarily. See PR:#483 and PR:#500 for details.
    // LOG(WARNING) << "Already thread initialized for stub";
    return 0;
  }

  StubTLS* tls = new (std::nothrow) StubTLS();
  if (!tls || THREAD_SETSPECIFIC(_bthread_key, tls) != 0) {
    LOG(FATAL) << "Failed binding tls data to bthread_key";
    return -1;
  }

  VLOG(2) << "Succ thread initialize stub impl!";

  return 0;
}

template <typename T, typename C, typename R, typename I, typename O>
int StubImpl<T, C, R, I, O>::thrd_clear() {
  MetricScope metric(this, "thrd_clear");
  StubTLS* tls = get_tls();
  if (!tls) {
    LOG(FATAL) << "Failed get tls stub object";
    return -1;
  }

  // clear predictor
  size_t ps = tls->predictor_pools.size();
  for (size_t pi = 0; pi < ps; ++pi) {
    Predictor* p = tls->predictor_pools[pi];
    if (p && p->is_inited() && return_predictor(p) != 0) {
      LOG(FATAL) << "Failed return predictor: " << pi;
      return -1;
    }
  }
  tls->predictor_pools.clear();

  // clear request
  size_t is = tls->request_pools.size();
  for (size_t ii = 0; ii < is; ++ii) {
    if (return_request(tls->request_pools[ii]) != 0) {
      LOG(FATAL) << "Failed return request: " << ii;
      return -1;
    }
  }
  tls->request_pools.clear();

  // clear response
  size_t os = tls->response_pools.size();
  for (size_t oi = 0; oi < os; ++oi) {
    if (return_response(tls->response_pools[oi]) != 0) {
      LOG(FATAL) << "Failed return response: " << oi;
      return -1;
    }
  }
  tls->response_pools.clear();
  return 0;
}

template <typename T, typename C, typename R, typename I, typename O>
int StubImpl<T, C, R, I, O>::thrd_finalize() {
  StubTLS* tls = get_tls();
  if (!tls || thrd_clear() != 0) {
    LOG(FATAL) << "Failed clreate tls in thrd finalize";
    return -1;
  }

  delete tls;
  return 0;
}

template <typename T, typename C, typename R, typename I, typename O>
Predictor* StubImpl<T, C, R, I, O>::fetch_predictor() {
  StubTLS* tls = get_tls();
  if (!tls) {
    LOG(FATAL) << "Failed get tls data when fetching predictor";
    return NULL;
  }

  PredictorImpl<T>* predictor = butil::get_object<PredictorImpl<T>>();
  if (!predictor) {
    LOG(FATAL) << "Failed fetch predictor";
    return NULL;
  }

  if (predictor->init(
          _gchannel, _service_stub, _infer, _debug, _options, this, _tag) !=
      0) {
    LOG(FATAL) << "Failed init fetched predictor";
    return NULL;
  }

  tls->predictor_pools.push_back(predictor);
  return predictor;
}

template <typename T, typename C, typename R, typename I, typename O>
int StubImpl<T, C, R, I, O>::return_predictor(Predictor* predictor) {
  if ((dynamic_cast<PredictorImpl<T>*>(predictor))->deinit() != 0) {
    LOG(FATAL) << "Failed deinit fetched predictor";
    return -1;
  }
  butil::return_object(dynamic_cast<PredictorImpl<T>*>(predictor));
  return 0;
}

template <typename T, typename C, typename R, typename I, typename O>
int StubImpl<T, C, R, I, O>::return_predictor(Predictor* predictor) const {
  if ((dynamic_cast<PredictorImpl<T>*>(predictor))->deinit() != 0) {
    LOG(FATAL) << "Failed deinit fetched predictor";
    return -1;
  }
  butil::return_object(dynamic_cast<PredictorImpl<T>*>(predictor));
  return 0;
}

template <typename T, typename C, typename R, typename I, typename O>
google::protobuf::Message* StubImpl<T, C, R, I, O>::fetch_request() {
  StubTLS* tls = get_tls();
  if (!tls) {
    LOG(FATAL) << "Failed get tls data when fetching request";
    return NULL;
  }

  I* req = butil::get_object<I>();
  if (!req) {
    LOG(FATAL) << "Failed get tls request item, type: " << typeid(I).name();
    return NULL;
  }

  req->Clear();
  tls->request_pools.push_back(req);
  return req;
}

template <typename T, typename C, typename R, typename I, typename O>
int StubImpl<T, C, R, I, O>::return_request(
    google::protobuf::Message* request) const {
  request->Clear();
  butil::return_object(dynamic_cast<I*>(request));
  return 0;
}

template <typename T, typename C, typename R, typename I, typename O>
int StubImpl<T, C, R, I, O>::return_request(
    google::protobuf::Message* request) {
  request->Clear();
  butil::return_object(dynamic_cast<I*>(request));
  return 0;
}

template <typename T, typename C, typename R, typename I, typename O>
google::protobuf::Message* StubImpl<T, C, R, I, O>::fetch_response() {
  StubTLS* tls = get_tls();
  if (!tls) {
    LOG(FATAL) << "Failed get tls data when fetching response";
    return NULL;
  }

  O* res = butil::get_object<O>();
  if (!res) {
    LOG(FATAL) << "Failed get tls response item, type: " << typeid(O).name();
    return NULL;
  }

  res->Clear();
  tls->response_pools.push_back(res);
  return res;
}

template <typename T, typename C, typename R, typename I, typename O>
int StubImpl<T, C, R, I, O>::return_response(
    google::protobuf::Message* response) const {
  response->Clear();
  butil::return_object(dynamic_cast<O*>(response));
  return 0;
}

template <typename T, typename C, typename R, typename I, typename O>
int StubImpl<T, C, R, I, O>::return_response(
    google::protobuf::Message* response) {
  response->Clear();
  butil::return_object(dynamic_cast<O*>(response));
  return 0;
}

template <typename T, typename C, typename R, typename I, typename O>
google::protobuf::RpcChannel* StubImpl<T, C, R, I, O>::init_channel(
    const VariantInfo& var, brpc::NamingServiceFilter* filter) {
  brpc::ChannelOptions chn_options;
  chn_options.ns_filter = filter;

  // parameters
  ASSIGN_CONF_ITEM(chn_options.protocol, var.parameters.protocol, NULL);
  ASSIGN_CONF_ITEM(_tag, var.parameters.route_tag, NULL);
  ASSIGN_CONF_ITEM(_max_channel, var.parameters.max_channel, NULL);
  ASSIGN_CONF_ITEM(_package_size, var.parameters.package_size, NULL);

  if (_max_channel < 1) {
    LOG(ERROR) << "Invalid MaxChannelPerRequest: " << _max_channel;
    return NULL;
  }

  // connection
  ASSIGN_CONF_ITEM(chn_options.max_retry, var.connection.cnt_retry_conn, NULL);
  ASSIGN_CONF_ITEM(
      chn_options.connect_timeout_ms, var.connection.tmo_conn, NULL);
  ASSIGN_CONF_ITEM(chn_options.timeout_ms, var.connection.tmo_rpc, NULL);
  ASSIGN_CONF_ITEM(
      chn_options.backup_request_ms, var.connection.tmo_hedge, NULL);

  // connection type
  std::string conn_type_str;
  ASSIGN_CONF_ITEM(conn_type_str, var.connection.type_conn, NULL);
  chn_options.connection_type = brpc::StringToConnectionType(conn_type_str);

  // naminginfo
  std::string cluster_naming_info;
  std::string cluster_loadbalancer;
  ASSIGN_CONF_ITEM(cluster_naming_info, var.naminginfo.cluster_naming, NULL);
  ASSIGN_CONF_ITEM(cluster_loadbalancer, var.naminginfo.load_balancer, NULL);

  // brpc single channel
  _channel = butil::get_object<brpc::Channel>();
  if (!_channel) {
    LOG(FATAL) << "Failed get channel object from butil::pool";
    return NULL;
  }

  if (_channel->Init(cluster_naming_info.c_str(),
                     cluster_loadbalancer.c_str(),
                     &chn_options) != 0) {
    LOG(ERROR) << "Failed to initialize channel, path: " << cluster_naming_info;
    return NULL;
  }

  // brpc parallel channel
  _pchannel = init_pchannel(_channel, _max_channel, _package_size, chn_options);
  if (_pchannel) {
    VLOG(2) << "Succ create parallel channel, count: " << _max_channel;
    return _pchannel;
  }

  return _channel;
}

template <typename T, typename C, typename R, typename I, typename O>
brpc::ParallelChannel* StubImpl<T, C, R, I, O>::init_pchannel(
    brpc::Channel* sub_channel,
    uint32_t channel_count,
    uint32_t package_size,
    const brpc::ChannelOptions& options) {
  if (channel_count <= 1) {  // noneed use parallel channel
    VLOG(2) << "channel count <= 1, noneed use pchannel.";
    return NULL;
  }

  _pchannel = butil::get_object<brpc::ParallelChannel>();
  if (!_pchannel) {
    VLOG(2) << "Failed get pchannel from object pool";
    return NULL;
  }

  brpc::ParallelChannelOptions pchan_options;
  pchan_options.timeout_ms = options.timeout_ms;
  if (_pchannel->Init(&pchan_options) != 0) {
    VLOG(2) << "Failed init parallel channel with tmo_us: "
            << pchan_options.timeout_ms;
    return NULL;
  }

  for (uint32_t si = 0; si < channel_count; ++si) {
    if (_pchannel->AddChannel(sub_channel,
                              brpc::DOESNT_OWN_CHANNEL,
                              new C(package_size, this),
                              new R(package_size, this)) != 0) {
      LOG(FATAL) << "Failed add channel at: " << si
                 << ", package_size:" << package_size;
      return NULL;
    }
  }

  return _pchannel;
}

}  // namespace sdk_cpp
}  // namespace paddle_serving
}  // namespace baidu
