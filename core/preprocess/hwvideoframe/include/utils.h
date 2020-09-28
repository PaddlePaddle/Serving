/*******************************************
 *
 * Copyright (c) 2020 Baidu.com, Inc. All Rights Reserved
 *
 ******************************************/
/**
 * @file rgb_swap.h
 * @author yinyijun@baidu.com
 * @date 2020-06-10
 **/

#ifndef BAIDU_CVPRE_UTILS_H
#define BAIDU_CVPRE_UTILS_H

#include <string>

#include <npp.h>

// verify return value of npp function
// throw an exception if failed
void verify_npp_ret(const std::string& function_name, NppStatus ret);

// verify return value of cuda runtime function
// throw an exception if failed
void verify_cuda_ret(const std::string& function_name, cudaError_t ret);

#endif // BAIDU_CVPRE_UTILS_H