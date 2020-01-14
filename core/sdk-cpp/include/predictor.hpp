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

class MetricScope;
class Stub;
template <typename T, typename C, typename R, typename I, typename O>
class StubImpl;

template <typename Arg1, typename Arg2>
inline ::google::protobuf::Closure* NewClosure(void (*function)(Arg1*, Arg2*),
                                               Arg1* arg1 = NULL,
                                               Arg2* arg2 = NULL) {
  FunctionClosure<Arg1, Arg2>* closure =
      butil::get_object<FunctionClosure<Arg1, Arg2>>();

  if (closure) {
    if (closure->init(function, true, false, true, arg1, arg2) != 0) {
      LOG(FATAL) << "Failed create closure objects";
      return NULL;
    }
  }

  return closure;
}

template <typename Arg1, typename Arg2>
int FunctionClosure<Arg1, Arg2>::init(FunctionType function,
                                      bool self_deleting,
                                      bool arg1_deleting,
                                      bool arg2_deleting,
                                      Arg1* arg1,
                                      Arg2* arg2) {
  _function = function;
  _self_deleting = self_deleting;
  _arg1_deleting = arg1_deleting;
  _arg2_deleting = arg2_deleting;

  if (arg2 == NULL) {
    GET_OBJECT_FROM_POOL(_arg2, Arg2, -1);
    _arg2_deleting = true;
  }

  return 0;
}

template <typename Arg1, typename Arg2>
void FunctionClosure<Arg1, Arg2>::Run() {
  bool self_delete = _self_deleting;
  bool arg1_delete = _arg1_deleting;
  bool arg2_delete = _arg2_deleting;

  _function(_arg1, _arg2);

  if (self_delete) {
    butil::return_object(this);
  }

  if (arg2_delete) {
    butil::return_object(_arg2);
  }
}

template <typename T>
int PredictorImpl<T>::init(google::protobuf::RpcChannel* chnl,
                           T* service,
                           const MethodDescriptor* infer,
                           const MethodDescriptor* debug,
                           const RpcParameters& options,
                           Stub* stub,
                           const std::string& tag) {
  MetricScope metric(stub, "rpc_init");
  butil::Timer tt(butil::Timer::STARTED);
  _service = service;
  _channel = chnl;
  _infer = infer;
  _debug = debug;
  _options = options;
  _stub = stub;
  _tag = tag;
  reset(_options, _cntl);
  _inited = true;
  return 0;
}

template <typename T>
int PredictorImpl<T>::reset(const RpcParameters& options,
                            brpc::Controller& cntl) {  // NOLINT
  cntl.Reset();
  if (options.compress_type.init) {
    cntl.set_request_compress_type(compress_types[options.compress_type.value]);
  }
  return 0;
}

template <typename T>
int PredictorImpl<T>::deinit() {
  // do nothing
  _inited = false;
  return 0;
}

template <typename T>
int PredictorImpl<T>::inference(google::protobuf::Message* req,
                                google::protobuf::Message* res) {
  MetricScope metric(_stub, "infer_sync");
  _service->CallMethod(_infer, &_cntl, req, res, NULL);
  if (_cntl.Failed()) {
    LOG(WARNING) << "inference call failed, message: " << _cntl.ErrorText();
    _stub->update_average(1, "failure");
    return -1;
  }
  return 0;
}

template <typename T>
int PredictorImpl<T>::inference(google::protobuf::Message* req,
                                google::protobuf::Message* res,
                                DoneType done,
                                brpc::CallId* cid) {
  MetricScope metric(_stub, "infer_async");
  // 异步接口不能使用当前predictor的controller成员，而应该
  // 在对象池临时申请一个独立的对象，且直到异步回调执行完
  // 成后才能释放，而该释放行为被NewClosure自动托管，用户
  // 无需关注。
  brpc::Controller* cntl = butil::get_object<brpc::Controller>();
  if (!cntl || reset(_options, *cntl) != 0) {
    LOG(FATAL) << "Failed get controller from object pool,"
               << "cntl is null: " << (cntl == NULL);
    _stub->update_average(1, "failure");
    return -1;
  }

  if (cid != NULL) {  // you can join this rpc with cid
    *cid = cntl->call_id();
  }

  _service->CallMethod(_infer, cntl, req, res, NewClosure(done, res, cntl));
  return 0;
}

template <typename T>
int PredictorImpl<T>::debug(google::protobuf::Message* req,
                            google::protobuf::Message* res,
                            butil::IOBufBuilder* debug_os) {
  MetricScope metric(_stub, "debug");
  _service->CallMethod(_debug, &_cntl, req, res, NULL);
  if (_cntl.Failed()) {
    LOG(WARNING) << "inference call failed, message: " << _cntl.ErrorText();
    _stub->update_average(1, "failure");
    return -1;
  }

  // copy debug info from response attachment
  (*debug_os) << _cntl.response_attachment();
  return 0;
}

template <typename T>
int PredictorImpl<T>::send_inference(google::protobuf::Message* req,
                                     google::protobuf::Message* res) {
  MetricScope metric(_stub, "infer_send");
  _inferid = _cntl.call_id();
  _service->CallMethod(_infer, &_cntl, req, res, brpc::DoNothing());
  return 0;
}

template <typename T>
int PredictorImpl<T>::recv_inference() {
  // waiting for callback done
  MetricScope metric(_stub, "infer_recv");
  brpc::Join(_inferid);
  if (_cntl.Failed()) {
    LOG(WARNING) << "Failed recv response from rpc"
                 << ", err: " << _cntl.ErrorText();
    _stub->update_average(1, "failure");
    return -1;
  }
  return 0;
}

template <typename T>
void PredictorImpl<T>::cancel_inference() {
  MetricScope metric(_stub, "infer_cancel");
  brpc::StartCancel(_inferid);
}

template <typename T>
const char* PredictorImpl<T>::tag() {
  return _tag.c_str();
}

}  // namespace sdk_cpp
}  // namespace paddle_serving
}  // namespace baidu
