/***************************************************************************
 * 
 * Copyright (c) 2018 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
/**
 * @file test_bsf.cpp
 * @author root(com@baidu.com)
 * @date 2018/09/20 13:54:52
 * @brief 
 *  
 **/

#include "test_bsf.h"

namespace baidu {
namespace paddle_serving {
namespace unittest {

butil::atomic<size_t> global_id;

void TestItem::auto_gen() {
    id = global_id.fetch_add(1);
    char buf[128];
    snprintf(buf, sizeof(buf), "test-%d", id);
    text = buf;
    printf("id:%d,text:%s\n", id, text.c_str());
}

void work(const std::vector<TestItem>& in, std::vector<TestItem>& out) {
    for (size_t i = 0; i < in.size(); ++i) {
        out[i] = in[i];
        usleep(50);
    }
}

TEST_F(TestBsf, test_single_thread) {
    // initialize TaskExecutor
    global_id.store(0, butil::memory_order_relaxed);
    im::bsf::TaskExecutor<im::bsf::Task<TestItem, TestItem> >::instance()->set_thread_callback_fn(
            boost::bind(&work, _1, _2));
    EXPECT_EQ((im::bsf::TaskExecutor<im::bsf::Task<TestItem, TestItem> >::instance()->start(1)), 0);
    
    std::vector<TestItem> in;
    std::vector<TestItem> out;

    TestItem::create(in, out, 5);

    im::bsf::TaskManager<TestItem, TestItem> task_manager;
    task_manager.schedule(in, out);
    printf("wait for bsf finish...\n");
    task_manager.wait();
    printf("bsf executed finished\n");
    ASSERT_EQ(out.size(), 5);
    for (size_t i = 0; i < out.size(); i++) {
        char temp[128];
        snprintf(temp, sizeof(temp), "test-%d", i);
        EXPECT_EQ(i, in[i].id);
        EXPECT_EQ(i, out[i].id);
        EXPECT_STREQ(temp, in[i].text.c_str());
        EXPECT_STREQ(temp, out[i].text.c_str());
    }

    im::bsf::TaskExecutor<im::bsf::Task<TestItem, TestItem> >::instance()->stop();
}

TEST_F(TestBsf, test_multi_thread) {
    // initialize TaskExecutor
    global_id.store(0, butil::memory_order_relaxed);
    im::bsf::TaskExecutor<im::bsf::Task<TestItem, TestItem> >::instance()->set_thread_callback_fn(
            boost::bind(&work, _1, _2));
    im::bsf::TaskExecutor<im::bsf::Task<TestItem, TestItem> >::instance()->set_batch_size(100);
    EXPECT_EQ((im::bsf::TaskExecutor<im::bsf::Task<TestItem, TestItem> >::instance()->start(3)), 0);
    
    size_t psize = 5;
    pthread_t pid[psize];
    for (size_t i = 0; i < psize; ++i) {
        pthread_create(&pid[i], NULL, &TestBsf::task_trigger, NULL);
    }

    for (size_t i = 0; i < psize; ++i) {
        pthread_join(pid[i], NULL);
    }

    im::bsf::TaskExecutor<im::bsf::Task<TestItem, TestItem> >::instance()->stop();
}

}
}
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
