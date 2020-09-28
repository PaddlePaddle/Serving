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

#include "core/predictor/framework/workflow.h"
#include <string>
#include "core/predictor/common/inner_common.h"
#include "core/predictor/framework/predictor_metric.h"  // PredictorMetric

namespace baidu {
namespace paddle_serving {
namespace predictor {

int Workflow::init(const configure::Workflow& conf) {
  const std::string& name = conf.name();
  _type = conf.workflow_type();
  _name = name;
  if (_dag.init(conf, name) != 0) {
    LOG(ERROR) << "Failed initialize dag: " << _name;
    return -1;
  }
  return 0;
}

DagView* Workflow::fetch_dag_view(const std::string& service_name,
                                  const uint64_t log_id) {
  DagView* view = NULL;
  if (_type == "Sequence") {
    view = butil::get_object<DagView>();
  } else if (_type == "Parallel") {
    view = butil::get_object<ParallelDagView>();
  } else {
    LOG(ERROR) << "(logid=" << log_id << ") Unknown dag type:" << _type << "!";
    return NULL;
  }
  if (view == NULL) {
    LOG(ERROR) << "(logid=" << log_id << ") create dag view from pool failed!";
    return NULL;
  }
  view->init(&_dag, service_name, log_id);
  return view;
}

void Workflow::return_dag_view(DagView* view) {
  view->deinit();
  if (_type == "Sequence") {
    butil::return_object<DagView>(view);
  } else if (_type == "Parallel") {
    butil::return_object<ParallelDagView>(dynamic_cast<ParallelDagView*>(view));
  } else {
    LOG(ERROR) << "Unknown dag type:" << _type << "!";
    return;
  }
}

int Workflow::reload() {
  // reload op's config here...

  return 0;
}

void Workflow::regist_metric(const std::string& service_name) {
  PredictorMetric::GetInstance()->regist_latency_metric(
      WORKFLOW_METRIC_PREFIX + service_name + NAME_DELIMITER + full_name());
  _dag.regist_metric(service_name);
}

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
