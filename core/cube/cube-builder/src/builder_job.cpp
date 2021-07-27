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

#include "core/cube/cube-builder/include/cube-builder/builder_job.h"
#include <stdio.h>
#include <iostream>
#include "butil/logging.h"
#include "core/cube/cube-builder/include/cube-builder/seqfile_reader.h"
#include "core/cube/cube-builder/include/cube-builder/util.h"
using std::string;
void Job::set_shard_num(int num) { shard_num = num; }

int Job::get_shard_num() { return shard_num; }

void Job::set_input_path(string path) { input_path = path; }

string Job::get_input_path() { return input_path; }

void Job::set_output_path(string path) { output_path = path; }

string Job::get_output_path() { return output_path; }

void Job::set_job_mode(mode mmode) { job_mode = mmode; }

mode Job::get_job_mode() { return job_mode; }

void Job::set_dict_name(string name) { dict_name = name; }

string Job::get_dict_name() { return dict_name; }

void mapFileLocal(Job job,
                  string file,
                  vector<CROVLBuilderIncremental *> reduces) {
  SequenceFileRecordReader reader(file.c_str());

  if (reader.open() != 0) {
    LOG(ERROR) << "open file failed! " << file;
    return;
  }
  if (reader.read_header() != 0) {
    LOG(ERROR) << "read header error! " << file;
    reader.close();
    return;
  }

  Record record(reader.get_header());
  int total_count = 0;

  while (reader.next(&record) == 0) {
    uint64_t key =
        *reinterpret_cast<uint64_t *>(const_cast<char *>(record.key.data()));

    total_count++;
    int part = key % job.get_shard_num();
    int64_t value_length = record.record_len - record.key_len;

    reduces[part]->add(key, value_length, record.value.c_str());
  }

  if (reader.close() != 0) {
    LOG(ERROR) << "close file failed! " << file;
    return;
  }
}
