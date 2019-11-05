/* Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */
#include <Python.h>
#include <algorithm>
#include <cstdlib>
#include <map>
#include <memory>
#include <mutex>  // NOLINT // for call_once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include "elastic-ctr/client/api/elastic_ctr_api.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"

namespace baidu {
namespace paddle_serving {
namespace elastic_ctr {

namespace py = pybind11;
using baidu::paddle_serving::predictor::elastic_ctr::ReqInstance;

PYBIND11_MODULE(elasticctr, m) {
  m.doc() = "C++ core of ElasticCTRPredictorApi";

  py::class_<ReqInstance>(m, "ReqInstance", py::buffer_protocol())
      .def(py::init<>());

  py::class_<ElasticCTRPredictorApi>(
      m, "ElasticCTRPredictorApi", py::buffer_protocol())
      .def(py::init<>())
      .def("init", &ElasticCTRPredictorApi::init)
      .def("thrd_initialize", &ElasticCTRPredictorApi::thrd_initialize)
      .def("thrd_clear", &ElasticCTRPredictorApi::thrd_clear)
      .def("thrd_finalize", &ElasticCTRPredictorApi::thrd_finalize)
      .def("destroy", &ElasticCTRPredictorApi::destroy)
      .def("add_instance", &ElasticCTRPredictorApi::add_instance)
      .def("add_slot", &ElasticCTRPredictorApi::add_slot)
      .def("inference",
           [](ElasticCTRPredictorApi &self) -> std::vector<std::vector<float>> {
             std::vector<std::vector<float>> results_vec;
             self.inference(results_vec);
             return results_vec;
           });
}
}  // namespace elastic_ctr
}  // namespace paddle_serving
}  // namespace baidu
