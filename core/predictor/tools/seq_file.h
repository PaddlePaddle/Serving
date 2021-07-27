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

#ifndef __SEQ_FILE_H_
#define __SEQ_FILE_H_

#include <fstream>

const int SYNC_MARKER_SIZE = 16;
const char SEQ_HEADER[] =
    "SEQ\x06"
    "\"org.apache.hadoop.io.BytesWritable\""
    "org.apache.hadoop.io.BytesWritable"
    "\x00\x00\x00\x00\x00\x00";
const int SYNC_INTERVAL = 2000;

class SeqFileWriter {
 public:
  SeqFileWriter(const char *file);
  ~SeqFileWriter();

 public:
  int write(const char *key,
            size_t key_len,
            const char *value,
            size_t value_len);

 private:
  void close();
  void _write_sync_marker();
  void _write_seq_header();

 private:
  char _sync_marker[SYNC_MARKER_SIZE];
  int _bytes_to_prev_sync;
  std::ofstream *_fs;
};

#endif  //__SEQ_FILE_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
