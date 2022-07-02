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
#include "core/predictor/common/inner_common.h"
#include "core/predictor/framework/channel.h"
#include "core/predictor/framework/dag.h"
#include "core/predictor/op/op.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

// class Op;

struct ViewNode {
  Op* op;  // op->full_name == service_workflow_stageindex_opname
  DagNode* conf;
  void reset() {
    op = NULL;
    conf = NULL;
  }
};

struct ViewStage {
  std::vector<ViewNode*> nodes;
  std::string full_name;  // service_workflow_stageindex
  std::string debug() { return "TOBE IMPLEMENTED!"; }
};

class DagView {
 public:
  DagView() : _bus(NULL) { _view.clear(); }

  ~DagView() {}

  int init(Dag* dag, const std::string& service_name, const uint64_t log_id);

  int deinit();

  int execute(const uint64_t log_id, butil::IOBufBuilder* debug_os);

  // The default execution strategy is in sequencing
  // You can derive a subclass to implement this func.
  // ParallelDagView maybe the one you want.
  virtual int execute_one_stage(ViewStage* vstage,
                                const uint64_t log_id,
                                butil::IOBufBuilder* debug_os);

  int set_request_channel(Channel& request, const uint64_t log_id);  // NOLINT

  const Channel* get_response_channel(const uint64_t log_id) const;

  const std::string& name() const { return _name; }

  const std::string& full_name() const { return _full_name; }

 private:
  std::string _name;
  std::string _full_name;
  std::vector<ViewStage*> _view;
  Bus* _bus;
};

struct Args {
  Op* _op;
  uint64_t _log_id;
  bool _debug;
  int errcode;
};

// The derived DagView supports parallel execution
// strategy, by implments the execute_one_stage().
class ParallelDagView : public DagView {
 public:
  virtual int execute_one_stage(ViewStage* vstage,
                                const uint64_t log_id,
                                butil::IOBufBuilder* debug_os);
};

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
