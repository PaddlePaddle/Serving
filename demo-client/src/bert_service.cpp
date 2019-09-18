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
#include <string>
#include <thread>  //NOLINT
#include "sdk-cpp/bert_service.pb.h"
#include "sdk-cpp/include/common.h"
#include "sdk-cpp/include/predictor_sdk.h"

using baidu::paddle_serving::sdk_cpp::Predictor;
using baidu::paddle_serving::sdk_cpp::PredictorApi;
using baidu::paddle_serving::predictor::bert_service::Request;
using baidu::paddle_serving::predictor::bert_service::Response;
using baidu::paddle_serving::predictor::bert_service::BertResInstance;
using baidu::paddle_serving::predictor::bert_service::BertReqInstance;
using baidu::paddle_serving::predictor::bert_service::Embedding_values;

int batch_size = 49;
int max_seq_len = 82;
int layer_num = 12;
int emb_size = 768;
int thread_num = 1;

std::atomic<int> g_concurrency(0);
std::vector<std::vector<int>> response_time;
char* data_filename = "./data/bert/demo_wiki_train";

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
/*
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
      if (fi < token_list.size()) {
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
  return 0;
}
*/

int create_req(Request* req,
               const std::vector<std::string>& data_list,
               int data_index,
               int batch_size) {
  // add data
  // avoid out of boundary
  int cur_index = data_index;
  if (cur_index >= data_list.size()) {
    cur_index = cur_index % data_list.size();
  }

  std::vector<std::string> feature_list = split(data_list[cur_index], ";");

  std::vector<std::string> src_field = split(feature_list[0], ":");
  std::vector<std::string> src_ids = split(src_field[1], " ");

  std::vector<std::string> pos_field = split(feature_list[1], ":");
  std::vector<std::string> pos_ids = split(pos_field[1], " ");

  std::vector<std::string> sent_field = split(feature_list[2], ":");
  std::vector<std::string> sent_ids = split(sent_field[1], " ");

  std::vector<std::string> mask_field = split(feature_list[3], ":");
  std::vector<std::string> input_mask = split(mask_field[1], " ");

  std::vector<int> shape;
  std::vector<std::string> shapes = split(src_field[0], " ");
  for (auto x: shapes) {
    shape.push_back(std::stoi(x));
  }

  for (int i = 0; i < batch_size && i < shape[0]; ++i) {
    BertReqInstance* ins = req->add_instances();
    if (!ins) {
      LOG(ERROR) << "Failed create req instance";
      return -1;
    }
    for (int fi = 0; fi < max_seq_len; fi++) {
      ins->add_token_ids(std::stoi(src_ids[i * max_seq_len + fi]));
      ins->add_position_ids(std::stoi(pos_ids[i * max_seq_len + fi]));
      ins->add_sentence_type_ids(std::stoi(sent_ids[i * max_seq_len + fi]));
      ins->add_input_masks(std::stof(input_mask[i * max_seq_len + fi]));
    }
  }
  return 0;
}

#if 0
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

    std::vector<std::string> feature_list = split(data_list[cur_index], ":");
    std::vector<std::string> shape_list = split(feature_list[0]," ");
    std::vector<std::string> token_list = split(feature_list[1], " ");
    std::vector<std::string> pos_list = split(feature_list[2], " ");
    std::vector<std::string> seg_list = split(feature_list[3], " ");
    std::vector<std::string> mask_list = split(feature_list[4], " ");
    for (int fi = 0; fi < max_seq_len; fi++) {
      if (fi < std::stoi(shape_list[1])) {
        ins->add_token_ids(std::stoi(token_list[fi + (i * max_seq_len)]));
        ins->add_sentence_type_ids(std::stoll(seg_list[fi + (i * max_seq_len)]));
        ins->add_position_ids(std::stoll(pos_list[fi + (i * max_seq_len)]));
        ins->add_input_masks(std::stof(mask_list[fi + (i * max_seq_len)]));
      } else {
        ins->add_token_ids(0);
        ins->add_sentence_type_ids(0);
        ins->add_position_ids(0);
        ins->add_input_masks(0.0);
      }
    }
  }
  return 0;
}
#endif

void print_res(const Request& req,
               const Response& res,
               std::string route_tag,
               uint64_t elapse_ms) {
  for (uint32_t ri = 0; ri < res.instances_size(); ri++) {
    const BertResInstance& res_ins = res.instances(ri);
    std::ostringstream oss;
    oss << "[";
    for (uint32_t bi = 0; bi < res_ins.instances_size(); bi++) {
      const Embedding_values& emb_ins = res_ins.instances(bi);
      oss << "[";
      for (uint32_t ei = 0; ei < emb_ins.values_size(); ei++) {
        oss << emb_ins.values(ei) << " ";
      }
      oss << "],";
    }
    oss << "]\n";
    LOG(INFO) << "Receive : " << oss.str();
  }
  LOG(INFO) << "Succ call predictor[ctr_prediction_service], the tag is: "
            << route_tag << ", elapse_ms: " << elapse_ms;
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
  while (turns < 1000) {
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
#if 0
    int data_index = turns * batch_size;
    if (create_req(&req, data_list, data_index, batch_size) != 0) {
      return;
    }
#else
    if (create_req(&req, data_list, turns, batch_size) != 0) {
      return;
    }
#endif
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
  }
}
int main(int argc, char** argv) {
  PredictorApi api;
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
