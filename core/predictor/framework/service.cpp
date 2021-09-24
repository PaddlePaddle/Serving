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
#pragma once
#include "core/predictor/framework/service.h"
#ifdef BCLOUD
#include <base/time.h>  // butil::Timer
#else
#include <butil/time.h>  // butil::Timer
#endif

#include <inttypes.h>
#include <cstring>
#include <list>
#include <string>
#include <vector>
#include "core/predictor/common/constant.h"
#include "core/predictor/common/inner_common.h"
#include "core/predictor/framework/channel.h"
#include "core/predictor/framework/dag_view.h"
#include "core/predictor/framework/manager.h"
#include "core/predictor/framework/predictor_metric.h"  // PredictorMetric
#include "core/predictor/framework/resource.h"
#include "core/predictor/framework/server.h"
#include <iostream>
#include <chrono>
namespace baidu {
namespace paddle_serving {
namespace predictor {
using baidu::paddle_serving::predictor::general_model::Response;
using baidu::paddle_serving::predictor::general_model::Request;
typedef google::protobuf::RepeatedField<google::protobuf::int64> Int64Vector;
typedef google::protobuf::RepeatedField<float> FloatVector;
enum ProtoDataType {
  P_INT64 = 0,
  P_FLOAT32,
  P_INT32,
  P_FP64,
  P_INT16,
  P_FP16,
  P_BF16,
  P_UINT8,
  P_INT8,
  P_BOOL,
  P_COMPLEX64,
  P_COMPLEX128,
  P_STRING = 20,
};
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
                            const uint64_t log_id,
                            butil::IOBufBuilder* debug_os) {
  TRACEPRINTF("(logid=%" PRIu64 ") start to inference", log_id);
  // when funtion call begins, framework will reset
  // thread local variables&resources automatically.
  if (Resource::instance().thread_clear() != 0) {
    LOG(ERROR) << "(logid=" << log_id << ") Failed thread clear whole resource";
    return ERR_INTERNAL_FAILURE;
  }
  if(print_count < 10){
    std::cout << "log_id = " << log_id << "  print_count = " << print_count <<
    " req = " << request->DebugString() << std::endl;
    print_count++;
  }
  
  TRACEPRINTF("(logid=%" PRIu64 ") finish to thread clear", log_id);

  if (_enable_map_request_to_workflow) {
    VLOG(2) << "(logid=" << log_id << ") enable map request == True";
    std::vector<Workflow*>* workflows =
        _map_request_to_workflow(request, log_id);
    if (!workflows || workflows->size() == 0) {
      LOG(ERROR) << "(logid=" << log_id
                 << ") Failed to map request to workflow";
      return ERR_INTERNAL_FAILURE;
    }
    size_t fsize = workflows->size();
    for (size_t fi = 0; fi < fsize; ++fi) {
      Workflow* workflow = (*workflows)[fi];
      if (workflow == NULL) {
        LOG(ERROR) << "(logid=" << log_id
                   << ") Failed to get valid workflow at: " << fi;
        return ERR_INTERNAL_FAILURE;
      }
      TRACEPRINTF("(logid=%" PRIu64 ") start to execute workflow[%s]",
                  log_id,
                  workflow->name().c_str());
      int errcode = -1;
      if (workflow->get_repeat_time() > 1) {
        errcode = _execute_repeat_workflow(
            workflow, request, response, log_id, debug_os);
      } else {
        errcode =
            _execute_workflow(workflow, request, response, log_id, debug_os);
      }

      TRACEPRINTF("(logid=%" PRIu64 ") finish to execute workflow[%s]",
                  log_id,
                  workflow->name().c_str());
      if (errcode < 0) {
        LOG(ERROR) << "(logid=" << log_id << ") Failed execute workflow["
                   << workflow->name() << "] in:" << name();
        return errcode;
      }
    }
  } else {
    VLOG(2) << "(logid=" << log_id << ") enable map request == False";
    TRACEPRINTF("(logid=%" PRIu64 ") start to execute one workflow", log_id);
    size_t fsize = _flows.size();
    for (size_t fi = 0; fi < fsize; ++fi) {
      TRACEPRINTF(
          "(logid=%" PRIu64 ") start to execute one workflow-%lu", log_id, fi);
      int errcode =
          execute_one_workflow(fi, request, response, log_id, debug_os);
      TRACEPRINTF(
          "(logid=%" PRIu64 ") finish to execute one workflow-%lu", log_id, fi);
      if (errcode < 0) {
        LOG(ERROR) << "(logid=" << log_id
                   << ") Failed execute 0-th workflow in:" << name();
        return errcode;
      }
    }
  }
  return ERR_OK;
}

int InferService::debug(const google::protobuf::Message* request,
                        google::protobuf::Message* response,
                        const uint64_t log_id,
                        butil::IOBufBuilder* debug_os) {
  return inference(request, response, log_id, debug_os);
}

int InferService::execute_one_workflow(uint32_t index,
                                       const google::protobuf::Message* request,
                                       google::protobuf::Message* response,
                                       const uint64_t log_id,
                                       butil::IOBufBuilder* debug_os) {
  if (index >= _flows.size()) {
    LOG(ERROR) << "(logid=" << log_id
               << ") Faield execute workflow, index: " << index
               << " >= max:" << _flows.size();
    return ERR_OVERFLOW_FAILURE;
  }
  Workflow* workflow = _flows[index];
  int errcode = -1;
  if (workflow->get_repeat_time() > 1) {
    errcode =
        _execute_repeat_workflow(workflow, request, response, log_id, debug_os);
  } else {
    errcode = _execute_workflow(workflow, request, response, log_id, debug_os);
  }
  return errcode;
}

int InferService::_execute_workflow(Workflow* workflow,
                                    const google::protobuf::Message* request,
                                    google::protobuf::Message* response,
                                    const uint64_t log_id,
                                    butil::IOBufBuilder* debug_os) {
  butil::Timer workflow_time(butil::Timer::STARTED);
  // create and submit beginer channel
  BuiltinChannel req_channel;
  req_channel.init(0, START_OP_NAME);
  req_channel = request;

  DagView* dv = workflow->fetch_dag_view(full_name(), log_id);
  dv->set_request_channel(req_channel, log_id);

  // call actual inference interface
  int errcode = dv->execute(log_id, debug_os);
  if (errcode < 0) {
    LOG(ERROR) << "(logid=" << log_id
               << ") Failed execute dag for workflow:" << workflow->name();
    return errcode;
  }

  TRACEPRINTF("(logid=%" PRIu64 ") finish to dv execute", log_id);
  // create ender channel and copy
  const Channel* res_channel = dv->get_response_channel(log_id);
  if (res_channel == NULL) {
    LOG(ERROR) << "(logid=" << log_id << ") Failed get response channel";
    return ERR_INTERNAL_FAILURE;
  }

  if (!_merger || !_merger->merge(res_channel->message(), response)) {
    LOG(ERROR) << "(logid=" << log_id
               << ") Failed merge channel res to response";
    return ERR_INTERNAL_FAILURE;
  }
  TRACEPRINTF("(logid=%" PRIu64 ") finish to copy from", log_id);

  workflow_time.stop();
  LOG(INFO) << "(logid=" << log_id
            << ") workflow total time: " << workflow_time.u_elapsed();
  PredictorMetric::GetInstance()->update_latency_metric(
      WORKFLOW_METRIC_PREFIX + dv->full_name(), workflow_time.u_elapsed());

  // return tls data to object pool
  workflow->return_dag_view(dv);
  TRACEPRINTF("(logid=%" PRIu64 ") finish to return dag view", log_id);
  return ERR_OK;
}

int InferService::_execute_repeat_workflow(
    Workflow* workflow,
    const google::protobuf::Message* request,
    google::protobuf::Message* response,
    const uint64_t log_id,
    butil::IOBufBuilder* debug_os) {
  butil::Timer workflow_time(butil::Timer::STARTED);
  // create and submit beginer channel
  // first time init.
  BuiltinChannel req_channel;
  req_channel.init(0, START_OP_NAME);
  req_channel = request;
  DagView* dv = workflow->fetch_dag_view(full_name(), log_id);
  dv->set_request_channel(req_channel, log_id);
  int64_t batch_size = ((Request*)request)->tensor(0).shape(0);
  int64_t no_batch_length = ((Request*)request)->tensor(0).shape(1);
  Request* real_input = (Request*)req_channel.data();

  std::cout << "batch_size == " << batch_size << std::endl; 
  std::cout << "workflow->get_repeat_time() = " << workflow->get_repeat_time() <<std::endl;
  
  // call actual inference interface
  // 直接每次修改req_channel里的request
  // 再从新执行一遍即可。
  for (int64_t i = 0; i < workflow->get_repeat_time(); ++i) {
    std::cout << "ysl repeat time = " << i << std::endl;
    auto start = std::chrono::system_clock::now();
    if(i < 10){
      std::cout << "repeat time = " << i << "  and real_input = " << real_input->DebugString() <<std::endl;
    }
    
    int errcode = dv->execute(log_id, debug_os);
    if (errcode < 0) {
      LOG(ERROR) << "(logid=" << log_id
                 << ") Failed execute dag for workflow:" << workflow->name();
      return errcode;
    }
    TRACEPRINTF("(logid=%" PRIu64 ") finish to dv execute", log_id);
    // create ender channel and copy
    const Channel* res_channel = dv->get_response_channel(log_id);
    if (res_channel == NULL) {
      LOG(ERROR) << "(logid=" << log_id << ") Failed get response channel";
      return ERR_INTERNAL_FAILURE;
    }
    const Int64Vector& output_int64_vector =
        ((Response*)(res_channel->message()))
            ->outputs(0)
            .tensor(0)
            .int64_data();
    if (output_int64_vector.size() != batch_size) {
      LOG(ERROR) << "(logid=" << log_id << ") response batch = " << output_int64_vector.size() << "!= input batch" << batch_size;
      LOG(ERROR) << "(repeat_time=" << i;
      if(i < 10){
        for (int out_num = 0; out_num < output_int64_vector.size(); ++out_num){
          std::cout << "out_put index = " << out_num << " value = " << output_int64_vector[out_num] << std::endl;
        }
      }
      return ERR_INTERNAL_FAILURE;
    }
    if(i < 10){
      for (int out_num = 0; out_num < output_int64_vector.size(); ++out_num){
        std::cout << "out_put index = " << out_num << " value = " << output_int64_vector[out_num] << std::endl;
      }
    }
    
    int64_t total_data_len = 0;
    int64_t elem_size = 0;
    int64_t add_index = 0;
    void* src_ptr = nullptr;
    void* dst_ptr = nullptr;

    for (int64_t j = 0; j < real_input->tensor_size(); ++j) {
      baidu::paddle_serving::predictor::general_model::Tensor* tensor_ptr =
          real_input->mutable_tensor(j);

      if (tensor_ptr->name() == "src_ids") {
        total_data_len = (no_batch_length + 1) * batch_size;
        elem_size = sizeof(int64_t);
        tensor_ptr->set_shape(1,no_batch_length + 1);
        tensor_ptr->mutable_int64_data()->Resize(total_data_len, 0);
        // 由于不再额外开辟内存，所以需要从后向前拷贝
        for (int64_t k = batch_size - 1; k >= 0; --k) {
          add_index = (k + 1) * (no_batch_length + 1) - 1;
          tensor_ptr->mutable_int64_data()->Set(add_index,
                                                output_int64_vector[k]);
          src_ptr = tensor_ptr->mutable_int64_data()->mutable_data() +
                    k * no_batch_length * elem_size;
          dst_ptr = tensor_ptr->mutable_int64_data()->mutable_data() +
                    k * (no_batch_length + 1) * elem_size;
          memmove(dst_ptr, src_ptr, no_batch_length * elem_size);
        }
      } else if (tensor_ptr->name() == "pos_ids") {
        total_data_len = (no_batch_length + 1) * batch_size;
        elem_size = sizeof(int64_t);
        tensor_ptr->set_shape(1,no_batch_length + 1);
        tensor_ptr->mutable_int64_data()->Resize(total_data_len, 0);
        // 由于不再额外开辟内存，所以需要从后向前拷贝
        for (int64_t k = batch_size - 1; k >= 0; --k) {
          add_index = (k + 1) * (no_batch_length + 1) - 1;
          src_ptr = tensor_ptr->mutable_int64_data()->mutable_data() +
                    k * no_batch_length * elem_size;
          dst_ptr = tensor_ptr->mutable_int64_data()->mutable_data() +
                    k * (no_batch_length + 1) * elem_size;
          int64_t last_value =
              *((long*)(src_ptr + (no_batch_length - 1) * elem_size));
          tensor_ptr->mutable_int64_data()->Set(add_index, last_value + 1);
          memmove(dst_ptr, src_ptr, no_batch_length * elem_size);
        }
      } else if (tensor_ptr->name() == "input_mask") {
        total_data_len =
            (no_batch_length + 1) * (no_batch_length + 1) * batch_size;
        elem_size = sizeof(float);
        int64_t no_batch_matrix_length =
            (no_batch_length + 1) * (no_batch_length + 1);
        tensor_ptr->set_shape(1,no_batch_length + 1);
        tensor_ptr->set_shape(2,no_batch_length + 1);
        tensor_ptr->mutable_float_data()->Clear();
        tensor_ptr->mutable_float_data()->Resize(total_data_len, 0);
        int64_t offset = 0;
        int64_t setIndex = 0;
        // 搬运的开销比直接赋值还要大，所以直接Resize为0
        // 从新赋值,只需要处理下三角部分即可.
        // 由于无须搬运，所以直接从前向后处理.
        for (int64_t k = 0; k < batch_size; ++k) {
          offset = k * no_batch_matrix_length;
          for (int64_t line_index = 0; line_index < no_batch_length + 1;
               ++line_index) {
            for (int64_t column_index = 0; column_index < line_index + 1;
                 ++column_index) {
              setIndex =
                  offset + line_index * (no_batch_length + 1) + column_index;
              tensor_ptr->mutable_float_data()->Set(setIndex, 1);
            }
          }
        }
      }
    }
    no_batch_length += 1;
    auto end = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    if(i < 10){
      std::cout << "repeat num =  " << i << " and timecost = "
              << duration.count()  << " ms\n";
    }
    
  }

  if (!_merger ||
      !_merger->merge(dv->get_response_channel(log_id)->message(), response)) {
    LOG(ERROR) << "(logid=" << log_id
               << ") Failed merge channel res to response";
    return ERR_INTERNAL_FAILURE;
  }
  TRACEPRINTF("(logid=%" PRIu64 ") finish to copy from", log_id);

  workflow_time.stop();
  LOG(INFO) << "(logid=" << log_id
            << ") workflow total time: " << workflow_time.u_elapsed();
  PredictorMetric::GetInstance()->update_latency_metric(
      WORKFLOW_METRIC_PREFIX + dv->full_name(), workflow_time.u_elapsed());

  // return tls data to object pool
  workflow->return_dag_view(dv);
  TRACEPRINTF("(logid=%" PRIu64 ") finish to return dag view", log_id);
  return ERR_OK;
}

std::vector<Workflow*>* InferService::_map_request_to_workflow(
    const google::protobuf::Message* request, const uint64_t log_id) {
  const google::protobuf::Descriptor* desc = request->GetDescriptor();
  const google::protobuf::FieldDescriptor* field =
      desc->FindFieldByName(_request_field_key);
  if (field == NULL) {
    LOG(ERROR) << "(logid=" << log_id << ") No field[" << _request_field_key
               << "] in [" << desc->full_name() << "].";
    return NULL;
  }
  if (field->is_repeated()) {
    LOG(ERROR) << "(logid=" << log_id << ") field[" << desc->full_name() << "."
               << _request_field_key << "] is repeated.";
    return NULL;
  }
  if (field->cpp_type() != google::protobuf::FieldDescriptor::CPPTYPE_STRING) {
    LOG(ERROR) << "(logid=" << log_id << ") field[" << desc->full_name() << "."
               << _request_field_key << "] should be string";
    return NULL;
  }
  const std::string& field_value =
      request->GetReflection()->GetString(*request, field);
  std::vector<Workflow*>* p_workflow =
      _request_to_workflow_map.seek(field_value);
  if (p_workflow == NULL) {
    LOG(ERROR) << "(logid=" << log_id << ") cannot find key[" << field_value
               << "] in _request_to_workflow_map";
    return NULL;
  }
  return p_workflow;
}

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu

