/*******************************************
 *
 * Copyright (c) 2020 Baidu.com, Inc. All Rights Reserved
 *
 ******************************************/
/**
 * @file ExtractFrameJpeg.h
 * @author yinyijun@baidu.com
 * @date 2020-06-01
 **/
#include "rgb_swap.h"

#include <npp.h>
#include "utils.h"

const int SwapChannel::_ORDER[CHANNEL_SIZE] = {2, 1, 0};

std::shared_ptr<OpContext> SwapChannel::operator()(std::shared_ptr<OpContext> input) {
    NppStatus ret = nppiSwapChannels_32f_C3IR(input->p_frame(), input->step(), input->nppi_size(), _ORDER);
    verify_npp_ret("nppiSwapChannels_32f_C3IR", ret);
    return input;
}
