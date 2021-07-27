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

#include "core/cube/cube-server/include/cube/server.h"
#include <brpc/server.h>
#include "core/cube/cube-server/include/cube/framework.h"

namespace rec {
namespace mcube {

Server::Server() {}

Server::~Server() {}

void Server::seek(::google::protobuf::RpcController* /*cntl_base*/,
                  const ::rec::mcube::DictRequest* request,
                  ::rec::mcube::DictResponse* response,
                  ::google::protobuf::Closure* done) {
  brpc::ClosureGuard done_guard(done);

  Framework* framework = Framework::instance();
  int ret = framework->seek(request, response);
  if (ret != 0) {
    LOG(ERROR) << "seek failed err=" << ret;
  }
}

}  // namespace mcube
}  // namespace rec
