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

std::vector<uint64_t> time_list;
std::vector<uint64_t> request_list;

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

  while (index < file_size) {
    // uint64_t key = strtoul(buffer, NULL, 10);

    keys.push_back(key_list[index]);
    index += 1;
    int ret = 0;
    if (keys.size() > FLAGS_batch) {
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
      seek_cost_total += seek_cost;
      if (seek_cost > seek_cost_max) {
        seek_cost_max = seek_cost;
      }
      if (seek_cost < seek_cost_min) {
        seek_cost_min = seek_cost;
      }

      keys.clear();
      values.clear();
    }
  }
  /*
    if (keys.size() > 0) {
      int ret = 0;
      values.resize(keys.size());
      TIME_FLAG(seek_start);
      ret = cube->seek(FLAGS_dict, keys, &values);
      TIME_FLAG(seek_end);
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
      seek_cost_total += seek_cost;
      if (seek_cost > seek_cost_max) {
        seek_cost_max = seek_cost;
      }
      if (seek_cost < seek_cost_min) {
        seek_cost_min = seek_cost;
      }
    }
  */
  g_concurrency--;

  // fclose(key_file);

  // ret = cube->destroy();
  if (ret != 0) {
    LOG(WARNING) << "destroy cube api failed err=" << ret;
  }

  uint64_t seek_cost_avg = seek_cost_total / seek_counter;
  LOG(INFO) << "seek cost avg = " << seek_cost_avg;
  LOG(INFO) << "seek cost max = " << seek_cost_max;
  LOG(INFO) << "seek cost min = " << seek_cost_min;

  time_list[thread_id] = seek_cost_avg;
  request_list[thread_id] = request;

  return 0;
}

int run_m(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  int thread_num = FLAGS_thread_num;
  request_list.resize(thread_num);
  time_list.resize(thread_num);
  std::vector<std::thread*> thread_pool;
  for (int i = 0; i < thread_num; i++) {
    thread_pool.push_back(new std::thread(run, argc, argv, i));
  }
  for (int i = 0; i < thread_num; i++) {
    thread_pool[i]->join();
    delete thread_pool[i];
  }
  uint64_t sum_time = 0;
  uint64_t max_time = 0;
  uint64_t min_time = 1000000;
  uint64_t request_num = 0;
  for (int i = 0; i < thread_num; i++) {
    sum_time += time_list[i];
    if (time_list[i] > max_time) {
      max_time = time_list[i];
    }
    if (time_list[i] < min_time) {
      min_time = time_list[i];
    }
    request_num += request_list[i];
  }
  uint64_t mean_time = sum_time / thread_num;
  LOG(INFO) << thread_num << " thread seek cost"
            << " avg = " << std::to_string(mean_time)
            << " max = " << std::to_string(max_time)
            << " min = " << std::to_string(min_time);
  LOG(INFO) << " total_request = " << std::to_string(request_num)
            << " speed = " << std::to_string(1000000 * thread_num / mean_time)
            << " query per second";
}

}  // namespace mcube
}  // namespace rec

int main(int argc, char** argv) { return ::rec::mcube::run_m(argc, argv); }
