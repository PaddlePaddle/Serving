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

#include <stdio.h>
#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <functional>

#include "core/cube/cube-server/include/cube/cube_bvar.h"
#include "core/cube/cube-server/include/cube/dict.h"
#include "core/cube/cube-server/include/cube/error.h"
#include "core/cube/cube-server/include/cube/util.h"

namespace rec {
namespace mcube {

static void munmap_deleter(void* data, uint32_t size) {
  if (data != MAP_FAILED) {
    munmap(data, size);
  }
}

Dict::Dict() : _seek_num(0), _base_dict(NULL) {}

Dict::~Dict() {}

void Dict::atom_dec_seek_num() { --_seek_num; }

void Dict::atom_inc_seek_num() { ++_seek_num; }

uint32_t Dict::atom_seek_num() { return _seek_num; }

int Dict::load(const std::string& dict_path,
               bool in_mem,
               const std::string& v_path) {
  TIME_FLAG(load_start);
  int ret = load_index(dict_path, v_path);
  if (ret != E_OK) {
    LOG(WARNING) << "load index failed";
    return ret;
  }
  LOG(INFO) << "load index in mem mode: " << in_mem ;
  if (in_mem) {
    ret = load_data(dict_path, v_path);
    if (ret != E_OK) {
      LOG(ERROR) << "load data failed";
      return ret;
    }
  } else {
    ret = load_data_mmap(dict_path, v_path);
    if (ret != E_OK) {
      LOG(ERROR) << "load data failed";
      return ret;
    }
  }
  set_version(v_path);
  TIME_FLAG(load_end);
  g_data_load_time << time_diff(load_start, load_end);
  return E_OK;
}

int Dict::load_index(const std::string& dict_path, const std::string& v_path) {
  std::string index_n_path(dict_path);
  index_n_path.append(v_path);
  index_n_path.append("/index.n");
  
  uint32_t cur_block_id = 0;
  if (_base_dict) cur_block_id = _base_dict->_block_set.size(); 
  LOG(INFO) << "index file path: " << index_n_path;
  //ERR HERE
  std::unique_ptr<FILE, decltype(&fclose)> pf(fopen(index_n_path.c_str(), "rb"),
                                              &fclose);
  if (pf.get() == NULL) {
    LOG(WARNING) << "open index: " << index_n_path << " failed";
    return E_DATA_ERROR;
  }

  int type = 0;
  if (fread(reinterpret_cast<void*>(&type), sizeof(int), 1, pf.get()) != 1) {
    LOG(ERROR) << "index syntax error";
    return E_DATA_ERROR;
  }

  uint32_t count = 0;
  if (fread(reinterpret_cast<void*>(&count), sizeof(uint32_t), 1, pf.get()) !=
      1) {
    LOG(ERROR) << "index syntax error";
    return E_DATA_ERROR;
  }

  uint32_t file_num = 0;
  if (fread(
          reinterpret_cast<void*>(&file_num), sizeof(uint32_t), 1, pf.get()) !=
      1) {
    LOG(ERROR) << "index syntax error";
    return E_DATA_ERROR;
  }
  LOG(INFO) << "index type:" << type << ", count:" << count
            << ", file_num:" << file_num;

  // read file_lens begin
  uint32_t file_cnt = file_num;
  uint64_t len[1024];
  std::vector<uint64_t> file_lens;
  file_lens.reserve(file_num);
  while (file_cnt > 0) {
    uint32_t to_read = file_cnt > 1024 ? 1024 : file_cnt;
    file_cnt -= to_read;
    if (fread(reinterpret_cast<void*>(&len),
              sizeof(uint64_t),
              to_read,
              pf.get()) != to_read) {
      return E_DATA_ERROR;
    }
    for (uint32_t i = 0; i < to_read; ++i) {
      file_lens.push_back(len[i]);
    }
  }

  if (file_lens.size() != file_num) {
    LOG(ERROR) << "file_num[" << file_num << "] != file_lens size["
               << file_lens.size() << "], shouldn't happen";
    return E_DATA_ERROR;
  }

  // try patch mode
  size_t file_idx = 0;
  if (_base_dict) {
    if (_base_dict->_slim_table.check_file_len(file_lens, &file_idx)) {
      LOG(INFO) << "index check file len ok in patch mode, set file_idx to "
                << file_idx;

      if (_slim_table.copy_data_from(_base_dict->_slim_table) != 0) {
        LOG(ERROR) << "copy data from old index failed in patch mode";
        return E_DATA_ERROR;
      }
    } else {
      if (_slim_table.copy_data_from(_base_dict->_slim_table) != 0) {
        LOG(ERROR) << "copy data from old index failed in patch mode";
        return E_DATA_ERROR;
      }
      file_idx = 0;
      LOG(INFO)
          << "index check fail, direct copy";
    }
  }
  LOG(INFO) << "resize slim table, new count: " << count/2;
  _slim_table.resize(count / 2);

  char file[1024];
  struct stat fstat;
  for (; file_idx < file_num; ++file_idx) {
    snprintf(file,
             sizeof(file),
             "%s%s/index.%lu",
             dict_path.c_str(),
             v_path.c_str(),
             file_idx);
    LOG(INFO) << "load file str: " << file;
    if (stat(file, &fstat) < 0) {
      if (errno == ENOENT) {
        LOG(WARNING) << "index." << file_idx << " not exist";
        _slim_table.add_file_len(0);
        continue;
      }
      return E_DATA_ERROR;
    }
    if ((uint64_t)fstat.st_size != file_lens[file_idx]) {
      LOG(ERROR) << "load_index failed, expect index file[" << file_idx
                 << "] size is " << file_lens[file_idx] << ", actual size is "
                 << (uint64_t)fstat.st_size;
      return E_DATA_ERROR;
    }
    LOG(INFO) << "loading from index." << file_idx << " . table size: " << _slim_table.size();
    if (!_slim_table.load(file, cur_block_id)) {
      return E_DATA_ERROR;
    }

    _slim_table.add_file_len(file_lens[file_idx]);
  }

  return E_OK;
}

int Dict::load_data(const std::string& dict_path, const std::string& v_path) {
  std::vector<uint32_t> block_size;
  uint64_t total_data_size = 0;
  if (_base_dict) {
    _block_set = _base_dict->_block_set;
    LOG(INFO)<< "load data base dict block set size: " << _block_set[0].size;
    for (size_t i = 0; i < _block_set.size(); ++i) {
      block_size.push_back(_block_set[i].size); 
      total_data_size += _block_set[i].size;     
    }
  }

  std::string data_n_path(dict_path);
  data_n_path.append(v_path);
  data_n_path.append("/data.n");
  FILE* pf = fopen(data_n_path.c_str(), "rb");
  if (pf == NULL) {
    LOG(ERROR) << "open data [" << data_n_path << "] failed";
    return E_DATA_ERROR;
  }
  uint32_t count = 0;
  if (fread(reinterpret_cast<void*>(&count), sizeof(uint32_t), 1, pf) != 1) {
    LOG(ERROR) << "data syntax error";
    fclose(pf);
    return E_DATA_ERROR;
  }

  for (uint32_t i = 0; i < count; ++i) {
    uint32_t size = 0;
    if (fread(reinterpret_cast<void*>(&size), sizeof(uint32_t), 1, pf) != 1) {
      LOG(ERROR) << "data syntax error";
      fclose(pf);
      return E_DATA_ERROR;
    }
    block_size.push_back(size);
    LOG(INFO) << "new block size: " << size;
    total_data_size += size;
  }
  g_data_size << (total_data_size / 1024 / 1024);
  fclose(pf);
  pf = NULL;

  uint32_t old_size = _block_set.size();
  LOG(INFO) << "load data old size: " << old_size;
  for (size_t i = 0; i < old_size; ++i) {
    if (_block_set[i].size != block_size[i]) {
      old_size = 0;
      break;
    }
  }
  LOG(INFO) << "load data block set count: " << count << " , old size: " << old_size;
  _block_set.resize(count + old_size);
  for (size_t i = old_size; i < _block_set.size(); ++i) {
    char data_path[1024];
    LOG(INFO) << "load from data." << i;
    //snprintf(
      //  data_path, 1024, "%s%s/data.%lu", dict_path.c_str(), v_path.c_str(), i);
    snprintf(data_path, 1024, "%s%s/data.%lu", dict_path.c_str(), v_path.c_str(), i - old_size);
    FILE* data_file = fopen(data_path, "rb");
    if (data_file == NULL) {
      LOG(WARNING) << "open data file [" << data_path << " ]failed";
      _block_set[i].s_data.reset();
      _block_set[i].size = 0;
      continue;
    }
    _block_set[i].s_data.reset(reinterpret_cast<char*>(malloc(block_size[i] * sizeof(char))));
    if (_block_set[i].s_data.get() == NULL) {
      LOG(ERROR) << "malloc data failed";
      fclose(data_file);
      return E_OOM;
    }
    _block_set[i].size = block_size[i];
    if (fread(reinterpret_cast<void*>(_block_set[i].s_data.get()),
              sizeof(char),
              _block_set[i].size,
              data_file) != _block_set[i].size) {
      LOG(ERROR) << "read data failed";
      fclose(data_file);
      return E_DATA_ERROR;
    }
    LOG(INFO) << "load new data to BlockSet succ";
    for (size_t ii = 0; ii < 20; ++ii) {
       LOG(INFO) << "data ptr: " << (int)(_block_set[i].s_data.get()[ii]);
    }
    fclose(data_file);
  }

  return E_OK;
}

int Dict::load_data_mmap(const std::string& dict_path,
                         const std::string& v_path) {
  std::string data_n_path(dict_path);
  data_n_path.append(v_path);
  data_n_path.append("/data.n");
  FILE* pf = fopen(data_n_path.c_str(), "rb");
  if (pf == NULL) {
    LOG(ERROR) << "open data [" << data_n_path << "] failed";
    return E_DATA_ERROR;
  }
  uint32_t count = 0;
  if (fread(reinterpret_cast<void*>(&count), sizeof(uint32_t), 1, pf) != 1) {
    LOG(ERROR) << "data syntax error";
    fclose(pf);
    return E_DATA_ERROR;
  }

  std::vector<uint32_t> block_size;
  uint64_t total_data_size = 0;
  for (uint32_t i = 0; i < count; ++i) {
    uint32_t size = 0;
    if (fread(reinterpret_cast<void*>(&size), sizeof(uint32_t), 1, pf) != 1) {
      LOG(ERROR) << "data syntax error";
      fclose(pf);
      return E_DATA_ERROR;
    }
    block_size.push_back(size);
    total_data_size += size;
  }
  g_data_size << (total_data_size / 1024 / 1024);
  fclose(pf);
  pf = NULL;

  uint32_t old_size = _block_set.size();
  _block_set.resize(count);
  for (size_t i = old_size; i < _block_set.size(); ++i) {
    char data_path[1024];
    LOG(INFO) << "load from data." << i;
    snprintf(
        data_path, 1024, "%s%s/data.%lu", dict_path.c_str(), v_path.c_str(), i);

    int data_fd = open(data_path,
                       O_RDONLY | O_NONBLOCK,
                       S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (data_fd < 0) {
      LOG(WARNING) << "open data file [" << data_path << "] failed";
      _block_set[i].s_data.reset();
      _block_set[i].size = 0;
      continue;
    }

    _block_set[i].s_data.reset(
        reinterpret_cast<char*>(
            mmap(NULL, block_size[i], PROT_READ, MAP_SHARED, data_fd, 0)),
        std::bind(munmap_deleter, std::placeholders::_1, block_size[i]));

    if (_block_set[i].s_data.get() == MAP_FAILED) {
      LOG(WARNING) << "map data file [" << data_path << "] failed";
      _block_set[i].s_data.reset();
      _block_set[i].size = 0;
      continue;
    }
    _block_set[i].size = block_size[i];
    _block_set[i].fd = data_fd;
  }

  return E_OK;
}

int Dict::destroy() {
  for (size_t i = 0; i < _block_set.size(); ++i) {
    if (_block_set[i].fd > 0) {
      close(_block_set[i].fd);
      _block_set[i].fd = -1;
    }
    _block_set[i].size = 0;
  }
  return E_OK;
}

void Dict::set_version(const std::string& v_path) {
  _rw_lock.w_lock();
  _version = (v_path == "") ? "" : v_path.substr(1);
  _rw_lock.unlock();
}

const std::string& Dict::version() { return _version; }

std::string Dict::guard_version() {
  _rw_lock.r_lock();
  std::string version = _version;
  _rw_lock.unlock();
  return version;
}

bool Dict::seek(uint64_t key, char* buff, uint64_t* buff_size) {
  slim_hash_map<uint64_t, uint64_t>::iterator it = _slim_table.find(key);
  if (it.get_node() == NULL) {
    *(reinterpret_cast<uint32_t*>(buff)) = 0;
    *buff_size = sizeof(uint32_t);
    g_unfound_key_num << 1;
    return false;
  }
  if (it == _slim_table.end()) {
    *(reinterpret_cast<uint32_t*>(buff)) = 0;
    *buff_size = sizeof(uint32_t);
    return false;
  }

  uint64_t flag = it->second;
  uint32_t id = (uint32_t)(flag >> 32);
  uint64_t addr = (uint32_t)(flag);
  LOG(INFO) << "search key: " << id << " , addr: " << addr;
  if (_block_set.size() > id) {
    uint32_t block_size = _block_set[id].size;
    char* block_data = NULL;
    block_data = _block_set[id].s_data.get();
    if (block_data && addr + sizeof(uint32_t) <= block_size) {
      uint32_t len = *(reinterpret_cast<uint32_t*>(block_data + addr));
      if (addr + len <= block_size && len >= sizeof(uint32_t)) {
        uint64_t default_buffer_size = *buff_size;

        *buff_size = len - sizeof(uint32_t);
        if (*buff_size > default_buffer_size) {
          g_long_value_num << 1;
          LOG(ERROR) << "value len is " << *buff_size
                     << ", larger than default_buffer_size "
                     << default_buffer_size;
          return false;
        }
        LOG(INFO) << "seek key: " << key << " , addr: " << addr;
        memcpy(buff,
               (block_data + addr + sizeof(uint32_t)),
               len - sizeof(uint32_t));
        return true;
      } else {
        *(reinterpret_cast<uint32_t*>(buff)) = 0;
        *buff_size = sizeof(uint32_t);
        return false;
      }
    } else {
      *(reinterpret_cast<uint32_t*>(buff)) = 0;
      *buff_size = sizeof(uint32_t);
      return false;
    }
  } else {
    *(reinterpret_cast<uint32_t*>(buff)) = 0;
    *buff_size = sizeof(uint32_t);
    return false;
  }

  return false;
}

}  // namespace mcube
}  // namespace rec
