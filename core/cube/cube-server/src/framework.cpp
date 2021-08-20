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

int Framework::init(std::string dict_name, uint32_t dict_split, bool in_mem) {
  Recycle* rec = Recycle::get_instance();
  int ret = rec->init();
  if (ret != 0) {
    LOG(ERROR) << "init recycle failed";
    return ret;
  }
  LOG(INFO) << "will not init here.";
  /*
  _dict[0] = new (std::nothrow) Dict();
  _dict[1] = nullptr;
  */
  //init_dict(dict_split);
  //VirtualDict* cur_dict = _dict[_dict_idx];
  _dict_map[dict_name] = new DoubleBufDict();
  DoubleBufDict* dict = _dict_map[dict_name];
  dict->set_dict_path("./data");
  dict->set_in_mem(in_mem);

  _max_val_size = 1024;
  
  //_in_mem = in_mem;

  //std::string version_file = _dict_path + "/VERSION";
  //std::string version_path = "";
  //std::ifstream input(version_file.c_str());
  //if (!std::getline(input, version_path)) {
  //  version_path = "";
  //} else {
  //  version_path = "/" + version_path;
 // }
  //input.close();

  //LOG(INFO) << "load dict from" << _dict_path << version_path;
  //if (_dict_split > 1) {
  //  _dict_set_path.clear();
  //  _dict_set_path.resize(_dict_split);
  //  std::stringstream dict_set_path_buf;
  //  for (size_t i = 0; i < _dict_split; ++i) {
  //    dict_set_path_buf.str(std::string());
  //    dict_set_path_buf.clear();
  //    dict_set_path_buf << _dict_path << "/" << i;
  //    _dict_set_path[i] = dict_set_path_buf.str();
  //  }
  //  ret = cur_dict->load(_dict_set_path, _in_mem, version_path);
  //} else {
  //  ret = cur_dict->load(_dict_path, _in_mem, version_path);
  //}

  //if (ret != 0) {
  //  LOG(WARNING) << "init: load dict data failed err=" << ret
  //               << ". starting service with empty data.";
  //} else {
  // LOG(INFO) << "load dict from " << _dict_path << version_path << " done";
  //}

  //_status = Status::F_RUNNING;

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
  dict->init_dict();
  dict->set_dict_path("./data");
}

VirtualDict* Framework::create_dict() {
    return new (std::nothrow) Dict();
}

void Framework::release(VirtualDict* dict) { dict->atom_dec_seek_num(); }

int Framework::status(Document* res) {
  res->SetObject();
  Document::AllocatorType& allocator = res->GetAllocator();
  for (auto it = _dict_map.begin(); it != _dict_map.end(); ++it) {
    Value dict_value;
    Value cur_version;
    Value bg_version;
    cur_version.SetString(StringRef(get_cur_version(it->first).c_str()));
    bg_version.SetString(StringRef((get_bg_version(it->first).c_str())));
    dict_value.AddMember("cur_version", cur_version, allocator);
    dict_value.AddMember("bg_version", bg_version, allocator);
    dict_value.AddMember("status", _status.load(), allocator);
    res->AddMember(StringRef(it->first.c_str()), dict_value, allocator);
  } 
  return 0;
}

int Framework::seek(const DictRequest* req, DictResponse* res) {
  g_request_num << 1;
  // get table name and cur dict
  std::string dict_name = req->dict_name();
  std::cout << "dict name: " << dict_name << std::endl; 
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
    _dict_map[dict_name]->init_dict();
    _dict_map[dict_name]->set_dict_path("./data");
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
    _dict_map[dict_name]->init_dict();
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
  if (version != "" && version == get_cur_version(dict_name)) {
    ret = 0;
  } else if (version == get_bg_version(dict_name)) {
    ret = bg_switch(dict_name);
  } else {
    LOG(WARNING) << "bg dict version not matched";
    ret = -1;
  }
  return ret;
}

}  // namespace mcube
}  // namespace rec
