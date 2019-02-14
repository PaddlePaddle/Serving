#ifndef BAIDU_PADDLE_SERVING_PREDICTOR_LOGGER_H
#define BAIDU_PADDLE_SERVING_PREDICTOR_LOGGER_H

#include <butil/comlog_sink.h>

namespace baidu {
namespace paddle_serving {
namespace predictor {

class LoggerWrapper {
public:
    static LoggerWrapper& instance() {
        static LoggerWrapper lw;
        return lw;
    }

    int initialize(const std::string& path, const std::string& name) {
        if (com_loadlog(path.c_str(), name.c_str()) != 0) {
            LOG(ERROR) << "Fail to com_loadlog from: "
                << path << "/" << name;
            return -1;
        }
        return 0;
    }

    int finalize() {
        return 0;
    }
};

}
}
}

#endif
