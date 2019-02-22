#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include "butil/logging.h"

#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

namespace baidu {
namespace paddle_serving {
namespace configure {

int read_proto_conf(const std::string &conf_path,
                    const std::string &conf_file,
                    google::protobuf::Message *conf)
{
    std::string file_str = conf_path + conf_file;
    int fd = open(file_str.c_str(), O_RDONLY);
    if (fd == -1) {
        LOG(WARNING) << "File not found: " << file_str.c_str();
        return -1;
    }

    google::protobuf::io::FileInputStream input(fd);
    bool success = google::protobuf::TextFormat::Parse(&input, conf);
    close(fd);
    if (!success) {
        return -1;
    }

    return 0;
}

int write_proto_conf(google::protobuf::Message *message,
                    const std::string &output_path,
                    const std::string &output_file)
{
    std::string binary_str;
    google::protobuf::TextFormat::PrintToString(*message, &binary_str);

    std::string file_str = output_path + output_file;
    std::ofstream fout_bin((file_str.c_str()));
    if (!fout_bin) {
        LOG(WARNING) << "Open file error: " << file_str.c_str();
        return -1;
    }

    fout_bin.write((char *)binary_str.c_str(), binary_str.size());
    fout_bin.close();

    return 0;
}

} // configure
} // paddle_serving
} // baidu
/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
