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

#include <gflags/gflags.h>
#include <algorithm>
#include <atomic>
#include <fstream>
#include <thread>  //NOLINT
#include "core/cube/cube-api/include/cube_api.h"
#define TIME_FLAG(flag) \
  struct timeval flag;  \
  gettimeofday(&(flag), NULL);

DEFINE_string(config_file, "./cube.conf", "m-cube config file");
DEFINE_string(keys, "keys", "keys to seek");
DEFINE_string(dict, "dict", "dict to seek");
DEFINE_uint64(batch, 500, "batch size");
DEFINE_int32(timeout, 200, "timeout in ms");
DEFINE_int32(retry, 3, "retry times");
DEFINE_bool(print_output, false, "print output flag");
DEFINE_int32(thread_num, 1, "thread num");
std::atomic<int> g_concurrency(0);

std::vector<std::vector<uint64_t>> time_list;
std::vector<uint64_t> request_list;
int turns = 1000;

namespace {
inline uint64_t time_diff(const struct timeval& start_time,
                          const struct timeval& end_time) {
  return (end_time.tv_sec - start_time.tv_sec) * 1000000 +
         (end_time.tv_usec - start_time.tv_usec);
}
}

namespace rec {
namespace mcube {
std::string string_to_hex(const std::string& input) {
  static const char* const lut = "0123456789ABCDEF";
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

int run(int argc, char** argv, int thread_id) {
  google::ParseCommandLineFlags(&argc, &argv, true);

  CubeAPI* cube = CubeAPI::instance();
  int ret = cube->init(FLAGS_config_file.c_str());
  if (ret != 0) {
    LOG(ERROR) << "init cube api failed err=" << ret;
    return ret;
  }
  /*
    FILE* key_file = fopen(FLAGS_keys.c_str(), "r");
    if (key_file == NULL) {
      LOG(ERROR) << "open key file [" << FLAGS_keys << "] failed";
      return -1;
    }
  */
  std::atomic<uint64_t> seek_counter(0);
  std::atomic<uint64_t> seek_cost_total(0);
  uint64_t seek_cost_max = 0;
  uint64_t seek_cost_min = 500000;

  char buffer[1024];
  std::vector<uint64_t> keys;
  std::vector<CubeValue> values;

  std::string line;
  std::vector<int64_t> key_list;
  std::ifstream key_file(FLAGS_keys.c_str());
  while (getline(key_file, line)) {
    key_list.push_back(std::stoll(line));
  }

  uint64_t file_size = key_list.size();
  uint64_t index = 0;
  uint64_t request = 0;
  while (g_concurrency.load() >= FLAGS_thread_num) {
  }
  g_concurrency++;
  time_list[thread_id].resize(turns);
  while (request < turns) {
    // uint64_t key = strtoul(buffer, NULL, 10);
    if (index >= file_size) {
      index = 0;
    }
    keys.push_back(key_list[index]);
    index += 1;
    int ret = 0;
    if (keys.size() >= FLAGS_batch) {
      TIME_FLAG(seek_start);
      ret = cube->seek(FLAGS_dict, keys, &values);
      TIME_FLAG(seek_end);
      request += 1;
      if (ret != 0) {
        LOG(WARNING) << "cube seek failed";
      } else if (FLAGS_print_output) {
        for (size_t i = 0; i < keys.size(); ++i) {
          fprintf(stdout,
                  "key:%lu value:%s\n",
                  keys[i],
                  string_to_hex(values[i].buff).c_str());
        }
      }
      ++seek_counter;
      uint64_t seek_cost = time_diff(seek_start, seek_end);
      time_list[thread_id][request - 1] = seek_cost;

      keys.clear();
      values.clear();
    }
  }
  g_concurrency--;

  // fclose(key_file);

  // ret = cube->destroy();
  if (ret != 0) {
    LOG(WARNING) << "destroy cube api failed err=" << ret;
  }

  request_list[thread_id] = request;

  return 0;
}

int run_m(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  int thread_num = FLAGS_thread_num;
  request_list.resize(thread_num);
  time_list.resize(thread_num);
  std::vector<std::thread*> thread_pool;
  TIME_FLAG(main_start);
  for (int i = 0; i < thread_num; i++) {
    thread_pool.push_back(new std::thread(run, argc, argv, i));
  }
  for (int i = 0; i < thread_num; i++) {
    thread_pool[i]->join();
    delete thread_pool[i];
  }
  TIME_FLAG(main_end);
  uint64_t sum_time = 0;
  uint64_t max_time = 0;
  uint64_t min_time = 1000000;
  std::vector<uint64_t> all_time_list;
  for (int i = 0; i < thread_num; i++) {
    for (int j = 0; j < request_list[i]; j++) {
      sum_time += time_list[i][j];
      if (time_list[i][j] > max_time) {
        max_time = time_list[i][j];
      }
      if (time_list[i][j] < min_time) {
        min_time = time_list[i][j];
      }
      all_time_list.push_back(time_list[i][j]);
    }
  }
  std::sort(all_time_list.begin(), all_time_list.end());
  uint64_t mean_time = sum_time / (thread_num * turns);
  uint64_t main_time = time_diff(main_start, main_end);
  uint64_t request_num = turns * thread_num;
  LOG(INFO)
      << "\n"
      << thread_num << " thread seek cost"
      << "\navg: " << std::to_string(mean_time) << "\n50 percent: "
      << std::to_string(all_time_list[static_cast<int>(0.5 * request_num)])
      << "\n80 percent: "
      << std::to_string(all_time_list[static_cast<int>(0.8 * request_num)])
      << "\n90 percent: "
      << std::to_string(all_time_list[static_cast<int>(0.9 * request_num)])
      << "\n99 percent: "
      << std::to_string(all_time_list[static_cast<int>(0.99 * request_num)])
      << "\n99.9 percent: "
      << std::to_string(all_time_list[static_cast<int>(0.999 * request_num)])
      << "\ntotal_request: " << std::to_string(request_num) << "\nspeed: "
      << std::to_string(turns * 1000000 / main_time)  // mean_time us
      << " query per second";
  return 0;
}

}  // namespace mcube
}  // namespace rec

int main(int argc, char** argv) { return ::rec::mcube::run_m(argc, argv); }
