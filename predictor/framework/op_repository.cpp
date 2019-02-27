#include "framework/op_repository.h"
#include "op/op.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

Op* OpRepository::get_op(std::string op_type) {
    ManagerMap::iterator iter = _repository.find(op_type);
    Op* op = NULL;
    if (iter != _repository.end()) {
        op = (iter->second)->get_op();
    } else {
        LOG(ERROR) << "Try to create unknown op[" << op_type << "]";
    }
    return op;
}

void OpRepository::return_op(Op* op) {
    if (op == NULL) {
        LOG(ERROR) << "Try to return NULL op";
        return;
    }
    ManagerMap::iterator iter = _repository.find(op->type());
    if (iter != _repository.end()) {
        iter->second->return_op(op);
    } else {
        LOG(ERROR) << "Try to return unknown op[" << op << "], op_type["
                << op->type() << "].";
    }
}

void OpRepository::return_op(const std::string& op_type, Op* op) {
    if (op == NULL) {
        LOG(ERROR) << "Try to return NULL op";
        return;
    }
    ManagerMap::iterator iter = _repository.find(op_type);
    if (iter != _repository.end()) {
        iter->second->return_op(op);
    } else {
        LOG(ERROR) << "Try to return unknown op[" << op << "], op_type["
                << op_type << "].";
    }
}

} // namespace predictor
} // namespace paddle_serving
} // namespace baidu
