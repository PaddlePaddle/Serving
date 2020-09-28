/*******************************************
 *
 * Copyright (c) 2020 Baidu.com, Inc. All Rights Reserved
 *
 ******************************************/
/**
 * @file center_crop.cpp
 * @author yinyijun@baidu.com
 * @date 2020-06-15
 **/
#include "center_crop.h"

#include <algorithm>

#include <npp.h>
#include "utils.h"

std::shared_ptr<OpContext> CenterCrop::operator()(std::shared_ptr<OpContext> input) {
    int new_width = std::min(_size, input->nppi_size().width);
    int new_height = std::min(_size, input->nppi_size().height);
    auto output = std::make_shared<OpContext>(new_height, new_width);
    int x_start = (input->nppi_size().width - new_width) / 2;
    int y_start = (input->nppi_size().height - new_height) / 2;
    Npp32f* p_src = input->p_frame()
                    + y_start * input->nppi_size().width * CHANNEL_SIZE
                    + x_start * CHANNEL_SIZE;
    NppStatus ret = nppiCopy_32f_C3R(p_src,
                                     input->step(),
                                     output->p_frame(),
                                     output->step(),
                                     output->nppi_size());
    verify_npp_ret("nppiCopy_32f_C3R", ret);
    return output;
}
