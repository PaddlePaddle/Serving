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

#include <pthread.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include "core/cube/cube-server/include/cube/cube_bvar.h"
#include "core/cube/cube-server/include/cube/dict.h"
#include "core/cube/cube-server/include/cube/dict_set.h"
#include "core/cube/cube-server/include/cube/framework.h"
#include "core/cube/cube-server/include/cube/recycle.h"
#include <dirent.h>
#include <utility>
#include <sys/stat.h>
using BUTIL_RAPIDJSON_NAMESPACE::Document;
using BUTIL_RAPIDJSON_NAMESPACE::Value;
using BUTIL_RAPIDJSON_NAMESPACE::StringRef;
namespace {
static ::rec::mcube::Framework* g_instance = nullptr;
}

namespace rec {
namespace mcube {

Framework* Framework::instance() {
  if (g_instance == nullptr) {
    g_instance = new Framework();
  }
  Recycle* rec = Recycle::get_instance();
  int ret = rec->init();
  return g_instance;
}

Framework::~Framework() {}

int Framework::init() {
  Recycle* rec = Recycle::get_instance();
  int ret = rec->init();
  if (ret != 0) {
    LOG(ERROR) << "init recycle failed";
    return ret;
  }

  _max_val_size = 1024;

  DIR* dp = nullptr;
  struct dirent* dirp = nullptr;
  struct stat st;

  if (stat("data", &st) < 0 || !S_ISDIR(st.st_mode)) {
    LOG(ERROR) << "the folder \"data\" not exist ";
    return -1;
  }
  dp = opendir("data"); 
  if (dp == nullptr) {
    LOG(ERROR) << "opendir \"data\" fail.";
    return -1;
  }
  LOG(INFO) << "dir data exist, traverse dict folder";
  while ((dirp = readdir(dp)) != nullptr) {
    // filtering by file type.i
    LOG(INFO) << "we are reading dir: " << std::string(dirp->d_name);  
    if (dirp->d_type == DT_REG) {
      LOG(INFO) << " is regular file, continue";
      continue;
    }
    if ((!strncmp(dirp->d_name, ".", 1)) || (!strncmp(dirp->d_name, "..", 2))) {
      continue;
    }

    // open data/dict
    std::string dict_path = "data/" + std::string(dirp->d_name);
    std::string dict_version_path = "data/" + std::string(dirp->d_name) + "/version.txt";
    std::ifstream fs(dict_version_path);
    if (!fs.good()) {
      LOG(WARNING) << "dict \""<< dirp->d_name << "\" not exist, skip.";
      continue;
    }
    LOG(INFO) << "dict \""<< dirp->d_name << "\" exist, read.";
    std::string dict_name(dirp->d_name);  
    _dict_map[dict_name] = new DoubleBufDict();
    DoubleBufDict* dict = _dict_map[dict_name];
    dict->init_dict(dict_name);
    dict->set_dict_path("./data/" + dict_name);
    dict->set_in_mem(true);
    std::vector<Record> records = dict->_version_table->load_records();
    LOG(INFO) << "dict \""<< dict_name << "\" version file read succ.";
    for (const Record& rec: records) {
      std::string content = rec.path + "|" + rec.cmd;
      if (rec.cmd == "base") {
        bg_load_base(dict_name, rec.path);
        bg_switch(dict_name);
        LOG(INFO) << "init, resume " << content;
      } else if(rec.cmd == "patch") {
        bg_load_patch(dict_name, rec.path);
        bg_switch(dict_name);
        LOG(INFO) << "init, resume " << content;
      }
    }  
  }
  return 0;
}

int Framework::destroy() {
  Recycle* recycle = Recycle::get_instance();
  int ret = recycle->destroy();
  if (ret != 0) {
    LOG(WARNING) << "destroy recycle failed";
  }
  return 0;
}

void Framework::init_dict(std::string dict_name, uint32_t dict_split) {
  DoubleBufDict* dict = _dict_map[dict_name];
  dict->init_dict(dict_name);
  dict->set_dict_path("./data/" + dict_name);
}

VirtualDict* Framework::create_dict() {
    return new (std::nothrow) Dict();
}

void Framework::release(VirtualDict* dict) { dict->atom_dec_seek_num(); }

int Framework::status(Document* res, std::string dict_name) {
  res->SetObject();
  Document::AllocatorType& allocator = res->GetAllocator();
  if (_dict_map.find(dict_name) == _dict_map.end()) {
    return 0;
  } 
  Value cur_version;
  Value bg_version;
  Value record;
  //char* cur_ver = const_cast<char*>(get_cur_version(dict_name).c_str());
  //char* bg_ver = const_cast<char*>(get_bg_version(dict_name).c_str());
  //LOG(INFO) << "status: dict name: " << dict_name << ", " << cur_ver << " "<< bg_ver << " " << _status.load();
  cur_version.SetString(StringRef(get_cur_version(dict_name).c_str()), get_cur_version(dict_name).size(), allocator);
  bg_version.SetString(StringRef(get_bg_version(dict_name).c_str())), get_bg_version(dict_name).size(), allocator;
  std::string record_str = _dict_map.at(dict_name)->_version_table->to_string();
  record.SetString(StringRef(record_str.c_str()), record_str.size(), allocator);
  //cur_version.SetString(StringRef(cur_ver));
  //bg_version.SetString(StringRef(bg_ver));
  res->AddMember("cur_version", cur_version, allocator);
  res->AddMember("bg_version", bg_version, allocator);
  res->AddMember("status", _status.load(), allocator);
  res->AddMember("record", record, allocator);
  return 0;
}

int Framework::seek(const DictRequest* req, DictResponse* res) {
  g_request_num << 1;
  // get table name and cur dict
  std::string dict_name = req->dict_name();
  LOG(INFO) << "dict name: " << dict_name; 
  VirtualDict* cur_dict = get_cur_dict(dict_name);
  if (!cur_dict) {
    LOG(WARNING) << "error seek, dict: "<< dict_name << " not exist";
    return -1;
  }
  char* val_buf = new char[_max_val_size];
  g_keys_num << req->keys_size();
  g_total_key_num << req->keys_size();

  std::vector<DictValue*> values(req->keys_size());
  for (int i = 0; i < req->keys_size(); ++i) {
    values[i] = res->add_values();
  }

  for (int i = 0; i < req->keys_size(); ++i) {
    uint64_t val_size = _max_val_size;
    // DictValue* val = res->add_values();
    DictValue* val = values[i];
    if (cur_dict->seek(req->keys(i), val_buf, &val_size)) {
      val->set_status(0);
      val->set_value(val_buf, val_size);
    } else {
      LOG(INFO) << "No value after seek";
      val->set_status(-1);
      val->set_value("");
    }
  }

  if (req->version_required()) {
    res->set_version(cur_dict->version());
  }

  // delete [] keys;
  delete[] val_buf;
  release(cur_dict);
  return 0;
}

int Framework::reload(std::string dict_name, const std::string& v_path) {
  int ret = bg_load_base(dict_name, v_path);
  if (ret != 0) {
    LOG(WARNING) << "background load dict base failed";
  } else {
    LOG(INFO) << "background load dict base succ";
    DoubleBufDict* dict = _dict_map.at(dict_name);
    dict->_version_table->add_record(1, v_path, "base");
  }

  ret = bg_switch(dict_name);
  if (ret != 0) {
    LOG(WARNING) << "switch background dict failed";
  } else {
    LOG(INFO) << "switch background dict succ";
  }

  ret = bg_unload(dict_name);
  if (ret != 0) {
    LOG(WARNING) << "unload background dict failed";
  } else {
    LOG(INFO) << "unload background dict succ";
  }

  return ret;
}

int Framework::patch(std::string dict_name, const std::string& v_path) {
  int ret = bg_load_patch(dict_name, v_path);
  if (ret != 0) {
    LOG(WARNING) << "background load dict patch failed";
  } else {
    LOG(INFO) << "background load dict patch succ";
    DoubleBufDict* dict = _dict_map.at(dict_name);
    dict->_version_table->add_record(1, v_path, "patch");
  }

  ret = bg_switch(dict_name);
  if (ret != 0) {
    LOG(WARNING) << "switch background dict failed";
  } else {
    LOG(INFO) << "switch background dict succ";
  }

  ret = bg_unload(dict_name);
  if (ret != 0) {
    LOG(WARNING) << "unload background dict failed";
  } else {
    LOG(INFO) << "unload background dict succ";
  }

  return ret;
}

int Framework::bg_load_base(std::string dict_name, const std::string& v_path) {
  LOG(INFO) << "dict name: "<< dict_name;
  if (_dict_map.find(dict_name) == _dict_map.end()) {
    LOG(INFO) << "dict name: "<< dict_name << " not exist, create one.";
    _dict_map[dict_name] = new DoubleBufDict();
    _dict_map[dict_name]->init_dict(dict_name);
    _dict_map[dict_name]->set_in_mem(true);
    _dict_map[dict_name]->set_dict_path("./data/" + dict_name);
  }
  int ret = bg_unload(dict_name);
  if (ret != 0) {
    LOG(WARNING) << "unload background dict failed";
  }
  DoubleBufDict* dict = _dict_map.at(dict_name);
  VirtualDict* bg_dict = create_dict();

  if (!bg_dict) {
    LOG(ERROR) << "create Dict failed";
    return -1;
  }

  _status = Status::F_LOADING;
  ret = bg_dict->load(dict->get_dict_path(), dict->get_in_mem(), v_path);
  _status = Status::F_RUNNING;
  if (ret != 0) {
    LOG(WARNING) << "load background dict failed";
    delete bg_dict;
    bg_dict = nullptr;
    return ret;
  } else {
    LOG(INFO) << "load background dict succ";
    set_bg_dict(dict_name, bg_dict);
  }

  return ret;
}

int Framework::bg_load_patch(std::string dict_name, const std::string& v_path) {
  if (_dict_map.find(dict_name) == _dict_map.end()) {
    LOG(WARNING) << "dict name: "<< dict_name << " not exist, create one.";
    _dict_map[dict_name] = new DoubleBufDict();
    _dict_map[dict_name]->init_dict(dict_name);
    _dict_map[dict_name]->set_dict_path("./data/" + dict_name);
  }
  int ret = bg_unload(dict_name);
  if (ret != 0) {
    LOG(WARNING) << "unload background dict failed";
  }
  DoubleBufDict* dict = _dict_map.at(dict_name);
  VirtualDict* bg_dict = create_dict();
  if (!bg_dict) {
    LOG(ERROR) << "create Dict failed";
    return -1;
  }

  _status = Status::F_LOADING;
  if (get_cur_dict(dict_name)) {
    bg_dict->set_base_dict(get_cur_dict(dict_name));
    LOG(INFO) << "set base dict from current dict ";
  }

  ret = bg_dict->load(dict->get_dict_path(), dict->get_in_mem(), v_path);
  _status = Status::F_RUNNING;
  if (ret != 0) {
    LOG(WARNING) << "load background dict failed";
    delete bg_dict;
    bg_dict = nullptr;
    return ret;
  } else {
    LOG(INFO) << "load background dict succ";
    set_bg_dict(dict_name, bg_dict);
  }
  return ret;
}

int Framework::bg_unload(std::string dict_name) {
  LOG(INFO) << "dict_name: " << dict_name << " bg unload.";
  VirtualDict* bg_dict = get_bg_dict(dict_name);
  if (bg_dict) {
    LOG(INFO) << "bg dict exist, recycle.";
    set_bg_dict(dict_name, nullptr);
    Recycle* recycle = Recycle::get_instance();
    recycle->recycle(bg_dict);
  }
  LOG(INFO) << "unload background dict succ";
  return 0;
}

int Framework::bg_switch(std::string dict_name) {
  DoubleBufDict* ddict = _dict_map.at(dict_name);
  ddict->bg_switch();  
  return 0;
}

int Framework::enable(std::string dict_name, const std::string& version) {
  int ret = 0;
  LOG(INFO) << "dict name: " << dict_name << " , version: " << version;
  if (version != "" && version == get_cur_version(dict_name)) {
    LOG(INFO) << "cur version: " << get_cur_version(dict_name);
    ret = 0;
  } else if (version == get_bg_version(dict_name)) {
    LOG(INFO) << "cur version: " << get_bg_version(dict_name);
    ret = bg_switch(dict_name);
  } else {
    LOG(WARNING) << "bg dict version not matched";
    ret = -1;
  }
  return ret;
}

}  // namespace mcube
}  // namespace rec
