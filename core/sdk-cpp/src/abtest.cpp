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

#include "core/sdk-cpp/include/abtest.h"

namespace baidu {
namespace paddle_serving {
namespace sdk_cpp {

int WeightedRandomRender::initialize(const google::protobuf::Message& conf) {
  srand((unsigned)time(NULL));
  try {
    const configure::WeightedRandomRenderConf& weighted_random_render_conf =
        dynamic_cast<const configure::WeightedRandomRenderConf&>(conf);

    std::string weights = weighted_random_render_conf.variant_weight_list();

    std::vector<std::string> splits;
    if (str_split(weights, WEIGHT_SEPERATOR, &splits) != 0) {
      LOG(ERROR) << "Failed split string:" << weights;
      return -1;
    }

    uint32_t weight_size = splits.size();
    _normalized_sum = 0;
    for (uint32_t wi = 0; wi < weight_size; ++wi) {
      char* end_pos = NULL;
      uint32_t ratio = strtoul(splits[wi].c_str(), &end_pos, 10);
      if (end_pos == splits[wi].c_str()) {
        LOG(ERROR) << "Error ratio(uint32) format:" << splits[wi] << " at "
                   << wi;
        return -1;
      }

      _variant_weight_list.push_back(ratio);
      _normalized_sum += ratio;
    }

    if (_normalized_sum <= 0) {
      LOG(ERROR) << "Zero normalized weight sum";
      return -1;
    }

    VLOG(2) << "Succ read weights list: " << weights
            << ", count: " << _variant_weight_list.size()
            << ", normalized: " << _normalized_sum;
  } catch (std::bad_cast& e) {
    LOG(ERROR) << "Failed init WeightedRandomRender"
               << "from configure, err:" << e.what();
    return -1;
  } catch (...) {
    LOG(ERROR) << "Failed init WeightedRandomRender"
               << "from configure, err message is unkown.";
    return -1;
  }

  return 0;
}

Variant* WeightedRandomRender::route(const VariantList& variants,
                                     const void* params) {
  return route(variants);
}

Variant* WeightedRandomRender::route(const VariantList& variants) {
  if (variants.size() != _variant_weight_list.size()) {
    LOG(ERROR) << "#(Weights) is not equal #(Stubs)"
               << ", size: " << _variant_weight_list.size() << " vs. "
               << variants.size();
    return NULL;
  }

  uint32_t sample = rand() % _normalized_sum;  // NOLINT
  uint32_t cand_size = _variant_weight_list.size();
  uint32_t cur_total = 0;
  for (uint32_t ci = 0; ci < cand_size; ++ci) {
    cur_total += _variant_weight_list[ci];
    if (sample < cur_total) {
      VLOG(2) << "Sample " << sample << " on " << ci
              << ", _normalized: " << _normalized_sum
              << ", weight: " << _variant_weight_list[ci];
      return variants[ci];
    }
  }

  LOG(ERROR) << "Errors accurs in sampling, sample:" << sample
             << ", total: " << _normalized_sum;

  return NULL;
}

}  // namespace sdk_cpp
}  // namespace paddle_serving
}  // namespace baidu
