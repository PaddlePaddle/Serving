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

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>  // NOLINT
#include "elastic-ctr/client/api/elastic_ctr_api.h"

using baidu::paddle_serving::elastic_ctr::ElasticCTRPredictorApi;
using baidu::paddle_serving::elastic_ctr::Prediction;

DEFINE_int32(batch_size, 10, "Infernce batch_size");

DEFINE_string(test_file, "", "test file");

const int VARIABLE_NAME_LEN = 256;
const int CTR_EMBEDDING_TABLE_SIZE = 100000001;

struct Sample {
  std::map<std::string, std::vector<uint64_t>> slots;
};

std::vector<Sample> samples;

int read_samples(const char* file) {
  std::ifstream fs(file);

  for (std::string line; std::getline(fs, line);) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;

    Sample sample;

    while (std::getline(ss, token, ' ')) {
      tokens.push_back(token);
    }

    if (tokens.size() <= 3) {
      continue;
    }

    for (std::size_t i = 2; i < tokens.size(); ++i) {
      std::size_t pos = tokens[i].find(':');
      if (pos == std::string::npos) {
        continue;
      }

      uint64_t x = std::strtoull(tokens[i].substr(0, pos).c_str(), NULL, 10);
      std::string slot_name = tokens[i].substr(pos + 1);

      if (sample.slots.find(slot_name) == sample.slots.end()) {
        std::vector<uint64_t> values;
        values.push_back(x % CTR_EMBEDDING_TABLE_SIZE);
        sample.slots[slot_name] = values;
      } else {
        auto it = sample.slots.find(slot_name);
        it->second.push_back(x);
      }
    }

    samples.push_back(sample);
  }

  LOG(INFO) << "Samples size = " << samples.size();

#if 1
  for (std::size_t i = 0; i < samples.size(); ++i) {
    LOG(INFO) << "=============Sample " << i << "=========";
    for (auto slot : samples[i].slots) {
      LOG(INFO) << "slot_name: " << slot.first.c_str();
      for (auto x : slot.second) {
        LOG(INFO) << x;
      }
    }
    LOG(INFO) << "========================================";
  }
#endif
  return 0;
}

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);

  ElasticCTRPredictorApi api;

#ifdef BCLOUD
  logging::LoggingSettings settings;
  settings.logging_dest = logging::LOG_TO_FILE;
  std::string log_filename(argv[0]);
  log_filename = log_filename.substr(log_filename.find_last_of('/') + 1);
  settings.log_file = (std::string("./log/") + log_filename + ".log").c_str();
  settings.delete_old = logging::DELETE_OLD_LOG_FILE;
  logging::InitLogging(settings);
  logging::ComlogSinkOptions cso;
  cso.process_name = log_filename;
  cso.enable_wf_device = true;
  logging::ComlogSink::GetInstance()->Setup(&cso);
#else
  struct stat st_buf;
  int ret = 0;
  if ((ret = stat("./log", &st_buf)) != 0) {
    mkdir("./log", 0777);
    ret = stat("./log", &st_buf);
    if (ret != 0) {
      LOG(WARNING) << "Log path ./log not exist, and create fail";
      return -1;
    }
  }
  FLAGS_log_dir = "./log";
  google::InitGoogleLogging(strdup(argv[0]));
  FLAGS_logbufsecs = 0;
  FLAGS_logbuflevel = -1;
#endif
  // predictor conf
  if (api.init("./conf", "slot.conf", "predictors.prototxt") != 0) {
    LOG(ERROR) << "Failed create predictors api!";
    return -1;
  }

  api.thrd_initialize();

  ret = read_samples(FLAGS_test_file.c_str());

  std::size_t index = 0;
  while (index < samples.size()) {
    api.thrd_clear();

    for (int i = 0; i < FLAGS_batch_size && index < samples.size(); ++i) {
      ReqInstance* ins = api.add_instance();
      if (!ins) {
        LOG(ERROR) << "Failed create req instance";
        return -1;
      }

      for (auto slot : samples[index].slots) {
        for (auto x : slot.second) {
          api.add_slot(ins, slot.first.c_str(), x);
        }
      }

      ++index;
    }

    std::vector<std::vector<float>> results_vec;
    if (api.inference(results_vec) != 0) {
      LOG(ERROR) << "failed call predictor";
      return -1;
    }

#if 1
    for (std::size_t i = 0; i < results_vec.size(); ++i) {
      LOG(INFO) << "sample " << i << ": [" << results_vec[i].at(0) << ", "
                << results_vec[i].at(1) << "]";
    }
#endif
  }  // end while

  api.thrd_finalize();

  api.destroy();
  return 0;
}
