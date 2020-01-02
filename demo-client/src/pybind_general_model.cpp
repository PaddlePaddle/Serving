#include <pybind11/pybind11.h>
#include "general_model.h"

#include <pybind11/stl.h>

namespace py = pybind11;

namespace baidu {
namespace paddle_serving {
namespace general_model {

PYBIND11_MODULE(paddle_serving_client, m) {
  m.doc() = R"pddoc(this is a practice
       )pddoc";
  py::class_<PredictorClient>(m, "PredictorClient", py::buffer_protocol())
      .def(py::init())
      .def("init",
           [](PredictorClient &self, const std::string & conf) {
             self.init(conf);
           })
      .def("connect",
           [](PredictorClient &self, const std::vector<std::string> & ep_list) {
             self.connect(ep_list);
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
