#ifndef BAIDU_PADDLE_SERVING_PREDICTOR_INNER_COMMON_H
#define BAIDU_PADDLE_SERVING_PREDICTOR_INNER_COMMON_H

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <strings.h>
#include <getopt.h>

#include <typeinfo>

#include <google/protobuf/text_format.h>
#include <boost/unordered_map.hpp>
#include <boost/function.hpp>
#include <boost/algorithm/string.hpp> // for boost::split&trim

#include <gflags/gflags.h>

#include <butil/logging.h>
#include <butil/time.h>
#include <butil/object_pool.h>
#include <brpc/channel.h>
#include <brpc/server.h>
#include <brpc/policy/giano_authenticator.h>
#include <bthread/bthread.h>
#include <error.h>

#include "configure.pb.h"
#include "configure_parser.h"

#include "common/utils.h"
#include "common/types.h"
#include "common/constant.h"

#endif
