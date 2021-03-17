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

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "core/preprocess/hwvideoframe/include/center_crop.h"
#include "core/preprocess/hwvideoframe/include/div.h"
#include "core/preprocess/hwvideoframe/include/image_io.h"
#include "core/preprocess/hwvideoframe/include/normalize.h"
#include "core/preprocess/hwvideoframe/include/resize.h"
#include "core/preprocess/hwvideoframe/include/resize_by_factor.h"
#include "core/preprocess/hwvideoframe/include/rgb_swap.h"
#include "core/preprocess/hwvideoframe/include/sub.h"

PYBIND11_MODULE(libgpupreprocess, m) {
  pybind11::class_<OpContext, std::shared_ptr<OpContext>>(m, "OpContext");
  pybind11::class_<Image2Gpubuffer>(m, "Image2Gpubuffer")
      .def(pybind11::init<>())
      .def("__call__", &Image2Gpubuffer::operator());
  pybind11::class_<Gpubuffer2Image>(m, "Gpubuffer2Image")
      .def(pybind11::init<>())
      .def("__call__", &Gpubuffer2Image::operator());
  pybind11::class_<RGB2BGR>(m, "RGB2BGR")
      .def(pybind11::init<>())
      .def("__call__", &RGB2BGR::operator());
  pybind11::class_<BGR2RGB>(m, "BGR2RGB")
      .def(pybind11::init<>())
      .def("__call__", &BGR2RGB::operator());
  pybind11::class_<Div>(m, "Div")
      .def(pybind11::init<float>())
      .def("__call__", &Div::operator());
  pybind11::class_<Sub>(m, "Sub")
      .def(pybind11::init<float>())
      .def(pybind11::init<const std::vector<float>&>())
      .def("__call__", &Sub::operator());
  pybind11::class_<Normalize>(m, "Normalize")
      .def(pybind11::init<const std::vector<float>&,
                          const std::vector<float>&,
                          bool>(),
           pybind11::arg("mean"),
           pybind11::arg("std"),
           pybind11::arg("channel_first") = false)
      .def("__call__", &Normalize::operator());
  pybind11::class_<CenterCrop>(m, "CenterCrop")
      .def(pybind11::init<int>())
      .def("__call__", &CenterCrop::operator());
  pybind11::class_<Resize>(m, "Resize")
      .def(pybind11::init<int, int, bool>(),
           pybind11::arg("size"),
           pybind11::arg("max_size") = 214748364,
           pybind11::arg("use_fixed_point") = false)
      .def(pybind11::init<const std::vector<int>&, int, bool>(),
           pybind11::arg("target_size"),
           pybind11::arg("max_size") = 214748364,
           pybind11::arg("use_fixed_point") = false)
      .def("__call__", &Resize::operator());
  pybind11::class_<ResizeByFactor>(m, "ResizeByFactor")
      .def(pybind11::init<int, int, bool>(),
           pybind11::arg("factor") = 32,
           pybind11::arg("max_side_len") = 2400,
           pybind11::arg("use_fixed_point") = false)
      .def("__call__", &ResizeByFactor::operator());
}
