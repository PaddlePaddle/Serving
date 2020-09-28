/*******************************************
 *
 * Copyright (c) 2020 Baidu.com, Inc. All Rights Reserved
 *
 ******************************************/
/**
 * @file div.cpp
 * @author yinyijun@baidu.com
 * @date 2020-06-11
 **/
#include "div.h"

#include <stdexcept>

#include <npp.h>
#include "utils.h"

Div::Div(float value) {
    _divisor = value;
}

std::shared_ptr<OpContext> Div::operator()(std::shared_ptr<OpContext> input) {
    NppStatus ret = nppsDivC_32f_I(_divisor, input->p_frame(), input->length());
    verify_npp_ret("nppsDivC_32f_I", ret);
    return input;
}
