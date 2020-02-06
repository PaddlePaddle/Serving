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

#include "core/configure/include/configure_parser.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>

#ifdef BCLOUD
#include "base/logging.h"
#else
#include "butil/logging.h"
#endif

#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/text_format.h"

namespace baidu {
namespace paddle_serving {
namespace configure {

int read_proto_conf(const std::string &conf_file_full_path,
                    google::protobuf::Message *conf) {
  int fd = open(conf_file_full_path.c_str(), O_RDONLY);
  if (fd == -1) {
    LOG(WARNING) << "File not found: " << conf_file_full_path.c_str();
    return -1;
  }

  google::protobuf::io::FileInputStream input(fd);
  bool success = google::protobuf::TextFormat::Parse(&input, conf);
  close(fd);
  if (!success) {
    return -1;
  }

  return 0;
}

int read_proto_conf(const std::string &conf_path,
                    const std::string &conf_file,
                    google::protobuf::Message *conf) {
  std::string file_str = conf_path + "/" + conf_file;
  int fd = open(file_str.c_str(), O_RDONLY);
  if (fd == -1) {
    LOG(WARNING) << "File not found: " << file_str.c_str();
    return -1;
  }

  google::protobuf::io::FileInputStream input(fd);
  bool success = google::protobuf::TextFormat::Parse(&input, conf);
  close(fd);
  if (!success) {
    return -1;
  }

  return 0;
}

int write_proto_conf(google::protobuf::Message *message,
                     const std::string &output_path,
                     const std::string &output_file) {
  std::string binary_str;
  google::protobuf::TextFormat::PrintToString(*message, &binary_str);

  std::string file_str = output_path + "/" + output_file;
  std::ofstream fout_bin((file_str.c_str()));
  if (!fout_bin) {
    LOG(WARNING) << "Open file error: " << file_str.c_str();
    return -1;
  }

  fout_bin.write(binary_str.c_str(), binary_str.size());
  fout_bin.close();

  return 0;
}

}  // namespace configure
}  // namespace paddle_serving
}  // namespace baidu
/* vim: set expandtab ts=2 sw=2 sts=2 tw=100: */
