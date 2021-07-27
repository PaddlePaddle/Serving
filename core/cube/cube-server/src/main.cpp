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

#include <signal.h>
#include <sys/stat.h>

#include <brpc/server.h>
#include <gflags/gflags.h>
#ifdef BCLOUD
#include "base/logging.h"
#else
#include "butil/logging.h"
#endif

#include "core/cube/cube-server/include/cube/control.h"
#include "core/cube/cube-server/include/cube/framework.h"
#include "core/cube/cube-server/include/cube/server.h"

DEFINE_int32(port, 8000, "TCP Port of this server");
DEFINE_int32(dict_split, 1, "data dict split for dictset");
DEFINE_bool(in_mem,
            true,
            "True[load data into memory] False[mmap data in disk]");
DECLARE_string(flagfile);

namespace rec {
namespace mcube {

bool g_signal_quit = false;
static void sigint_handler(int) { g_signal_quit = true; }  // sigint_handler

int run(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);

// initialize logger instance
#ifdef BCLOUD
  logging::LoggingSettings settings;
  settings.logging_dest = logging::LOG_TO_FILE;

  std::string filename(argv[0]);
  filename = filename.substr(filename.find_last_of('/') + 1);
  settings.log_file =
      strdup((std::string("./log/") + filename + ".log").c_str());
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

  Framework* framework = Framework::instance();
  ret = framework->init(FLAGS_dict_split, FLAGS_in_mem);
  if (ret != 0) {
    LOG(ERROR) << "init predict framework failed";
    return ret;
  }

  Server cube;
  Control cntl;

  brpc::Server server;
  server.set_version("Cube Service");
  brpc::ServerOptions option;

  if (server.AddService(&cube, brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
    LOG(ERROR) << "Failed to add predict service";
    return -1;
  }

  if (server.AddService(&cntl, brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
    LOG(ERROR) << "Failed to add predict service";
    return -1;
  }

  if (server.Start(FLAGS_port, &option) != 0) {
    LOG(ERROR) << "Fail to start service";
    return -1;
  }
  LOG(INFO) << "cube service start";

  signal(SIGINT, sigint_handler);
  while (!g_signal_quit) {
    sleep(1);
  }

  return 0;
}

}  // namespace mcube
}  // namespace rec

int main(int argc, char** argv) {
  if (google::SetCommandLineOption("bvar_dump", "true").empty()) {
    LOG(ERROR) << "Failed to dump bvar file";
    return -1;
  }
  google::SetCommandLineOption("flagfile", "conf/gflags.conf");
  return ::rec::mcube::run(argc, argv);
}

/* vim: set ts=4 sw=4 sts=4 tw=100 */
