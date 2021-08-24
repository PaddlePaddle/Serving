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

#include <fstream>
#include <vector>

#include "core/general-client/include/brpc_client.h"

using namespace std;  // NOLINT

using baidu::paddle_serving::client::ServingClient;
using baidu::paddle_serving::client::ServingBrpcClient;
using baidu::paddle_serving::client::PredictorInputs;
using baidu::paddle_serving::client::PredictorOutputs;

DEFINE_string(server_port, "127.0.0.1:9292", "");
DEFINE_string(client_conf, "serving_client_conf.prototxt", "");
DEFINE_string(test_type, "brpc", "");

int main(int argc, char* argv[]) {

  google::ParseCommandLineFlags(&argc, &argv, true);
  std::string url = FLAGS_server_port;
  std::string conf = FLAGS_client_conf;
  std::string test_type = FLAGS_test_type;
  LOG(INFO) << "url = " << url << ";"
            << "client_conf = " << conf << ";"
            << "type = " << test_type;
  std::unique_ptr<ServingClient> client;
  if (test_type == "brpc") {
    client.reset(new ServingBrpcClient());
  } else {
    client.reset(new ServingBrpcClient());
  }
  std::vector<std::string> confs;
  confs.push_back(conf);
  if (client->init(confs, url) != 0) {
    LOG(ERROR) << "Failed to init client!";
    return 0;
  }

  std::vector<float> float_feed = {0.0137f, -0.1136f, 0.2553f, -0.0692f,
            0.0582f, -0.0727f, -0.1583f, -0.0584f,
            0.6283f, 0.4919f, 0.1856f, 0.0795f, -0.0332f};
  std::vector<int> float_shape = {1, 13};
  std::string feed_name = "x";
  std::vector<std::string> fetch_name = {"price"};
  std::vector<int> lod;

  PredictorInputs input;
  PredictorOutputs output;

  input.add_float_data(float_feed, feed_name, float_shape, lod);

  if (client->predict(input, output, fetch_name, 0) != 0) {
    LOG(ERROR) << "Failed to predict!";
  }
  else {
    LOG(INFO) << output.print();
  }
  
  return 0;
}
