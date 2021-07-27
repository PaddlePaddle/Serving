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

#include "general_model.h"  // NOLINT

using namespace std;  // NOLINT

using baidu::paddle_serving::general_model::PredictorClient;
using baidu::paddle_serving::general_model::FetchedMap;

int main(int argc, char* argv[]) {
  PredictorClient* client = new PredictorClient();
  client->init("inference.conf");
  client->set_predictor_conf("./", "predictor.conf");
  client->create_predictor();
  std::vector<std::vector<float>> float_feed;
  std::vector<std::vector<int64_t>> int_feed;
  std::vector<std::string> float_feed_name;
  std::vector<std::string> int_feed_name = {"words", "label"};
  std::vector<std::string> fetch_name = {"cost", "acc", "prediction"};

  std::string line;
  int64_t text_id = 0;
  int64_t label = 0;
  int text_id_num = 0;
  int label_num = 0;
  int line_num = 0;
  while (cin >> text_id_num) {
    int_feed.clear();
    float_feed.clear();
    std::vector<int64_t> ids;
    ids.reserve(text_id_num);
    for (int i = 0; i < text_id_num; ++i) {
      cin >> text_id;
      ids.push_back(text_id);
    }
    int_feed.push_back(ids);
    cin >> label_num;
    cin >> label;
    int_feed.push_back({label});

    FetchedMap result;

    client->predict(float_feed,
                    float_feed_name,
                    int_feed,
                    int_feed_name,
                    fetch_name,
                    &result);

    cout << label << "\t" << result["prediction"][1] << endl;

    line_num++;
    if (line_num % 100 == 0) {
      cerr << "line num: " << line_num << endl;
    }
  }
}
