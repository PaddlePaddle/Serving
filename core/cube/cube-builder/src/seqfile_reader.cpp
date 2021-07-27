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

#include "core/cube/cube-builder/include/cube-builder/seqfile_reader.h"
#include <arpa/inet.h>
#include "butil/logging.h"
#include "core/cube/cube-builder/include/cube-builder/vtext.h"

int SequenceFileRecordReader::open() {
  if (_raw_reader->open() != 0) {
    return -1;
  }
  LOG(INFO) << "open sequence file ok! file:" << _path.c_str();
  return 0;
}
int SequenceFileRecordReader::close() {
  if (_raw_reader->close() != 0) {
    return -1;
  }
  LOG(INFO) << "close sequence file ok! file:" << _path.c_str();
  return 0;
}
int SequenceFileRecordReader::next(Record* record) {
  uint32_t record_len = 0;
  int64_t ret = _raw_reader->read(&record_len);
  if (ret == 0) {
    return 1;  // ?????1???????????????
  } else if (ret != sizeof(record_len)) {
    LOG(ERROR) << "read sequence file:" << _path
               << " record(record_len) errno:" << ret
               << ", errmsg:" << _raw_reader->errno_to_str(ret);
    return -1;
  }
  record->record_len = static_cast<int>(ntohl(record_len));
  // got marker
  if (record->record_len == -1) {
    std::string marker;
    if ((ret = _raw_reader->read_buf(&marker, 16)) != 16) {
      LOG(ERROR) << "read sequence file:" << _path
                 << " record(marker) errno:" << ret
                 << ", errmsg:" << _raw_reader->errno_to_str(ret);
      return -1;
    }
    if (marker != record->sync_marker) {
      LOG(ERROR) << "read sequence file:" << _path
                 << " record(sync_marker) error!";
      return -1;
    }
    if ((ret = _raw_reader->read(&record->record_len)) !=
        sizeof(record->record_len)) {
      LOG(ERROR) << "read sequence file:" << _path
                 << " record(len) errno:" << ret
                 << ", errmsg:" << _raw_reader->errno_to_str(ret);
      return -1;
    }
    record->record_len = static_cast<int>(ntohl(record->record_len));
  }
  uint32_t key_len = 0;
  if ((ret = _raw_reader->read(&key_len)) != sizeof(key_len)) {
    LOG(ERROR) << "read sequence file:" << _path
               << " record(key_len) errno:" << ret
               << ", errmsg:" << _raw_reader->errno_to_str(ret);
    return -1;
  }
  record->key_len = static_cast<int>(ntohl(key_len));
  if ((ret = _raw_reader->read_buf(&record->key, record->key_len)) !=
      record->key_len) {
    LOG(ERROR) << "read sequence file:" << _path
               << " record(key_len) errno:" << ret
               << ", errmsg:" << _raw_reader->errno_to_str(ret);
    return -1;
  }
  if ((ret = _raw_reader->read_buf(&record->value,
                                   record->record_len - record->key_len)) !=
      (record->record_len - record->key_len)) {
    LOG(ERROR) << "read sequence file:" << _path
               << " record(value_len) errno:" << ret
               << ", errmsg:" << _raw_reader->errno_to_str(ret);
    return -1;
  }
  return 0;
}

int SequenceFileRecordReader::read_header() {
  LOG(INFO) << "start to read sequence file header:" << _path;
  char version[4];
  if (_raw_reader->read_buf(&version, 4) != 4) {
    LOG(ERROR) << "read sequence file header(version) error:" << _path;
    return -1;
  }
  _header.version = version[3];
  if (!VString::read_string(_raw_reader, &_header.key_class)) {
    LOG(ERROR) << "read sequence file header(key_class) error:" << _path;
    return -1;
  }
  if (!VString::read_string(_raw_reader, &_header.value_class)) {
    LOG(ERROR) << "read sequence file header(value_class) error:" << _path;
    return -1;
  }
  if (_raw_reader->read(&_header.is_compress) != sizeof(bool)) {
    LOG(ERROR) << "read sequence file header(is_compress) error:" << _path;
    return -1;
  }
  if (_raw_reader->read(&_header.is_block_compress) != sizeof(bool)) {
    LOG(ERROR) << "read sequence file header(is_block_compress) error:"
               << _path;
    return -1;
  }
  if (_header.is_compress) {
    if (!VString::read_string(_raw_reader, &_header.compress_class)) {
      LOG(ERROR) << "read sequence file header(compress_class) error:" << _path;
      return -1;
    }
  }
  int32_t meta_cnt = 0;
  if (_raw_reader->read(&meta_cnt) != sizeof(int32_t)) {
    LOG(ERROR) << "read sequence file header(meta_cnt) error:" << _path;
    return -1;
  }
  _header.metas.resize(meta_cnt);
  for (int32_t i = 0; i != meta_cnt; ++i) {
    if (!VString::read_string(_raw_reader, &_header.metas[i].key)) {
      LOG(ERROR) << "read sequence file header(meta_key) error:" << _path;
      return -1;
    }
    if (!VString::read_string(_raw_reader, &_header.metas[i].value)) {
      LOG(ERROR) << "read sequence file header(meta_value) error:" << _path;
      return -1;
    }
  }
  if (_raw_reader->read_buf(&_header.sync_marker, 16) != 16) {
    LOG(ERROR) << "read sequence file header(sync_marker) error:" << _path;
    return -1;
  }

  LOG(INFO) << "sync_marker:" << _header.sync_marker;
  LOG(INFO) << "read sequence file header ok:" << _path;
  return 0;
}
