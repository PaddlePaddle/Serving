#pragma once
#include <google/protobuf/message.h>

namespace baidu {
namespace paddle_serving {
namespace configure {
int read_proto_conf(const std::string &conf_path,
                   const std::string &conf_file,
                   google::protobuf::Message *conf);

int write_proto_conf(google::protobuf::Message *message,
                    const std::string &output_path,
                    const std::string &output_file);

} // configure
} // paddle_serving
} // baidu
