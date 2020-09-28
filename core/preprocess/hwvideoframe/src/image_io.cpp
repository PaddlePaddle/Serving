/*******************************************
 *
 * Copyright (c) 2020 Baidu.com, Inc. All Rights Reserved
 *
 ******************************************/
/**
 * @file image_io.cpp
 * @author yinyijun@baidu.com
 * @date 2020-06-08
 **/
#include "image_io.h"

#include <stdexcept>

#include <cuda_runtime.h>
#include <pybind11/numpy.h>
#include "utils.h"

std::shared_ptr<OpContext> Image2Gpubuffer::operator()(pybind11::array_t<float> input) {
    pybind11::buffer_info buf = input.request();
    if (buf.format != pybind11::format_descriptor<float>::format()) {
        throw std::runtime_error("Incompatible format: expected a float numpy!");
    }
    if (buf.ndim != 3) {
        throw std::runtime_error("Number of dimensions must be three");
    }
    if (buf.shape[2] != CHANNEL_SIZE) {
        throw std::runtime_error("Number of channels must be three");
    }
    auto result = std::make_shared<OpContext>(buf.shape[0], buf.shape[1]);
    auto ret = cudaMemcpy(result->p_frame(), static_cast<float*>(buf.ptr), result->size(), cudaMemcpyHostToDevice);
    verify_cuda_ret("cudaMemcpy", ret);
    return result;
}

pybind11::array_t<float> Gpubuffer2Image::operator()(std::shared_ptr<OpContext> input) {
    auto result = pybind11::array_t<float>({ input->nppi_size().height, input->nppi_size().width, (int)CHANNEL_SIZE });
    pybind11::buffer_info buf = result.request();
    auto ret = cudaMemcpy(static_cast<float*>(buf.ptr), input->p_frame(), input->size(), cudaMemcpyDeviceToHost);
    verify_cuda_ret("cudaMemcpy", ret);
    return result;
}
