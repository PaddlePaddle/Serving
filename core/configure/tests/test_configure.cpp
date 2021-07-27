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

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include "core/configure/include/configure_parser.h"
#include "core/configure/inferencer_configure.pb.h"
#include "core/configure/sdk_configure.pb.h"
#include "core/configure/server_configure.pb.h"

using baidu::paddle_serving::configure::EngineDesc;
using baidu::paddle_serving::configure::ModelToolkitConf;

using baidu::paddle_serving::configure::ResourceConf;

using baidu::paddle_serving::configure::DAGNodeDependency;
using baidu::paddle_serving::configure::DAGNode;
using baidu::paddle_serving::configure::Workflow;
using baidu::paddle_serving::configure::WorkflowConf;

using baidu::paddle_serving::configure::InferService;
using baidu::paddle_serving::configure::InferServiceConf;

using baidu::paddle_serving::configure::ConnectionConf;
using baidu::paddle_serving::configure::WeightedRandomRenderConf;
using baidu::paddle_serving::configure::NamingConf;
using baidu::paddle_serving::configure::RpcParameter;
using baidu::paddle_serving::configure::Predictor;
using baidu::paddle_serving::configure::VariantConf;
using baidu::paddle_serving::configure::SDKConf;

using baidu::paddle_serving::configure::SigmoidConf;

const char *output_dir = "./conf/";
const char *model_toolkit_conf_file = "model_toolkit.prototxt";
const char *resource_conf_file = "resource.prototxt";
const char *workflow_conf_file = "workflow.prototxt";
const char *service_conf_file = "service.prototxt";
const char *sdk_conf_file = "predictors.prototxt";
const char *sigmoid_conf_file = "inferencer.prototxt";

int test_write_conf() {
  // model_toolkit conf
  ModelToolkitConf model_toolkit_conf;

  // This engine has a default version
  EngineDesc *engine = model_toolkit_conf.add_engines();
  engine->set_name("image_classification_resnet");
  engine->set_type("FLUID_CPU_NATIVE_DIR");
  engine->set_reloadable_meta("./data/model/paddle/fluid_time_file");
  engine->set_reloadable_type("timestamp_ne");
  engine->set_model_data_path("./data/model/paddle/fluid/SE_ResNeXt50_32x4d");
  engine->set_runtime_thread_num(0);
  engine->set_batch_infer_size(0);
  engine->set_enable_batch_align(0);
  engine->set_sparse_param_service_type(EngineDesc::LOCAL);
  engine->set_sparse_param_service_table_name("local_kv");
  engine->set_enable_memory_optimization(true);

  int ret = baidu::paddle_serving::configure::write_proto_conf(
      &model_toolkit_conf, output_dir, model_toolkit_conf_file);
  if (ret != 0) {
    return ret;
  }

  // resource conf
  ResourceConf resource_conf;
  resource_conf.set_model_toolkit_path(output_dir);
  resource_conf.set_model_toolkit_file("model_toolkit.prototxt");
  resource_conf.set_cube_config_file("./conf/cube.conf");
  ret = baidu::paddle_serving::configure::write_proto_conf(
      &resource_conf, output_dir, resource_conf_file);
  if (ret != 0) {
    return ret;
  }

  // workflow entries conf
  WorkflowConf workflow_conf;
  Workflow *workflow = workflow_conf.add_workflows();
  workflow->set_name("workflow1");
  workflow->set_workflow_type("Sequence");

  DAGNode *dag_node = workflow->add_nodes();
  dag_node->set_name("image_reader_op");
  dag_node->set_type("ReaderOp");

  dag_node = workflow->add_nodes();
  dag_node->set_name("imag_classify_op");
  dag_node->set_type("ClassifyOp");
  DAGNodeDependency *node_dependency = dag_node->add_dependencies();
  node_dependency->set_name("image_reader_op");
  node_dependency->set_mode("RO");

  dag_node = workflow->add_nodes();
  dag_node->set_name("write_json_op");
  dag_node->set_type("WriteOp");
  node_dependency = dag_node->add_dependencies();
  node_dependency->set_name("image_classify_op");
  node_dependency->set_mode("RO");

  workflow = workflow_conf.add_workflows();
  workflow->set_name("workflow2");
  workflow->set_workflow_type("Sequence");

  dag_node = workflow->add_nodes();
  dag_node->set_name("dense_op");
  dag_node->set_type("DenseOp");

  ret = baidu::paddle_serving::configure::write_proto_conf(
      &workflow_conf, output_dir, workflow_conf_file);
  if (ret != 0) {
    return ret;
  }

  InferServiceConf infer_service_conf;
  infer_service_conf.set_port(0);
  InferService *infer_service = infer_service_conf.add_services();
  infer_service->set_name("ImageClassifyService");
  infer_service->add_workflows("workflow1");
  infer_service->add_workflows("workflow2");

  infer_service = infer_service_conf.add_services();
  infer_service->set_name("BuiltinDenseFormatService");
  infer_service->add_workflows("workflow2");

  ret = baidu::paddle_serving::configure::write_proto_conf(
      &infer_service_conf, output_dir, service_conf_file);
  if (ret != 0) {
    return ret;
  }

  SDKConf sdk_conf;
  VariantConf *default_variant_conf = sdk_conf.mutable_default_variant_conf();
  default_variant_conf->set_tag("default");

  ConnectionConf *connection_conf =
      default_variant_conf->mutable_connection_conf();
  connection_conf->set_connect_timeout_ms(2000);
  connection_conf->set_rpc_timeout_ms(20000);
  connection_conf->set_connect_retry_count(2);
  connection_conf->set_max_connection_per_host(100);
  connection_conf->set_hedge_request_timeout_ms(-1);
  connection_conf->set_hedge_fetch_retry_count(2);
  connection_conf->set_connection_type("pooled");

  NamingConf *naming_conf = default_variant_conf->mutable_naming_conf();
  naming_conf->set_cluster_filter_strategy("Default");
  naming_conf->set_load_balance_strategy("la");

  RpcParameter *rpc_parameter = default_variant_conf->mutable_rpc_parameter();
  rpc_parameter->set_compress_type(0);
  rpc_parameter->set_package_size(20);
  rpc_parameter->set_protocol("baidu_std");
  rpc_parameter->set_max_channel_per_request(3);

  Predictor *predictor = sdk_conf.add_predictors();
  predictor->set_name("ximage");
  predictor->set_service_name(
      "baidu.paddle_serving.predictor.image_classification."
      "ImageClassifyService");
  predictor->set_endpoint_router("WeightedRandomRender");

  WeightedRandomRenderConf *weighted_random_render_conf =
      predictor->mutable_weighted_random_render_conf();
  weighted_random_render_conf->set_variant_weight_list("50");

  VariantConf *variant_conf = predictor->add_variants();
  variant_conf->set_tag("var1");
  naming_conf = variant_conf->mutable_naming_conf();
  naming_conf->set_cluster("list://127.0.0.1:8010");

  ret = baidu::paddle_serving::configure::write_proto_conf(
      &sdk_conf, output_dir, sdk_conf_file);
  if (ret != 0) {
    return ret;
  }

  SigmoidConf sigmoid_conf;
  sigmoid_conf.set_dnn_model_path("data/dnn_model");
  sigmoid_conf.set_sigmoid_w_file("data/dnn_model/_sigmoid_.w_0");
  sigmoid_conf.set_sigmoid_b_file("data/dnn_model/_sigmoid_.b_0");
  sigmoid_conf.set_exp_max_input(0.75);
  sigmoid_conf.set_exp_min_input(0.25);

  ret = baidu::paddle_serving::configure::write_proto_conf(
      &sigmoid_conf, output_dir, sigmoid_conf_file);
  if (ret != 0) {
    return ret;
  }

  return 0;
}

int test_read_conf() {
  int ret = 0;

  ModelToolkitConf model_toolkit_conf;
  ret = baidu::paddle_serving::configure::read_proto_conf(
      output_dir, model_toolkit_conf_file, &model_toolkit_conf);
  if (ret != 0) {
    std::cout << "Read conf fail: " << model_toolkit_conf_file << std::endl;
    return -1;
  }

  ResourceConf resource_conf;
  ret = baidu::paddle_serving::configure::read_proto_conf(
      output_dir, resource_conf_file, &resource_conf);
  if (ret != 0) {
    std::cout << "Read conf fail: " << resource_conf_file << std::endl;
    return -1;
  }

  WorkflowConf workflow_conf;
  ret = baidu::paddle_serving::configure::read_proto_conf(
      output_dir, workflow_conf_file, &workflow_conf);
  if (ret != 0) {
    std::cout << "Read conf fail: " << workflow_conf_file << std::endl;
    return -1;
  }

  InferServiceConf service_conf;
  ret = baidu::paddle_serving::configure::read_proto_conf(
      output_dir, service_conf_file, &service_conf);
  if (ret != 0) {
    std::cout << "Read conf fail: " << service_conf_file << std::endl;
    return -1;
  }

  SDKConf sdk_conf;
  ret = baidu::paddle_serving::configure::read_proto_conf(
      output_dir, sdk_conf_file, &sdk_conf);
  if (ret != 0) {
    std::cout << "Read conf fail: " << sdk_conf_file << std::endl;
    return -1;
  }

  SigmoidConf sigmoid_conf;
  ret = baidu::paddle_serving::configure::read_proto_conf(
      output_dir, sigmoid_conf_file, &sigmoid_conf);
  if (ret != 0) {
    std::cout << "Read conf fail: " << sdk_conf_file << std::endl;
    return -1;
  }

  return 0;
}

int main() {
  int ret = 0;
  struct stat stat_buf;
  if (stat(output_dir, &stat_buf) != 0) {
    int ret = mkdir("./conf", 0777);
    if (ret != 0) {
      std::cout << "mkdir ./conf fail" << std::endl;
      return -1;
    }
    if (stat("./conf", &stat_buf) != 0) {
      std::cout << "./conf not exist and creating it failed" << std::endl;
      return -1;
    }
  }

  ret = test_write_conf();
  if (ret != 0) {
    std::cout << "test_write_conf fail" << std::endl;
    return -1;
  }

  std::cout << "test_write_conf success" << std::endl;

  ret = test_read_conf();
  if (ret != 0) {
    std::cout << "test_read_conf fail" << std::endl;
    return -1;
  }
  std::cout << "test_read_conf success" << std::endl;

  return 0;
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
