#pragma once

#include "common/inner_common.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

class IMerger {
public:
    virtual bool merge(const google::protobuf::Message*, google::protobuf::Message*) = 0;
};

class DefaultMerger : public IMerger {
public:
    bool merge(
            const google::protobuf::Message* s, google::protobuf::Message* d) {
        if (!s || !d) {
            return false;
        } 

        d->MergeFrom(*s);
        return true;
    }
};

template<typename T>
class Singleton {
public:
    static T* instance() {
        static T ins;
        return &ins;
    }
};

class MergerManager {
public:
    typedef IMerger MergerT;

    static MergerManager& instance() {
        static MergerManager ins;
        return ins;
    }

    bool set(std::string name, MergerT* merger) {
        if (_mergers.find(name) != _mergers.end()) {
            LOG(ERROR) << "Duplicated merger: " << name;
            return false;
        }
        _mergers[name] = merger;
        return true;
    }

    bool get(const std::string& name, MergerT*& merger) {
        std::map<std::string, MergerT*>::iterator iter = 
            _mergers.find(name);
        if (iter == _mergers.end()) {
            return false;
        }
        merger = iter->second;
        return true;
    }

private:
    MergerManager() {
        set("default", Singleton<DefaultMerger>::instance());
    }
    
private:
    std::map<std::string, MergerT*> _mergers;
};

#define DECLARE_MERGER(M) \
    static bool regist_self() {\
        if (!baidu::paddle_serving::predictor::MergerManager::instance().set(\
                #M, baidu::paddle_serving::predictor::Singleton<M>::instance())) {\
            LOG(ERROR) << "Failed regist merger: " << #M;\
            return false;\
        }\
        LOG(INFO) << "Succ regist merger: " << #M;\
        return true;\
    }

#define PDS_STR_CAT(a, b) PDS_STR_CAT_I(a, b)
#define PDS_STR_CAT_I(a, b) a ## b

#define DEFINE_MERGER(M)\
__attribute__((constructor)) static void PDS_STR_CAT(GlobalRegistObject, __LINE__)(void)\
{\
    M::regist_self();\
}

} // predictor
} // paddle_serving
} // baidu
