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

#include <brpc/server.h>

#include "core/cube/cube-server/include/cube/control.h"
#include "core/cube/cube-server/include/cube/framework.h"

namespace rec {
namespace mcube {

using ::rec::mcube::HttpRequest;
using ::rec::mcube::HttpResponse;

using ::google::protobuf::RpcController;
using ::google::protobuf::Closure;

using ::brpc::HttpHeader;
using ::brpc::URI;
using ::brpc::Controller;
using ::brpc::ClosureGuard;

using BUTIL_RAPIDJSON_NAMESPACE::Document;

std::string rapidjson_value_to_string(
    const BUTIL_RAPIDJSON_NAMESPACE::Value& value) {
  BUTIL_RAPIDJSON_NAMESPACE::StringBuffer buffer;
  BUTIL_RAPIDJSON_NAMESPACE::PrettyWriter<
      BUTIL_RAPIDJSON_NAMESPACE::StringBuffer>
      writer(buffer);
  value.Accept(writer);
  return buffer.GetString();
}

Control::Control() {}

Control::~Control() {}

void Control::cmd(::google::protobuf::RpcController* cntl_base,
                  const ::rec::mcube::HttpRequest* /*request*/,
                  ::rec::mcube::HttpResponse* /*response*/,
                  ::google::protobuf::Closure* done) {
  ClosureGuard done_guard(done);
  Controller* cntl = static_cast<Controller*>(cntl_base);

  std::string cmd_str = cntl->request_attachment().to_string();
  Document cmd;
  cmd.Parse(cmd_str.c_str());
  LOG(INFO) << "HANDLE CMD: " << cmd_str;
  if (!cmd.IsObject()) {
    LOG(ERROR) << "parse command failed";
    cntl->http_response().set_status_code(brpc::HTTP_STATUS_BAD_REQUEST);
    return;
  }

  if (!cmd.HasMember("cmd") || !cmd["cmd"].IsString()) {
    cntl->http_response().set_status_code(brpc::HTTP_STATUS_BAD_REQUEST);
    return;
  }

  std::string cmd_name = cmd["cmd"].GetString();

  std::string version_path = "";
  if (cmd.HasMember("version_path") && cmd["version_path"].IsString()) {
    version_path = cmd["version_path"].GetString();
  }

  int ret = 0;
  Document response;
  if (cmd_name.compare("status") == 0) {
    ret = handle_status(cmd, &response);
  } else if (_cmd_mutex.try_lock()) {
    if (cmd_name.compare("reload_base") == 0) {
      ret = handle_reload_base(cmd, version_path);
    } else if (cmd_name.compare("reload_patch") == 0) {
      ret = handle_reload_patch(cmd, version_path);
    } else if (cmd_name.compare("bg_load_base") == 0) {
      ret = handle_bg_load_base(cmd, version_path);
    } else if (cmd_name.compare("bg_load_patch") == 0) {
      ret = handle_bg_load_patch(cmd, version_path);
    } else if (cmd_name.compare("bg_unload") == 0) {
      ret = handle_bg_unload(cmd);
    } else if (cmd_name.compare("bg_switch") == 0) {
      ret = handle_bg_switch(cmd);
    } else if (cmd_name.compare("enable") == 0) {
      ret = handle_enable(cmd);
    } else {
      ret = -1;
      LOG(ERROR) << "unknown cmd: " << cmd_name;
    }
    _cmd_mutex.unlock();
  } else {
    LOG(ERROR) << "try to get cmd mutex failed cmd: " << cmd_name;
    ret = -1;
  }

  cntl->response_attachment().append(rapidjson_value_to_string(response));
  if (ret == 0) {
    cntl->http_response().set_status_code(brpc::HTTP_STATUS_OK);
  } else {
    cntl->http_response().set_status_code(
        brpc::HTTP_STATUS_INTERNAL_SERVER_ERROR);
  }

  LOG(INFO) << "CMD DONE: " << cmd_str;
  return;
}

int Control::handle_status(const Document& /*cmd*/, Document* res) {
  Framework* framework = Framework::instance();
  return framework->status(res);
}

int Control::handle_reload_patch(const Document& /*cmd*/,
                                 const std::string& v_path) {
  Framework* framework = Framework::instance();
  return framework->patch(v_path);
}

int Control::handle_reload_base(const Document& /*cmd*/,
                                const std::string& v_path) {
  Framework* framework = Framework::instance();
  return framework->reload(v_path);
}

int Control::handle_bg_load_patch(const Document& /*cmd*/,
                                  const std::string& v_path) {
  Framework* framework = Framework::instance();
  return framework->bg_load_patch(v_path);
}

int Control::handle_bg_load_base(const Document& /*cmd*/,
                                 const std::string& v_path) {
  Framework* framework = Framework::instance();
  return framework->bg_load_base(v_path);
}

int Control::handle_bg_unload(const Document& /*cmd*/) {
  Framework* framework = Framework::instance();
  return framework->bg_unload();
}

int Control::handle_bg_switch(const Document& /*cmd*/) {
  Framework* framework = Framework::instance();
  return framework->bg_switch();
}

int Control::handle_enable(const Document& cmd) {
  if (!cmd.HasMember("version") || !cmd["version"].IsString()) {
    return -1;
  }
  Framework* framework = Framework::instance();
  return framework->enable(cmd["version"].GetString());
}

}  // namespace mcube
}  // namespace rec
