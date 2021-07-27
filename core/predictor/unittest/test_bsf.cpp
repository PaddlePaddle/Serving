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

#include "core/predictor/unittest/test_bsf.h"
#include <vector>

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

void work(const std::vector<TestItem>& in,
          std::vector<TestItem>& out) {  // NOLINT
  for (size_t i = 0; i < in.size(); ++i) {
    out[i] = in[i];
    usleep(50);
  }
}

TEST_F(TestBsf, test_single_thread) {
  // initialize TaskExecutor
  global_id.store(0, butil::memory_order_relaxed);
  im::bsf::TaskExecutor<im::bsf::Task<TestItem, TestItem>>::instance()
      ->set_thread_callback_fn(boost::bind(&work, _1, _2));
  EXPECT_EQ(
      (im::bsf::TaskExecutor<im::bsf::Task<TestItem, TestItem>>::instance()
           ->start(1)),
      0);

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

  im::bsf::TaskExecutor<im::bsf::Task<TestItem, TestItem>>::instance()->stop();
}

TEST_F(TestBsf, test_multi_thread) {
  // initialize TaskExecutor
  global_id.store(0, butil::memory_order_relaxed);
  im::bsf::TaskExecutor<im::bsf::Task<TestItem, TestItem>>::instance()
      ->set_thread_callback_fn(boost::bind(&work, _1, _2));
  im::bsf::TaskExecutor<im::bsf::Task<TestItem, TestItem>>::instance()
      ->set_batch_size(100);
  EXPECT_EQ(
      (im::bsf::TaskExecutor<im::bsf::Task<TestItem, TestItem>>::instance()
           ->start(3)),
      0);

  const size_t psize = 5;
  std::unique_ptr<pthread_t*> pid;
  pid.reset(new pthread_t[psize]);
  for (size_t i = 0; i < psize; ++i) {
    pthread_create(&pid[i], NULL, &TestBsf::task_trigger, NULL);
  }

  for (size_t i = 0; i < psize; ++i) {
    pthread_join(pid[i], NULL);
  }

  im::bsf::TaskExecutor<im::bsf::Task<TestItem, TestItem>>::instance()->stop();
}
}  // namespace unittest
}  // namespace paddle_serving
}  // namespace baidu

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
