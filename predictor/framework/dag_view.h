#ifndef BAIDU_PADDLE_SERVING_PREDICTOR_DAG_VIEW_H
#define BAIDU_PADDLE_SERVING_PREDICTOR_DAG_VIEW_H

#include "op/op.h"
#include "common/inner_common.h"
#include "framework/channel.h"
#include "framework/dag.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

class Op;

struct ViewNode {
  Op* op; // op->full_name == service_workflow_stageindex_opname
  DagNode* conf;
  void reset() {
    op = NULL;
    conf = NULL;
  }
};

struct ViewStage {
  std::vector<ViewNode*> nodes;
  std::string full_name; // service_workflow_stageindex
  std::string debug() {
    return "TOBE IMPLEMENTED!";
  }
};

class DagView {
public:
  DagView() : _bus(NULL) {
    _view.clear();
  }

  ~DagView() {}
  
  int init(Dag* dag, const std::string& service_name);

  int deinit();

  int execute(base::IOBufBuilder* debug_os);

  // The default execution strategy is in sequencing
  // You can derive a subclass to implement this func.
  // ParallelDagView maybe the one you want.
  virtual int execute_one_stage(ViewStage* vstage,
          base::IOBufBuilder* debug_os);

  int set_request_channel(Channel& request);

  const Channel* get_response_channel() const;

  const std::string& name() const {
    return _name;
  }

  const std::string& full_name() const {
    return _full_name;
  }

private:
  std::string _name;
  std::string _full_name;
  std::vector<ViewStage*> _view;
  Bus* _bus;
};

// The derived DagView supports parallel execution
// strategy, by implments the execute_one_stage().
class ParallelDagView : public DagView {
public:  
  int execute_one_stage(ViewStage* vstage, base::IOBufBuilder*) {
    return 0;
  }
};

} // predictor
} // paddle_serving
} // baidu

#endif // BAIDU_PADDLE_SERVING_PREDICTOR_DAG_VIEW_H
