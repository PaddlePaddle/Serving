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
#include "core/sdk-cpp/ctr_prediction.pb.h"
#include "core/sdk-cpp/include/common.h"
#include "core/sdk-cpp/include/predictor_sdk.h"
using baidu::paddle_serving::sdk_cpp::Predictor;
using baidu::paddle_serving::sdk_cpp::PredictorApi;
using baidu::paddle_serving::predictor::ctr_prediction::Request;
using baidu::paddle_serving::predictor::ctr_prediction::Response;
using baidu::paddle_serving::predictor::ctr_prediction::CTRReqInstance;
using baidu::paddle_serving::predictor::ctr_prediction::CTRResInstance;

int sparse_num = 26;
int dense_num = 13;
int hash_dim = 1000001;

DEFINE_int32(batch_size, 50, "Set the batch size of test file.");
DEFINE_int32(concurrency, 1, "Set the max concurrency of requests");
DEFINE_int32(repeat, 1, "Number of data samples iteration count. Default 1");
DEFINE_bool(enable_profiling,
            false,
            "Enable profiling. Will supress a lot normal output");

std::vector<float> cont_min = {0, -3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
std::vector<float> cont_diff = {
    20, 603, 100, 50, 64000, 500, 100, 50, 500, 10, 10, 10, 50};
char* data_filename = "./data/ctr_prediction/data.txt";
std::atomic<int> g_concurrency(0);
std::vector<std::vector<int>> response_time;

std::vector<std::string> split(const std::string& str,
                               const std::string& pattern) {
  std::vector<std::string> res;
  if (str == "") return res;
  std::string strs = str + pattern;
  size_t pos = strs.find(pattern);
  while (pos != strs.npos) {
    std::string temp = strs.substr(0, pos);
    res.push_back(temp);
    strs = strs.substr(pos + 1, strs.size());
    pos = strs.find(pattern);
  }
  return res;
}

/**
 * Simulate CPython hash function on string objects
 *
 * Our model training process use this function to convert string objects to
 * unique ids.
 *
 * See string_hash() in
 * https://svn.python.org/projects/python/trunk/Objects/stringobject.c
 */
int64_t hash(std::string str) {
  int64_t len;
  unsigned char* p;
  int64_t x;

  len = str.size();
  p = (unsigned char*)str.c_str();
  x = *p << 7;
  while (--len >= 0) {
    x = (1000003 * x) ^ *p++;
  }
  x ^= str.size();
  if (x == -1) {
    x = -2;
  }
  return x;
}

int create_req(Request* req,
               const std::vector<std::string>& data_list,
               int start_index,
               int batch_size) {
  for (int i = 0; i < batch_size; ++i) {
    CTRReqInstance* ins = req->add_instances();
    if (!ins) {
      LOG(ERROR) << "Failed create req instance";
      return -1;
    }

    // add data
    // avoid out of boundary
    int cur_index = start_index + i;
    if (cur_index >= data_list.size()) {
      cur_index = cur_index % data_list.size();
    }

    std::vector<std::string> feature_list = split(data_list[cur_index], "\t");
    for (int fi = 0; fi < dense_num; fi++) {
      if (feature_list[fi] == "") {
        ins->add_dense_ids(0.0);
      } else {
        float dense_id = std::stof(feature_list[fi]);
        dense_id = (dense_id - cont_min[fi]) / cont_diff[fi];
        ins->add_dense_ids(dense_id);
      }
    }
    for (int fi = dense_num; fi < (dense_num + sparse_num); fi++) {
      int64_t sparse_id =
          hash(std::to_string(fi) + feature_list[fi]) % hash_dim;
      if (sparse_id < 0) {
        // diff between c++ and python
        sparse_id += hash_dim;
      }
      ins->add_sparse_ids(sparse_id);
    }
  }
  return 0;
}

void print_res(const Request& req,
               const Response& res,
               std::string route_tag,
               uint64_t elapse_ms) {
  if (res.err_code() != 0) {
    LOG(ERROR) << "Get result fail :" << res.err_msg();
    return;
  }
  for (uint32_t i = 0; i < res.predictions_size(); ++i) {
    const CTRResInstance& res_ins = res.predictions(i);
    std::ostringstream oss;
    oss << "[" << res_ins.prob0() << " " << res_ins.prob1() << "]";
    LOG(INFO) << "Receive result " << oss.str();
  }
  LOG(INFO) << "Succ call predictor[ctr_prediction_service], the tag is: "
            << route_tag << ", elapse_ms: " << elapse_ms;
}

void thread_worker(PredictorApi* api,
                   int thread_id,
                   const std::vector<std::string>& data_list) {
  // init
  Request req;
  Response res;
  std::string line;

  api->thrd_initialize();

  for (int i = 0; i < FLAGS_repeat; ++i) {
    int start_index = 0;

    while (true) {
      if (start_index >= data_list.size()) {
        break;
      }

      api->thrd_clear();

      Predictor* predictor = api->fetch_predictor("ctr_prediction_service");
      if (!predictor) {
        LOG(ERROR) << "Failed fetch predictor: ctr_prediction_service";
        return;
      }

      req.Clear();
      res.Clear();

      // wait for other thread
      while (g_concurrency.load() >= FLAGS_concurrency) {
      }
      g_concurrency++;
      LOG(INFO) << "Current concurrency " << g_concurrency.load();

      if (create_req(&req, data_list, start_index, FLAGS_batch_size) != 0) {
        return;
      }
      start_index += FLAGS_batch_size;
      LOG(INFO) << "start_index = " << start_index;

      timeval start;
      gettimeofday(&start, NULL);

      if (predictor->inference(&req, &res) != 0) {
        LOG(ERROR) << "failed call predictor with req:"
                   << req.ShortDebugString();
        return;
      }
      g_concurrency--;

      timeval end;
      gettimeofday(&end, NULL);
      uint64_t elapse_ms = (end.tv_sec * 1000 + end.tv_usec / 1000) -
                           (start.tv_sec * 1000 + start.tv_usec / 1000);

      response_time[thread_id].push_back(elapse_ms);

      if (!FLAGS_enable_profiling) {
        print_res(req, res, predictor->tag(), elapse_ms);
      }

      LOG(INFO) << "Done. Current concurrency " << g_concurrency.load();
    }  // end while
  }    // end for

  api->thrd_finalize();
}

void calc_time() {
  std::vector<int> time_list;
  for (auto a : response_time) {
    time_list.insert(time_list.end(), a.begin(), a.end());
  }

  LOG(INFO) << "Total request : " << (time_list.size());
  LOG(INFO) << "Batch size : " << FLAGS_batch_size;
  LOG(INFO) << "Max concurrency : " << FLAGS_concurrency;
  LOG(INFO) << "enable_profiling: " << FLAGS_enable_profiling;
  LOG(INFO) << "repeat count: " << FLAGS_repeat;

  float total_time = 0;
  float max_time = 0;
  float min_time = 1000000;
  for (int i = 0; i < time_list.size(); ++i) {
    total_time += time_list[i];
    if (time_list[i] > max_time) max_time = time_list[i];
    if (time_list[i] < min_time) min_time = time_list[i];
  }

  float mean_time = total_time / (time_list.size());
  float var_time;
  for (int i = 0; i < time_list.size(); ++i) {
    var_time += (time_list[i] - mean_time) * (time_list[i] - mean_time);
  }
  var_time = var_time / time_list.size();

  LOG(INFO) << "Total time : " << total_time / FLAGS_concurrency << "ms";
  LOG(INFO) << "Variance : " << var_time << "ms";
  LOG(INFO) << "Max time : " << max_time << "ms";
  LOG(INFO) << "Min time : " << min_time << "ms";

  float qps = 0.0;
  if (total_time > 0) {
    qps = (time_list.size() * 1000) / (total_time / FLAGS_concurrency);
  }
  LOG(INFO) << "QPS: " << qps << "/s";

  LOG(INFO) << "Latency statistics: ";
  sort(time_list.begin(), time_list.end());

  int percent_pos_50 = time_list.size() * 0.5;
  int percent_pos_80 = time_list.size() * 0.8;
  int percent_pos_90 = time_list.size() * 0.9;
  int percent_pos_99 = time_list.size() * 0.99;
  int percent_pos_999 = time_list.size() * 0.999;
  if (time_list.size() != 0) {
    LOG(INFO) << "Mean time : " << mean_time;
    LOG(INFO) << "50 percent ms: " << time_list[percent_pos_50];
    LOG(INFO) << "80 percent ms: " << time_list[percent_pos_80];
    LOG(INFO) << "90 percent ms: " << time_list[percent_pos_90];
    LOG(INFO) << "99 percent ms: " << time_list[percent_pos_99];
    LOG(INFO) << "99.9 percent ms: " << time_list[percent_pos_999];
  } else {
    LOG(INFO) << "N/A";
  }
}
int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);

  // initialize
  PredictorApi api;
  response_time.resize(FLAGS_concurrency);

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
  if (api.create("./conf", "predictors.prototxt") != 0) {
    LOG(ERROR) << "Failed create predictors api!";
    return -1;
  }

  LOG(INFO) << "data sample file: " << data_filename;

  if (FLAGS_enable_profiling) {
    LOG(INFO) << "In profiling mode, lot of normal output will be supressed. "
              << "Use --enable_profiling=false to turn off this mode";
  }

  // read data
  std::ifstream data_file(data_filename);
  if (!data_file) {
    std::cout << "read file error \n" << std::endl;
    return -1;
  }

  std::vector<std::string> data_list;
  std::string line;
  while (getline(data_file, line)) {
    data_list.push_back(line);
  }

  // create threads
  std::vector<std::thread*> thread_pool;
  for (int i = 0; i < FLAGS_concurrency; ++i) {
    thread_pool.push_back(new std::thread(thread_worker, &api, i, data_list));
  }

  for (int i = 0; i < FLAGS_concurrency; ++i) {
    thread_pool[i]->join();
    delete thread_pool[i];
  }

  calc_time();

  api.destroy();
  return 0;
}
