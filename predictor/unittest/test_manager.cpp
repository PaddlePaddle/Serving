#include "test_tool.h"
#include "test_manager.h"
#include "framework/manager.h"
#include "framework/service.h"

namespace baidu {
namespace paddle_serving {
namespace unittest {

using baidu::paddle_serving::predictor::Manager;
using baidu::paddle_serving::predictor::InferService;
using baidu::paddle_serving::predictor::ParallelInferService;
using baidu::paddle_serving::predictor::FLAGS_use_parallel_infer_service;
using baidu::paddle_serving::predictor::InferServiceManager;

struct ManagerItem {
    int a; 
    float b;

    int init(const comcfg::ConfigUnit& c) {
        return 0;
    }

    static const char* tag() {
        return "Item";
    }
};

TEST_F(TestManager, test_manager_instance) {
    ManagerItem* item = Manager<ManagerItem>::instance().create_item();
    EXPECT_FALSE(item == NULL);
    item->a = 1;
    item->b = 2.0;
}

TEST_F(TestManager, test_infer_service_create) {
    InferService seq;
    ParallelInferService par;

    FLAGS_use_parallel_infer_service = false;
    EXPECT_EQ(typeid(seq),
            typeid(*InferServiceManager::instance().create_item()));

    FLAGS_use_parallel_infer_service = true;
    EXPECT_EQ(typeid(par), 
            typeid(*InferServiceManager::instance().create_item()));
}

TEST_F(TestManager, test_conf_success) {
    const char* conf_content = 
        "[@Item]\n\
        name: item1\n\
        a:b\n\
        [@Item]\n\
        name: item2\n\
        c:d";

    AutoTempFile file(conf_content);

    typedef Manager<ManagerItem> mgr;
    EXPECT_EQ(mgr::instance().initialize("./", file.name()), 0);

    ManagerItem* item11 = mgr::instance().item("item1");
    ManagerItem* item12 = &mgr::instance()["item1"];
    EXPECT_EQ(item11, item12);

    ManagerItem* item21 = mgr::instance().item("item2");
    ManagerItem* item22 = &mgr::instance()["item2"];
    EXPECT_EQ(item21, item22);
}

TEST_F(TestManager, test_conf_success_item_not_found) {
    const char* conf_content = 
        "[@Item1]\n\
        name: item1\n\
        a:b\n\
        [@Item2]\n\
        name: item2\n\
        c:d";

    AutoTempFile file(conf_content);

    typedef Manager<ManagerItem> mgr;
    EXPECT_EQ(mgr::instance().initialize("./", file.name()), 0);
}

TEST_F(TestManager, test_conf_failed_name_not_found) {
    const char* conf_content = 
        "[@Item]\n\
        name2: item1\n\
        a:b\n\
        [@Item]\n\
        name: item2\n\
        c:d";

    AutoTempFile file(conf_content);

    typedef Manager<ManagerItem> mgr;
    EXPECT_EQ(mgr::instance().initialize("./", file.name()), -1);
}

}
}
}
