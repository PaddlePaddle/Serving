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
#include "core/cube/cube-builder/include/cube-builder/define.h"
#include "core/cube/cube-builder/include/cube-builder/raw_reader.h"

class VInt {
 public:
  static int32_t decode_vint_size(const char &value) {
    if (value >= -112) {
      return 1;
    } else if (value < -120) {
      return -119 - value;
    }
    return -111 - value;
  }
  static bool is_negative_vint(const char &value) {
    return value < -120 || (value >= -112 && value < 0);
  }
  static bool read_vint(RawReader *reader, int32_t *vint) {
    char first_byte;
    if (reader->read(&first_byte) <= 0) {
      return false;
    }
    int32_t len = decode_vint_size(first_byte);
    if (len == 1) {
      *vint = first_byte;
      return true;
    }
    char ch;
    int32_t bitlen = 0;
    int32_t i = 0, lch;
    for (int idx = len - 2; idx >= 0; idx--) {
      if (reader->read(&ch) <= 0) {
        return false;
      }
      bitlen = 8 * idx;
      lch = ch;
      i = i | ((lch << bitlen) & (0xFFL << bitlen));
    }
    *vint = (is_negative_vint(first_byte) ? (i ^ (int32_t)(-1)) : i);
    return true;
  }
};

class VString {
 public:
  static const char *encode(std::string str) { return encode(str, true); }

  static const char *encode(std::string str, bool /*replace*/) {
    // todo
    return str.c_str();
  }

  static std::string decode(char *bytes) { return decode(bytes, true); }

  static std::string decode(char *bytes, bool /*replace*/) {
    // todo
    return std::string(bytes);
  }

  // todo
  static bool read_string(RawReader *reader, std::string *str) {
    int length;
    if (!VInt::read_vint(reader, &length)) {
      return false;
    }
    if (reader->read_buf(str, length) != length) {
      return false;
    }
    return true;
  }
};
