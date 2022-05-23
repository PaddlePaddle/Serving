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

#include "core/predictor/framework/dag.h"
#include <string>
#include <utility>  // make_pair
#include <vector>
#include "core/predictor/common/inner_common.h"
#include "core/predictor/framework/predictor_metric.h"  // PredictorMetric
#include "core/predictor/op/op.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

Dag::Dag() {
  _index_nodes.clear();
  _name_nodes.clear();
  _stages.clear();
}

Dag::~Dag() { deinit(); }

int Dag::deinit() {
  for (std::vector<DagStage*>::iterator iter = _stages.begin();
       iter != _stages.end();
       ++iter) {
    if (*iter != NULL) {
      delete *iter;
    }
  }
  _stages.clear();

  for (std::vector<DagNode*>::iterator iter = _index_nodes.begin();
       iter != _index_nodes.end();
       ++iter) {
    DagNode* node = *iter;
    if (node != NULL) {
      void* conf = node->conf;
      if (conf != NULL) {
        Op* op = OpRepository::instance().get_op(node->type);
        if (op == NULL) {
          LOG(ERROR) << "Failed to get_op, op type[" << node->type << "]";
          return -1;
        }
        op->delete_config(conf);
        OpRepository::instance().return_op(node->type, op);
      }
      delete node;
    }
  }
  _index_nodes.clear();
  _name_nodes.clear();
  return 0;
}

EdgeMode Dag::parse_mode(std::string& mode) {
  if (mode == "RO") {
    return RO;
  } else if (mode == "RW") {
    return RW;
  } else {
    return UNKNOWN;
  }
}

// [@Node]
// name: preprocess
// type: ProcessorOp
// [.@Depend]
// name: StartupOp
// mode: RO
// [@Node]
// name: discret_extractor
// type: DiscretExtractOp
// [.@Depend]
// name: StartupOp
// mode: RO
// [.@Depend]
// name: preprocess
// mode: RW
// [@Node]
// name: dnn_inference
// type: PaddleV2InferenceOp
// [.@Depend]
// name: discret_extractor
// mode: RO
// [@Node]
// name: postprocess
// type: PostProcessOp
// [.@Depend]
// name: dnn_inference
// mode: RO
#if 0
int Dag::init(const char* path, const char* file, const std::string& name) {
    comcfg::Configure conf;
    if (conf.load(path, file) != 0) {
        LOG(ERROR) << "Failed load conf from"
            << path << "/" << file << " in dag: "
            << name;
        return ERR_INTERNAL_FAILURE;
    }

    return init(conf, name);
}
#endif

int Dag::init(const configure::Workflow& conf, const std::string& name) {
  _dag_name = name;
  _index_nodes.clear();
  _name_nodes.clear();
  for (uint32_t i = 0; i < conf.nodes_size(); i++) {
    DagNode* node = new (std::nothrow) DagNode();
    if (node == NULL) {
      LOG(ERROR) << "Failed create new dag node";
      return ERR_MEM_ALLOC_FAILURE;
    }
    node->id = i + 1;  // 0 is reserved for begginer-op
    node->name = conf.nodes(i).name();
    node->type = conf.nodes(i).type();
    for (int add_index = 0; add_index < conf.nodes(i).address_size();
         ++add_index) {
      node->address.push_back(conf.nodes(i).address(add_index));
    }
    uint32_t depend_size = conf.nodes(i).dependencies_size();
    for (uint32_t j = 0; j < depend_size; j++) {
      const configure::DAGNodeDependency& depend =
          conf.nodes(i).dependencies(j);
      std::string name = depend.name();
      std::string mode = depend.mode();
      node->depends.insert(std::make_pair(name, parse_mode(mode)));
    }
    Op* op = OpRepository::instance().get_op(node->type);
    if (op == NULL) {
      LOG(ERROR) << "Failed to get_op, op type[" << node->type << "]";
      return ERR_INTERNAL_FAILURE;
    }
    // node->conf could be NULL
    node->conf = op->create_config(conf.nodes(i));
    OpRepository::instance().return_op(node->type, op);
    _name_nodes.insert(std::make_pair(node->name, node));
    _index_nodes.push_back(node);
  }

  if (topo_sort() != 0) {
    LOG(ERROR) << "Topo sort dag[" << _dag_name << "] failed!";
    return ERR_INTERNAL_FAILURE;
  }

  if (FLAGS_el_log_level == 16) {
    LOG(INFO) << "DAG: " << _dag_name << ", Op Num: " << _index_nodes.size();
    for (uint32_t nid = 0; nid < _index_nodes.size(); nid++) {
      DagNode* node = _index_nodes[nid];
      LOG(INFO) << "OP-" << node->id << "-" << node->name << "-" << node->type
                << " depends: " << node->depends.size()
                << " address: " << node->address.size();

      boost::unordered_map<std::string, EdgeMode>::iterator it;
      for (it = node->depends.begin(); it != node->depends.end(); it++) {
        LOG(INFO) << " " << it->first << " " << it->second;
      }
    }
    LOG(INFO) << "";
  }

  return ERR_OK;
}

uint32_t Dag::nodes_size() { return _index_nodes.size(); }

const DagNode* Dag::node_by_id(uint32_t id) { return _index_nodes[id]; }

const DagNode* Dag::node_by_id(uint32_t id) const { return _index_nodes[id]; }

const DagNode* Dag::node_by_name(std::string& name) {
  return _name_nodes[name];
}

const DagNode* Dag::node_by_name(const std::string& name) const {
  boost::unordered_map<std::string, DagNode*>::const_iterator it;
  it = _name_nodes.find(name);
  if (it == _name_nodes.end()) {
    LOG(WARNING) << "Not found op by name:" << name;
    return NULL;
  }
  return it->second;
}

uint32_t Dag::stage_size() { return _stages.size(); }

const DagStage* Dag::stage_by_index(uint32_t index) { return _stages[index]; }

int Dag::topo_sort() {
  std::stringstream ss;
  uint32_t nodes_size = _index_nodes.size();
  std::vector<uint32_t> in_degree(nodes_size, 0);
  std::vector<std::vector<uint32_t>> in_egde(nodes_size);
  for (uint32_t nid = 0; nid < nodes_size; nid++) {
    in_degree[nid] += _index_nodes[nid]->depends.size();
    for (auto it = _index_nodes[nid]->depends.begin();
         it != _index_nodes[nid]->depends.end();
         ++it) {
      uint32_t pnid = Dag::node_by_name(it->first)->id -
                      1;  // 0 is reserved for begginer-op
      in_egde[pnid].push_back(nid);
    }
  }
  for (int i = 0; i < in_degree.size(); ++i) {
    VLOG(2) << "(" << _index_nodes[i]->name << ") in_degree[" << i
            << "]: " << in_degree[i];
  }
  int sorted_num = 0;
  DagStage* stage = new (std::nothrow) DagStage();
  if (stage == NULL) {
    LOG(ERROR) << "Invalid stage!";
    return ERR_MEM_ALLOC_FAILURE;
  }
  ss.str("");
  ss << _stages.size();
  stage->name = ss.str();
  stage->full_name = full_name() + NAME_DELIMITER + stage->name;
  for (uint32_t nid = 0; nid < nodes_size; ++nid) {
    if (in_degree[nid] == 0) {
      ++sorted_num;
      stage->nodes.push_back(_index_nodes[nid]);
      // assign stage number after stage created
      _index_nodes[nid]->stage = _stages.size();
      // assign dag node full name after stage created
      _index_nodes[nid]->full_name =
          stage->full_name + NAME_DELIMITER + _index_nodes[nid]->name;
    }
  }

  if (stage->nodes.size() == 0) {
    LOG(ERROR) << "Invalid Dag!";
    return ERR_INTERNAL_FAILURE;
  }
  _stages.push_back(stage);

  while (sorted_num < nodes_size) {
    auto pre_nodes = _stages.back()->nodes;
    DagStage* stage = new (std::nothrow) DagStage();
    ss.str("");
    ss << _stages.size();
    stage->name = ss.str();
    stage->full_name = full_name() + NAME_DELIMITER + stage->name;
    for (uint32_t pi = 0; pi < pre_nodes.size(); ++pi) {
      uint32_t pnid = pre_nodes[pi]->id - 1;
      for (uint32_t ei = 0; ei < in_egde[pnid].size(); ++ei) {
        uint32_t nid = in_egde[pnid][ei];
        --in_degree[nid];
        if (in_degree[nid] == 0) {
          ++sorted_num;
          stage->nodes.push_back(_index_nodes[nid]);
          // assign stage number after stage created
          _index_nodes[nid]->stage = _stages.size();
          // assign dag node full name after stage created
          _index_nodes[nid]->full_name =
              stage->full_name + NAME_DELIMITER + _index_nodes[nid]->name;
        }
      }
    }
    if (stage->nodes.size() == 0) {
      LOG(ERROR) << "Invalid Dag!";
      return ERR_INTERNAL_FAILURE;
    }
    _stages.push_back(stage);
  }

  return ERR_OK;
}

void Dag::regist_metric(const std::string& service_name) {
  for (int stage_idx = 0; stage_idx < _stages.size(); ++stage_idx) {
    DagStage* stage = _stages[stage_idx];
    PredictorMetric::GetInstance()->regist_latency_metric(
        STAGE_METRIC_PREFIX + service_name + NAME_DELIMITER + stage->full_name);
    for (int node_idx = 0; node_idx < stage->nodes.size(); ++node_idx) {
      DagNode* node = stage->nodes[node_idx];
      PredictorMetric::GetInstance()->regist_latency_metric(
          OP_METRIC_PREFIX + service_name + NAME_DELIMITER + node->full_name);
      Op* op = OpRepository::instance().get_op(node->type);
      if (op == NULL) {
        LOG(ERROR) << "Failed to get_op, op type[" << node->type << "]";
        return;
      }
      op->set_full_name(service_name + NAME_DELIMITER + node->full_name);
      op->set_config(node->conf);
      op->regist_metric();
      OpRepository::instance().return_op(node->type, op);
    }
  }
}

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
