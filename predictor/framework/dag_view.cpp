#include "framework/dag_view.h"
#include <baidu/rpc/traceprintf.h> // TRACEPRINTF
#include "common/inner_common.h"
#include "framework/op_repository.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

int DagView::init(Dag* dag, const std::string& service_name) {
    _name = dag->name();
    _full_name = service_name + NAME_DELIMITER + dag->name();
    _bus = butil::get_object<Bus>();
    _bus->clear();
    uint32_t stage_size = dag->stage_size();
    // create tls stage view
    for (uint32_t si = 0; si < stage_size; si++) {
        const DagStage* stage = dag->stage_by_index(si);
        if (stage == NULL) {
            LOG(FATAL) << "Failed get stage by index:" << si;
            return ERR_INTERNAL_FAILURE;
        }
        ViewStage* vstage = butil::get_object<ViewStage>();
        if (vstage == NULL) {
            LOG(FATAL) 
                << "Failed get vstage from object pool" 
                << "at:" << si;
            return ERR_MEM_ALLOC_FAILURE;
        }
        vstage->full_name = service_name + NAME_DELIMITER + stage->full_name;
        uint32_t node_size = stage->nodes.size();
        // create tls view node
        for (uint32_t ni = 0; ni < node_size; ni++) {
            DagNode* node = stage->nodes[ni];
            ViewNode* vnode = butil::get_object<ViewNode>();
            if (vnode == NULL) {
                LOG(FATAL) << "Failed get vnode at:" << ni;
                return ERR_MEM_ALLOC_FAILURE;
            }
            // factory type
            Op* op = OpRepository::instance().get_op(node->type);
            if (op == NULL) {
                LOG(FATAL) << "Failed get op with type:" 
                    << node->type;
                return ERR_INTERNAL_FAILURE;
            }

            // initialize a TLS op object
            if (op->init(_bus, dag, node->id, node->name, node->type, node->conf) != 0) {
                LOG(WARNING) << "Failed init op, type:" << node->type;
                return ERR_INTERNAL_FAILURE;
            }
            op->set_full_name(service_name + NAME_DELIMITER + node->full_name);
            vnode->conf = node;
            vnode->op = op;
            vstage->nodes.push_back(vnode);
        }
        _view.push_back(vstage);
    }

    return ERR_OK;
}

int DagView::deinit() {
    uint32_t stage_size = _view.size();
    for (uint32_t si = 0; si < stage_size; si++) {
        ViewStage* vstage = _view[si];
        uint32_t node_size = vstage->nodes.size();
        for (uint32_t ni = 0; ni < node_size; ni++) {
            ViewNode* vnode = vstage->nodes[ni];
            vnode->op->deinit();
            OpRepository::instance().return_op(vnode->op);
            vnode->reset();
            // clear item
            butil::return_object(vnode);
        }
        // clear vector
        vstage->nodes.clear();
        butil::return_object(vstage);
    }
    _view.clear();
    _bus->clear();
    butil::return_object(_bus);
    return ERR_OK;
}

int DagView::execute(butil::IOBufBuilder* debug_os) {
    uint32_t stage_size = _view.size();
    for (uint32_t si = 0; si < stage_size; si++) {
        TRACEPRINTF("start to execute stage[%u]", si);
        int errcode = execute_one_stage(_view[si], debug_os);
        TRACEPRINTF("finish to execute stage[%u]", si);
        if (errcode < 0) {
            LOG(FATAL) 
                << "failed execute stage[" 
                << _view[si]->debug();
            return errcode;
        }
    }
    return ERR_OK;
}

// The default execution strategy is in sequencing
// You can derive a subclass to implement this func.
// ParallelDagView maybe the one you want.
int DagView::execute_one_stage(ViewStage* vstage,
        butil::IOBufBuilder* debug_os) {
    butil::Timer stage_time(butil::Timer::STARTED);
    uint32_t node_size = vstage->nodes.size(); 
    for (uint32_t ni = 0; ni < node_size; ni++) {
        ViewNode* vnode = vstage->nodes[ni];
        DagNode* conf = vnode->conf;
        Op* op = vnode->op;
        TRACEPRINTF("start to execute op[%s]", op->name());
        int errcode = op->process(debug_os != NULL);
        TRACEPRINTF("finish to execute op[%s]", op->name());
        if (errcode < 0) {
            LOG(FATAL) 
                << "Execute failed, Op:" << op->debug_string();
            return errcode;
        }

        if (errcode > 0) {
            LOG(INFO) 
                << "Execute ignore, Op:" << op->debug_string();
            continue;
        }

        if (debug_os) {
            (*debug_os)
                << "{\"op_name\": \"" << op->name()
                << "\", \"debug_str:\": \"" 
                << op->debug_string()
                << "\", \"time_info\": \"" << op->time_info() << "\"}";
        }

        //LOG(DEBUG) << "Execute succ, Op:" << op->debug_string();
    }
    stage_time.stop();
    PredictorMetric::GetInstance()->update_latency_metric(
            STAGE_METRIC_PREFIX + vstage->full_name, stage_time.u_elapsed());
    return ERR_OK;
}

int DagView::set_request_channel(Channel& request) {
    // Each workflow should get the very beginning 
    // request (channel), and commit it to bus, for
    // the first stage ops consuming.

    request.share_to_bus(_bus);

    return ERR_OK;
}

const Channel* DagView::get_response_channel() const {
    // Caller obtains response channel from bus, and
    // writes it to rpc response(protbuf/json)
    if (_view.size() < 1) {
        LOG(FATAL) << "invalid empty view stage!" << noflush;
        return NULL;
    }

    ViewStage* last_stage = _view[_view.size() - 1];
    if (last_stage->nodes.size() != 1 
                    || last_stage->nodes[0] == NULL) {
        LOG(FATAL) << "Invalid last stage, size[" 
                << last_stage->nodes.size()
                << "] != 1" << noflush;
        return NULL;
    }

    Op* last_op = last_stage->nodes[0]->op;
    if (last_op == NULL) {
        LOG(FATAL) << "Last op is NULL";
        return NULL;
    }
    return last_op->mutable_channel();
}

} // predictor
} // paddle_serving
} // baidu
