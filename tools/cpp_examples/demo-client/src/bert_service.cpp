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
#include <iostream>
#include <string>
#include <thread>  //NOLINT
#include <vector>
#include "core/sdk-cpp/bert_service.pb.h"
#include "core/sdk-cpp/include/common.h"
#include "core/sdk-cpp/include/predictor_sdk.h"
using baidu::paddle_serving::sdk_cpp::Predictor;
using baidu::paddle_serving::sdk_cpp::PredictorApi;
using baidu::paddle_serving::predictor::bert_service::Request;
using baidu::paddle_serving::predictor::bert_service::Response;
using baidu::paddle_serving::predictor::bert_service::BertResInstance;
using baidu::paddle_serving::predictor::bert_service::BertReqInstance;
using baidu::paddle_serving::predictor::bert_service::EmbeddingValues;

extern int batch_size = 1;
extern int max_seq_len = 128;
extern int layer_num = 12;
extern int thread_num = 1;
extern int max_turn = 1000;

std::atomic<int> g_concurrency(0);
std::vector<std::vector<int>> response_time;
std::vector<std::vector<int>> infer_time;
char* data_filename = "./data/bert/demo_wiki_data";

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

int create_req(Request* req,
               const std::vector<std::string>& data_list,
               int data_index,
               int batch_size) {
  for (int i = 0; i < batch_size; ++i) {
    BertReqInstance* ins = req->add_instances();
    if (!ins) {
      LOG(ERROR) << "Failed create req instance";
      return -1;
    }
    // add data
    // avoid out of boundary
    int cur_index = data_index + i;
    if (cur_index >= data_list.size()) {
      cur_index = cur_index % data_list.size();
    }

    std::vector<std::string> feature_list = split(data_list[cur_index], ";");
    std::vector<std::string> token_list = split(feature_list[0], " ");
    std::vector<std::string> seg_list = split(feature_list[1], " ");
    std::vector<std::string> pos_list = split(feature_list[2], " ");
    for (int fi = 0; fi < max_seq_len; fi++) {
      if (std::stoi(token_list[fi]) != 0) {
        ins->add_token_ids(std::stoi(token_list[fi]));
        ins->add_sentence_type_ids(std::stoi(seg_list[fi]));
        ins->add_position_ids(std::stoi(pos_list[fi]));
        ins->add_input_masks(1.0);
      } else {
        ins->add_token_ids(0);
        ins->add_sentence_type_ids(0);
        ins->add_position_ids(0);
        ins->add_input_masks(0.0);
      }
    }
  }
  req->set_max_seq_len(max_seq_len);
  return 0;
}

void print_res(const Request& req,
               const Response& res,
               std::string route_tag,
               uint64_t elapse_ms) {
  for (uint32_t ri = 0; ri < res.instances_size(); ri++) {
    const BertResInstance& res_ins = res.instances(ri);
    std::ostringstream oss;
    oss << "[";
    for (uint32_t bi = 0; bi < res_ins.instances_size(); bi++) {
      const EmbeddingValues& emb_ins = res_ins.instances(bi);
      oss << "[";
      for (uint32_t ei = 0; ei < emb_ins.values_size(); ei++) {
        oss << emb_ins.values(ei) << " ";
      }
      oss << "],";
    }
    oss << "]\n";
    LOG(INFO) << "Receive : " << oss.str();
  }
  LOG(INFO) << "Succ call predictor[bert_service], the tag is: " << route_tag
            << ", elapse_ms: " << elapse_ms;
}

void thread_worker(PredictorApi* api,
                   int thread_id,
                   int batch_size,
                   int server_concurrency,
                   const std::vector<std::string>& data_list) {
  Request req;
  Response res;
  api->thrd_initialize();
  std::string line;
  int turns = 0;
  while (turns < max_turn) {
    timeval start;
    gettimeofday(&start, NULL);
    api->thrd_clear();
    Predictor* predictor = api->fetch_predictor("bert_service");
    if (!predictor) {
      LOG(ERROR) << "Failed fetch predictor: bert_service";
      return;
    }
    req.Clear();
    res.Clear();
    while (g_concurrency.load() >= server_concurrency) {
    }
    g_concurrency++;
    LOG(INFO) << "Current concurrency " << g_concurrency.load();
    int data_index = turns * batch_size;
    if (create_req(&req, data_list, data_index, batch_size) != 0) {
      return;
    }
    if (predictor->inference(&req, &res) != 0) {
      LOG(ERROR) << "failed call predictor with req:" << req.ShortDebugString();
      return;
    }
    timeval end;
    gettimeofday(&end, NULL);
    uint64_t elapse_ms = (end.tv_sec * 1000 + end.tv_usec / 1000) -
                         (start.tv_sec * 1000 + start.tv_usec / 1000);
    response_time[thread_id].push_back(elapse_ms);
    print_res(req, res, predictor->tag(), elapse_ms);
    g_concurrency--;
    LOG(INFO) << "Done. Current concurrency " << g_concurrency.load();
    turns++;
  }
  api->thrd_finalize();
}

void calc_time(int server_concurrency, int batch_size) {
  std::vector<int> time_list;
  for (auto a : response_time) {
    time_list.insert(time_list.end(), a.begin(), a.end());
  }
  LOG(INFO) << "Total request : " << (time_list.size());
  LOG(INFO) << "Batch size : " << batch_size;
  LOG(INFO) << "Max concurrency : " << server_concurrency;
  LOG(INFO) << "Max Seq Len : " << max_seq_len;
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
  LOG(INFO) << "Total time : " << total_time / server_concurrency
            << " Variance : " << var_time << " Max time : " << max_time
            << " Min time : " << min_time;
  float qps = 0.0;
  if (total_time > 0)
    qps = (time_list.size() * 1000) / (total_time / server_concurrency);
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
    LOG(INFO) << "N/A";
    LOG(INFO) << "N/A";
    LOG(INFO) << "N/A";
    LOG(INFO) << "N/A";
    LOG(INFO) << "N/A";
  }
}
int main(int argc, char** argv) {
  PredictorApi api;
  if (argc > 1) {
    thread_num = std::stoi(argv[1]);
    batch_size = std::stoi(argv[2]);
    max_seq_len = std::stoi(argv[3]);
  }
  response_time.resize(thread_num);
  int server_concurrency = thread_num;
// log set
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
  for (int i = 0; i < server_concurrency; ++i) {
    thread_pool.push_back(new std::thread(thread_worker,
                                          &api,
                                          i,
                                          batch_size,
                                          server_concurrency,
                                          std::ref(data_list)));
  }
  for (int i = 0; i < server_concurrency; ++i) {
    thread_pool[i]->join();
    delete thread_pool[i];
  }
  calc_time(server_concurrency, batch_size);
  api.destroy();
  return 0;
}
