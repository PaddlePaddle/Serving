#include "common/inner_common.h"
#include "framework/workflow.h"
#include "framework/predictor_metric.h" // PredictorMetric

namespace baidu {
namespace paddle_serving {
namespace predictor {

int Workflow::init(const comcfg::ConfigUnit& conf) {
    const std::string& name = conf["name"].to_cstr();
    const std::string& path = conf["path"].to_cstr();
    const std::string& file = conf["file"].to_cstr();
    comcfg::Configure wf_conf;
    if (wf_conf.load(path.c_str(), file.c_str()) != 0) {
        LOG(ERROR) 
            << "Failed load workflow, conf:"
            << path << "/" << file << "!";
        return -1;
    }
    _type = wf_conf["workflow_type"].to_cstr();
    _name = name;
    if (_dag.init(wf_conf, name) != 0) {
        LOG(ERROR) << "Failed initialize dag: " << _name;
        return -1;
    }
    return 0;
}

DagView* Workflow::fetch_dag_view(const std::string& service_name) {
    DagView* view = NULL;
    if (_type == "Sequence") {
        view = butil::get_object<DagView>();
    } else if (_type == "Parallel") {
        view = butil::get_object<ParallelDagView>();
    } else {
        LOG(FATAL) 
            << "Unknown dag type:" << _type << "!";
        return NULL;
    }
    if (view == NULL) {
        LOG(FATAL) << "create dag view from pool failed!";
        return NULL;
    }
    view->init(&_dag, service_name);
    return view;
}

void Workflow::return_dag_view(DagView* view) {
    view->deinit();
    if (_type == "Sequence") {
        butil::return_object<DagView>(view);
    } else if (_type == "Parallel") {
        butil::return_object<ParallelDagView>(
            dynamic_cast<ParallelDagView*>(view));
    } else {
        LOG(FATAL) 
            << "Unknown dag type:" << _type << "!";
        return ;
    }
}

int Workflow::reload() {
    // reload op's config here...

    return 0;
}

void Workflow::regist_metric(const std::string& service_name) {
    PredictorMetric::GetInstance()->regist_latency_metric(
            WORKFLOW_METRIC_PREFIX + service_name + NAME_DELIMITER + full_name());
    _dag.regist_metric(service_name);
}

} // predictor
} // paddle_serving
} // baidu
