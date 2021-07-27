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
#include "general_model.h"  // NOLINT

#include <pybind11/stl.h>  // NOLINT

namespace py = pybind11;

using baidu::paddle_serving::general_model::FetchedMap;

namespace baidu {
namespace paddle_serving {
namespace general_model {

PYBIND11_MODULE(paddle_serving_client, m) {
  m.doc() = R"pddoc(this is a practice
       )pddoc";
  py::class_<PredictorClient>(m, "PredictorClient", py::buffer_protocol())
      .def(py::init())
      .def("init",
           [](PredictorClient &self, const std::string &conf) {
             self.init(conf);
           })
      .def("set_predictor_conf",
           [](PredictorClient &self,
              const std::string &conf_path,
              const std::string &conf_file) {
             self.set_predictor_conf(conf_path, conf_file);
           })
      .def("create_predictor",
           [](PredictorClient &self) { self.create_predictor(); })
      .def("predict",
           [](PredictorClient &self,
              const std::vector<std::vector<float>> &float_feed,
              const std::vector<std::string> &float_feed_name,
              const std::vector<std::vector<int64_t>> &int_feed,
              const std::vector<std::string> &int_feed_name,
              const std::vector<std::string> &fetch_name,
              FetchedMap *fetch_result) {
             return self.predict(float_feed,
                                 float_feed_name,
                                 int_feed,
                                 int_feed_name,
                                 fetch_name,
                                 fetch_result);
           });
}

}  // namespace general_model
}  // namespace paddle_serving
}  // namespace baidu
