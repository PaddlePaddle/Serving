/*******************************************
 *
 * Copyright (c) 2020 Baidu.com, Inc. All Rights Reserved
 *
 ******************************************/
/**
 * @file rgb_swap.h
 * @author yinyijun@baidu.com
 * @date 2020-06-01
 **/

#ifndef BAIDU_CVPRE_RGB_SWAP_H
#define BAIDU_CVPRE_RGB_SWAP_H

#include <memory>
#include <npp.h>
#include "op_context.h"

// swap channel 0 and channel 2 for every pixel
// both RGB2BGR and BGR2RGB use this operator
class SwapChannel {
public:
    SwapChannel(){};
    std::shared_ptr<OpContext> operator()(std::shared_ptr<OpContext> input);

private:
    static const int _ORDER[CHANNEL_SIZE];  // describing how channel values are permutated
};

class RGB2BGR : public SwapChannel {};
class BGR2RGB : public SwapChannel {};

#endif // BAIDU_CVPRE_RGB_SWAP_H
