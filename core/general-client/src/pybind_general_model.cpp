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
             // see more: https://github.com/pybind/pybind11/issues/1042
             std::vector<int64_t> *ptr = new std::vector<int64_t>(
                 std::move(self.get_int64_by_name_with_rv(model_idx, name)));
             auto capsule = py::capsule(ptr, [](void *p) {
               delete reinterpret_cast<std::vector<int64_t> *>(p);
             });
             return py::array(ptr->size(), ptr->data(), capsule);
           })
      .def("get_float_by_name",
           [](PredictorRes &self, int model_idx, std::string &name) {
             std::vector<float> *ptr = new std::vector<float>(
                 std::move(self.get_float_by_name_with_rv(model_idx, name)));
             auto capsule = py::capsule(ptr, [](void *p) {
               delete reinterpret_cast<std::vector<float> *>(p);
             });
             return py::array(ptr->size(), ptr->data(), capsule);
           })
      .def("get_int32_by_name",
           [](PredictorRes &self, int model_idx, std::string &name) {
             std::vector<int32_t> *ptr = new std::vector<int32_t>(
                 std::move(self.get_int32_by_name_with_rv(model_idx, name)));
             auto capsule = py::capsule(ptr, [](void *p) {
               delete reinterpret_cast<std::vector<int32_t> *>(p);
             });
             return py::array(ptr->size(), ptr->data(), capsule);
           })
      .def("get_string_by_name",
           [](PredictorRes &self, int model_idx, std::string &name) {
             return self.get_string_by_name_with_rv(model_idx, name);
           })
      .def("get_shape",
           [](PredictorRes &self, int model_idx, std::string &name) {
             std::vector<int> *ptr = new std::vector<int>(
                 std::move(self.get_shape_by_name_with_rv(model_idx, name)));
             auto capsule = py::capsule(ptr, [](void *p) {
               delete reinterpret_cast<std::vector<int> *>(p);
             });
             return py::array(ptr->size(), ptr->data(), capsule);
           })
      .def("get_lod",
           [](PredictorRes &self, int model_idx, std::string &name) {
             std::vector<int> *ptr = new std::vector<int>(
                 std::move(self.get_lod_by_name_with_rv(model_idx, name)));
             auto capsule = py::capsule(ptr, [](void *p) {
               delete reinterpret_cast<std::vector<int> *>(p);
             });
             return py::array(ptr->size(), ptr->data(), capsule);
           })
      .def("variant_tag", [](PredictorRes &self) { return self.variant_tag(); })
      .def("get_engine_names",
           [](PredictorRes &self) { return self.get_engine_names(); })
      .def("get_tensor_alias_names", [](PredictorRes &self, int model_idx) {
        return self.get_tensor_alias_names(model_idx);
      });

  py::class_<PredictorClient>(m, "PredictorClient", py::buffer_protocol())
      .def(py::init())
      .def("init_gflags",
           [](PredictorClient &self, std::vector<std::string> argv) {
             self.init_gflags(argv);
           })
      .def("init",
           [](PredictorClient &self, const std::vector<std::string> &conf) {
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
      .def("numpy_predict",
           [](PredictorClient &self,
              const std::vector<py::array_t<float>> &float_feed,
              const std::vector<std::string> &float_feed_name,
              const std::vector<std::vector<int>> &float_shape,
              const std::vector<std::vector<int>> &float_lod_slot_batch,
              const std::vector<py::array_t<int32_t>> &int32_feed,
              const std::vector<std::string> &int32_feed_name,
              const std::vector<std::vector<int>> &int32_shape,
              const std::vector<std::vector<int>> &int32_lod_slot_batch,
              const std::vector<py::array_t<int64_t>> &int64_feed,
              const std::vector<std::string> &int64_feed_name,
              const std::vector<std::vector<int>> &int64_shape,
              const std::vector<std::vector<int>> &int64_lod_slot_batch,
              const std::vector<std::string> &string_feed,
              const std::vector<std::string> &string_feed_name,
              const std::vector<std::vector<int>> &string_shape,
              const std::vector<std::vector<int>> &string_lod_slot_batch,
              const std::vector<std::string> &fetch_name,
              PredictorRes &predict_res_batch,
              const int &pid,
              const uint64_t log_id) {
             return self.numpy_predict(float_feed,
                                       float_feed_name,
                                       float_shape,
                                       float_lod_slot_batch,
                                       int32_feed,
                                       int32_feed_name,
                                       int32_shape,
                                       int32_lod_slot_batch,
                                       int64_feed,
                                       int64_feed_name,
                                       int64_shape,
                                       int64_lod_slot_batch,
                                       string_feed,
                                       string_feed_name,
                                       string_shape,
                                       string_lod_slot_batch,
                                       fetch_name,
                                       predict_res_batch,
                                       pid,
                                       log_id);
           },
           py::call_guard<py::gil_scoped_release>());
}

}  // namespace general_model
}  // namespace paddle_serving
}  // namespace baidu
