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

namespace baidu {
namespace paddle_serving {
namespace predictor {

enum EdgeMode { RO = 0, RW = 1, UNKNOWN };

struct DagNode {
  uint32_t id;
  uint32_t stage;
  std::string name;       // opname
  std::string full_name;  // workflow_stageindex_opname
  std::string type;
  std::vector<std::string> address;
  void* conf;
  boost::unordered_map<std::string, EdgeMode> depends;
};

struct DagStage {
  std::vector<DagNode*> nodes;
  std::string name;       // stageindex
  std::string full_name;  // workflow_stageindex
};

class Dag {
 public:
  Dag();

  virtual ~Dag();

  EdgeMode parse_mode(std::string& mode);  // NOLINT

  int init(const char* path, const char* file, const std::string& name);

  int init(const configure::Workflow& conf, const std::string& name);

  int deinit();

  uint32_t nodes_size();

  const DagNode* node_by_id(uint32_t id);

  const DagNode* node_by_id(uint32_t id) const;

  const DagNode* node_by_name(std::string& name);  // NOLINT

  const DagNode* node_by_name(const std::string& name) const;

  uint32_t stage_size();

  const DagStage* stage_by_index(uint32_t index);

  const std::string& name() const { return _dag_name; }

  const std::string& full_name() const { return _dag_name; }

  void regist_metric(const std::string& service_name);

 private:
  int topo_sort();

 private:
  std::string _dag_name;
  boost::unordered_map<std::string, DagNode*> _name_nodes;
  std::vector<DagNode*> _index_nodes;
  std::vector<DagStage*> _stages;
};

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
