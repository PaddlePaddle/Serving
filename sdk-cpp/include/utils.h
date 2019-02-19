/***************************************************************************
 * 
 * Copyright (c) 2018 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file utils.h
 * @author root(com@baidu.com)
 * @date 2018/07/09 19:43:36
 * @brief 
 *  
 **/

#ifndef  BAIDU_PADDLE_SERVING_SDK_CPP_UTILS_H
#define  BAIDU_PADDLE_SERVING_SDK_CPP_UTILS_H

#include "common.h"

namespace baidu {
namespace paddle_serving {
namespace sdk_cpp {

inline int str_split(
        const std::string& source,
        const std::string& delim,
        std::vector<std::string>* vector_spliter) {

    int delim_length = delim.length();
    int total_length = source.length();
    int last = 0;

    if (delim_length == 0) {
        vector_spliter->push_back(source);
        return 0;
    }

    if (delim_length == 1) {
        size_t index = source.find_first_of(delim, last);
        while (index != std::string::npos) {
            vector_spliter->push_back(source.substr(last, 
                        index - last));
            last = index + delim_length;
            index = source.find_first_of(delim, last);
        }
    } else {
        size_t index = source.find(delim, last);
        while (index != std::string::npos) {
            vector_spliter->push_back(source.substr(last, 
                        index - last));
            last = index + delim_length;
            index = source.find(delim, last);
        }
    }

    if (last < total_length) {
        vector_spliter->push_back(source.substr(last, 
                    total_length - last));
    }
    return 0;
}

} // sdk_cpp
} // paddle_serving
} // baidu

#endif  //BAIDU_PADDLE_SERVING_SDK_CPP_UTILS_H

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
