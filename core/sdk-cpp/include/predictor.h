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
#include <vector>
#include "core/sdk-cpp/include/common.h"
#include "core/sdk-cpp/include/endpoint_config.h"
#include "core/sdk-cpp/include/stub.h"

namespace baidu {
namespace paddle_serving {
namespace sdk_cpp {

#define GET_OBJECT_FROM_POOL(param, T, err)               \
  do {                                                    \
    param = butil::get_object<T>();                       \
    if (!param) {                                         \
      LOG(ERROR) << "Failed get object from pool"         \
                 << ", arg:" << #param << "type: " << #T; \
      return err;                                         \
    }                                                     \
  } while (0)

static const brpc::CompressType compress_types[] = {brpc::COMPRESS_TYPE_NONE,
                                                    brpc::COMPRESS_TYPE_SNAPPY,
                                                    brpc::COMPRESS_TYPE_GZIP,
                                                    brpc::COMPRESS_TYPE_ZLIB,
                                                    brpc::COMPRESS_TYPE_LZ4};

typedef void (*DoneType)(google::protobuf::Message* res,
                         brpc::Controller* controller);

template <typename Arg1, typename Arg2>
class FunctionClosure : public ::google::protobuf::Closure {
 public:
  typedef void (*FunctionType)(Arg1* arg1, Arg2* arg2);

  FunctionClosure() {}

  ~FunctionClosure() {}

  int init(FunctionType function,
           bool self_deleting,
           bool arg1_deleting,
           bool arg2_deleting,
           Arg1* arg1 = NULL,
           Arg2* arg2 = NULL);

  void Run();

 private:
  FunctionType _function;
  Arg1* _arg1;
  Arg2* _arg2;
  bool _self_deleting;
  bool _arg1_deleting;
  bool _arg2_deleting;
};

class InterfaceAdaptor {
 public:
  typedef google::protobuf::Message RequestMessage;
  typedef google::protobuf::Message ResponseMessage;

  virtual int partition(RequestMessage& request,                 // NOLINT
                        std::vector<RequestMessage*>& out) = 0;  // NOLINT
  virtual int merge(std::vector<ResponseMessage*>& response,     // NOLINT
                    ResponseMessage& out) = 0;                   // NOLINT
};

class EchoAdaptor : public InterfaceAdaptor {
 public:
  typedef google::protobuf::Message RequestMessage;
  typedef google::protobuf::Message ResponseMessage;

  int partition(RequestMessage& request,              // NOLINT
                std::vector<RequestMessage*>& out) {  // NOLINT
    return 0;
  }

  int merge(std::vector<ResponseMessage*>& response,  // NOLINT
            ResponseMessage*& out) {                  // NOLINT
    return 0;
  }
};

class Predictor {
 public:
  // synchronize interface
  virtual int inference(google::protobuf::Message* req,
                        google::protobuf::Message* res) = 0;

  // asynchronize interface
  virtual int inference(google::protobuf::Message* req,
                        google::protobuf::Message* res,
                        DoneType done,
                        brpc::CallId* cid = NULL) = 0;

  // synchronize interface
  virtual int debug(google::protobuf::Message* req,
                    google::protobuf::Message* res,
                    butil::IOBufBuilder* debug_os) = 0;

  // un-blocked interface
  virtual int send_inference(google::protobuf::Message* req,
                             google::protobuf::Message* res) = 0;
  virtual int recv_inference() = 0;
  virtual void cancel_inference() = 0;

  virtual const char* tag() = 0;

  virtual const google::protobuf::Service* service() = 0;

  virtual const brpc::Controller* controller() = 0;

  virtual const google::protobuf::RpcChannel* channel() = 0;

  virtual const Stub* stub() = 0;

  virtual bool is_inited() = 0;
};

template <typename T>
class PredictorImpl : public Predictor {
 public:
  typedef google::protobuf::MethodDescriptor MethodDescriptor;

  PredictorImpl()
      : _service(NULL),
        _stub(NULL),
        _infer(NULL),
        _debug(NULL),
        _channel(NULL),
        _inited(false) {
    // _inferid = 0;
  }

  ~PredictorImpl() {}

  int init(google::protobuf::RpcChannel* chnl,
           T* service,
           const MethodDescriptor* infer,
           const MethodDescriptor* debug,
           const RpcParameters& options,
           Stub* stub,
           const std::string& tag);

  int reset(const RpcParameters& options, brpc::Controller& cntl);  // NOLINT

  int deinit();

  bool is_inited() { return _inited; }

  // 同步接口
  int inference(google::protobuf::Message* req, google::protobuf::Message* res);

  // 异步接口
  int inference(google::protobuf::Message* req,
                google::protobuf::Message* res,
                DoneType done,
                brpc::CallId* cid = NULL);

  // Debug同步接口
  int debug(google::protobuf::Message* req,
            google::protobuf::Message* res,
            butil::IOBufBuilder* debug_os);

  // 半同步(非阻塞)接口
  int send_inference(google::protobuf::Message* req,
                     google::protobuf::Message* res);

  // 半同步(非阻塞)接口
  int recv_inference();

  // 半同步(非阻塞)接口
  void cancel_inference();

  const char* tag();

  const google::protobuf::Service* service() { return _service; }

  const brpc::Controller* controller() { return &_cntl; }

  const google::protobuf::RpcChannel* channel() { return _channel; }

  const Stub* stub() { return _stub; }

 private:
  T* _service;
  Stub* _stub;
  const MethodDescriptor* _infer;
  const MethodDescriptor* _debug;
  google::protobuf::RpcChannel* _channel;
  brpc::Controller _cntl;
  brpc::CallId _inferid;
  RpcParameters _options;
  std::string _tag;
  bool _inited;
};

}  // namespace sdk_cpp
}  // namespace paddle_serving
}  // namespace baidu

#include "predictor.hpp"
