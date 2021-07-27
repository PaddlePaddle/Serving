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
#include "core/sdk-cpp/echo_service.pb.h"
#include "core/sdk-cpp/include/common.h"
#include "core/sdk-cpp/include/predictor_sdk.h"

using baidu::paddle_serving::sdk_cpp::Predictor;
using baidu::paddle_serving::sdk_cpp::PredictorApi;
using baidu::paddle_serving::predictor::echo_service::RequestAndResponse;

int create_req(RequestAndResponse& req) {  // NOLINT
  req.set_a(1);
  req.set_b(0.1);
  return 0;
}

void print_res(const RequestAndResponse& req,
               const RequestAndResponse& res,
               std::string route_tag,
               uint64_t elapse_ms) {
  LOG(INFO) << "Reqeive result: a = " << res.a() << ", b = " << res.b();

  LOG(INFO) << "Succ call predictor[echo_service], the tag is: " << route_tag
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

  RequestAndResponse req;
  RequestAndResponse res;

  api.thrd_initialize();

  while (true) {
    timeval start;
    gettimeofday(&start, NULL);

    api.thrd_clear();

    Predictor* predictor = api.fetch_predictor("echo_service");
    if (!predictor) {
      LOG(ERROR) << "Failed fetch predictor: echo_service";
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

    usleep(50);
  }  // while (true)

  api.thrd_finalize();
  api.destroy();

#ifndef BCLOUD
  google::ShutdownGoogleLogging();
#endif

  return 0;
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
