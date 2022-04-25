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
// limitations under the License

#include "core/predictor/framework/cache.h"
#include <dirent.h>
#include <sys/stat.h>
#include <fstream>
#include <string>
#include <utility>
#include "core/cube/cube-builder/include/cube-builder/seqfile_reader.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

int CubeCache::clear() {
  for (auto it = _map_cache.begin(); it != _map_cache.end(); ++it) {
    if (it->second) {
      delete (it->second);
      it->second = nullptr;
    }
  }
  _map_cache.clear();

  return 0;
}

rec::mcube::CubeValue* CubeCache::get_data(uint64_t key) {
  auto it = _map_cache.find(key);
  if (it != _map_cache.end()) {
    return it->second;
  }
  return nullptr;
}

int CubeCache::reload_data(const std::string& cache_path) {
  LOG(INFO) << "cube cache is loading data, path: " << cache_path;
  DIR* dp = nullptr;
  struct dirent* dirp = nullptr;
  struct stat st;

  // clear cache data
  clear();

  // loading data from cache files
  if (stat(cache_path.c_str(), &st) < 0 || !S_ISDIR(st.st_mode)) {
    LOG(WARNING) << "No cube cache directory " << cache_path << " provided, ignore it";
    return -1;
  }
  if ((dp = opendir(cache_path.c_str())) == nullptr) {
    LOG(ERROR) << "opendir " << cache_path << " fail.";
    return -1;
  }
  while ((dirp = readdir(dp)) != nullptr) {
    // filtering by file type.
    if (dirp->d_type != DT_REG) {
      continue;
    }
    // Filter upper-level directories and hidden files
    if ((!strncmp(dirp->d_name, ".", 1)) || (!strncmp(dirp->d_name, "..", 2))) {
      continue;
    }
    // Match the file whose name prefix is ​​'part-'
    if (std::string(dirp->d_name).find("part-") != std::string::npos) {
      SequenceFileRecordReader reader(cache_path + "/" + dirp->d_name);

      if (reader.open() != 0) {
        LOG(ERROR) << "open file failed! " << dirp->d_name;
        continue;
      }
      if (reader.read_header() != 0) {
        LOG(ERROR) << "read header error! " << dirp->d_name;
        reader.close();
        continue;
      }

      Record record(reader.get_header());
      while (reader.next(&record) == 0) {
        uint64_t key =
            *reinterpret_cast<uint64_t*>(const_cast<char*>(record.key.data()));

        auto it_find = _map_cache.find(key);
        if (it_find != _map_cache.end()) {
          // load dumplicate key
          LOG(WARNING) << "Load dumplicate key:" << key
                       << " from file:" << dirp->d_name;
          continue;
        }
        rec::mcube::CubeValue* new_value = new rec::mcube::CubeValue();
        new_value->error = 0;
        new_value->buff.swap(record.value);
        _map_cache.insert(std::make_pair(key, new_value));
      }

      LOG(WARNING) << "Load cube cache file " << dirp->d_name << " done.";
    }
    LOG(WARNING) << "Load all cube cache files done";
  }
  return 0;
}

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
