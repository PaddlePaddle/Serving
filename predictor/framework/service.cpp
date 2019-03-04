// Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "framework/service.h"
#include <butil/time.h>  // butil::Timer
#include <list>
#include <string>
#include <vector>
#include "common/constant.h"
#include "common/inner_common.h"
#include "framework/channel.h"
#include "framework/dag_view.h"
#include "framework/manager.h"
#include "framework/predictor_metric.h"  // PredictorMetric
#include "framework/resource.h"
#include "framework/server.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

int InferService::init(const configure::InferService& conf) {
  _infer_service_format = conf.name();

  std::string merger = conf.merger();
  if (merger == "") {
    merger = "default";
  }
  if (!MergerManager::instance().get(merger, _merger)) {
    LOG(ERROR) << "Failed get merger: " << merger;
    return ERR_INTERNAL_FAILURE;
  } else {
    LOG(WARNING) << "Succ get merger: " << merger
                 << " for service: " << _infer_service_format;
  }

  ServerManager& svr_mgr = ServerManager::instance();
  if (svr_mgr.add_service_by_format(_infer_service_format) != 0) {
    LOG(ERROR) << "Not found service by format name:" << _infer_service_format
               << "!";
    return ERR_INTERNAL_FAILURE;
  }

  _enable_map_request_to_workflow = conf.enable_map_request_to_workflow();
  LOG(INFO) << "service[" << _infer_service_format
            << "], enable_map_request_to_workflow["
            << _enable_map_request_to_workflow << "].";

  if (_enable_map_request_to_workflow) {
    if (_request_to_workflow_map.init(
            MAX_WORKFLOW_NUM_IN_ONE_SERVICE /*load_factor=80*/) != 0) {
      LOG(ERROR) << "init request to workflow map failed, bucket_count["
                 << MAX_WORKFLOW_NUM_IN_ONE_SERVICE << "].";
      return ERR_INTERNAL_FAILURE;
    }
    int err = 0;
    _request_field_key = conf.request_field_key().c_str();
    if (_request_field_key == "") {
      LOG(ERROR) << "read request_field_key failed, request_field_key["
                 << _request_field_key << "].";
      return ERR_INTERNAL_FAILURE;
    }

    LOG(INFO) << "service[" << _infer_service_format << "], request_field_key["
              << _request_field_key << "].";
    uint32_t value_mapped_workflows_size = conf.value_mapped_workflows_size();
    for (uint32_t fi = 0; fi < value_mapped_workflows_size; fi++) {
      std::vector<std::string> tokens;
      std::vector<Workflow*> workflows;
      std::string list = conf.value_mapped_workflows(fi).workflow();
      boost::split(tokens, list, boost::is_any_of(","));
      uint32_t tsize = tokens.size();
      for (uint32_t ti = 0; ti < tsize; ++ti) {
        boost::trim_if(tokens[ti], boost::is_any_of(" "));
        Workflow* workflow = WorkflowManager::instance().item(tokens[ti]);
        if (workflow == NULL) {
          LOG(ERROR) << "Failed get workflow by name:" << tokens[ti]
                     << ", ti: " << ti;
          return ERR_INTERNAL_FAILURE;
        }
        workflow->regist_metric(full_name());
        workflows.push_back(workflow);
      }

      const std::string& request_field_value =
          conf.value_mapped_workflows(fi).request_field_value();
      if (_request_to_workflow_map.insert(request_field_value, workflows) ==
          NULL) {
        LOG(ERROR) << "insert [" << request_field_value << "," << list
                   << "] to _request_to_workflow_map failed.";
        return ERR_INTERNAL_FAILURE;
      }
      LOG(INFO) << "workflow[" << list << "], request_field_value["
                << request_field_value << "].";
    }
  } else {
    uint32_t flow_size = conf.workflows_size();
    for (uint32_t fi = 0; fi < flow_size; fi++) {
      const std::string& workflow_name = conf.workflows(fi);
      Workflow* workflow = WorkflowManager::instance().item(workflow_name);
      if (workflow == NULL) {
        LOG(ERROR) << "Failed get workflow by name:" << workflow_name;
        return ERR_INTERNAL_FAILURE;
      }
      workflow->regist_metric(full_name());
      _flows.push_back(workflow);
    }
  }

  LOG(INFO) << "Succ load infer_service: " << _infer_service_format << "!";

  return ERR_OK;
}

int InferService::reload() { return ERR_OK; }

const std::string& InferService::name() const { return _infer_service_format; }

// ´®ÐÐÖ´ÐÐÃ¿¸öworkflow
int InferService::inference(const google::protobuf::Message* request,
                            google::protobuf::Message* response,
                            butil::IOBufBuilder* debug_os) {
  TRACEPRINTF("start to inference");
  // when funtion call begins, framework will reset
  // thread local variables&resources automatically.
  if (Resource::instance().thread_clear() != 0) {
    LOG(ERROR) << "Failed thread clear whole resource";
    return ERR_INTERNAL_FAILURE;
  }

  TRACEPRINTF("finish to thread clear");

  if (_enable_map_request_to_workflow) {
    std::vector<Workflow*>* workflows = _map_request_to_workflow(request);
    if (!workflows || workflows->size() == 0) {
      LOG(ERROR) << "Failed to map request to workflow";
      return ERR_INTERNAL_FAILURE;
    }
    size_t fsize = workflows->size();
    for (size_t fi = 0; fi < fsize; ++fi) {
      Workflow* workflow = (*workflows)[fi];
      if (workflow == NULL) {
        LOG(ERROR) << "Failed to get valid workflow at: " << fi;
        return ERR_INTERNAL_FAILURE;
      }
      TRACEPRINTF("start to execute workflow[%s]", workflow->name().c_str());
      int errcode = _execute_workflow(workflow, request, response, debug_os);
      TRACEPRINTF("finish to execute workflow[%s]", workflow->name().c_str());
      if (errcode < 0) {
        LOG(ERROR) << "Failed execute workflow[" << workflow->name()
                   << "] in:" << name();
        return errcode;
      }
    }
  } else {
    TRACEPRINTF("start to execute one workflow");
    size_t fsize = _flows.size();
    for (size_t fi = 0; fi < fsize; ++fi) {
      TRACEPRINTF("start to execute one workflow-%lu", fi);
      int errcode = execute_one_workflow(fi, request, response, debug_os);
      TRACEPRINTF("finish to execute one workflow-%lu", fi);
      if (errcode < 0) {
        LOG(ERROR) << "Failed execute 0-th workflow in:" << name();
        return errcode;
      }
    }
  }
  return ERR_OK;
}

int InferService::debug(const google::protobuf::Message* request,
                        google::protobuf::Message* response,
                        butil::IOBufBuilder* debug_os) {
  return inference(request, response, debug_os);
}

int InferService::execute_one_workflow(uint32_t index,
                                       const google::protobuf::Message* request,
                                       google::protobuf::Message* response,
                                       butil::IOBufBuilder* debug_os) {
  if (index >= _flows.size()) {
    LOG(ERROR) << "Faield execute workflow, index: " << index
               << " >= max:" << _flows.size();
    return ERR_OVERFLOW_FAILURE;
  }
  Workflow* workflow = _flows[index];
  return _execute_workflow(workflow, request, response, debug_os);
}

int InferService::_execute_workflow(Workflow* workflow,
                                    const google::protobuf::Message* request,
                                    google::protobuf::Message* response,
                                    butil::IOBufBuilder* debug_os) {
  butil::Timer workflow_time(butil::Timer::STARTED);
  // create and submit beginer channel
  BuiltinChannel req_channel;
  req_channel.init(0, START_OP_NAME);
  req_channel = request;

  DagView* dv = workflow->fetch_dag_view(full_name());
  dv->set_request_channel(req_channel);

  // call actual inference interface
  int errcode = dv->execute(debug_os);
  if (errcode < 0) {
    LOG(ERROR) << "Failed execute dag for workflow:" << workflow->name();
    return errcode;
  }

  TRACEPRINTF("finish to dv execute");
  // create ender channel and copy
  const Channel* res_channel = dv->get_response_channel();
  if (!_merger || !_merger->merge(res_channel->message(), response)) {
    LOG(ERROR) << "Failed merge channel res to response";
    return ERR_INTERNAL_FAILURE;
  }
  TRACEPRINTF("finish to copy from");

  workflow_time.stop();
  PredictorMetric::GetInstance()->update_latency_metric(
      WORKFLOW_METRIC_PREFIX + dv->full_name(), workflow_time.u_elapsed());

  // return tls data to object pool
  workflow->return_dag_view(dv);
  TRACEPRINTF("finish to return dag view");
  return ERR_OK;
}

std::vector<Workflow*>* InferService::_map_request_to_workflow(
    const google::protobuf::Message* request) {
  const google::protobuf::Descriptor* desc = request->GetDescriptor();
  const google::protobuf::FieldDescriptor* field =
      desc->FindFieldByName(_request_field_key);
  if (field == NULL) {
    LOG(ERROR) << "No field[" << _request_field_key << "] in ["
               << desc->full_name() << "].";
    return NULL;
  }
  if (field->is_repeated()) {
    LOG(ERROR) << "field[" << desc->full_name() << "." << _request_field_key
               << "] is repeated.";
    return NULL;
  }
  if (field->cpp_type() != google::protobuf::FieldDescriptor::CPPTYPE_STRING) {
    LOG(ERROR) << "field[" << desc->full_name() << "." << _request_field_key
               << "] should be string";
    return NULL;
  }
  const std::string& field_value =
      request->GetReflection()->GetString(*request, field);
  std::vector<Workflow*>* p_workflow =
      _request_to_workflow_map.seek(field_value);
  if (p_workflow == NULL) {
    LOG(ERROR) << "cannot find key[" << field_value
               << "] in _request_to_workflow_map";
    return NULL;
  }
  return p_workflow;
}

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
