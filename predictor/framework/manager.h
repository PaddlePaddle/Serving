#ifndef BAIDU_PADDLE_SERVING_PREDICTOR_MANAGER_H
#define BAIDU_PADDLE_SERVING_PREDICTOR_MANAGER_H

#include "common/inner_common.h"
#include "framework/workflow.h"
#include "common/constant.h"
#include "framework/service.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

using configure::WorkflowConf;
using configure::InferServiceConf;

class Workflow;
//class InferService;
//class ParallelInferService;

template<typename I>
I* create_item_impl() {
    return new (std::nothrow) I();
}

template<>
inline InferService* create_item_impl<InferService>() {
    if (FLAGS_use_parallel_infer_service) {
        return new (std::nothrow) ParallelInferService();
    } else {
        return new (std::nothrow) InferService();
    } 
}

class WorkflowManager {
public:
    static WorkflowManager& instance() {
        static WorkflowManager mgr; 
        return mgr;
    }

    int initialize(const std::string path, const std::string file) {
        WorkflowConf workflow_conf;
        if (configure::read_proto_conf(path, file, &workflow_conf) != 0) {
            LOG(FATAL) << "Failed load manager<" << Workflow::tag() << "> configure from " << path << "/" << file;
            return -1;
        }

        try {

        uint32_t item_size = workflow_conf.workflows_size();
        for (uint32_t ii = 0; ii < item_size; ii++) {
            std::string name = workflow_conf.workflows(ii).name();
            Workflow* item = new (std::nothrow) Workflow();
            if (item == NULL) {
                LOG(FATAL) << "Failed create " << Workflow::tag() << " for: " << name;
                return -1;
            }
            if (item->init(workflow_conf.workflows(ii)) != 0) {
                LOG(FATAL) 
                    << "Failed init item: " << name << " at:"
                    << ii << "!";
                return -1;
            }

            std::pair<
                typename boost::unordered_map<std::string, Workflow*>::iterator, bool>
                r = _item_map.insert(std::make_pair(name, item));
            if (!r.second) {
                LOG(FATAL) 
                    << "Failed insert item:" << name << " at:"
                    << ii << "!";
                return -1;
            }

            LOG(INFO) 
                << "Succ init item:" << name << " from conf:"
                << path << "/" << file << ", at:" << ii << "!";
        } 
        
        } catch (...) {
            LOG(FATAL) 
                << "Config[" << path << "/" << file << "] format "
                << "invalid, load failed";
            return -1;
        }
        return 0;
    }

    Workflow* create_item() {
        return create_item_impl<Workflow>();
    }

    Workflow* item(const std::string& name) {
        typename boost::unordered_map<std::string, Workflow*>::iterator it;
        it = _item_map.find(name);
        if (it == _item_map.end()) {
            LOG(WARNING) << "Not found item: " << name << "!";
            return NULL;
        }

        return it->second;
    }

    Workflow& operator[](const std::string& name) {
        Workflow* i = item(name);
        if (i == NULL) {
            std::string err = "Not found item in manager for:";
            err += name;
            throw std::overflow_error(err);
        }
        return *i;
    }

    int reload() {
        int ret = 0;
        typename boost::unordered_map<std::string, Workflow*>::iterator it
            = _item_map.begin();
        for (; it != _item_map.end(); ++it) {
            if (it->second->reload() != 0) {
                LOG(WARNING) << "failed reload item: " << it->first << "!";
                ret = -1;
            }
        }

        LOG(INFO) << "Finish reload " 
            << _item_map.size() 
            << " " << Workflow::tag() << "(s)";
        return ret;
    }

    int finalize() {
        return 0;
    }

private:
    WorkflowManager() {}

private:
    boost::unordered_map<std::string, Workflow*> _item_map;
};

class InferServiceManager {
public:
    static InferServiceManager& instance() {
        static InferServiceManager mgr; 
        return mgr;
    }

    int initialize(const std::string path, const std::string file) {
        InferServiceConf infer_service_conf;
        if (configure::read_proto_conf(path, file, &infer_service_conf) != 0) {
            LOG(FATAL) << "Failed load manager<" << InferService::tag() << "> configure!";
            return -1;
        }

        try {

        uint32_t item_size = infer_service_conf.services_size();
        for (uint32_t ii = 0; ii < item_size; ii++) {
            std::string name = infer_service_conf.services(ii).name();
            InferService* item = new (std::nothrow) InferService();
            if (item == NULL) {
                LOG(FATAL) << "Failed create " << InferService::tag() << " for: " << name;
                return -1;
            }
            if (item->init(infer_service_conf.services(ii)) != 0) {
                LOG(FATAL) 
                    << "Failed init item: " << name << " at:"
                    << ii << "!";
                return -1;
            }

            std::pair<
                typename boost::unordered_map<std::string, InferService*>::iterator, bool>
                r = _item_map.insert(std::make_pair(name, item));
            if (!r.second) {
                LOG(FATAL) 
                    << "Failed insert item:" << name << " at:"
                    << ii << "!";
                return -1;
            }

            LOG(INFO) 
                << "Succ init item:" << name << " from conf:"
                << path << "/" << file << ", at:" << ii << "!";
        } 
        
        } catch (...) {
            LOG(FATAL) 
                << "Config[" << path << "/" << file << "] format "
                << "invalid, load failed";
            return -1;
        }
        return 0;
    }

    InferService* create_item() {
        return create_item_impl<InferService>();
    }

    InferService* item(const std::string& name) {
        typename boost::unordered_map<std::string, InferService*>::iterator it;
        it = _item_map.find(name);
        if (it == _item_map.end()) {
            LOG(WARNING) << "Not found item: " << name << "!";
            return NULL;
        }

        return it->second;
    }

    InferService& operator[](const std::string& name) {
        InferService* i = item(name);
        if (i == NULL) {
            std::string err = "Not found item in manager for:";
            err += name;
            throw std::overflow_error(err);
        }
        return *i;
    }

    int reload() {
        int ret = 0;
        typename boost::unordered_map<std::string, InferService*>::iterator it
            = _item_map.begin();
        for (; it != _item_map.end(); ++it) {
            if (it->second->reload() != 0) {
                LOG(WARNING) << "failed reload item: " << it->first << "!";
                ret = -1;
            }
        }

        LOG(INFO) << "Finish reload " 
            << _item_map.size() 
            << " " << InferService::tag() << "(s)";
        return ret;
    }

    int finalize() {
        return 0;
    }

private:
    InferServiceManager() {}

private:
    boost::unordered_map<std::string, InferService*> _item_map;
};

} // predictor
} // paddle_serving
} // baidu

#endif
