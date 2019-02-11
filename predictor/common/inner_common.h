#ifndef BAIDU_PADDLE_SERVING_PREDICTOR_INNER_COMMON_H
#define BAIDU_PADDLE_SERVING_PREDICTOR_INNER_COMMON_H

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <strings.h>
#include <getopt.h>

#include <google/protobuf/text_format.h>
#include <boost/unordered_map.hpp>
#include <boost/function.hpp>
#include <boost/algorithm/string.hpp> // for boost::split&trim

#include <baas-lib-c/baas.h>
#include <baas-lib-c/giano_mock_helper.h>
#include <gflags/gflags.h>

#include <base/logging.h>
#include <base/time.h>
#include <base/object_pool.h>
#include <baidu/rpc/channel.h>
#include <baidu/rpc/server.h>
#include <baidu/rpc/policy/giano_authenticator.h>
#include <bthread.h>
#include <error.h>

#include "Configure.h"
#include <comlog/comlog.h>

#include "common/utils.h"
#include "common/types.h"
#include "common/constant.h"

#endif
