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
#include <thread>  // NOLINT
#include "core/sdk-cpp/builtin_format.pb.h"
#include "core/sdk-cpp/image_class.pb.h"
#include "core/sdk-cpp/include/common.h"
#include "core/sdk-cpp/include/predictor_sdk.h"

#ifndef BCLOUD
using json2pb::JsonToProtoMessage;
#endif

using baidu::paddle_serving::sdk_cpp::Predictor;
using baidu::paddle_serving::sdk_cpp::PredictorApi;
using baidu::paddle_serving::predictor::format::XImageReqInstance;
using baidu::paddle_serving::predictor::format::DensePrediction;
using baidu::paddle_serving::predictor::image_classification::Request;
using baidu::paddle_serving::predictor::image_classification::Response;

DEFINE_int32(concurrency, 1, "Set the max concurrent number of requests");
DEFINE_int32(requests, 100, "Number of requests to send per thread");
DEFINE_int32(batch_size, 1, "Batch size");

std::atomic<int> g_concurrency(0);

std::vector<std::vector<uint64_t>> g_round_time;
std::vector<char*> g_image_data;
std::vector<size_t> g_image_lengths;

const std::vector<std::string> g_image_paths{
    "./data/images/ILSVRC2012_val_00000001.jpeg",
    "./data/images/ILSVRC2012_val_00000002.jpeg",
    "./data/images/ILSVRC2012_val_00000003.jpeg",
    "./data/images/ILSVRC2012_val_00000004.jpeg",
    "./data/images/ILSVRC2012_val_00000005.jpeg",
    "./data/images/ILSVRC2012_val_00000006.jpeg",
    "./data/images/ILSVRC2012_val_00000007.jpeg",
    "./data/images/ILSVRC2012_val_00000008.jpeg",
    "./data/images/ILSVRC2012_val_00000009.jpeg",
    "./data/images/ILSVRC2012_val_00000010.jpeg"};

int prepare_data() {
  for (auto x : g_image_paths) {
    FILE* fp = fopen(x.c_str(), "rb");
    if (!fp) {
      LOG(ERROR) << "Failed open image: " << x.c_str();
      continue;
    }

    fseek(fp, 0L, SEEK_END);
    size_t isize = ftell(fp);
    char* ibuf = new (std::nothrow) char[isize];
    if (!ibuf) {
      LOG(ERROR) << "Failed malloc image buffer";
      fclose(fp);
      return -1;
    }

    fseek(fp, 0, SEEK_SET);
    fread(ibuf, sizeof(ibuf[0]), isize, fp);
    g_image_data.push_back(ibuf);
    g_image_lengths.push_back(isize);

    fclose(fp);
  }

  return 0;
}

int create_req(Request& req) {  // NOLINT
  for (int i = 0; i < FLAGS_batch_size; ++i) {
    XImageReqInstance* ins = req.add_instances();
    if (!ins) {
      LOG(ERROR) << "Failed create req instance";
      return -1;
    }

    int id = i % g_image_data.size();
    ins->set_image_binary(g_image_data[id], g_image_lengths[id]);
    ins->set_image_length(g_image_lengths[id]);
  }

  return 0;
}

void extract_res(const Request& req, const Response& res) {
  uint32_t sample_size = res.predictions_size();
  std::string err_string;
  for (uint32_t si = 0; si < sample_size; ++si) {
    DensePrediction json_msg;
    std::string json = res.predictions(si).response_json();
    butil::IOBuf buf;
    buf.clear();
    buf.append(json);
    butil::IOBufAsZeroCopyInputStream wrapper(buf);
    if (!JsonToProtoMessage(&wrapper, &json_msg, &err_string)) {
      LOG(ERROR) << "Failed parse json from str:" << json;
      return;
    }

    uint32_t csize = json_msg.categories_size();
    if (csize <= 0) {
      LOG(ERROR) << "sample-" << si << "has no"
                 << "categories props";
      continue;
    }
    float max_prop = json_msg.categories(0);
    uint32_t max_idx = 0;
    for (uint32_t ci = 1; ci < csize; ++ci) {
      if (json_msg.categories(ci) > max_prop) {
        max_prop = json_msg.categories(ci);
        max_idx = ci;
      }
    }

    LOG(INFO) << "instance " << si << "has class " << max_idx;
  }  // end for
}

void thread_worker(PredictorApi* api, int thread_id) {
  Request req;
  Response res;

  api->thrd_initialize();

  for (int i = 0; i < FLAGS_requests; ++i) {
    api->thrd_clear();

    Predictor* predictor = api->fetch_predictor("ximage");
    if (!predictor) {
      LOG(ERROR) << "Failed fetch predictor: ximage";
      return;
    }

    req.Clear();
    res.Clear();

    if (create_req(req) != 0) {
      return;
    }

    while (g_concurrency.load() >= FLAGS_concurrency) {
    }
    g_concurrency++;
#if 1
    LOG(INFO) << "Current concurrency " << g_concurrency.load();
#endif

    timeval start;
    timeval end;

    gettimeofday(&start, NULL);
    if (predictor->inference(&req, &res) != 0) {
      LOG(ERROR) << "failed call predictor with req:" << req.ShortDebugString();
      return;
    }
    gettimeofday(&end, NULL);

    g_round_time[thread_id].push_back(end.tv_sec * 1000 + end.tv_usec / 1000 -
                                      start.tv_sec * 1000 -
                                      start.tv_usec / 1000);

    extract_res(req, res);
    res.Clear();

    g_concurrency--;
#if 1
    LOG(INFO) << "Done. Currenct concurrency " << g_concurrency.load();
#endif
  }  // for (int i = 0; i < FLAGS_requests; ++i)

  api->thrd_finalize();
}

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);

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
#endif

  g_round_time.resize(FLAGS_concurrency);

  if (api.create("./conf", "predictors.prototxt") != 0) {
    LOG(ERROR) << "Failed create predictors api!";
    return -1;
  }

  if (prepare_data() != 0) {
    LOG(ERROR) << "Prepare data fail";
    return -1;
  }

  std::vector<std::thread*> worker_threads;
  int i = 0;
  for (; i < FLAGS_concurrency; ++i) {
    worker_threads.push_back(new std::thread(thread_worker, &api, i));
  }

  for (i = 0; i < FLAGS_concurrency; ++i) {
    worker_threads[i]->join();
    delete worker_threads[i];
  }

  api.destroy();

  std::vector<uint64_t> round_times;
  for (auto x : g_round_time) {
    round_times.insert(round_times.end(), x.begin(), x.end());
  }

  std::sort(round_times.begin(), round_times.end());

  int percent_pos_50 = round_times.size() * 0.5;
  int percent_pos_80 = round_times.size() * 0.8;
  int percent_pos_90 = round_times.size() * 0.9;
  int percent_pos_99 = round_times.size() * 0.99;
  int percent_pos_999 = round_times.size() * 0.999;

  uint64_t total_ms = 0;
  for (auto x : round_times) {
    total_ms += x;
  }

  LOG(INFO) << "Batch size: " << FLAGS_batch_size;
  LOG(INFO) << "Total requests: " << round_times.size();
  LOG(INFO) << "Max concurrency: " << FLAGS_concurrency;
  LOG(INFO) << "Total ms (absolute time): " << total_ms / FLAGS_concurrency;

  double qps = 0.0;
  if (total_ms != 0) {
    qps = (static_cast<double>(FLAGS_concurrency * FLAGS_requests) /
           (total_ms / FLAGS_concurrency)) *
          1000;
  }

  LOG(INFO) << "QPS: " << qps << "/s";

  LOG(INFO) << "Latency statistics: ";
  if (round_times.size() != 0) {
    LOG(INFO) << "Average ms: "
              << static_cast<float>(total_ms) / round_times.size();
    LOG(INFO) << "50 percent ms: " << round_times[percent_pos_50];
    LOG(INFO) << "80 percent ms: " << round_times[percent_pos_80];
    LOG(INFO) << "90 percent ms: " << round_times[percent_pos_90];
    LOG(INFO) << "99 percent ms: " << round_times[percent_pos_99];
    LOG(INFO) << "99.9 percent ms: " << round_times[percent_pos_999];
  } else {
    LOG(INFO) << "N/A";
  }

  return 0;
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
