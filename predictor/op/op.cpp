#include "op/op.h"
#include <base/time.h> // base::Timer
#include "common/utils.h"
#include "common/constant.h"
#include "framework/channel.h"
#include "framework/dag.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

int Op::init(Bus* bus, Dag* dag, uint32_t id, const std::string& name,
        const std::string& type, void* conf) {
    _bus = bus;
    _dag = dag;
    _id = id;
    _name = name;
    _type = type;
    set_config(conf);

    _timer = base::get_object<TimerFlow>();
    if (!_timer) {
        LOG(FATAL) << "Invalid timerflow in op:"
            << this->name();
        return -1;
    }

    _timer->init();
    _has_calc = false;
    _has_init = true;

    Channel* channel = mutable_channel();
    if (channel == NULL) {
        LOG(FATAL) 
            << "Failed mutable channel in op: " 
            << this->id() << ", " << this->name() << "!";
        return -1;
    }

    return custom_init();
}

int Op::deinit() {
    if (_timer) {
        base::return_object(_timer);
    }

    _bus = NULL;
    _dag = NULL;
    _timer = NULL;

    if (release_channel() != 0) {
        LOG(FATAL) << "Failed release channel in op:"  
            << this->id() << ", " << this->name() << "!";
        return -1;
    }

    return custom_deinit();
}

int Op::check_time(const char* tag) {
    if (!_timer) {
        LOG(FATAL) << "Invalid timer in op";
        return -1;
    }

    if (!_timer->check(tag)) {
        LOG(FATAL) << "Failed check timer:" << tag;
        return -1;
    }

    return 0;
}

int Op::process(bool debug) {
    base::Timer op_time(base::Timer::STARTED);
    if (debug && _timer) {
        _timer->start();
    }
    if (!_has_init) {
        LOG(FATAL) 
            << "Make sure op has been init before inference";
        return ERR_INTERNAL_FAILURE;
    }

    if (_has_calc) {
        LOG(DEBUG)    
            << "Op: " << _name << " already processed before";
        return ERR_OK;
    }

    // 1. dependency inference
    /*
    DagNode* node = _dag->node_by_name(this->name());
    if (node == NULL) {
        LOG(FATAL) << "Failed get node of op:" << this->name();
        return -1;
    }
    boost::unordered_map<std::string, EdgeMode>& depends =
        node->depends;
    boost::unordered_map<std::string, EdgeMode>::iterator it;
    for (it = depends.begin(); it != depends.end(); it++) {
        Op* depend_op = view->find(it->first);
        if (depend_op->process() != 0) {
            LOG(WARNING) << "Op: " << _name << " processed failed!";
            return -1;
        }
    }*/
    if (debug && _timer) {
        _timer->check("depend");
    }

    // 2. current inference 
    if (inference() != 0) {
        return ERR_OP_INFER_FAILURE;
    }
    if (debug && _timer) {
        _timer->check("infer");
    }

    // 3. share output to bus
    Channel* channel = mutable_channel();
    channel->share_to_bus(_bus);

    // 4. mark has calculated
    _has_calc = true;

    if (debug && _timer) {
        _timer->check("share");
        _timer->end();
    }

    op_time.stop();
    PredictorMetric::GetInstance()->update_latency_metric(
            OP_METRIC_PREFIX + full_name(), op_time.u_elapsed());
    LOG(NOTICE) << " " << name() << "_time=[" << op_time.u_elapsed() << "]" << noflush;
    return ERR_OK;
}

std::string Op::time_info() {
    if (_timer) {
        return _timer->info();
    } else {
        return "Invalid Timer!";
    }
}

bool Op::is_mutable(const std::string& op) {
    if (op == START_OP_NAME) {
        return false;
    }
    DagNode* node = const_cast<DagNode*>(_dag->node_by_name(_name));
    if (node->depends.find(op) == node->depends.end()) {
        LOG(WARNING) 
            << "op: " << _name << " doesnot depend on"
            << "op: " << op << "!";
        return false;
    }
    
    if (node->depends[op] != RW) {
        LOG(WARNING) 
            << "op: " << _name << " has no RW access"
            << "ot op: " << op << ", mode: " << node->depends[op]
            << ", please use get_argment() instead.";
        return false;
    }

    return true;
}

bool Op::is_mutable(const std::string& op) const {
    if (op == START_OP_NAME) {
        return false;
    }
    DagNode* node = const_cast<DagNode*>(
            _dag->node_by_name(_name));
    if (node->depends.find(op) == node->depends.end()) {
        LOG(WARNING) 
            << "op: " << _name << " doesnot depend on"
            << "op: " << op << "!";
        return false;
    }
    
    if (node->depends[op] != RW) {
        LOG(WARNING) 
            << "op: " << _name << " has no RW access"
            << "ot op: " << op << ", mode: " << node->depends[op]
            << ", please use get_argment() instead.";
        return false;
    }

    return true;
}

bool Op::is_readable(const std::string& op) {
    if (op == START_OP_NAME) {
        return true;
    }
    DagNode* node = const_cast<DagNode*>(_dag->node_by_name(_name));
    if (node->depends.find(op) == node->depends.end()) {
        LOG(WARNING) 
            << "op: " << _name << " doesnot depend on"
            << "op: " << op << "!";
        return false;
    }
    
    if (node->depends[op] != RW && node->depends[op] != RO) {
        LOG(WARNING) 
            << "op: " << _name << " has no RO access"
            << "ot op: " << op << ", mode: " << node->depends[op]
            << ", please check your configuration.";
        return false;
    }

    return true;
}

bool Op::is_readable(const std::string& op) const {
    if (op == START_OP_NAME) {
        return true;
    }
    DagNode* node = const_cast<DagNode*>(_dag->node_by_name(_name));
    if (node->depends.find(op) == node->depends.end()) {
        LOG(WARNING) 
            << "op: " << _name << " doesnot depend on "
            << "op: " << op << "!";
        return false;
    }
    
    if (node->depends[op] != RW && node->depends[op] != RO) {
        LOG(WARNING) 
            << "op: " << _name << " has no RO access"
            << "ot op: " << op << ", mode: " << node->depends[op]
            << ", please check your configuration.";
        return false;
    }

    return true;
}

// 获得依赖Op的Channel对象
Channel* Op::mutable_depend_channel(const std::string& op) {
    if (!is_mutable(op)) {
        LOG(WARNING) 
            << "Op: " << _name << " cannot mutable op: "
            << op << "!";
        return NULL;
    }
    
    // 从bus中获取依赖op的channel
    return _bus->channel_by_name(op);
}

// 获得依赖Op的Channel对象
const Channel* Op::get_depend_channel(const std::string& op) const {
    // 从dag中获取依赖op的mode属性
    if (!is_readable(op)) {
        LOG(WARNING) 
            << "op: " << _name << " doesnot depend on op: "
            << op << "!";
        return NULL;
    }

    // 从bus中获取依赖op的channel
    return _bus->channel_by_name(op); 
}

google::protobuf::Message* Op::mutable_message() {
    return mutable_channel()->message();
}

const google::protobuf::Message* Op::get_message() const {
    return get_channel()->message();
}

bool Op::has_calc() { return _has_calc; }

const char* Op::name() const {
    return _name.c_str();
}

const std::string& Op::type() const {
    return _type;
}

uint32_t Op::id() const {
    return _id;
}

const std::string Op::debug_string() {
    const Channel* channel = get_channel();
    if (!channel) {
        LOG(FATAL) << "Invalid channel!";
        return "Invalid channel in OP";
    }
    return channel->debug_string();
}

const google::protobuf::Message* Op::get_request_message() {
    return _bus->channel_by_name(START_OP_NAME)->message();
}

} // predictor
} // paddle_serving
} // baidu
