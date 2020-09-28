/*******************************************
 *
 * Copyright (c) 2020 Baidu.com, Inc. All Rights Reserved
 *
 ******************************************/
/**
 * @file normalize.h
 * @author yinyijun@baidu.com
 * @date 2020-06-04
 **/

#ifndef BAIDU_CVPRE_NORMALIZE_H
#define BAIDU_CVPRE_NORMALIZE_H

#include <memory>
#include <vector>
#include <npp.h>
#include "op_context.h"

// utilize normalize operator on gpu
class Normalize {
public:
    Normalize(const std::vector<float> &mean, const std::vector<float> &std, bool channel_first = false);
    std::shared_ptr<OpContext> operator()(std::shared_ptr<OpContext> input);

private:
    Npp32f _mean[CHANNEL_SIZE];
    Npp32f _std[CHANNEL_SIZE]; 
    bool _channel_first;            // indicate whether the channel is dimension 0, unsupported
};

#endif // BAIDU_CVPRE_NORMALIZE_H