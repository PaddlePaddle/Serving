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

#include "core/cube/cube-builder/include/cube-builder/crovl_builder_increment.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include "boost/lexical_cast.hpp"
#include "core/cube/cube-builder/include/cube-builder/curl_simple.h"
#include "core/cube/cube-builder/include/cube-builder/define.h"
#include "core/cube/cube-builder/include/cube-builder/util.h"
#include "json/json.h"

static const uint64_t MB = 1048576;
// CSpinLock g_sLock;

CROVLBuilderIncremental::CROVLBuilderIncremental() {
  _index = NULL;
  _data = NULL;
  // _index_file = NULL;
  _inner_sort_size = 32 * MB;
  _memory_quota = 4 * 1024 * MB;
}

bool CROVLBuilderIncremental::Clear() {
  if (_index != NULL) {
    delete[] _index;
    _index = NULL;
  }

  if (_data != NULL) {
    delete[] _data;
    _data = NULL;
  }

  _data_file_list.clear();

  return true;
}

CROVLBuilderIncremental::~CROVLBuilderIncremental() { Clear(); }

bool CROVLBuilderIncremental::Init(int index_type,
                                   uint32_t data_file_len,
                                   const char *mode,
                                   const char *data_dir,
                                   const char *data_real_dir,
                                   const char *dict_name,
                                   const std::string &shard_id,
                                   const std::string &split_id,
                                   const std::string &last_version,
                                   const std::string &cur_version,
                                   const std::string &depend_version,
                                   const std::string &master_address,
                                   const std::string &id,
                                   const std::string &key,
                                   const std::string &extra,
                                   bool bFix,
                                   uint32_t nFixLen) {
  if (index_type != IT_HASH) {
    LOG(ERROR) << "Error: incremental_build not support IT_SORT";
    return false;
  }

  if (index_type != IT_HASH && index_type != IT_SORT) {
    LOG(ERROR) << "Error: invalid index type";
    return false;
  }

  if (index_type == IT_HASH && bFix) {
    LOG(ERROR) << "Error: can not set fix when index type is IT_HASH";
    return false;
  }

  if (nFixLen > MAX_FIXDATA_LEN) {
    LOG(ERROR) << "Error: fix length too large";
    return false;
  }

  if (data_file_len < MAX_DATA_LEN + 4 || data_file_len > MAX_DATA_FILE_LEN) {
    LOG(ERROR) << "Error: invalid data file length" << data_file_len;
    return false;
  }

  if (strlen(data_dir) > MAX_DATA_DIR_LEN) {
    LOG(ERROR) << "Error: data directory too long :" << data_dir;
    return false;
  }

  Clear();

  strncpy(_data_dir, data_dir, sizeof(_data_dir));
  strncpy(_data_real_dir, data_real_dir, sizeof(_data_real_dir));

  _index_type = index_type;
  _data_file_len = data_file_len;
  _data_file = 0;
  _data_buf_len = 0;
  _index_buf_len = 0;
  _index_file_num = 0;  // incremental_build for index
  _count = 0;
  _cur_count = 0;
  _fix = bFix;
  _fix_len = nFixLen;
  _index_file_len = 0;

  _dict_name = dict_name;
  _shard_id = shard_id;
  _split_id = split_id;
  _last_version = last_version;
  _cur_version = cur_version;
  _depend_version = depend_version;
  _master_address = master_address;
  _id = id;
  _key = key;
  _extra = extra;

  if (_fix) {
    _rec_size = nFixLen + sizeof(uint64_t);
  } else {
    _rec_size = 2 * sizeof(uint64_t);
  }

  _index = new uint64_t[INDEX_BUFFER];

  if (!_fix) {
    _data = new char[_data_file_len];
  } else if (_fix_len != 8) {
    _data = new char[_rec_size * INDEX_BUFFER];
  }

  if (!checkDirectory(_data_dir)) {
    LOG(ERROR) << "create _data_dir path failed: " << _data_dir;
    return false;
  }

  // read old info from data.n and index.n
  _mode = mode;
  LOG(INFO) << "mode: " << mode;

  if (strcmp(mode, "base") != 0) {
    if (_master_address == "") {
      if (!read_last_meta_from_local()) {
        LOG(ERROR) << "read last meta from db error! ";
        return false;
      }
    } else {
      //if (!read_last_meta_from_transfer()) {
      //  LOG(ERROR) << "read last meta from transfer error! ";
      //  return false;
      //}
      if (_master_address == "") {
        if (!read_last_meta_from_local()) {
          LOG(ERROR) << "read last meta from db error! ";
          return false;
        }
      }
    }
  }
  LOG(INFO) << "incremental_build  data file:" << _data_file
            << ", count:" << _count << ", index file:" << _index_file_num;
  return true;
}

bool CROVLBuilderIncremental::flush_data() {
  LOG(INFO) << "flush data" << _data_file;
  if (_data_buf_len == 0) {
    LOG(ERROR) << "_data_buf_len == 0";
    return true;
  }

  char file[MAX_DATA_DIR_LEN * 2];
  snprintf(file, sizeof(file), "%s/data.%lu", _data_dir, _data_file);
  FILE *fp;
  if ((fp = fopen(file, "ab")) == NULL) {
    LOG(ERROR) << "open file failed! " << file;
    return false;
  }

  if (fwrite(_data, 1, _data_buf_len, fp) != _data_buf_len) {
    fclose(fp);
    LOG(ERROR) << "write file:" << file << " error!";
    return false;
  }
  fclose(fp);

  _data_file_list.push_back(_data_buf_len);
  _data_buf_len = 0;
  ++_data_file;
  return true;
}

bool CROVLBuilderIncremental::FlushIndex() {
  LOG(INFO) << "flush index:" << _index_file_num;

  if (_index_buf_len == 0) {
    LOG(ERROR) << "_index_buf_len == 0";
    return true;
  }

  char file[MAX_DATA_DIR_LEN * 2];
  snprintf(file, sizeof(file), "%s/index.%d", _data_dir, _index_file_num);
  FILE *fp;
  if ((fp = fopen(file, "ab")) == NULL) {
    LOG(ERROR) << "open file failed! " << file;
    return false;
  }

  if (_fix && _fix_len != 8) {
    if (fwrite(_data, _rec_size, _index_buf_len, fp) != _index_buf_len) {
      LOG(ERROR) << "_index_buf_len error:" << _index_buf_len;
      return false;
    }
    _index_buf_len = 0;
    return true;
  }

  if (fwrite(_index, sizeof(uint64_t), _index_buf_len, fp) != _index_buf_len) {
    LOG(ERROR) << "write file:" << file << " error!";
    return false;
  }
  fclose(fp);

  _index_file_len += _index_buf_len * sizeof(uint64_t);
  _index_buf_len = 0;
  return true;
}

int CROVLBuilderIncremental::add(uint64_t nKey,
                                 uint32_t nLen,
                                 const char *pData) {
  uint64_t naddr;
  uint32_t nTotalLen;

  if (nLen > MAX_DATA_LEN || _fix) {
    return 0;
  }

  nTotalLen = nLen + 4;

  do {
    if (_data_buf_len + nTotalLen > _data_file_len) {
      if (!flush_data()) {
        break;
      }
    }

    *reinterpret_cast<uint32_t *>(_data + _data_buf_len) = nTotalLen;
    memcpy(_data + _data_buf_len + 4, pData, nLen);
    naddr = (_data_file << 32) + _data_buf_len;
    _data_buf_len += nTotalLen;

    if (_index_buf_len + 2 > INDEX_BUFFER) {
      if (!FlushIndex()) {
        break;
      }
    }

    _index[_index_buf_len] = nKey;
    _index[_index_buf_len + 1] = naddr;
    _index_buf_len += 2;
    ++_count;
    ++_cur_count;
    return 1;
  } while (false);

  return -1;
}

int CROVLBuilderIncremental::add(uint64_t nKey, uint64_t nValue) {
  if (!_fix || _fix_len != 8) {
    return 0;
  }

  if (_index_buf_len + 2 > INDEX_BUFFER) {
    if (!FlushIndex()) {
      return -1;
    }
  }

  _index[_index_buf_len] = nKey;
  _index[_index_buf_len + 1] = nValue;
  _index_buf_len += 2;
  ++_count;
  ++_cur_count;
  return 1;
}

int CROVLBuilderIncremental::add(uint64_t nKey, const char *pData) {
  if (!_fix) {
    return 0;
  }

  if (_fix && _fix_len == 8) {
    return add(nKey, *reinterpret_cast<uint64_t *>(const_cast<char *>(pData)));
  }

  if (_index_buf_len + 1 > INDEX_BUFFER) {
    if (!FlushIndex()) {
      return -1;
    }
  }

  *reinterpret_cast<uint64_t *>(_data + _index_buf_len * _rec_size) = nKey;
  memcpy(
      _data + (_index_buf_len * _rec_size + sizeof(uint64_t)), pData, _fix_len);
  _index_buf_len++;
  ++_count;
  ++_cur_count;
  return 1;
}

bool CROVLBuilderIncremental::done() {
  LOG(INFO) << "done";
  if (!_fix) {
    if (!flush_data()) {
      return false;
    }
  }

  if (!FlushIndex()) {
    return false;
  }

  if (!_fix) {
    if (_data_file == 0) {
      return false;
    }
  }

  char buffer[64];
  char file[MAX_DATA_DIR_LEN * 2];

  // write data.n
  snprintf(file, sizeof(file), "%s/data.n", _data_dir);
  FILE *fp;
  if ((fp = fopen(file, "wb")) == NULL) {
    LOG(ERROR) << "open file failed! " << file;
    return false;
  }

  *reinterpret_cast<uint32_t *>(buffer) = _data_file;
  if (fwrite(buffer, sizeof(uint32_t), 1, fp) != 1) {
    fclose(fp);
    return false;
  }

  for (uint32_t i = 0; i < _data_file_list.size(); ++i) {
    *reinterpret_cast<uint32_t *>(buffer) = _data_file_list[i];

    if (fwrite(buffer, sizeof(uint32_t), 1, fp) != 1) {
      fclose(fp);
      return false;
    }
  }
  fclose(fp);

  // write index.n
  if (_index_type == IT_HASH) {
    if (_count > (uint64_t)((uint32_t)-1)) {
      return false;
    }

    snprintf(file, sizeof(file), "%s/index.n", _data_dir);

    if ((fp = fopen(file, "wb")) == nullptr) {
      LOG(ERROR) << "open file failed! " << file;
      return false;
    }

    *reinterpret_cast<uint32_t *>(buffer) = IT_HASH;
    *reinterpret_cast<uint32_t *>(buffer + sizeof(uint32_t)) = (uint32_t)_count;

    if (fwrite(buffer, sizeof(uint32_t), 2, fp) != 2) {
      fclose(fp);
      return false;
    }

    // fix empty data bug
    // if the version no data, filter this version, index_file_num no add
    // only can patch is null, not base
    if (_cur_count > 0) {
      *reinterpret_cast<uint32_t *>(buffer) = ++_index_file_num;
    } else {
      *reinterpret_cast<uint32_t *>(buffer) = _index_file_num;
    }

    if (fwrite(buffer, sizeof(uint32_t), 1, fp) != 1) {
      fclose(fp);
      return false;
    }

    if (_cur_count > 0) {
      _index_file_list.push_back(_index_file_len);
    }
    for (uint32_t i = 0; i < _index_file_list.size(); ++i) {
      *reinterpret_cast<uint64_t *>(buffer) = _index_file_list[i];

      if (fwrite(buffer, sizeof(uint64_t), 1, fp) != 1) {
        fclose(fp);
        return false;
      }
    }
    fclose(fp);

    if (_master_address == "") {
      if (!write_cur_meta_to_local()) {
        LOG(ERROR) << "write cur meta to local error";
        return false;
      }
    } else {
      if (!write_cur_meta_to_transfer()) {
        LOG(ERROR) << "write cur meta to db error master addr:"
                   << _master_address.c_str();
        return false;
      }
    }

  } else {
    snprintf(file, sizeof(file), "%s/index.d", _data_dir);
    LOG(ERROR) << "HASH is error!";
  }

  if (!Clear()) {
    LOG(ERROR) << "clear error!";
    return false;
  }

  return true;
}

void CROVLBuilderIncremental::archive() { CmdTarfiles(_data_dir); }

void CROVLBuilderIncremental::md5sum() { CmdMd5sum(_data_dir); }

bool CROVLBuilderIncremental::read_last_meta_from_transfer() {
  std::string url = "http://" + _master_address + "/dict/meta_info?name=" +
                    _dict_name + "&shard=" + _shard_id + "&split=" + _split_id +
                    "&version=" + _last_version + "&depend=" + _depend_version;

  LOG(INFO) << "name:" << _dict_name.c_str() << " shard:" << _shard_id.c_str()
            << " split:" << _split_id.c_str()
            << " last version:" << _last_version.c_str()
            << " depend version:" << _depend_version.c_str();

  CurlSimple cs;
  std::string result = cs.curl_get(url.c_str());

  if (result == "") {
    LOG(ERROR) << "curl get error!";
    return false;
  }

  LOG(INFO) << "curl result:" << result.c_str();
  Json::Reader reader(Json::Features::strictMode());
  Json::Value val;

  if (!reader.parse(result, val)) {
    LOG(ERROR) << "parse result json error!";
    return false;
  }

  if (!val.isObject()) {
    LOG(ERROR) << "no valild json error!";
    return false;
  }

  if (val["success"].isNull() || !val["success"].isString()) {
    LOG(ERROR) << "parse field success error!";
    return false;
  }

  std::string success = val["success"].asString();

  if (success != "0") {
    LOG(ERROR) << "parse field success error!" << success.c_str();
    return false;
  }

  if (val["data"].isNull() || !val["data"].isObject()) {
    LOG(ERROR) << "parse field data error!";
    return false;
  }

  Json::Value data = val["data"];

  if (data["meta"].isNull() || !data["meta"].isString()) {
    LOG(ERROR) << "parse field meta error!";
    return false;
  }

  Json::Reader meta_reader(Json::Features::strictMode());
  Json::Value meta;

  if (!meta_reader.parse(data["meta"].asString(), meta)) {
    LOG(ERROR) << "parse meta json error!";
    return false;
  }

  if (!meta.isObject()) {
    LOG(ERROR) << "parse meta json error!";
    return false;
  }

  if (meta["data_len_list"].isNull() || !meta["data_len_list"].isArray()) {
    LOG(ERROR) << "parse data_len_list json error!";
    return false;
  }

  Json::Value data_len_list = meta["data_len_list"];
  _data_file = data_len_list.size();

  if (_data_file == 0) {
    LOG(ERROR) << "data len list size is 0!";
    return false;
  }

  for (int i = 0; i < static_cast<int>(_data_file); ++i) {
    LOG(INFO) << "data_len:" << data_len_list[i].asString().c_str();

    try {
      _data_file_list.push_back(
          boost::lexical_cast<uint32_t>(data_len_list[i].asString()));
    } catch (boost::bad_lexical_cast &e) {
      LOG(ERROR) << "bad lexical cast:" << e.what();
      return false;
    }
  }

  if (meta["index_len_list"].isNull() || !meta["index_len_list"].isArray()) {
    LOG(ERROR) << "parse index_len_list json error!";
    return false;
  }

  Json::Value index_len_list = meta["index_len_list"];
  _index_file_num = index_len_list.size();

  if (_index_file_num == 0) {
    LOG(ERROR) << "index len list size is 0!";
    return false;
  }

  for (uint32_t i = 0; i < _index_file_num; ++i) {
    LOG(INFO) << "index_len:" << index_len_list[i].asString().c_str();

    try {
      _index_file_list.push_back(
          boost::lexical_cast<uint64_t>(index_len_list[i].asString()));
    } catch (boost::bad_lexical_cast &e) {
      LOG(ERROR) << "bad lexical cast:" << e.what();
      return false;
    }
  }

  if (meta["index_total_count"].isNull() ||
      !meta["index_total_count"].isString()) {
    LOG(ERROR) << "parse index_total_count json error!";
    return false;
  }

  LOG(INFO) << "index_total_count:"
            << meta["index_total_count"].asString().c_str();
  try {
    _count =
        boost::lexical_cast<uint64_t>(meta["index_total_count"].asString());
  } catch (boost::bad_lexical_cast &e) {
    LOG(ERROR) << "bad lexical cast:" << e.what();
    return false;
  }

  return true;
}

bool CROVLBuilderIncremental::write_cur_meta_to_transfer() {
  CurlSimple cs;
  std::string url = "http://" + _master_address + "/dict/meta_info/register";

  std::map<std::string, std::string> args;
  args["name"] = _dict_name;
  args["shard"] = _shard_id;
  args["split"] = _split_id;
  args["version"] = _cur_version;
  args["depend"] = _depend_version;

  LOG(INFO) << "name:" << _dict_name.c_str() << " shard:" << _shard_id.c_str()
            << " split:" << _split_id.c_str()
            << " cur version:" << _cur_version.c_str()
            << " depend version:" << _depend_version.c_str();

  Json::Value root;

  try {
    root["index_total_count"] = boost::lexical_cast<std::string>(_count);
  } catch (boost::bad_lexical_cast &e) {
    LOG(ERROR) << "bad lexical cast:" << e.what();
    return false;
  }

  Json::Value data;

  for (size_t i = 0; i < _data_file_list.size(); ++i) {
    try {
      data.append(boost::lexical_cast<std::string>(_data_file_list[i]));
    } catch (boost::bad_lexical_cast &e) {
      LOG(ERROR) << "bad lexical cast:" << e.what();
      return false;
    }
  }

  root["data_len_list"] = data;

  Json::Value index;

  for (size_t i = 0; i < _index_file_list.size(); ++i) {
    try {
      index.append(boost::lexical_cast<std::string>(_index_file_list[i]));
    } catch (boost::bad_lexical_cast &e) {
      LOG(ERROR) << "bad lexical cast:" << e.what();
      return false;
    }
  }

  root["index_len_list"] = index;

  Json::FastWriter writer;
  // Json::StreamWriterBuilder writer;
  std::string meta = writer.write(root);

  // std::string meta = Json::writeString(writer, root);
  if (meta[meta.size() - 1] == '\n') {
    meta.erase(meta.size() - 1, 1);
  }

  LOG(INFO) << "meta:" << meta.c_str() << " size:" << meta.size();
  args["meta"] = meta;
  std::string result = cs.curl_post(url.c_str(), args);

  if (result == "") {
    LOG(ERROR) << "curl get error!";
    return false;
  }

  LOG(INFO) << "curl result:" << result.c_str();

  Json::Reader reader(Json::Features::strictMode());
  Json::Value val;

  if (!reader.parse(result, val)) {
    LOG(ERROR) << "parse result json error!";
    return false;
  }

  if (!val.isObject()) {
    LOG(ERROR) << "no valild json error!";
    return false;
  }

  if (val["success"].isNull() || !val["success"].isString()) {
    LOG(ERROR) << "parse field success error!";
    return false;
  }

  std::string success = val["success"].asString();

  if (success != "0") {
    LOG(ERROR) << "parse field success error!" << success.c_str();
    return false;
  }

  LOG(INFO) << "curl post ok";
  return true;
}

bool CROVLBuilderIncremental::read_last_meta_from_local() {
  char file[MAX_DATA_DIR_LEN * 2];

  snprintf(file,
           sizeof(file),
           "%s/../../meta_info/%s_%s_%s_%s.json",
           _data_dir,
           _last_version.c_str(),
           _depend_version.c_str(),
           _shard_id.c_str(),
           _split_id.c_str());

  // read from local meta file
  std::string input_meta;
  Json::Reader meta_reader(Json::Features::strictMode());
  Json::Value meta;

  std::ifstream ifs;
  ifs.open(file);
  if (!ifs) {
    LOG(ERROR) << "open file failed! " << file;
    return false;
  }

  if (!meta_reader.parse(ifs, meta)) {
    LOG(ERROR) << "parse meta json error!";
    return false;
  }
  ifs.close();

  if (!meta.isObject()) {
    LOG(ERROR) << "parse meta json error!";
    return false;
  }

  if (meta["data_len_list"].isNull() || !meta["data_len_list"].isArray()) {
    LOG(ERROR) << "parse data_len_list json error !";
    return false;
  }

  Json::Value data_len_list = meta["data_len_list"];
  _data_file = data_len_list.size();

  if (_data_file == 0) {
    LOG(ERROR) << "data len list size is 0 !";
    return false;
  }

  for (int i = 0; i < static_cast<int>(_data_file); ++i) {
    LOG(INFO) << "data_len:" << data_len_list[i].asString().c_str();

    try {
      _data_file_list.push_back(
          boost::lexical_cast<uint32_t>(data_len_list[i].asString()));
    } catch (boost::bad_lexical_cast &e) {
      LOG(ERROR) << "bad lexical cast:" << e.what();
      return false;
    }
  }

  if (meta["index_len_list"].isNull() || !meta["index_len_list"].isArray()) {
    LOG(ERROR) << "parse index_len_list json error!";
    return false;
  }

  Json::Value index_len_list = meta["index_len_list"];
  _index_file_num = index_len_list.size();

  if (_index_file_num == 0) {
    LOG(ERROR) << "index len list size is 0!";
    return false;
  }

  for (uint32_t i = 0; i < _index_file_num; ++i) {
    LOG(INFO) << "index_len:" << index_len_list[i].asString().c_str();

    try {
      _index_file_list.push_back(
          boost::lexical_cast<uint64_t>(index_len_list[i].asString()));
    } catch (boost::bad_lexical_cast &e) {
      LOG(ERROR) << "bad lexical cast:" << e.what();
      return false;
    }
  }

  if (meta["index_total_count"].isNull() ||
      !meta["index_total_count"].isString()) {
    LOG(ERROR) << "parse index_total_count json error!";
    return false;
  }

  LOG(INFO) << "index_total_count:"
            << meta["index_total_count"].asString().c_str();
  try {
    _count =
        boost::lexical_cast<uint64_t>(meta["index_total_count"].asString());
  } catch (boost::bad_lexical_cast &e) {
    LOG(ERROR) << "bad lexical cast:" << e.what();
    return false;
  }

  return true;
}

bool CROVLBuilderIncremental::write_cur_meta_to_local() {
  char file[MAX_DATA_DIR_LEN * 2];
  std::string meta_path = _data_dir;
  meta_path = meta_path + "/../../meta_info";
  if (!checkDirectory(meta_path)) {
    LOG(ERROR) << "create meta_path path failed:" << meta_path.c_str();
    return false;
  }
  snprintf(file,
           sizeof(file),
           "%s/%s_%s_%s_%s.json",
           meta_path.c_str(),
           _cur_version.c_str(),
           _depend_version.c_str(),
           _shard_id.c_str(),
           _split_id.c_str());

  Json::Value meta;

  try {
    meta["index_total_count"] = boost::lexical_cast<std::string>(_count);
  } catch (boost::bad_lexical_cast &e) {
    LOG(ERROR) << "bad lexical cast:" << e.what();
    return false;
  }

  Json::Value data;

  for (size_t i = 0; i < _data_file_list.size(); ++i) {
    try {
      data.append(boost::lexical_cast<std::string>(_data_file_list[i]));
    } catch (boost::bad_lexical_cast &e) {
      LOG(ERROR) << "bad lexical cast:" << e.what();
      return false;
    }
  }

  meta["data_len_list"] = data;

  Json::Value index;

  for (size_t i = 0; i < _index_file_list.size(); ++i) {
    try {
      index.append(boost::lexical_cast<std::string>(_index_file_list[i]));
    } catch (boost::bad_lexical_cast &e) {
      LOG(ERROR) << "bad lexical cast:" << e.what();
      return false;
    }
  }

  meta["index_len_list"] = index;

  std::ofstream ofs;
  ofs.open(file);
  if (!ofs) {
    LOG(ERROR) << "open file failed!" << file;
    return false;
  }
  ofs << meta.toStyledString();
  ofs.close();
  return true;
}
