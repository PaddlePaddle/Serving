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

int create_req(Request& req) {  // NOLINT
  static const char* TEST_IMAGE_PATH = "./data/images/what.jpg";

  FILE* fp = fopen(TEST_IMAGE_PATH, "rb");
  if (!fp) {
    LOG(ERROR) << "Failed open image: " << TEST_IMAGE_PATH;
    return -1;
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
  XImageReqInstance* ins = req.add_instances();
  if (!ins) {
    LOG(ERROR) << "Failed create req instance";
    delete[] ibuf;
    fclose(fp);
    return -1;
  }

  ins->set_image_binary(ibuf, isize);
  ins->set_image_length(isize);

  delete[] ibuf;
  fclose(fp);

  return 0;
}

void print_res(const Request& req,
               const Response& res,
               std::string route_tag,
               uint64_t elapse_ms) {
  static const char* GT_TEXT_PATH = "./data/images/groundtruth.txt";
  std::vector<std::string> gt_labels;

  std::ifstream file(GT_TEXT_PATH);
  std::string temp_str;
  while (std::getline(file, temp_str)) {
    gt_labels.push_back(temp_str);
  }

  DensePrediction json_msg;
  uint32_t sample_size = res.predictions_size();
  std::string err_string;
  for (uint32_t si = 0; si < sample_size; ++si) {
    std::string json = res.predictions(si).response_json();
    butil::IOBuf buf;
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

    LOG(INFO) << "sample-" << si << "'s classify result: " << gt_labels[max_idx]
              << ", prop: " << max_prop;
  }

  LOG(INFO) << "Succ call predictor[ximage], the tag is: " << route_tag
            << ", elapse_ms: " << elapse_ms;
}

int main(int argc, char** argv) {
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

  if (api.create("./conf", "predictors.prototxt") != 0) {
    LOG(ERROR) << "Failed create predictors api!";
    return -1;
  }

  Request req;
  Response res;

  api.thrd_initialize();

  int count = 0;
  while (count < 10) {
    timeval start;
    gettimeofday(&start, NULL);

    api.thrd_clear();

    Predictor* predictor = api.fetch_predictor("ximage");
    if (!predictor) {
      LOG(ERROR) << "Failed fetch predictor: ximage";
      return -1;
    }

    req.Clear();
    res.Clear();

    if (create_req(req) != 0) {
      return -1;
    }

    butil::IOBufBuilder debug_os;
    if (predictor->debug(&req, &res, &debug_os) != 0) {
      LOG(ERROR) << "failed call predictor with req:" << req.ShortDebugString();
      return -1;
    }

    butil::IOBuf debug_buf;
    debug_os.move_to(debug_buf);
    LOG(INFO) << "Debug string: " << debug_buf;

    timeval end;
    gettimeofday(&end, NULL);

    uint64_t elapse_ms = (end.tv_sec * 1000 + end.tv_usec / 1000) -
                         (start.tv_sec * 1000 + start.tv_usec / 1000);

    print_res(req, res, predictor->tag(), elapse_ms);
    res.Clear();

    ++count;
  }  // while (true)

  api.thrd_finalize();
  api.destroy();

  return 0;
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
