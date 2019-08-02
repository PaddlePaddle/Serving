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

#pragma once

#include <string>
#include <vector>

class CROVLBuilderIncremental {
 public:
  static const uint32_t MAX_DATA_LEN = 67108860;
  static const uint32_t MAX_DATA_FILE_LEN = 1073741824;
  static const uint32_t MAX_DATA_DIR_LEN = 512;
  static const uint32_t MAX_FIXDATA_LEN = 504;

  static const uint32_t INDEX_BUFFER = 4194304;
  int _index_type;
  uint32_t _data_file_len;
  uint64_t _data_file;
  uint32_t _data_buf_len;
  uint32_t _index_buf_len;
  uint32_t _index_file_num;
  uint64_t _index_file_len;
  uint64_t _count;
  uint64_t _cur_count;
  bool _fix;
  uint32_t _fix_len;
  uint32_t _rec_size;

  uint64_t* _index;
  char* _data;
  std::vector<uint32_t> _data_file_list;
  std::vector<uint64_t> _index_file_list;

  char _data_dir[MAX_DATA_DIR_LEN + 1];
  char _data_real_dir[MAX_DATA_DIR_LEN + 1];

  char _last_data_dir[MAX_DATA_DIR_LEN + 1];
  char _last_data_tmp_dir[MAX_DATA_DIR_LEN + 1];

  uint64_t _inner_sort_size;
  uint64_t _memory_quota;

  bool flush_data();
  bool FlushIndex();
  bool Clear();

  std::string _mode;
  std::string _dict_name;
  std::string _shard_id;
  std::string _split_id;
  std::string _last_version;
  std::string _cur_version;
  std::string _depend_version;
  std::string _master_address;
  std::string _id;
  std::string _key;
  std::string _extra;

 public:
  CROVLBuilderIncremental();

  ~CROVLBuilderIncremental();

  bool Init(int index_type,
            uint32_t data_file_len,
            const char* mode,
            const char* data_dir,
            const char* data_real_dir,
            const char* dict_name,
            const std::string& shard_id,
            const std::string& split_id,
            const std::string& last_version,
            const std::string& cur_version,
            const std::string& depend_version,
            const std::string& master_address,
            const std::string& id = "",
            const std::string& key = "",
            const std::string& extra = "",
            bool bFix = false,
            uint32_t nFixLen = 8);

  int add(uint64_t nKey, uint32_t nLen, const char* pData);

  int add(uint64_t nKey, uint64_t nValue);

  int add(uint64_t nKey, const char* pData);

  bool done();

  void archive();

  void md5sum();

  bool read_last_meta_from_transfer();

  bool write_cur_meta_to_transfer();

  bool read_last_meta_from_local();

  bool write_cur_meta_to_local();
};
