#include <Python.h>
#include <pybind11/pybind11.h>
#include <unordered_map>
#include "general_model.h"

#include <pybind11/stl.h>

namespace py = pybind11;

using baidu::paddle_serving::general_model::FetchedMap;

namespace baidu {
namespace paddle_serving {
namespace general_model {

PYBIND11_MODULE(serving_client, m) {
  m.doc() = R"pddoc(this is a practice
       )pddoc";
  py::class_<PredictorClient>(m, "PredictorClient", py::buffer_protocol())
      .def(py::init())
      .def("init",
           [](PredictorClient &self, const std::string & conf) {
             self.init(conf);
           })
      .def("set_predictor_conf",
           [](PredictorClient &self, const std::string & conf_path,
              const std::string & conf_file) {
             self.set_predictor_conf(conf_path, conf_file);
           })
      .def("create_predictor",
           [](PredictorClient & self) {
             self.create_predictor();
           })
      .def("predict",
           [](PredictorClient &self,
              const std::vector<std::vector<float> > & float_feed,
              const std::vector<std::string> & float_feed_name,
              const std::vector<std::vector<int64_t> > & int_feed,
              const std::vector<std::string> & int_feed_name,
              const std::vector<std::string> & fetch_name) {

             return self.predict(float_feed, float_feed_name,
                                 int_feed, int_feed_name, fetch_name);
           });
}

}  // namespace general_model
}  // namespace paddle_serving
}  // namespace baidu
