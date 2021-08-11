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

#include <gflags/gflags.h>
#include <time.h>
#include <iostream>
#include <vector>
#include "butil/logging.h"
#include "core/cube/cube-builder/include/cube-builder/builder_job.h"
#include "core/cube/cube-builder/include/cube-builder/crovl_builder_increment.h"
#include "core/cube/cube-builder/include/cube-builder/util.h"

DEFINE_string(dict_name, "", "dict name, no need");
DEFINE_string(input_path, "", "source data input path");
DEFINE_string(output_path, "", "source data input path");

DEFINE_string(job_mode, "base", "job mode base/delta default:base");
DEFINE_int32(last_version, 0, "last version, job mode delta need");
DEFINE_int32(cur_version, 0, "current version, no need");
DEFINE_int32(depend_version, 0, "depend version, job mode delta need");
DEFINE_int32(shard_num, -1, "shard num");

DEFINE_string(master_address, "", "master address, no need");
DEFINE_bool(only_build, true, "wheather build need transfer");

int main(int argc, char *argv[]) {
  google::SetVersionString("1.0.0.0");
  google::SetUsageMessage("Usage : ./cube-builder --help ");
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::SetLogDestination(google::GLOG_INFO, "./log_");
  google::SetStderrLogging(google::GLOG_WARNING);

  LOG(INFO) << "start";

  string last_version = std::to_string(FLAGS_last_version);
  string cur_version = std::to_string(FLAGS_cur_version);
  string depend_version = std::to_string(FLAGS_depend_version);

  if (FLAGS_input_path == "" || FLAGS_output_path == "" ||
      FLAGS_shard_num == -1) {
    LOG(ERROR) << "Param error! Usage: " << argv[0] << " --help";
    return -1;
  }
  if (FLAGS_job_mode == "base") {
    if (FLAGS_only_build) {
      time_t t;
      time(&t);
      cur_version = std::to_string(t);
      depend_version = cur_version;
    }

  } else if (FLAGS_job_mode == "delta") {
    //if (FLAGS_last_version == 0 || FLAGS_depend_version == 0) {
     // LOG(ERROR) << "Param error! need last_version and depend_version! Usage: "
     //            << argv[0] << " --help";
     // return -1;
    //} else {
      if (FLAGS_only_build) {
        time_t t;
        time(&t);
        cur_version = std::to_string(t);
        depend_version = cur_version; //hack here 
      }
    //}
  } else {
    LOG(ERROR) << "Job mode error! Usage: " << argv[0] << " --help";
    return -1;
  }

  Job job;
  job.set_dict_name(FLAGS_dict_name);
  job.set_shard_num(FLAGS_shard_num);
  job.set_input_path(FLAGS_input_path);
  job.set_output_path(FLAGS_output_path + "/" + depend_version + "_" +
                      cur_version);
  job.set_job_mode(FLAGS_job_mode);

  vector<string> files;
  getAllFiles(job.get_input_path(), &files);

  if (!checkDirectory(job.get_output_path())) {
    LOG(ERROR) << "create output_path path failed: "
               << job.get_output_path().c_str();
    return -1;
  }

  vector<CROVLBuilderIncremental *> reduces;
  for (auto i = 0; i < job.get_shard_num(); i++) {
    string tar_path = job.get_output_path() + "/" + job.get_dict_name() +
                      "_part" + std::to_string(i) + ".tar";
    string build_path = job.get_output_path() + "/" + job.get_dict_name() +
                        "_part" + std::to_string(i);

    CROVLBuilderIncremental *_builder = new CROVLBuilderIncremental();
    if (!_builder->Init(IT_HASH,
                        MAX_BLOCK_SIZE,
                        job.get_job_mode().c_str(),
                        build_path.c_str(),
                        tar_path.c_str(),
                        job.get_dict_name().c_str(),
                        std::to_string(i),
                        std::to_string(0),
                        last_version,
                        cur_version,
                        depend_version,
                        FLAGS_master_address.c_str())) {
      LOG(ERROR) << "CROVLBuilderIncremental init failed " << build_path;
      return -1;
    }
    reduces.push_back(_builder);
  }

  for (auto file : files) {
    mapFileLocal(job, file, reduces);
    LOG(INFO) << "next file to reduce!";
  }
  for (auto reduce : reduces) {
    reduce->done();
    reduce->archive();
    reduce->md5sum();
  }
  google::ShutdownGoogleLogging();
  return 0;
}
