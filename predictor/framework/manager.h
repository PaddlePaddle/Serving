#ifndef BAIDU_PADDLE_SERVING_PREDICTOR_MANAGER_H
#define BAIDU_PADDLE_SERVING_PREDICTOR_MANAGER_H

#include "common/inner_common.h"
#include "framework/workflow.h"
#include "common/constant.h"
#include "framework/service.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

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

template<typename T>
class Manager {
public:
    static Manager<T>& instance() {
        static Manager<T> mgr; 
        return mgr;
    }

    int initialize(const std::string path, const std::string file) {
        comcfg::Configure conf; 
        if (conf.load(path.c_str(), file.c_str()) != 0) {
            LOG(FATAL) 
                << "Failed load manager<" << typeid(T).name()
                << "> configure!";
            return -1;
        }

        try {

        uint32_t item_size = conf[T::tag()].size();
        for (uint32_t ii = 0; ii < item_size; ii++) {
            std::string name = conf[T::tag()][ii]["name"].to_cstr();
            T* item = new (std::nothrow) T();
            if (item == NULL) {
                LOG(FATAL) << "Failed create " << T::tag() << " for: " << name;
                return -1;
            }
            if (item->init(conf[T::tag()][ii]) != 0) {
                LOG(FATAL) 
                    << "Failed init item: " << name << " at:"
                    << ii << "!";
                return -1;
            }

            std::pair<
                typename boost::unordered_map<std::string, T*>::iterator, bool>
                r = _item_map.insert(std::make_pair(name, item));
            if (!r.second) {
                LOG(FATAL) 
                    << "Failed insert item:" << name << " at:"
                    << ii << "!";
                return -1;
            }

            LOG(TRACE) 
                << "Succ init item:" << name << " from conf:"
                << path << "/" << file << ", at:" << ii << "!";
        } 
        
        } catch (comcfg::ConfigException e) {
            LOG(FATAL) 
                << "Config[" << path << "/" << file << "] format "
                << "invalid, err: " << e.what();
            return -1;
        } catch (...) {
            LOG(FATAL) 
                << "Config[" << path << "/" << file << "] format "
                << "invalid, load failed";
            return -1;
        }
        return 0;
    }

    T* create_item() {
        return create_item_impl<T>();
    }

    T* item(const std::string& name) {
        typename boost::unordered_map<std::string, T*>::iterator it;
        it = _item_map.find(name);
        if (it == _item_map.end()) {
            LOG(WARNING) << "Not found item: " << name << "!";
            return NULL;
        }

        return it->second;
    }

    T& operator[](const std::string& name) {
        T* i = item(name);
        if (i == NULL) {
            std::string err = "Not found item in manager for:";
            err += name;
            throw std::overflow_error(err);
        }
        return *i;
    }

    int reload() {
        int ret = 0;
        typename boost::unordered_map<std::string, T*>::iterator it
            = _item_map.begin();
        for (; it != _item_map.end(); ++it) {
            if (it->second->reload() != 0) {
                LOG(WARNING) << "failed reload item: " << it->first << "!";
                ret = -1;
            }
        }

        LOG(TRACE) << "Finish reload " 
            << _item_map.size() 
            << " " << T::tag() << "(s)";
        return ret;
    }

    int finalize() {
        return 0;
    }

private:
    Manager<T>() {}

private:
    boost::unordered_map<std::string, T*> _item_map;
};

typedef Manager<InferService> InferServiceManager;
typedef Manager<Workflow> WorkflowManager;

} // predictor
} // paddle_serving
} // baidu

#endif
