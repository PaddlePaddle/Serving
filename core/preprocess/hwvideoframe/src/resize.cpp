/*******************************************
 *
 * Copyright (c) 2020 Baidu.com, Inc. All Rights Reserved
 *
 ******************************************/
/**
 * @file resize.cpp
 * @author yinyijun@baidu.com
 * @date 2020-06-15
 **/
#include "resize.h"

#include <sstream>
#include <math.h>

#include "utils.h"

std::shared_ptr<OpContext> Resize::operator()(std::shared_ptr<OpContext> input) {
    int resized_width = 0, resized_height = 0;
    if (_size == -1) {
        resized_width = std::min(_target_size[0], _max_size);
        resized_height = std::min(_target_size[1], _max_size);
    } else {
        int im_max_size = std::max(input->nppi_size().height, input->nppi_size().width);
        float percent = (float)_size / std::min(input->nppi_size().height, input->nppi_size().width);
        if (round(percent * im_max_size) > _max_size) {
            percent = float(_max_size) / float(im_max_size);
        }
        resized_width = int(round(input->nppi_size().width * percent));
        resized_height = int(round(input->nppi_size().height * percent));
    }
    auto output = std::make_shared<OpContext>(resized_height, resized_width);
    auto ret = resize_linear(input->p_frame(),
                               output->p_frame(),
                               input->nppi_size().width,
                               input->nppi_size().height,
                               output->nppi_size().width,
                               output->nppi_size().height,
                               CHANNEL_SIZE,
                               _use_fixed_point);
    verify_cuda_ret("resize_linear", ret);
    return output;
}
