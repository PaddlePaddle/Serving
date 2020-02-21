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

#include <fstream>
#include "core/sdk-cpp/builtin_format.pb.h"
#include "core/sdk-cpp/include/common.h"
#include "core/sdk-cpp/include/predictor_sdk.h"
#include "core/sdk-cpp/text_classification.pb.h"

using baidu::paddle_serving::sdk_cpp::Predictor;
using baidu::paddle_serving::sdk_cpp::PredictorApi;
using baidu::paddle_serving::predictor::text_classification::TextReqInstance;
using baidu::paddle_serving::predictor::text_classification::TextResInstance;
using baidu::paddle_serving::predictor::text_classification::Request;
using baidu::paddle_serving::predictor::text_classification::Response;

const char *g_test_file = "./data/text_classification/test_set.txt";
DEFINE_int32(batch_size, 50, "Set the batch size of test file.");

// Text Classification Data Feed
//
// Input format:
// ([termid list], truth_label)
// Where 'termid list' is a variant length id list, `truth label` is a single
// number (0 or 1)
//
const int MAX_LINE_SIZE = 1024 * 1024;
std::vector<int> g_pred_labels;
const float g_decision_boundary = 0.500;

class DataFeed {
 public:
  virtual ~DataFeed() {}
  virtual void init();
  virtual bool set_file(const char *filename);
  std::vector<std::vector<int64_t>> &get_test_input() { return _test_input; }
  std::vector<int> &get_labels() { return _test_label; }
  uint32_t sample_id() { return _current_sample_id; }
  void set_sample_id(uint32_t sample_id) { _current_sample_id = sample_id; }

 private:
  std::vector<std::vector<int64_t>> _test_input;
  std::vector<int> _test_label;
  uint32_t _current_sample_id;
  int _batch_size;
  std::shared_ptr<int> _batch_id_buffer;
  std::shared_ptr<int> _label_buffer;
};

void DataFeed::init() {
  _batch_id_buffer.reset(new int[10240 * 1024], [](int *p) { delete[] p; });
  _label_buffer.reset(new int[10240 * 1024], [](int *p) { delete[] p; });
}

bool DataFeed::set_file(const char *filename) {
  std::ifstream ifs(filename);
  char *line = new char[MAX_LINE_SIZE];
  int len = 0;
  char *sequence_begin_ptr = NULL;
  char *sequence_end_ptr = NULL;
  char *id_begin_ptr = NULL;
  char *id_end_ptr = NULL;
  char *label_ptr = NULL;
  int label = -1;
  int id = -1;
  while (!ifs.eof()) {
    std::vector<int64_t> vec;
    ifs.getline(line, MAX_LINE_SIZE);
    len = strlen(line);
    if (line[0] != '(' || line[len - 1] != ')') {
      continue;
    }
    line[len - 1] = '\0';

    sequence_begin_ptr = strchr(line, '(') + 1;
    if (*sequence_begin_ptr != '[') {
      continue;
    }

    sequence_end_ptr = strchr(sequence_begin_ptr, ']');
    if (sequence_end_ptr == NULL) {
      continue;
    }
    *sequence_end_ptr = '\0';

    id_begin_ptr = sequence_begin_ptr;
    while (id_begin_ptr != NULL) {
      id_begin_ptr++;
      id_end_ptr = strchr(id_begin_ptr, ',');
      if (id_end_ptr != NULL) {
        *id_end_ptr = '\0';
      }
      id = atoi(id_begin_ptr);
      id_begin_ptr = id_end_ptr;
      vec.push_back(id);
    }

    label_ptr = strchr(sequence_end_ptr + 1, ',');
    if (label_ptr == NULL) {
      continue;
    }
    *label_ptr = '\0';

    label_ptr++;
    label = atoi(label_ptr);

    _test_input.push_back(vec);
    _test_label.push_back(label);
  }

  ifs.close();

  std::cout << "read record" << _test_input.size() << std::endl;

  return 0;
}

int create_req(std::shared_ptr<DataFeed> data_feed, Request &req) {  // NOLINT
  std::vector<std::vector<int64_t>> &inputs = data_feed->get_test_input();
  uint32_t current_sample_id = data_feed->sample_id();
  int idx = 0;

  for (idx = 0;
       idx < FLAGS_batch_size && current_sample_id + idx < inputs.size();
       ++idx) {
    TextReqInstance *req_instance = req.add_instances();
    std::vector<int64_t> &sample = inputs.at(current_sample_id + idx);
    for (auto x : sample) {
      req_instance->add_ids(x);
    }
  }

  if (idx < FLAGS_batch_size) {
    return -1;
  }

  data_feed->set_sample_id(current_sample_id + FLAGS_batch_size);
  return 0;
}

void extract_res(const Request &req, const Response &res) {
  uint32_t sample_size = res.predictions_size();
  std::string err_string;
  for (uint32_t si = 0; si < sample_size; ++si) {
    const TextResInstance &res_instance = res.predictions(si);

    if (res_instance.class_1_prob() < g_decision_boundary) {
      g_pred_labels.push_back(0);
    } else if (res_instance.class_1_prob() >= g_decision_boundary) {
      g_pred_labels.push_back(1);
    }
  }
}

int main(int argc, char **argv) {
  PredictorApi api;

// initialize logger instance
#ifdef BCLOUD
  logging::LoggingSettings settings;
  settings.logging_dest = logging::LOG_TO_FILE;

  std::string filename(argv[0]);
  filename = filename.substr(filename.find_last_of('/') + 1);
  settings.log_file = (std::string("./log/") + filename + ".log").c_str();
  settings.delete_old = logging::DELETE_OLD_LOG_FILE;
  logging::InitLogging(settings);

  logging::ComlogSinkOptions cso;
  cso.process_name = filename;
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

  g_pred_labels.clear();

  std::shared_ptr<DataFeed> local_feed(new DataFeed());
  local_feed->init();
  local_feed->set_file(g_test_file);

  if (api.create("./conf", "predictors.prototxt") != 0) {
    LOG(ERROR) << "Failed create predictors api!";
    return -1;
  }

  Request req;
  Response res;

  api.thrd_initialize();

  uint64_t elapse_ms = 0;
  int batch_count = 0;
  while (true) {
    api.thrd_clear();

    Predictor *predictor = api.fetch_predictor("text_classification");
    if (!predictor) {
      LOG(ERROR) << "Failed fetch predictor: text_classification";
      return -1;
    }

    req.Clear();
    res.Clear();

    if (create_req(local_feed, req) != 0) {
      break;
    }

    ++batch_count;

    timeval start;
    gettimeofday(&start, NULL);

    if (predictor->inference(&req, &res) != 0) {
      LOG(ERROR) << "failed call predictor with req:" << req.ShortDebugString();
      return -1;
    }

    timeval end;
    gettimeofday(&end, NULL);

    elapse_ms += (end.tv_sec * 1000 + end.tv_usec / 1000) -
                 (start.tv_sec * 1000 + start.tv_usec / 1000);

#if 1
    LOG(INFO) << "single round elapse time "
              << (end.tv_sec * 1000000 + end.tv_usec) -
                     (start.tv_sec * 1000000 + start.tv_usec);
#endif
    extract_res(req, res);
    res.Clear();
  }  // while (true)

  int correct = 0;
  std::vector<int> &truth_label = local_feed->get_labels();
  for (int i = 0; i < g_pred_labels.size(); ++i) {
    if (g_pred_labels[i] == truth_label[i]) {
      ++correct;
    }
  }

  LOG(INFO) << "Elapse ms " << elapse_ms;
  LOG(INFO) << "Exe ms per batch " << elapse_ms / batch_count;
  double qps = (static_cast<double>(g_pred_labels.size()) / elapse_ms) * 1000;

  LOG(INFO) << "QPS: " << qps / FLAGS_batch_size << "/s";
  LOG(INFO) << "Accuracy "
            << static_cast<double>(correct) / g_pred_labels.size();

  LOG(INFO) << "correct " << correct;
  LOG(INFO) << "g_pred_labels " << g_pred_labels.size();
  api.thrd_finalize();
  api.destroy();

  return 0;
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
