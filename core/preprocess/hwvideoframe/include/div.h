/*******************************************
 *
 * Copyright (c) 2020 Baidu.com, Inc. All Rights Reserved
 *
 ******************************************/
/**
 * @file div.h
 * @author yinyijun@baidu.com
 * @date 2020-06-11
 **/

#ifndef BAIDU_CVPRE_DIV_H
#define BAIDU_CVPRE_DIV_H

#include <memory>
#include <npp.h>
#include "op_context.h"

// divide by some float number for all pixel
class Div {
public:
    Div(float value);
    std::shared_ptr<OpContext> operator()(std::shared_ptr<OpContext> input);

private:
    Npp32f _divisor;
};

#endif // BAIDU_CVPRE_DIV_H