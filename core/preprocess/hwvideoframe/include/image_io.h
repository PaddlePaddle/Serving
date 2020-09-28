/*******************************************
 *
 * Copyright (c) 2020 Baidu.com, Inc. All Rights Reserved
 *
 ******************************************/
/**
 * @file image_io.h
 * @author yinyijun@baidu.com
 * @date 2020-06-08
 **/

#ifndef BAIDU_CVPRE_IMAGE_IO_H
#define BAIDU_CVPRE_IMAGE_IO_H

#include <memory>
#include <npp.h>
#include <pybind11/numpy.h>

#include "op_context.h"

// Input operator that copy numpy data to gpu buffer
class Image2Gpubuffer {
public:
    std::shared_ptr<OpContext> operator()(pybind11::array_t<float> array);
};

// Output operator that copy gpu buffer data to numpy
class Gpubuffer2Image {
public:
    pybind11::array_t<float> operator()(std::shared_ptr<OpContext> input);
};

#endif // BAIDU_CVPRE_IMAGE_IO_H