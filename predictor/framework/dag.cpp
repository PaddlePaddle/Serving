#include "common/inner_common.h"
#include "framework/dag.h"
#include "op/op.h"
#include "framework/predictor_metric.h" // PredictorMetric

namespace baidu {
namespace paddle_serving {
namespace predictor {

Dag::Dag() {
    _index_nodes.clear();
    _name_nodes.clear();
    _stages.clear();
}

Dag::~Dag() {
    deinit();
}

int Dag::deinit() {
    for (std::vector<DagStage*>::iterator iter = _stages.begin(); iter != _stages.end(); ++iter) {
        if (*iter != NULL) {
            delete *iter;
        }
    }
    _stages.clear();

    for (std::vector<DagNode*>::iterator iter = _index_nodes.begin();
            iter != _index_nodes.end();
            ++iter) {
        DagNode* node = *iter;
        if (node != NULL) {
            void* conf = node->conf;
            if (conf != NULL) {
                Op* op = OpRepository::instance().get_op(node->type);
                if (op == NULL) {
                    LOG(FATAL) << "Failed to get_op, op type[" << node->type << "]";
                    return -1;
                }
                op->delete_config(conf);
                OpRepository::instance().return_op(node->type, op);
            }
            delete node;
        }
    }
    _index_nodes.clear();
    _name_nodes.clear();
    return 0;
}

EdgeMode Dag::parse_mode(std::string& mode) {
    if (mode == "RO") {
        return RO;
    } else if (mode == "RW") {
        return RW;
    } else {
        return UNKNOWN;
    }
}

// [@Node]
// name: preprocess
// type: ProcessorOp
// [.@Depend]
// name: StartupOp
// mode: RO
// [@Node]
// name: discret_extractor
// type: DiscretExtractOp
// [.@Depend]
// name: StartupOp
// mode: RO
// [.@Depend]
// name: preprocess
// mode: RW
// [@Node]
// name: dnn_inference
// type: PaddleV2InferenceOp
// [.@Depend]
// name: discret_extractor
// mode: RO
// [@Node]
// name: postprocess
// type: PostProcessOp
// [.@Depend]
// name: dnn_inference
// mode: RO
int Dag::init(const char* path, const char* file, const std::string& name) {
    comcfg::Configure conf;
    if (conf.load(path, file) != 0) {
        LOG(FATAL) << "Failed load conf from" 
            << path << "/" << file << " in dag: "
            << name;
        return ERR_INTERNAL_FAILURE;
    }

    return init(conf, name);
}

int Dag::init(const comcfg::Configure& conf, const std::string& name) {
    _dag_name = name;
    _index_nodes.clear();
    _name_nodes.clear();
    for (uint32_t i = 0; i < conf["Node"].size(); i++) {
        DagNode* node = new (std::nothrow) DagNode();
        if (node == NULL) {
            LOG(ERROR) << "Failed create new dag node";
            return ERR_MEM_ALLOC_FAILURE;
        }
        node->id = i + 1; // 0 is reserved for begginer-op
        node->name = conf["Node"][i]["name"].to_cstr();
        node->type = conf["Node"][i]["type"].to_cstr();
        uint32_t depend_size = conf["Node"][i]["Depend"].size();
        for (uint32_t j = 0; j < depend_size; j++) {
            const comcfg::ConfigUnit& depend = 
                conf["Node"][i]["Depend"][j];
            std::string name = depend["name"].to_cstr();
            std::string mode = depend["mode"].to_cstr();
            node->depends.insert(
                    std::make_pair(name, parse_mode(mode)));
        }
        Op* op = OpRepository::instance().get_op(node->type);
        if (op == NULL) {
            LOG(FATAL) << "Failed to get_op, op type[" << node->type << "]";
            return ERR_INTERNAL_FAILURE;
        }
        // node->conf could be NULL
        node->conf = op->create_config(conf["Node"][i]);
        OpRepository::instance().return_op(node->type, op);
        _name_nodes.insert(std::make_pair(node->name, node));
        _index_nodes.push_back(node);
    }

    if (topo_sort() != 0) {
        LOG(FATAL) << "Topo sort dag[" << _dag_name << "] failed!";
        return ERR_INTERNAL_FAILURE;
    }

    if (FLAGS_el_log_level == 16) {
        LOG(DEBUG) << "DAG: " << _dag_name << noflush;
        LOG(DEBUG) << ", Op Num: " << _index_nodes.size();
        for (uint32_t nid = 0; nid < _index_nodes.size(); nid++) {
            DagNode* node = _index_nodes[nid];
            LOG(DEBUG) 
                << ", OP-" << node->id << "-" << node->name << "-" 
                << node->type << noflush;
            LOG(DEBUG) << " depends: " << node->depends.size() << noflush;

            boost::unordered_map<std::string, EdgeMode>::iterator it;
            for (it = node->depends.begin(); it != node->depends.end(); it++) {
                LOG(DEBUG) << " " << it->first << " " << it->second << noflush;
            }
        }
        LOG(DEBUG) << "";
    }

    return ERR_OK;
}

uint32_t Dag::nodes_size() {
    return _index_nodes.size();
}

const DagNode* Dag::node_by_id(uint32_t id) {
    return _index_nodes[id];
}

const DagNode* Dag::node_by_id(uint32_t id) const {
    return _index_nodes[id];
}

const DagNode* Dag::node_by_name(std::string& name) {
    return _name_nodes[name];
}

const DagNode* Dag::node_by_name(const std::string& name) const {
    boost::unordered_map<std::string, DagNode*>::const_iterator it;
    it = _name_nodes.find(name);
    if (it == _name_nodes.end()) {
        LOG(WARNING) << "Not found op by name:" << name;
        return NULL;
    }
    return it->second;
}

uint32_t Dag::stage_size() {
    return _stages.size();
}

const DagStage* Dag::stage_by_index(uint32_t index) {
    return _stages[index];
}
    
int Dag::topo_sort() {
    // TODO ¼òµ¥ÆÌÆ½
    std::stringstream ss;
    for (uint32_t nid = 0; nid < _index_nodes.size(); nid++) {
        DagStage* stage = new (std::nothrow) DagStage();
        if (stage == NULL) {
            LOG(ERROR) << "Invalid stage!"; 
            return ERR_MEM_ALLOC_FAILURE;
        }
        stage->nodes.push_back(_index_nodes[nid]);
        ss.str("");
        ss << _stages.size();
        stage->name = ss.str();
        stage->full_name = full_name() + NAME_DELIMITER + stage->name; 
        _stages.push_back(stage);

        // assign stage number after stage created
        _index_nodes[nid]->stage = nid;
        // assign dag node full name after stage created
        _index_nodes[nid]->full_name = stage->full_name + NAME_DELIMITER + _index_nodes[nid]->name;
    }
    return ERR_OK;
}

void Dag::regist_metric(const std::string& service_name) {
    for (int stage_idx = 0; stage_idx < _stages.size(); ++stage_idx) {
        DagStage* stage = _stages[stage_idx];
        PredictorMetric::GetInstance()->regist_latency_metric(
                STAGE_METRIC_PREFIX + service_name + NAME_DELIMITER + stage->full_name);
        for (int node_idx = 0; node_idx < stage->nodes.size(); ++node_idx) {
            DagNode* node = stage->nodes[node_idx];
            PredictorMetric::GetInstance()->regist_latency_metric(
                    OP_METRIC_PREFIX + service_name + NAME_DELIMITER + node->full_name);
            Op* op = OpRepository::instance().get_op(node->type);
            if (op == NULL) {
                LOG(FATAL) << "Failed to get_op, op type[" << node->type << "]";
                return;
            }
            op->set_full_name(service_name + NAME_DELIMITER + node->full_name);
            op->set_config(node->conf);
            op->regist_metric();
            OpRepository::instance().return_op(node->type, op);
        }
    } 
}

} // predictor
} // paddle_serving
} // baidu
