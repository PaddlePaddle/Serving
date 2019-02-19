/***************************************************************************
 * 
 * Copyright (c) 2018 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
/**
 * @file ../src/abtest.cpp
 * @author wanlijin01(wanlijin01@baidu.com)
 * @date 2018/07/09 17:41:27
 * @brief 
 *  
 **/

#include "abtest.h"

namespace baidu {
namespace paddle_serving {
namespace sdk_cpp {


int WeightedRandomRender::initialize(
        const comcfg::ConfigUnit& conf) {
    srand((unsigned)time(NULL));
    try {
        std::string weights 
            = conf["VariantWeightList"].to_cstr();

        std::vector<std::string> splits;
        if (str_split(weights, WEIGHT_SEPERATOR, &splits) != 0) {
            LOG(FATAL) << "Failed split string:" <<
                weights;
            return -1;
        }

        uint32_t weight_size = splits.size();
        _normalized_sum = 0;
        for (uint32_t wi = 0; wi < weight_size; ++wi) {
            char* end_pos = NULL;
            uint32_t ratio = strtoul(
                    splits[wi].c_str(), &end_pos, 10);
            if (end_pos == splits[wi].c_str()) {
                LOG(FATAL) << "Error ratio(uint32) format:"
                    << splits[wi] << " at " << wi;
                return -1;
            }

            _variant_weight_list.push_back(ratio);
            _normalized_sum += ratio;
        }

        if (_normalized_sum <= 0) {
            LOG(FATAL) << "Zero normalized weight sum";
            return -1;
        }

        LOG(INFO) << "Succ read weights list: " << weights
            << ", count: " << _variant_weight_list.size()
            << ", normalized: " << _normalized_sum;
    } catch (comcfg::ConfigException& e) {
        LOG(FATAL) << "Failed init WeightedRandomRender" 
            << "from configure, err:" << e.what();
        return -1;
    } catch (...) {
        LOG(FATAL) << "Failed init WeightedRandomRender" 
            << "from configure, err message is unkown.";
        return -1;
    }

    return 0;
}

Variant* WeightedRandomRender::route(
        const VariantList& variants,
        const void* params) {
    return route(variants);
}

Variant* WeightedRandomRender::route(
        const VariantList& variants) {
    if (variants.size() != _variant_weight_list.size()) {
        LOG(FATAL) << "#(Weights) is not equal #(Stubs)"
            << ", size: " << _variant_weight_list.size()
            << " vs. " << variants.size();
        return NULL; 
    }
    
    uint32_t sample = rand() % _normalized_sum;
    uint32_t cand_size = _variant_weight_list.size();
    uint32_t cur_total = 0;
    for (uint32_t ci = 0; ci < cand_size; ++ci) {
        cur_total += _variant_weight_list[ci];
        if (sample < cur_total) {
            LOG(INFO) << "Sample " << sample << " on " << ci
                << ", _normalized: " << _normalized_sum
                << ", weight: " << _variant_weight_list[ci];
            return variants[ci];
        }
    }

    LOG(FATAL) << "Errors accurs in sampling, sample:"
        << sample << ", total: " << _normalized_sum;

    return NULL;
}

} // sdk_cpp
} // paddle_serving
} // baidu

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
