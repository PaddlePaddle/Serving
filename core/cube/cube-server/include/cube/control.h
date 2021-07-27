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

#include "core/cube/cube-server/control.pb.h"

#include "butil/third_party/rapidjson/document.h"
#include "butil/third_party/rapidjson/prettywriter.h"
#include "butil/third_party/rapidjson/stringbuffer.h"

namespace rec {
namespace mcube {

class Control : public ControlService {
 public:
  Control();

  virtual ~Control();

  virtual void cmd(::google::protobuf::RpcController* controller,
                   const ::rec::mcube::HttpRequest* request,
                   ::rec::mcube::HttpResponse* response,
                   ::google::protobuf::Closure* done);

 private:
  int handle_status(const BUTIL_RAPIDJSON_NAMESPACE::Document& cmd,
                    BUTIL_RAPIDJSON_NAMESPACE::Document* res);

  int handle_reload_base(const BUTIL_RAPIDJSON_NAMESPACE::Document& cmd,
                         const std::string& v_path);
  int handle_reload_patch(const BUTIL_RAPIDJSON_NAMESPACE::Document& cmd,
                          const std::string& v_path);

  int handle_bg_load_base(const BUTIL_RAPIDJSON_NAMESPACE::Document& cmd,
                          const std::string& v_path);
  int handle_bg_load_patch(const BUTIL_RAPIDJSON_NAMESPACE::Document& cmd,
                           const std::string& v_path);

  int handle_bg_unload(const BUTIL_RAPIDJSON_NAMESPACE::Document& cmd);

  int handle_bg_switch(const BUTIL_RAPIDJSON_NAMESPACE::Document& cmd);

  int handle_enable(const BUTIL_RAPIDJSON_NAMESPACE::Document& cmd);

  std::mutex _cmd_mutex;
};  // class Control

}  // namespace mcube
}  // namespace rec
