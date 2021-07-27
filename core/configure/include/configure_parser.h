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
#include <google/protobuf/message.h>
#include <string>

namespace baidu {
namespace paddle_serving {
namespace configure {

int read_proto_conf(const std::string &conf_full_path,
                    google::protobuf::Message *conf);

int read_proto_conf(const std::string &conf_path,
                    const std::string &conf_file,
                    google::protobuf::Message *conf);

int write_proto_conf(google::protobuf::Message *message,
                     const std::string &output_path,
                     const std::string &output_file);

}  // namespace configure
}  // namespace paddle_serving
}  // namespace baidu
