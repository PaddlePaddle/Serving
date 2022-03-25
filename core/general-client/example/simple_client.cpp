// Copyright (c) 2021 PaddlePaddle Authors. All Rights Reserved.
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

using baidu::paddle_serving::client::ServingClient;
using baidu::paddle_serving::client::ServingBrpcClient;
using baidu::paddle_serving::client::PredictorInputs;
using baidu::paddle_serving::client::PredictorOutputs;

DEFINE_string(server_port, "127.0.0.1:9292", "ip:port");
DEFINE_string(client_conf, "serving_client_conf.prototxt", "Path of client conf");
DEFINE_string(test_type, "brpc", "brpc");
// fit_a_line, bert
DEFINE_string(sample_type, "fit_a_line", "List: fit_a_line, bert");

namespace {
int prepare_fit_a_line(PredictorInputs& input, std::vector<std::string>& fetch_name) {
  std::vector<float> float_feed = {0.0137f, -0.1136f, 0.2553f, -0.0692f,
            0.0582f, -0.0727f, -0.1583f, -0.0584f,
            0.6283f, 0.4919f, 0.1856f, 0.0795f, -0.0332f};
  std::vector<int> float_shape = {1, 13};
  std::string feed_name = "x";
  fetch_name = {"price"};
  std::vector<int> lod;
  input.add_float_data(float_feed, feed_name, float_shape, lod);
  return 0;
}

int prepare_bert(PredictorInputs& input, std::vector<std::string>& fetch_name) {
  {
    std::vector<float> float_feed(128, 0.0f);
    float_feed[0] = 1.0f;
    std::vector<int> float_shape = {1, 128, 1};
    std::string feed_name = "input_mask";
    std::vector<int> lod;
    input.add_float_data(float_feed, feed_name, float_shape, lod);
  }
  {
    std::vector<int64_t> feed(128, 0);
    std::vector<int> shape = {1, 128, 1};
    std::string feed_name = "position_ids";
    std::vector<int> lod;
    input.add_int64_data(feed, feed_name, shape, lod);
  }
  {
    std::vector<int64_t> feed(128, 0);
    feed[0] = 101;
    std::vector<int> shape = {1, 128, 1};
    std::string feed_name = "input_ids";
    std::vector<int> lod;
    input.add_int64_data(feed, feed_name, shape, lod);
  }
  {
    std::vector<int64_t> feed(128, 0);
    std::vector<int> shape = {1, 128, 1};
    std::string feed_name = "segment_ids";
    std::vector<int> lod;
    input.add_int64_data(feed, feed_name, shape, lod);
  }
  
  fetch_name = {"pooled_output"};
  return 0;
}
} // namespace

int main(int argc, char* argv[]) {

  google::ParseCommandLineFlags(&argc, &argv, true);
  std::string url = FLAGS_server_port;
  std::string conf = FLAGS_client_conf;
  std::string test_type = FLAGS_test_type;
  std::string sample_type = FLAGS_sample_type;
  LOG(INFO) << "url = " << url << ";"
            << "client_conf = " << conf << ";"
            << "test_type = " << test_type
            << "sample_type = " << sample_type;
  std::unique_ptr<ServingClient> client;
  // default type is brpc
  // will add grpc&http in the future
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

  PredictorInputs input;
  PredictorOutputs output;
  std::vector<std::string> fetch_name;

  if (sample_type == "fit_a_line") {
    prepare_fit_a_line(input, fetch_name);
  }
  else if (sample_type == "bert") {
    prepare_bert(input, fetch_name);
  }
  else {
    prepare_fit_a_line(input, fetch_name);
  }

  if (client->predict(input, output, fetch_name, 0) != 0) {
    LOG(ERROR) << "Failed to predict!";
  }
  else {
    LOG(INFO) << output.print();
  }
  
  return 0;
}
