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

#include "bvar/bvar.h"
#include "bvar/recorder.h"
#include "bvar/window.h"

namespace bvar {
DECLARE_bool(bvar_dump);
DECLARE_string(bvar_dump_file);
}

namespace rec {
namespace mcube {

extern bvar::IntRecorder g_keys_num;
extern bvar::Adder<uint64_t> g_request_num;
extern bvar::IntRecorder g_data_load_time;
extern bvar::IntRecorder g_data_size;
extern bvar::Adder<uint64_t> g_long_value_num;
extern bvar::Adder<uint64_t> g_unfound_key_num;
extern bvar::Adder<uint64_t> g_total_key_num;

}  // namespace mcube
}  // namespace rec
