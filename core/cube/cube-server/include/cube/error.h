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

namespace rec {
namespace mcube {

/*
struct Error {
    enum ErrorCode {
        E_OK = 0,
        E_CONF_ERROR,
        E_PARAM_ERROR,
        E_SYS_ERROR,
        E_INTERNAL_ERROR,
        E_NETWORK_ERROR,
        E_DATA_ERROR,
        E_NO_IMPL,
        E_UNKNOWN,
        E_NUM,
        E_NOT_FOUND,
        E_NO_RES,
        E_NO_REQ_ERROR,
        E_TYPE_CONVERT_ERROR
    }; // enum ErrorCode
}; // struct Error
*/

const int E_OK = 0;
const int E_DATA_ERROR = -1;
const int E_OOM = -2;
const int E_NOT_IMPL = -3;

}  // namespace mcube
}  // namespace rec
