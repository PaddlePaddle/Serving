/*******************************************
 *
 * Copyright (c) 2020 Baidu.com, Inc. All Rights Reserved
 *
 ******************************************/
/**
 * @file resize.h
 * @author yinyijun@baidu.com
 * @date 2020-06-15
 **/

#ifndef BAIDU_CVPRE_RESIZE_H
#define BAIDU_CVPRE_RESIZE_H

#include <memory>
#include <vector>

#include <npp.h>
#include "op_context.h"

extern "C" cudaError_t resize_linear(const float* input,
                                     float* output,
                                     const int inputWidth,
                                     const int inputHeight,
                                     const int outputWidth,
                                     const int outputHeight,
                                     const int inputChannels,
                                     const bool use_fixed_point);

// Resize the input numpy array Image to the given size.
// only support linear interpolation
// only support RGB channels
class Resize {
public:
    // size is an int, smaller edge of the image will be matched to this number.
    Resize(int size, int max_size = 214748364, bool use_fixed_point = false, int interpolation = 0)
            : _size(size), _max_size(max_size), _use_fixed_point(use_fixed_point), _interpolation(interpolation) {};
    // size is a sequence like (w, h), output size will be matched to this
    Resize(std::vector<int> size, int max_size = 214748364, bool use_fixed_point = false, int interpolation = 0)
            : _size(-1), _max_size(max_size), _use_fixed_point(use_fixed_point), _interpolation(interpolation) {
        _target_size[0] = size[0];
        _target_size[1] = size[1];
    }
    std::shared_ptr<OpContext> operator()(std::shared_ptr<OpContext> input);

private:
    int _size;              // target of smaller edge
    int _target_size[2];    // target size sequence (w, h)
    int _max_size;
    bool _use_fixed_point;
    int _interpolation;     // unused
};

#endif // BAIDU_CVPRE_RESIZE_H