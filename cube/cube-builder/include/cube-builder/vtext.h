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

#ifndef CUBE_CUBE_BUILDER_INCLUDE_CUBE_BUILDER_VTEXT_H_
#define CUBE_CUBE_BUILDER_INCLUDE_CUBE_BUILDER_VTEXT_H_

#include <string>
#include "cube-builder/define.h"
#include "cube-builder/raw_reader.h"

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
  // todo
  /*static int32_t writeVInt(OutputStream& os, int i) {
      int32_t rc = 0;
      if (i >= -112 && i <= 127) {
          rc = os.write((char*)&i, (int64_t)sizeof(char));
          return rc;
      }
      int len = -112;
      if (i < 0) {
          i ^= -1L; // take one's complement'
          len = -120;
      }
      int32_t tmp = i;
      while (tmp != 0) {
          tmp = tmp >> 8;
          len--;
      }
      os.write((char*)&len, sizeof(char));
      len = (len < -120) ? -(len + 120) : -(len + 112);
      for (int idx = len - 1; idx >= 0; idx--) {
          int32_t shiftbits = idx * 8;
          int mask = 0xFFL << shiftbits;
          int val = (i & mask) >> shiftbits;
          os.write((char*)&val, sizeof(char));
      }
      return rc;
  }*/
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

#endif  // CUBE_CUBE_BUILDER_INCLUDE_CUBE_BUILDER_VTEXT_H_
