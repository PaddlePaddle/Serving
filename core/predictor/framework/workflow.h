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
#include "core/predictor/common/inner_common.h"
#include "core/predictor/framework/dag.h"
#include "core/predictor/framework/dag_view.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

template <typename T>
class Manager;

class Workflow {
 public:
  Workflow() {}

  static const char* tag() { return "workflow"; }

  // Each workflow object corresponds to an independent
  // configure file, so you can share the object between
  // different apps.
  int init(const configure::Workflow& conf);

  DagView* fetch_dag_view(const std::string& service_name,
                          const uint64_t log_id);

  int deinit() { return 0; }

  void return_dag_view(DagView* view);

  int reload();

  const std::string& name() { return _name; }

  const std::string& full_name() { return _name; }

  void regist_metric(const std::string& service_name);

 private:
  Dag _dag;
  std::string _type;
  std::string _name;
};

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
