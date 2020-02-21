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

#include "examples/demo-serving/op/kvdb_echo_op.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

using baidu::paddle_serving::predictor::format::KVDBReq;
using baidu::paddle_serving::predictor::format::KVDBRes;
using baidu::paddle_serving::predictor::echo_kvdb_service::Request;
using baidu::paddle_serving::predictor::echo_kvdb_service::Response;

int KVDBEchoOp::inference() { debug(); }
int KVDBEchoOp::debug() {
  // TODO: implement DEBUG mode
  baidu::paddle_serving::predictor::Resource& resource =
      baidu::paddle_serving::predictor::Resource::instance();
  std::shared_ptr<RocksDBWrapper> db = resource.getDB();
  const Request* req = dynamic_cast<const Request*>(get_request_message());
  Response* res = mutable_data<Response>();
  LOG(INFO) << "Receive request in KVDB echo service: "
            << req->ShortDebugString();
  for (size_t i = 0; i < req->reqs_size(); i++) {
    auto kvdbreq = req->reqs(i);
    std::string op = kvdbreq.op();
    std::string key = kvdbreq.key();
    std::string val = kvdbreq.value();
    if (op == "SET") {
      db->Put(key, val);
      KVDBRes* kvdb_value_res = res->mutable_ress()->Add();
      kvdb_value_res->set_value("OK");
    } else if (op == "GET") {
      std::string getvalue = db->Get(key);
      KVDBRes* kvdb_value_res = res->mutable_ress()->Add();
      kvdb_value_res->set_value(getvalue);
    }
  }
  return 0;
}

DEFINE_OP(KVDBEchoOp);
}
}
}
