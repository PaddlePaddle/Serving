/*******************************************
 *
 * Copyright (c) 2020 Baidu.com, Inc. All Rights Reserved
 *
 ******************************************/
/**
 * @file normalize.cpp
 * @author yinyijun@baidu.com
 * @date 2020-06-04
 **/
#include "normalize.h"
#include <stdexcept>

#include <npp.h>
#include "utils.h"

Normalize::Normalize(const std::vector<float> &mean, const std::vector<float> &std, bool channel_first) {
    if (mean.size() != CHANNEL_SIZE) {
        throw std::runtime_error("size of mean must be three");
    }
    if (std.size() != CHANNEL_SIZE) {
        throw std::runtime_error("size of std must be three");
    }
    for (size_t i = 0; i < CHANNEL_SIZE; i++) {
        _mean[i] = mean[i];
        _std[i] = std[i];
    }
    _channel_first = channel_first;
}

std::shared_ptr<OpContext> Normalize::operator()(std::shared_ptr<OpContext> input) {
    NppStatus ret = nppiSubC_32f_C3IR(_mean, input->p_frame(), input->step(), input->nppi_size());
    verify_npp_ret("nppiSubC_32f_C3IR", ret);
    ret = nppiDivC_32f_C3IR(_std, input->p_frame(), input->step(), input->nppi_size());
    verify_npp_ret("nppiDivC_32f_C3IR", ret);
    return input;
}
