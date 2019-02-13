#include <iostream>
#include <fstream>
#include <bthread_unstable.h> // bthread_set_worker_startfn
#include "common/inner_common.h"
#include "framework/workflow.h"
#include "framework/service.h"
#include "framework/manager.h"
#include "framework/server.h"
#include "framework/logger.h"
#include "framework/resource.h"
#include "common/constant.h"

using baidu::paddle_serving::predictor::ServerManager;
using baidu::paddle_serving::predictor::WorkflowManager;
using baidu::paddle_serving::predictor::LoggerWrapper;
using baidu::paddle_serving::predictor::InferServiceManager;
using baidu::paddle_serving::predictor::Resource;
using baidu::paddle_serving::predictor::FLAGS_workflow_path;
using baidu::paddle_serving::predictor::FLAGS_workflow_file;
using baidu::paddle_serving::predictor::FLAGS_inferservice_path;
using baidu::paddle_serving::predictor::FLAGS_inferservice_file;
using baidu::paddle_serving::predictor::FLAGS_logger_path;
using baidu::paddle_serving::predictor::FLAGS_logger_file;
using baidu::paddle_serving::predictor::FLAGS_resource_path;
using baidu::paddle_serving::predictor::FLAGS_resource_file;
using baidu::paddle_serving::predictor::FLAGS_reload_interval_s;
using baidu::paddle_serving::predictor::FLAGS_port;

void print_revision(std::ostream& os, void*) {
#if defined(PDSERVING_VERSION)
    os << PDSERVING_VERSION;
#else
    os << "undefined";
#endif
#if defined(PDSERVING_BUILDTIME)
    os << ", BuildAt: " << PDSERVING_BUILDTIME;
#endif
}

static bvar::PassiveStatus<std::string> s_predictor_revision(
        "predictor_revision", print_revision, NULL);

DEFINE_bool(v, false, "print version, bool");
DEFINE_bool(g, false, "user defined gflag path");
DECLARE_string(flagfile);

void pthread_worker_start_fn() {
    Resource::instance().thread_initialize();
}

static void g_change_server_port() {
    comcfg::Configure conf;
    if (conf.load(FLAGS_inferservice_path.c_str(), FLAGS_inferservice_file.c_str()) != 0) {
        LOG(WARNING) << "failed to load configure[" << FLAGS_inferservice_path
                << "," << FLAGS_inferservice_file << "].";
        return;
    }
    uint32_t port = 0;
    int err = conf["port"].get_uint32(&port, 0);
    if (err == 0) {
        FLAGS_port = port;
        LOG(INFO) << "use configure[" << FLAGS_inferservice_path << "/"
            << FLAGS_inferservice_file << "] port[" << port << "] instead of flags";
    }
    return;
}

#ifdef UNIT_TEST
int ut_main(int argc, char** argv) {
#else 
int main(int argc, char** argv) {
#endif
    google::ParseCommandLineFlags(&argc, &argv, true);

    if (FLAGS_v) {
        print_revision(std::cout, NULL);
        std::cout << std::flush;
        return 0;
    }

    if (!FLAGS_g) {
        google::SetCommandLineOption("flagfile", "conf/gflags.conf");
    }
    
    google::ParseCommandLineFlags(&argc, &argv, true);
    
    g_change_server_port();

    // initialize logger instance
    if (LoggerWrapper::instance().initialize(
                FLAGS_logger_path, FLAGS_logger_file) != 0) {
        LOG(ERROR) << "Failed initialize logger, conf:" 
            << FLAGS_logger_path << "/" << FLAGS_logger_file;
        return -1;
    }

    LOG(INFO) << "Succ initialize logger";

    // initialize resource manager
    if (Resource::instance().initialize(
                FLAGS_resource_path, FLAGS_resource_file) != 0) {
        LOG(ERROR) << "Failed initialize resource, conf:" 
            << FLAGS_resource_path << "/" << FLAGS_resource_file;
        return -1; 
    }
    LOG(INFO) << "Succ initialize resource";

    // initialize workflow manager
    if (WorkflowManager::instance().initialize(
            FLAGS_workflow_path, FLAGS_workflow_file) != 0) {
        LOG(ERROR) << "Failed initialize workflow manager, conf:" 
            << FLAGS_workflow_path << "/" << FLAGS_workflow_file;
        return -1;
    }
    LOG(INFO) << "Succ initialize workflow";

    // initialize service manager
    if (InferServiceManager::instance().initialize(
            FLAGS_inferservice_path, FLAGS_inferservice_file) != 0) {
        LOG(ERROR) 
            << "Failed initialize infer service manager, conf:" 
            << FLAGS_inferservice_path << "/" << FLAGS_inferservice_file;
        return -1;
    }
    LOG(INFO) << "Succ initialize inferservice";

    int errcode = bthread_set_worker_startfn(pthread_worker_start_fn);
    if (errcode != 0) {
        LOG(FATAL) << "Failed call pthread worker start function, error_code[" << errcode << "]";
        return -1;
    }
    LOG(INFO) << "Succ call pthread worker start function";

    if (ServerManager::instance().start_and_wait() != 0) {
        LOG(ERROR) << "Failed start server and wait!";
        return -1;
    }
    LOG(INFO) << "Succ start service manager";

    if (InferServiceManager::instance().finalize() != 0) {
        LOG(ERROR) << "Failed finalize infer service manager.";
    }

    if (WorkflowManager::instance().finalize() != 0) {
        LOG(ERROR) << "Failed finalize workflow manager";
    }

    if (Resource::instance().finalize() != 0) {
        LOG(ERROR) << "Failed finalize resource manager";
    }

    if (LoggerWrapper::instance().finalize() != 0) {
        LOG(ERROR) << "Failed finalize logger wrapper";
    }

    LOG(INFO) << "Paddle Inference Server exit successfully!";
    return 0;
}
