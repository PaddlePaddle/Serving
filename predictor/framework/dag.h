#ifndef BAIDU_PADDLE_SERVING_PREDICTOR_DAG_H
#define BAIDU_PADDLE_SERVING_PREDICTOR_DAG_H

#include "common/inner_common.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

enum EdgeMode {
    RO = 0, 
    RW = 1,
    UNKNOWN
};

struct DagNode {
    uint32_t id;
    uint32_t stage;
    std::string name; // opname
    std::string full_name; // workflow_stageindex_opname
    std::string type;
    void* conf;
    boost::unordered_map<std::string, EdgeMode> depends;
};

struct DagStage {
    std::vector<DagNode*> nodes;
    std::string name; // stageindex
    std::string full_name; // workflow_stageindex
};

class Dag {
public:
    Dag();

    virtual ~Dag();

    EdgeMode parse_mode(std::string& mode);
    
    int init(const char* path, const char* file, const std::string& name);

    int init(const comcfg::Configure& conf, const std::string& name);
    
    int deinit();

    uint32_t nodes_size();

    const DagNode* node_by_id(uint32_t id);

    const DagNode* node_by_id(uint32_t id) const;

    const DagNode* node_by_name(std::string& name);

    const DagNode* node_by_name(const std::string& name) const;

    uint32_t stage_size();

    const DagStage* stage_by_index(uint32_t index);

    const std::string& name() const {
        return _dag_name;
    }

    const std::string& full_name() const {
        return _dag_name;
    }

    void regist_metric(const std::string& service_name);
    
private:
    int topo_sort();

private:
    std::string _dag_name;
    boost::unordered_map<std::string, DagNode*> _name_nodes;
    std::vector<DagNode*> _index_nodes;
    std::vector<DagStage*> _stages;
};

} // predictor
} // paddle_serving
} // baidu

#endif // BAIDU_PADDLE_SERVING_PREDICTOR_DAG_H
