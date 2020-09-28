
#include <sstream>
#include <stdexcept>

#include <npp.h>

#include "utils.h"

void verify_npp_ret(const std::string& function_name, NppStatus ret) {
    if (ret != NPP_SUCCESS) {
        std::ostringstream ss;
        ss << function_name << ", ret: " << ret;
        throw std::runtime_error(ss.str());
    }
}

void verify_cuda_ret(const std::string& function_name, cudaError_t ret) {
    if (ret != cudaSuccess) {
        std::ostringstream ss;
        ss << function_name << ", ret: " << ret;
        throw std::runtime_error(ss.str());
    }
}