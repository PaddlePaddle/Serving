#ifndef BAIDU_PADDLE_SERVING_PREDICTOR_WORKFLOW_H
#define BAIDU_PADDLE_SERVING_PREDICTOR_WORKFLOW_H

#include "common/inner_common.h"
#include "framework/dag.h"
#include "framework/dag_view.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

template<typename T>
class Manager;

class Workflow {
public:
    Workflow() {}

    static const char* tag() {
        return "Workflow";
    }

    // Each workflow object corresponds to an independent
    // configure file, so you can share the object between
    // different apps.
    int init(const comcfg::ConfigUnit& conf);
        
    DagView* fetch_dag_view(const std::string& service_name);

    int deinit() { return 0; }
        
    void return_dag_view(DagView* view);

    int reload();
    
    const std::string& name() {
        return _name;
    }
    
    const std::string& full_name() {
        return _name;
    }

    void regist_metric(const std::string& service_name);

private:
    Dag _dag;
    std::string _type;
    std::string _name;
};

} // predictor
} // paddle_serving
} // baidu

#endif
