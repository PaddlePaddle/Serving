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
#include <utility>
#include <vector>
#include "core/predictor/common/inner_common.h"
#include "core/predictor/framework/merger.h"
#include "core/predictor/framework/workflow.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

class InferService {
 public:
  typedef OpChannel<google::protobuf::Message> BuiltinChannel;

  static const char* tag() { return "service"; }

  InferService()
      : _last_change_timestamp(0),
        _enable_map_request_to_workflow(false),
        _request_field_key(""),
        _merger(NULL) {
    _flows.clear();
    _request_to_workflow_map.clear();
  }

  int init(const configure::InferService& conf);

  int deinit() { return 0; }

  int reload();

  const std::string& name() const;

  const std::string& full_name() const { return _infer_service_format; }

  // Execute each workflow serially
  virtual int inference(const google::protobuf::Message* request,
                        google::protobuf::Message* response,
                        const uint64_t log_id,
                        butil::IOBufBuilder* debug_os = NULL);

  int debug(const google::protobuf::Message* request,
            google::protobuf::Message* response,
            const uint64_t log_id,
            butil::IOBufBuilder* debug_os);

  int execute_one_workflow(uint32_t index,
                           const google::protobuf::Message* request,
                           google::protobuf::Message* response,
                           const uint64_t log_id,
                           butil::IOBufBuilder* debug_os);

 private:
  int _execute_workflow(Workflow* workflow,
                        const google::protobuf::Message* request,
                        google::protobuf::Message* response,
                        const uint64_t log_id,
                        butil::IOBufBuilder* debug_os);

  std::vector<Workflow*>* _map_request_to_workflow(
      const google::protobuf::Message* request, const uint64_t log_id);

 private:
  std::vector<Workflow*> _flows;
  std::string _infer_service_format;
  uint64_t _last_change_timestamp;
  bool _enable_map_request_to_workflow;
  std::string _request_field_key;
  ::butil::FlatMap<std::string, std::vector<Workflow*>>
      _request_to_workflow_map;
  IMerger* _merger;
};

class ParallelInferService : public InferService {
 public:
  // Execute workflows in parallel
  int inference(const google::protobuf::Message* request,
                google::protobuf::Message* response,
                const uint64_t log_id,
                butil::IOBufBuilder* debug_os) {
    return 0;
  }
};

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
