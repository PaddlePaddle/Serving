// Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
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

#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctime>
#include <iostream>

#include <unistd.h>
#include "seq_file.h"

SeqFileWriter::SeqFileWriter(const char *file) {
  _fs = new std::ofstream(file, std::ios::binary);
  std::srand(std::time(0));
  for (int i = 0; i < SYNC_MARKER_SIZE; ++i) {
    _sync_marker[i] = std::rand() % 255;
  }

  _write_seq_header();

  _bytes_to_prev_sync = 0;
}

void SeqFileWriter::close() {
  _fs->close();
  delete _fs;
}

SeqFileWriter::~SeqFileWriter() { close(); }

void SeqFileWriter::_write_sync_marker() {
  char begin[] = {'\xFF', '\xFF', '\xFF', '\xFF'};
  _fs->write(begin, 4);

  _fs->write(_sync_marker, SYNC_MARKER_SIZE);
}

void SeqFileWriter::_write_seq_header() {
  _fs->write(SEQ_HEADER, sizeof(SEQ_HEADER) - 1);
  _fs->write(_sync_marker, SYNC_MARKER_SIZE);
}

int SeqFileWriter::write(const char *key,
                         size_t key_len,
                         const char *value,
                         size_t value_len) {
  if (key_len != sizeof(int64_t)) {
    std::cout << "Key length not equal to " << sizeof(int64_t) << std::endl;
    return -1;
  }

  uint32_t record_len = key_len + value_len;
  uint32_t b_record_len = htonl(record_len);
  uint32_t b_key_len = htonl((uint32_t)key_len);
  // std::cout << "b_record_len " << b_record_len << " record_len " <<
  // record_len << std::endl;
  _fs->write((char *)&b_record_len, sizeof(uint32_t));
  _fs->write((char *)&b_key_len, sizeof(uint32_t));
  _fs->write(key, key_len);
  _fs->write(value, value_len);
  _bytes_to_prev_sync += record_len;

  if (_bytes_to_prev_sync >= SYNC_INTERVAL) {
    _write_sync_marker();
    _bytes_to_prev_sync = 0;
  }

  return 0;
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
