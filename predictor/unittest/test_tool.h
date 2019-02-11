#ifndef BAIDU_PADDLE_SERVING_PREDICTOR_TEST_TOOL_H
#define BAIDU_PADDLE_SERVING_PREDICTOR_TEST_TOOL_H

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include<unistd.h>
#include <sys/time.h>
#include <gtest/gtest.h>

namespace baidu {
namespace paddle_serving {
namespace unittest {

class AutoTempFile {
public:
    AutoTempFile(const char* content) {
        _need_del = false;
        _name = generate_temp_name();
        FILE* fd = fopen(_name.c_str(), "w");
        if (!fd) {
            return ;
        }
        fprintf(fd, "%s", content);
        fclose(fd);
        _need_del = true;
    }

    ~AutoTempFile() {
        if (_need_del) {
            remove(_name.c_str());
        }
    }

    const char* name() {
        return _name.c_str();
    }

private:
    std::string generate_temp_name() {
        timeval tv;
        srand(time(0));
        gettimeofday(&tv, NULL);
        std::ostringstream oss;
        oss << "uttest_temp_";
        oss << tv.tv_sec * 1000 + tv.tv_usec / 1000;
        oss << "_";
        oss << (int)getpid();
        oss << "_";
        oss << rand();
        oss << ".conf";
        return oss.str();
    }

private:
    std::string _name;
    bool _need_del;
};

}
}
}

#endif
