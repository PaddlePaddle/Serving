/***************************************************************************
 * 
 * Copyright (c) 2018 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
/**
 * @file test_bsf.h
 * @author root(com@baidu.com)
 * @date 2018/09/20 13:53:01
 * @brief 
 *  
 **/

#ifndef  BAIDU_PADDLE_SERVING_PREDICTOR_TEST_BSF_H
#define  BAIDU_PADDLE_SERVING_PREDICTOR_TEST_BSF_H

#include <gtest/gtest.h>
#include "common/inner_common.h"
#include "framework/bsf.h"

namespace baidu {
namespace paddle_serving {
namespace unittest {

#ifndef DEFINE_UP_DOWN  
#define DEFINE_UP_DOWN  \
    void SetUp() {}     \
    void TearDown() {}

struct TestItem {

    void auto_gen();

    bool operator==(const TestItem& other) {
        return text == other.text && id == other.id;
    }

    static void create(std::vector<TestItem>& in, std::vector<TestItem>& out, size_t size) {
        in.clear();
        out.clear();
        for (size_t i = 0; i < size; i++) {
            TestItem item;
            item.auto_gen();
            in.push_back(item);
            item.id += 1000000;
            out.push_back(item);
        }
    }

    std::string text;
    size_t id;
};

class TestBsf : public ::testing::Test {
public:
    TestBsf() {}
    virtual ~TestBsf() {}

    static void* task_trigger(void* arg) {
        for (size_t i = 0; i < 100; i++) {
            std::vector<TestItem> in;
            std::vector<TestItem> out;

            size_t count = rand() % 10 + 1;
            TestItem::create(in, out, count);

            im::bsf::TaskManager<TestItem, TestItem> task_manager;
            task_manager.schedule(in, out);
            printf("wait for bsf finish..., count:%d, first:%d\n", count, in[0].id);
            task_manager.wait();
            printf("bsf executed finished, count:%d, first:%d\n", count, in[0].id);
            EXPECT_EQ(out.size(), count);
            for (size_t i = 0; i < out.size(); i++) {
                EXPECT_EQ(in[i].id, out[i].id);
                char temp[128];
                snprintf(temp, sizeof(temp), "test-%d", in[i].id);
                EXPECT_STREQ(temp, in[i].text.c_str());
                EXPECT_STREQ(temp, out[i].text.c_str());
            }
        }
    }

    DEFINE_UP_DOWN
};

#undef DEFINE_UP_DOWN
#endif

}
}
}

#endif  //BAIDU_PADDLE_SERVING_PREDICTOR_TEST_FTL_H

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
