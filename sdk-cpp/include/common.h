/***************************************************************************
 * 
 * Copyright (c) 2018 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file common.h
 * @author wanlijin01(wanlijin01@baidu.com)
 * @date 2018/07/09 20:24:19
 * @brief 
 *  
 **/

#ifndef  BAIDU_PADDLE_SERVING_CPP_SDK_COMMON_H
#define  BAIDU_PADDLE_SERVING_CPP_SDK_COMMON_H

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <strings.h>
#include <getopt.h>
#include <exception>

#include <google/protobuf/message.h>
#include <boost/unordered_map.hpp>
#include <gflags/gflags.h>

#include <bvar/bvar.h>
#include <butil/logging.h>
#include <butil/time.h>
#include <butil/object_pool.h>
#include <brpc/channel.h>
#include <brpc/parallel_channel.h>
#include <brpc/traceprintf.h>
#include <bthread/bthread.h>
#include <error.h>
#include <json2pb/json_to_pb.h>

#include "Configure.h"

#include "utils.h"

#endif  //BAIDU_PADDLE_SERVING_CPP_SDK_COMMON_H

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
