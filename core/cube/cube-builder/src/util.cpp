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

#include "core/cube/cube-builder/include/cube-builder/util.h"
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "butil/logging.h"

void getAllFiles(std::string path, std::vector<std::string> *files) {
  DIR *dir;
  struct dirent *ptr;
  if ((dir = opendir(path.c_str())) == NULL) {
    perror("Open dri error...");
    exit(1);
  }
  while ((ptr = readdir(dir)) != NULL) {
    if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
      continue;
    } else if ((ptr->d_type) == 8) {  // file
      if (ptr->d_name[0] != '.') files->push_back(path + "/" + ptr->d_name);
    } else if (ptr->d_type == 10) {  // link file
      continue;
    } else if (ptr->d_type == 4) {
      getAllFiles(path + "/" + ptr->d_name, files);
    }
  }
  closedir(dir);
}

std::string string_to_hex(const std::string &input) {
  static const char *const lut = "0123456789ABCDEF";
  size_t len = input.length();
  std::string output;
  output.reserve(2 * len);
  for (size_t i = 0; i < len; ++i) {
    const unsigned char c = input[i];
    output.push_back(lut[c >> 4]);
    output.push_back(lut[c & 15]);
  }
  return output;
}

bool checkDirectory(const std::string folder) {
  LOG(INFO) << "check dir:" << folder;
  if (access(folder.c_str(), F_OK) == 0) {
    return 1;
  }
  LOG(WARNING) << "no dir will mkdir:" << folder;
  return (mkdir(folder.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0);
}

void CmdTarfiles(const std::string folder) {
  std::string cmd = "cd " + folder + " && tar -cvPf " + folder + ".tar .";
  LOG(INFO) << "tar file cmd:" << cmd;
  system(cmd.c_str());
}

void CmdMd5sum(const std::string folder) {
  std::string cmd = "md5sum " + folder + ".tar > " + folder + ".tar.md5";
  LOG(INFO) << "md5sum file cmd:" << cmd;
  system(cmd.c_str());
}
