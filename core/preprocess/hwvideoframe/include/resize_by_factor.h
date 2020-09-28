/*******************************************
 *
 * Copyright (c) 2020 Baidu.com, Inc. All Rights Reserved
 *
 ******************************************/
/**
 * @file resize_by_factor.h
 * @author wubohao@baidu.com
 * @date 2020-07-21
 **/

#ifndef BAIDU_CVPRE_RESIZE_BY_FACTOR_H
#define BAIDU_CVPRE_RESIZE_BY_FACTOR_H

#include <memory>
#include <vector>

#include <npp.h>
#include "op_context.h"

extern "C" cudaError_t resize_linear(const float *input,
                                     float *output,
                                     const int inputWidth,
                                     const int inputHeight,
                                     const int outputWidth,
                                     const int outputHeight,
                                     const int inputChannels,
                                     const bool use_fixed_point);

// Resize the input numpy array Image to a size multiple of factor which is usually required by a network
// only support linear interpolation
// only support RGB channels
class ResizeByFactor
{
public:
    // Resize factor. make width and height multiple factor of the value of factor. Default is 32
    ResizeByFactor(int factor = 32, int max_side_len = 2400, bool use_fixed_point = false, int interpolation = 0)
        : _factor(factor), _max_side_len(max_side_len), _use_fixed_point(use_fixed_point), _interpolation(interpolation){};
    std::shared_ptr<OpContext> operator()(std::shared_ptr<OpContext> input);

private:
    int _factor; // target of smaller edge
    int _max_side_len;
    bool _use_fixed_point;
    int _interpolation; // unused
};

#endif // BAIDU_CVPRE_RESIZE_BY_FACTOR_H