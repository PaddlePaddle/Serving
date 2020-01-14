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

#include <iostream>
#include <string>
#include <vector>
#include "core/cube/cube-builder/include/cube-builder/crovl_builder_increment.h"
#include "core/cube/cube-builder/include/cube-builder/define.h"
using std::string;
using std::vector;

class Job {
 public:
  void set_shard_num(int num);

  int get_shard_num();

  void set_input_path(string path);

  string get_input_path();

  void set_output_path(string path);

  string get_output_path();

  void set_job_mode(mode mmode);

  mode get_job_mode();

  void set_dict_name(string name);

  string get_dict_name();

 private:
  int shard_num;
  string input_path;
  string output_path;
  mode job_mode;
  string dict_name;
};

void mapFileLocal(Job job,
                  string file,
                  vector<CROVLBuilderIncremental*> reduces);
