/*******************************************
 *
 * Copyright (c) 2020 Baidu.com, Inc. All Rights Reserved
 *
 ******************************************/
/**
 * @file sub.h
 * @author yinyijun@baidu.com
 * @date 2020-07-06
 **/

#ifndef BAIDU_CVPRE_SUB_H
#define BAIDU_CVPRE_SUB_H

#include <memory>
#include <vector>
#include <npp.h>
#include "op_context.h"

// subtract by some float numbers
class Sub {
public:
    Sub(float subtractor);
    Sub(const std::vector<float> &subtractors);
    std::shared_ptr<OpContext> operator()(std::shared_ptr<OpContext> input);

private:
    Npp32f _subtractors[CHANNEL_SIZE];
};

#endif // BAIDU_CVPRE_SUB_H