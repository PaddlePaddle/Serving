/*******************************************
 *
 * Copyright (c) 2020 Baidu.com, Inc. All Rights Reserved
 *
 ******************************************/
/**
 * @file sub.cpp
 * @author yinyijun@baidu.com
 * @date 2020-06-11
 **/
#include "sub.h"

#include <stdexcept>

#include <npp.h>
#include "utils.h"

Sub::Sub(float subtractor) {
    for (size_t i = 0; i < CHANNEL_SIZE; i++) {
        _subtractors[i] = subtractor;
    }
}

Sub::Sub(const std::vector<float> &subtractors) {
    if (subtractors.size() != CHANNEL_SIZE) {
        throw std::runtime_error("size of subtractors must be three");
    }
    for (size_t i = 0; i < CHANNEL_SIZE; i++) {
        _subtractors[i] = subtractors[i];
    }
}

std::shared_ptr<OpContext> Sub::operator()(std::shared_ptr<OpContext> input) {
    NppStatus ret = nppiSubC_32f_C3IR(_subtractors, input->p_frame(), input->step(), input->nppi_size());
    verify_npp_ret("nppiSubC_32f_C3IR", ret);
    return input;
}
