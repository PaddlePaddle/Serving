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
#ifndef SERVING_DEMO_CLIENT_SRC_DATA_PRE_H_
#define SERVING_DEMO_CLIENT_SRC_DATA_PRE_H_
#include <sys/stat.h>
#include <iostream>
#include <map>
#include <string>
#include <vector>

std::vector<std::string> split(const std::string& str,
                               const std::string& pattern) {
  std::vector<std::string> res;
  if (str == "") return res;
  std::string strs = str + pattern;
  size_t pos = strs.find(pattern);
  while (pos != strs.npos) {
    std::string temp = strs.substr(0, pos);
    res.push_back(temp);
    strs = strs.substr(pos + 1, strs.size());
    pos = strs.find(pattern);
  }
  return res;
}

std::map<std::string, uint64_t> dict;
char* dict_filename = "./data/bert/vocab.txt";

int make_dict() {
  std::ifstream dict_file(dict_filename);
  if (!dict_file) {
    std::cout << "read dict failed" << std::endl;
    return -1;
  }
  std::string line;
  uint64_t index = 0;
  while (getline(dict_file, line)) {
    dict[line] = 0;
    index += 1;
  }
  return 0;
}

class BertData {
 public:
  int gen_data(std::string line) {
    std::vector<std::string> data_list;
    data_list = split(line, " ");
    tokenization(data_list);
    return 0;
  }

  int tokenization(std::vector<std::string> data_list) {}

 private:
  std::vector<uint64_t> token_list;
  std::vector<uint64_t> seg_list;
  std::vector<uint64_t> pos_list;
  std::vector<float> input_masks;
};

#endif  // SERVING_DEMO_CLIENT_SRC_DATA_PRE_H_
