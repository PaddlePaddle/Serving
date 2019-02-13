#ifndef BAIDU_PADDLE_SERVING_PREDICTOR_OP_REPOSITORY_H
#define BAIDU_PADDLE_SERVING_PREDICTOR_OP_REPOSITORY_H

#include "common/inner_common.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

#define REGISTER_OP(op)                                                 \
    ::baidu::paddle_serving::predictor::OpRepository::instance().regist_op<op>(#op)

class Op;

class Factory {
public:
    virtual Op* get_op() = 0;
    virtual void return_op(Op* op) = 0;
};

template<typename OP_TYPE>
class OpFactory : public Factory {
public:
    Op* get_op() {
        return butil::get_object<OP_TYPE>();
    }

    void return_op(Op* op) {
        butil::return_object<OP_TYPE>(dynamic_cast<OP_TYPE*>(op));
    }

    static OpFactory<OP_TYPE>& instance() {
        static OpFactory<OP_TYPE> ins; 
        return ins;
    }
};

class OpRepository {
public:
    typedef boost::unordered_map<std::string, Factory*> ManagerMap;

    OpRepository() {}
    ~OpRepository() {}

    static OpRepository& instance() {
        static OpRepository repo;
        return repo;
    }

    template<typename OP_TYPE>
    void regist_op(std::string op_type) {
        _repository[op_type] = &OpFactory<OP_TYPE>::instance();
        LOG(INFO) << "Succ regist op: " << op_type << "!";
    }

    Op* get_op(std::string op_type);

    void return_op(Op* op);

    void return_op(const std::string& op_type, Op* op);

private:
    ManagerMap _repository;
};

} // predictor
} // paddle_serving
} // baidu

#endif
