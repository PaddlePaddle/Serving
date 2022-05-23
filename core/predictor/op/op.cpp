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

#include "core/predictor/op/op.h"

#ifdef BCLOUD
#include <base/time.h>  // base::Timer
#else
#include <butil/time.h>
#endif

#include <string>
#include "core/predictor/common/constant.h"
#include "core/predictor/common/utils.h"
#include "core/predictor/framework/channel.h"
#include "core/predictor/framework/dag.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

int Op::init(Bus* bus,
             Dag* dag,
             uint32_t id,
             const std::string& name,
             const std::string& type,
             void* conf,
             const std::vector<std::string>& address,
             const uint64_t log_id) {
  _bus = bus;
  _dag = dag;
  _id = id;
  _name = name;
  _type = type;
  _address = address;
  set_config(conf);

  _timer = butil::get_object<TimerFlow>();
  if (!_timer) {
    LOG(ERROR) << "(logid=" << log_id
               << ") Invalid timerflow in op:" << this->name();
    return -1;
  }

  _timer->init();
  _has_calc = false;
  _has_init = true;

  Channel* channel = mutable_channel();
  if (channel == NULL) {
    LOG(ERROR) << "(logid=" << log_id
               << ") Failed mutable channel in op: " << this->id() << ", "
               << this->name() << "!";
    return -1;
  }

  _pre_node_names.clear();
  return custom_init();
}

int Op::deinit() {
  if (_timer) {
    butil::return_object(_timer);
  }

  _bus = NULL;
  _dag = NULL;
  _timer = NULL;

  if (release_channel() != 0) {
    LOG(ERROR) << "Failed release channel in op:" << this->id() << ", "
               << this->name() << "!";
    return -1;
  }

  return custom_deinit();
}

int Op::check_time(const char* tag) {
  if (!_timer) {
    LOG(ERROR) << "Invalid timer in op";
    return -1;
  }

  if (!_timer->check(tag)) {
    LOG(ERROR) << "Failed check timer:" << tag;
    return -1;
  }

  return 0;
}

int Op::process(const uint64_t log_id, bool debug) {
  butil::Timer op_time(butil::Timer::STARTED);
  if (debug && _timer) {
    _timer->start();
  }
  if (!_has_init) {
    LOG(ERROR) << "(logid=" << log_id
               << ") Make sure op has been init before inference";
    return ERR_INTERNAL_FAILURE;
  }

  /*
  if (_has_calc) {
    LOG(INFO) << "(logid=" << log_id << ") Op: " << _name
              << " already processed before";
    return ERR_OK;
  }
  */

  // 1. dependency inference
  /*
  DagNode* node = _dag->node_by_name(this->name());
  if (node == NULL) {
      LOG(ERROR) << "Failed get node of op:" << this->name();
      return -1;
  }
  boost::unordered_map<std::string, EdgeMode>& depends =
      node->depends;
  boost::unordered_map<std::string, EdgeMode>::iterator it;
  for (it = depends.begin(); it != depends.end(); it++) {
      Op* depend_op = view->find(it->first);
      if (depend_op->process() != 0) {
          LOG(WARNING) << "Op: " << _name << " processed failed!";
          return -1;
      }
  }*/

  if (debug && _timer) {
    _timer->check("depend");
  }

  // 2. current inference
  if (inference() != 0) {
    return ERR_OP_INFER_FAILURE;
  }
  if (debug && _timer) {
    _timer->check("infer");
  }

  // 3. share output to bus
  if (!_has_calc) {
    Channel* channel = mutable_channel();
    channel->share_to_bus(_bus, log_id);
  }

  // 4. mark has calculated
  _has_calc = true;

  if (debug && _timer) {
    _timer->check("share");
    _timer->end();
  }

  op_time.stop();
  PredictorMetric::GetInstance()->update_latency_metric(
      OP_METRIC_PREFIX + full_name(), op_time.u_elapsed());
  LOG(INFO) << "(logid=" << log_id << ") " << name() << "_time=["
            << op_time.u_elapsed() << "]";
  return ERR_OK;
}

std::string Op::time_info() {
  if (_timer) {
    return _timer->info();
  } else {
    return "Invalid Timer!";
  }
}

bool Op::is_mutable(const std::string& op) {
  if (op == START_OP_NAME) {
    return false;
  }
  DagNode* node = const_cast<DagNode*>(_dag->node_by_name(_name));
  if (node->depends.find(op) == node->depends.end()) {
    LOG(WARNING) << "op: " << _name << " doesnot depend on"
                 << "op: " << op << "!";
    return false;
  }

  if (node->depends[op] != RW) {
    LOG(WARNING) << "op: " << _name << " has no RW access"
                 << "ot op: " << op << ", mode: " << node->depends[op]
                 << ", please use get_argment() instead.";
    return false;
  }

  return true;
}

bool Op::is_mutable(const std::string& op) const {
  if (op == START_OP_NAME) {
    return false;
  }
  DagNode* node = const_cast<DagNode*>(_dag->node_by_name(_name));
  if (node->depends.find(op) == node->depends.end()) {
    LOG(WARNING) << "op: " << _name << " doesnot depend on"
                 << "op: " << op << "!";
    return false;
  }

  if (node->depends[op] != RW) {
    LOG(WARNING) << "op: " << _name << " has no RW access"
                 << "ot op: " << op << ", mode: " << node->depends[op]
                 << ", please use get_argment() instead.";
    return false;
  }

  return true;
}

bool Op::is_readable(const std::string& op) {
  if (op == START_OP_NAME) {
    return true;
  }
  DagNode* node = const_cast<DagNode*>(_dag->node_by_name(_name));
  if (node->depends.find(op) == node->depends.end()) {
    LOG(WARNING) << "op: " << _name << " doesnot depend on"
                 << "op: " << op << "!";
    return false;
  }

  if (node->depends[op] != RW && node->depends[op] != RO) {
    LOG(WARNING) << "op: " << _name << " has no RO access"
                 << "ot op: " << op << ", mode: " << node->depends[op]
                 << ", please check your configuration.";
    return false;
  }

  return true;
}

bool Op::is_readable(const std::string& op) const {
  if (op == START_OP_NAME) {
    return true;
  }
  DagNode* node = const_cast<DagNode*>(_dag->node_by_name(_name));
  if (node->depends.find(op) == node->depends.end()) {
    LOG(WARNING) << "op: " << _name << " doesnot depend on "
                 << "op: " << op << "!";
    return false;
  }

  if (node->depends[op] != RW && node->depends[op] != RO) {
    LOG(WARNING) << "op: " << _name << " has no RO access"
                 << "ot op: " << op << ", mode: " << node->depends[op]
                 << ", please check your configuration.";
    return false;
  }

  return true;
}

// Get the Channel object of dependent OP
Channel* Op::mutable_depend_channel(const std::string& op) {
  if (!is_mutable(op)) {
    LOG(WARNING) << "Op: " << _name << " cannot mutable op: " << op << "!";
    return NULL;
  }

  // Get the Channel object of dependent OP from bus
  return _bus->channel_by_name(op);
}

// Get the Channel object of dependent OP
const Channel* Op::get_depend_channel(const std::string& op) const {
  // Get the `mode` attribute of dependent OP from dag
  if (!is_readable(op)) {
    LOG(WARNING) << "op: " << _name << " doesnot depend on op: " << op << "!";
    return NULL;
  }

  // Get the Channel object of dependent OP from bus
  return _bus->channel_by_name(op);
}

google::protobuf::Message* Op::mutable_message() {
  return mutable_channel()->message();
}

const google::protobuf::Message* Op::get_message() const {
  return get_channel()->message();
}

bool Op::has_calc() { return _has_calc; }

const char* Op::name() const { return _name.c_str(); }

const std::string& Op::type() const { return _type; }

uint32_t Op::id() const { return _id; }

const std::string Op::debug_string() {
  const Channel* channel = get_channel();
  if (!channel) {
    LOG(ERROR) << "Invalid channel!";
    return "Invalid channel in OP";
  }
  return channel->debug_string();
}

const google::protobuf::Message* Op::get_request_message() {
  return _bus->channel_by_name(START_OP_NAME)->message();
}

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
