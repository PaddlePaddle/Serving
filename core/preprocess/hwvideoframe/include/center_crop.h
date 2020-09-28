/*******************************************
 *
 * Copyright (c) 2020 Baidu.com, Inc. All Rights Reserved
 *
 ******************************************/
/**
 * @file center_crop.h
 * @author yinyijun@baidu.com
 * @date 2020-06-15
 **/

#ifndef BAIDU_CVPRE_CENTER_CROP_H
#define BAIDU_CVPRE_CENTER_CROP_H

#include <memory>
#include <npp.h>
#include "op_context.h"

// Crops the given Image at the center.
// the size must not bigger than any inputs' height and width
class CenterCrop {
public:
    CenterCrop(int size) : _size(size) {};
    std::shared_ptr<OpContext> operator()(std::shared_ptr<OpContext> input);

private:
    int _size;
};

#endif // BAIDU_CVPRE_CENTER_CROP_H