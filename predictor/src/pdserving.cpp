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

#ifdef BCLOUD
#include <bthread_unstable.h>  // bthread_set_worker_startfn
#else
#include <bthread/unstable.h>  // bthread_set_worker_startfn
#endif

#include <fstream>
#include <iostream>

#include "predictor/common/constant.h"
#include "predictor/common/inner_common.h"
#include "predictor/framework/manager.h"
#include "predictor/framework/resource.h"
#include "predictor/framework/server.h"
#include "predictor/framework/service.h"
#include "predictor/framework/workflow.h"

using baidu::paddle_serving::predictor::ServerManager;
using baidu::paddle_serving::predictor::WorkflowManager;
using baidu::paddle_serving::predictor::InferServiceManager;
using baidu::paddle_serving::predictor::Resource;
using baidu::paddle_serving::predictor::FLAGS_workflow_path;
using baidu::paddle_serving::predictor::FLAGS_workflow_file;
using baidu::paddle_serving::predictor::FLAGS_inferservice_path;
using baidu::paddle_serving::predictor::FLAGS_inferservice_file;
using baidu::paddle_serving::predictor::FLAGS_logger_path;
using baidu::paddle_serving::predictor::FLAGS_logger_file;
using baidu::paddle_serving::predictor::FLAGS_resource_path;
using baidu::paddle_serving::predictor::FLAGS_resource_file;
using baidu::paddle_serving::predictor::FLAGS_reload_interval_s;
using baidu::paddle_serving::predictor::FLAGS_port;

using baidu::paddle_serving::configure::InferServiceConf;
using baidu::paddle_serving::configure::read_proto_conf;

void print_revision(std::ostream& os, void*) {
#if defined(PDSERVING_VERSION)
  os << PDSERVING_VERSION;
#else
  os << "undefined";
#endif
#if defined(PDSERVING_BUILDTIME)
  os << ", BuildAt: " << PDSERVING_BUILDTIME;
#endif
}

static bvar::PassiveStatus<std::string> s_predictor_revision(
    "predictor_revision", print_revision, NULL);

DEFINE_bool(V, false, "print version, bool");
DEFINE_bool(g, false, "user defined gflag path");
DECLARE_string(flagfile);

namespace bthread {
extern pthread_mutex_t g_task_control_mutex;
}

void pthread_worker_start_fn() {
  while (pthread_mutex_lock(&bthread::g_task_control_mutex) != 0) {}
  Resource::instance().thread_initialize();
  pthread_mutex_unlock(&bthread::g_task_control_mutex);
}

static void g_change_server_port() {
  InferServiceConf conf;
  if (read_proto_conf(FLAGS_inferservice_path.c_str(),
                      FLAGS_inferservice_file.c_str(),
                      &conf) != 0) {
    LOG(WARNING) << "failed to load configure[" << FLAGS_inferservice_path
                 << "," << FLAGS_inferservice_file << "].";
    return;
  }
  uint32_t port = conf.port();
  if (port != 0) {
    FLAGS_port = port;
    LOG(INFO) << "use configure[" << FLAGS_inferservice_path << "/"
              << FLAGS_inferservice_file << "] port[" << port
              << "] instead of flags";
  }
  return;
}

#ifdef UNIT_TEST
int ut_main(int argc, char** argv) {
#else
int main(int argc, char** argv) {
#endif
  google::ParseCommandLineFlags(&argc, &argv, true);

  if (FLAGS_V) {
    print_revision(std::cout, NULL);
    std::cout << std::flush;
    return 0;
  }

  if (!FLAGS_g) {
    google::SetCommandLineOption("flagfile", "conf/gflags.conf");
  }

  google::ParseCommandLineFlags(&argc, &argv, true);

  g_change_server_port();

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
  if (FLAGS_log_dir == "") {
    FLAGS_log_dir = "./log";
  }

  struct stat st_buf;
  int ret = 0;
  if ((ret = stat(FLAGS_log_dir.c_str(), &st_buf)) != 0) {
    mkdir(FLAGS_log_dir.c_str(), 0777);
    ret = stat(FLAGS_log_dir.c_str(), &st_buf);
    if (ret != 0) {
      LOG(WARNING) << "Log path " << FLAGS_log_dir
                   << " not exist, and create fail";
      return -1;
    }
  }
  google::InitGoogleLogging(strdup(argv[0]));
  FLAGS_logbufsecs = 0;
  FLAGS_logbuflevel = -1;
#endif
  LOG(INFO) << "Succ initialize logger";

  // initialize resource manager
  if (Resource::instance().initialize(FLAGS_resource_path,
                                      FLAGS_resource_file) != 0) {
    LOG(ERROR) << "Failed initialize resource, conf:" << FLAGS_resource_path
               << "/" << FLAGS_resource_file;
    return -1;
  }
  LOG(INFO) << "Succ initialize resource";

  // initialize workflow manager
  if (WorkflowManager::instance().initialize(FLAGS_workflow_path,
                                             FLAGS_workflow_file) != 0) {
    LOG(ERROR) << "Failed initialize workflow manager, conf:"
               << FLAGS_workflow_path << "/" << FLAGS_workflow_file;
    return -1;
  }
  LOG(INFO) << "Succ initialize workflow";

  // initialize service manager
  if (InferServiceManager::instance().initialize(
          FLAGS_inferservice_path, FLAGS_inferservice_file) != 0) {
    LOG(ERROR) << "Failed initialize infer service manager, conf:"
               << FLAGS_inferservice_path << "/" << FLAGS_inferservice_file;
    return -1;
  }
  LOG(INFO) << "Succ initialize inferservice";

  int errcode = bthread_set_worker_startfn(pthread_worker_start_fn);
  if (errcode != 0) {
    LOG(ERROR) << "Failed call pthread worker start function, error_code["
               << errcode << "]";
    return -1;
  }
  LOG(INFO) << "Succ call pthread worker start function";

  if (ServerManager::instance().start_and_wait() != 0) {
    LOG(ERROR) << "Failed start server and wait!";
    return -1;
  }
  LOG(INFO) << "Succ start service manager";

  if (InferServiceManager::instance().finalize() != 0) {
    LOG(ERROR) << "Failed finalize infer service manager.";
  }

  if (WorkflowManager::instance().finalize() != 0) {
    LOG(ERROR) << "Failed finalize workflow manager";
  }

  if (Resource::instance().finalize() != 0) {
    LOG(ERROR) << "Failed finalize resource manager";
  }

#ifdef BCLOUD
#else
  google::ShutdownGoogleLogging();
#endif
  LOG(INFO) << "Paddle Inference Server exit successfully!";
  return 0;
}
