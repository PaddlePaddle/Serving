// Copyright (c) 2021 PaddlePaddle Authors. All Rights Reserved.
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
//
#pragma once

#include <atomic>
#include <condition_variable>
#include <list>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>

namespace baidu {
namespace paddle_serving {
namespace predictor {

namespace general_model {
class Request;
class Response;
}  // namespace general_model

struct CacheEntry {
  explicit CacheEntry() {}
  std::string buf_;
};

class RequestCache {
 public:
  explicit RequestCache(const int64_t size);
  ~RequestCache();

  static RequestCache* GetSingleton();

  int Hash(const predictor::general_model::Request& req, uint64_t* key);

  int Get(const predictor::general_model::Request& req,
          predictor::general_model::Response* res,
          uint64_t* key = nullptr);

  int Put(const predictor::general_model::Request& req,
          const predictor::general_model::Response& res,
          uint64_t* key = nullptr);

  void ThreadLoop();

  bool Empty();

  int Clear();

 private:
  int BuildResponse(const CacheEntry& entry,
                    predictor::general_model::Response* res);

  int BuildCacheEntry(const predictor::general_model::Response& res,
                      CacheEntry* entry);

  void UpdateLru(uint64_t key);

  bool Enabled();

  int64_t GetFreeCacheSize();

  int RemoveOne();

  int AddTask(uint64_t key, const predictor::general_model::Response& res);

  int PutImpl(const predictor::general_model::Response& res, uint64_t key);

  uint64_t cache_size_;
  uint64_t used_size_;
  std::unordered_map<uint64_t, CacheEntry> map_;
  std::list<uint64_t> lru_;
  std::recursive_mutex cache_mtx_;
  std::atomic<bool> bstop_{false};
  std::condition_variable condition_;
  std::mutex queue_mutex_;
  std::queue<
      std::pair<uint64_t, std::shared_ptr<predictor::general_model::Response>>>
      task_queue_;
  std::unique_ptr<std::thread> thread_ptr_;
};

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu