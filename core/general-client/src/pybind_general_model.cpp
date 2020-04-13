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

#include <Python.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <unordered_map>
#include "core/general-client/include/general_model.h"

namespace py = pybind11;

namespace baidu {
namespace paddle_serving {
namespace general_model {

PYBIND11_MODULE(serving_client, m) {
  m.doc() = R"pddoc(this is a practice
       )pddoc";

  py::class_<PredictorRes>(m, "PredictorRes", py::buffer_protocol())
      .def(py::init())
      .def("get_int64_by_name",
           [](PredictorRes &self, int model_idx, std::string &name) {
             return self.get_int64_by_name(model_idx, name);
           },
           py::return_value_policy::reference)
      .def("get_float_by_name",
           [](PredictorRes &self, int model_idx, std::string &name) {
             return self.get_float_by_name(model_idx, name);
           },
           py::return_value_policy::reference)
      .def("variant_tag",
           [](PredictorRes &self) { return self.variant_tag(); })
      .def("models_num",
           [](PredictorRes &self) {return self.models_num(); });

  py::class_<PredictorClient>(m, "PredictorClient", py::buffer_protocol())
      .def(py::init())
      .def("init_gflags",
           [](PredictorClient &self, std::vector<std::string> argv) {
             self.init_gflags(argv);
           })
      .def("init",
           [](PredictorClient &self, const std::string &conf) {
             return self.init(conf);
           })
      .def("set_predictor_conf",
           [](PredictorClient &self,
              const std::string &conf_path,
              const std::string &conf_file) {
             self.set_predictor_conf(conf_path, conf_file);
           })
      .def("create_predictor_by_desc",
           [](PredictorClient &self, const std::string &sdk_desc) {
             self.create_predictor_by_desc(sdk_desc);
           })
      .def("create_predictor",
           [](PredictorClient &self) { self.create_predictor(); })
      .def("destroy_predictor",
           [](PredictorClient &self) { self.destroy_predictor(); })
      .def("predict",
           [](PredictorClient &self,
              const std::vector<std::vector<float>> &float_feed,
              const std::vector<std::string> &float_feed_name,
              const std::vector<std::vector<int64_t>> &int_feed,
              const std::vector<std::string> &int_feed_name,
              const std::vector<std::string> &fetch_name,
              PredictorRes &predict_res,
              const int &pid) {
             return self.predict(float_feed,
                                 float_feed_name,
                                 int_feed,
                                 int_feed_name,
                                 fetch_name,
                                 predict_res,
                                 pid);
           })
      .def("batch_predict",
           [](PredictorClient &self,
              const std::vector<std::vector<std::vector<float>>>
                  &float_feed_batch,
              const std::vector<std::string> &float_feed_name,
              const std::vector<std::vector<std::vector<int64_t>>>
                  &int_feed_batch,
              const std::vector<std::string> &int_feed_name,
              const std::vector<std::string> &fetch_name,
              PredictorRes &predict_res_batch,
              const int &pid) {
             return self.batch_predict(float_feed_batch,
                                       float_feed_name,
                                       int_feed_batch,
                                       int_feed_name,
                                       fetch_name,
                                       predict_res_batch,
                                       pid);
           });
}

}  // namespace general_model
}  // namespace paddle_serving
}  // namespace baidu
