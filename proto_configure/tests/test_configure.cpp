#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include "configure.pb.h"
#include "configure_parser.h"

using baidu::paddle_serving::configure::EngineDesc;
using baidu::paddle_serving::configure::VersionedEngine;
using baidu::paddle_serving::configure::ModelToolkitConf;

using baidu::paddle_serving::configure::ResourceConf;

using baidu::paddle_serving::configure::DAGNodeDependency;
using baidu::paddle_serving::configure::DAGNode;
using baidu::paddle_serving::configure::Workflow;
using baidu::paddle_serving::configure::WorkflowConf;

using baidu::paddle_serving::configure::InferService;
using baidu::paddle_serving::configure::InferServiceConf;

const std::string output_dir = "./conf/";
const std::string model_toolkit_conf_file = "model_toolkit.prototxt";
const std::string resource_conf_file = "resource.prototxt";
const std::string workflow_conf_file = "workflow.prototxt";
const std::string service_conf_file = "service.prototxt";

int test_write_conf()
{
    // model_toolkit conf
    ModelToolkitConf model_toolkit_conf;

    // This engine has a default version
    VersionedEngine *engine = model_toolkit_conf.add_engines();
    engine->set_name("image_classification_resnet");
    EngineDesc *engine_desc = engine->mutable_default_version();
    engine_desc->set_type("FLUID_CPU_NATIVE_V2");
    engine_desc->set_reloadable_meta("./data/model/paddle/fluid_time_file");
    engine_desc->set_reloadable_type("timestamp_ne");
    engine_desc->set_model_data_path("./data/model/paddle/fluid/SE_ResNeXt50_32x4d");
    engine_desc->set_runtime_thread_num(0);
    engine_desc->set_batch_infer_size(0);
    engine_desc->set_enable_batch_align(0);

    // This engine has two versioned branches
    engine = model_toolkit_conf.add_engines();
    engine->set_name("image_classification_resnet_versioned");
    // Version 1
    engine_desc = engine->add_versions();
    engine_desc->set_type("FLUID_CPU_NATIVE_DIR");
    engine_desc->set_reloadable_meta("./data/model/paddle/fluid_time_file");
    engine_desc->set_reloadable_type("timestamp_ne");
    engine_desc->set_model_data_path("./data/model/paddle/fluid/SE_ResNeXt50_32x4d");
    engine_desc->set_runtime_thread_num(0);
    engine_desc->set_batch_infer_size(0);
    engine_desc->set_enable_batch_align(0);
    // Version 2
    engine_desc = engine->add_versions();
    engine_desc->set_type("FLUID_CPU_NATIVE_DIR");
    engine_desc->set_reloadable_meta("./data/model/paddle/fluid_time_file_2");
    engine_desc->set_reloadable_type("timestamp_ne_2");
    engine_desc->set_model_data_path("./data/model/paddle/fluid/SE_ResNeXt50_32x4d_2");
    engine_desc->set_runtime_thread_num(0);
    engine_desc->set_batch_infer_size(0);
    engine_desc->set_enable_batch_align(0);

    int ret = baidu::paddle_serving::configure::write_proto_conf(&model_toolkit_conf, output_dir, model_toolkit_conf_file);
    if (ret != 0) {
        return ret;
    }

    // resource conf
    ResourceConf resource_conf;
    resource_conf.set_model_toolkit_path(output_dir);
    resource_conf.set_model_toolkit_file("resource.prototxt");
    ret = baidu::paddle_serving::configure::write_proto_conf(&resource_conf, output_dir, resource_conf_file);
    if (ret != 0) {
        return ret;
    }

    // workflow entries conf
    WorkflowConf workflow_conf;
    Workflow *workflow = workflow_conf.add_workflow();
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

    workflow = workflow_conf.add_workflow();
    workflow->set_name("workflow2");
    workflow->set_workflow_type("Sequence");

    dag_node = workflow->add_nodes();
    dag_node->set_name("dense_op");
    dag_node->set_type("DenseOp");

    ret = baidu::paddle_serving::configure::write_proto_conf(&workflow_conf, output_dir, workflow_conf_file);
    if (ret != 0) {
        return ret;
    }

    InferServiceConf infer_service_conf;
    InferService *infer_service = infer_service_conf.add_service();
    infer_service->set_name("ImageClassifyService");
    infer_service->add_workflow("workflow1");
    infer_service->add_workflow("workflow2");

    infer_service = infer_service_conf.add_service();
    infer_service->set_name("BuiltinDenseFormatService");
    infer_service->add_workflow("workflow2");

    ret = baidu::paddle_serving::configure::write_proto_conf(&infer_service_conf, output_dir, service_conf_file);
    if (ret != 0) {
        return ret;
    }
    return 0;
}

int test_read_conf()
{
    int ret = 0;

    ModelToolkitConf model_toolkit_conf;
    ret = baidu::paddle_serving::configure::read_proto_conf(output_dir, model_toolkit_conf_file, &model_toolkit_conf);
    if (ret != 0) {
        std::cout << "Read conf fail: " << model_toolkit_conf_file << std::endl;
        return -1;
    }

    ResourceConf resource_conf;
    ret = baidu::paddle_serving::configure::read_proto_conf(output_dir, resource_conf_file, &resource_conf);
    if (ret != 0) {
        std::cout << "Read conf fail: " << resource_conf_file << std::endl;
        return -1;
    }

    WorkflowConf workflow_conf;
    ret = baidu::paddle_serving::configure::read_proto_conf(output_dir, workflow_conf_file, &workflow_conf);
    if (ret != 0) {
        std::cout << "Read conf fail: " << workflow_conf_file << std::endl;
        return -1;
    }

    InferServiceConf service_conf;
    ret = baidu::paddle_serving::configure::read_proto_conf(output_dir, service_conf_file, &service_conf);
    if (ret != 0) {
        std::cout << "Read conf fail: " << service_conf_file << std::endl;
        return -1;
    }

    return 0;
}

int main()
{
    int ret = 0;
    struct stat stat_buf;
    if (stat(output_dir.c_str(), &stat_buf) != 0) {
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
