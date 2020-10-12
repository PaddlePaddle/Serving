// Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <pybind11/buffer_info.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "ExtractFrameBGRARaw.h"
#include "ExtractFrameJpeg.h"
simplelogger::Logger* logger =
    simplelogger::LoggerFactory::CreateConsoleLogger();

PYBIND11_MODULE(hwextract, m) {
  pybind11::class_<baidu::xvision::ExtractFrameJpeg>(m, "HwExtractFrameJpeg")
      .def(pybind11::init<int>())
      .def("init_handler", &baidu::xvision::ExtractFrameJpeg::init)
      .def("extract_frame", &baidu::xvision::ExtractFrameJpeg::extract_frame);
  pybind11::class_<baidu::xvision::ExtractFrameBGRARaw>(m,
                                                        "HwExtractFrameBGRARaw")
      .def(pybind11::init<int>())
      .def("init_handler", &baidu::xvision::ExtractFrameBGRARaw::init)
      .def("extract_frame",
           &baidu::xvision::ExtractFrameBGRARaw::extract_frame);
  pybind11::class_<baidu::xvision::FrameResult>(
      m, "HwFrameResult", pybind11::buffer_protocol())
      .def(pybind11::init())
      .def("height", &baidu::xvision::FrameResult::height)
      .def("width", &baidu::xvision::FrameResult::width)
      .def("len", &baidu::xvision::FrameResult::len)
      .def("free_memory",
           &baidu::xvision::FrameResult::free_memory)  // for gcc 4.8.2 , this
                                                       // must be called ,both
                                                       // in cpp or python
      .def_buffer([](baidu::xvision::FrameResult& m) -> pybind11::buffer_info {
        return pybind11::buffer_info(
            m.get_frame(),
            sizeof(uint8_t),
            pybind11::format_descriptor<uint8_t>::format(),
            2,
            {m.rows(), m.cols()},
            {sizeof(uint8_t) * m.cols(), sizeof(uint8_t)});
      });
}
