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
#include "core/predictor/framework/request_cache.h"

#include "core/predictor/common/inner_common.h"

#include "core/sdk-cpp/general_model_service.pb.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

using baidu::paddle_serving::predictor::general_model::Request;
using baidu::paddle_serving::predictor::general_model::Response;

RequestCache::RequestCache(const int64_t size)
    : cache_size_(size), used_size_(0) {
  bstop_ = false;
  thread_ptr_ = std::unique_ptr<std::thread>(
      new std::thread([this]() { this->ThreadLoop(); }));
}

RequestCache::~RequestCache() {
  bstop_ = true;
  condition_.notify_all();
  thread_ptr_->join();
}

RequestCache* RequestCache::GetSingleton() {
  static RequestCache cache(FLAGS_request_cache_size);
  return &cache;
}

int RequestCache::Hash(const Request& req, uint64_t* key) {
  uint64_t log_id = req.log_id();
  bool profile_server = req.profile_server();
  Request* r = const_cast<Request*>(&req);
  r->clear_log_id();
  r->clear_profile_server();
  std::string buf = req.SerializeAsString();
  *key = std::hash<std::string>{}(buf);
  r->set_log_id(log_id);
  r->set_profile_server(profile_server);
  return 0;
}

int RequestCache::Get(const Request& req, Response* res, uint64_t* key) {
  if (!Enabled()) {
    return -1;
  }
  uint64_t local_key = 0;
  Hash(req, &local_key);
  if (key != nullptr) {
    *key = local_key;
  }
  std::lock_guard<std::recursive_mutex> lk(cache_mtx_);
  auto iter = map_.find(local_key);
  if (iter == map_.end()) {
    LOG(INFO) << "key not found in cache";
    return -1;
  }
  auto entry = iter->second;
  BuildResponse(entry, res);
  UpdateLru(local_key);

  return 0;
}

int RequestCache::Put(const Request& req, const Response& res, uint64_t* key) {
  if (!Enabled()) {
    return -1;
  }
  uint64_t local_key = 0;
  if (key != nullptr && *key != 0) {
    local_key = *key;
  } else {
    Hash(req, &local_key);
  }
  if (key != nullptr) {
    *key = local_key;
  }

  AddTask(local_key, res);
  return 0;
}

int RequestCache::PutImpl(const Response& res, uint64_t key) {
  std::lock_guard<std::recursive_mutex> lk(cache_mtx_);
  auto iter = map_.find(key);
  if (iter != map_.end()) {
    LOG(WARNING) << "key[" << key << "] already exists in cache";
    return -1;
  }

  CacheEntry entry;
  if (BuildCacheEntry(res, &entry) != 0) {
    LOG(WARNING) << "key[" << key << "] build cache entry failed";
    return -1;
  }
  map_.insert({key, entry});
  UpdateLru(key);

  return 0;
}

int RequestCache::BuildResponse(const CacheEntry& entry,
                                predictor::general_model::Response* res) {
  if (res == nullptr) {
    return -1;
  }
  res->ParseFromString(entry.buf_);
  res->clear_profile_time();
  return 0;
}

int RequestCache::BuildCacheEntry(const Response& res, CacheEntry* entry) {
  if (entry == nullptr) {
    return -1;
  }
  std::lock_guard<std::recursive_mutex> lk(cache_mtx_);
  int size = res.ByteSize();
  if (size >= cache_size_) {
    LOG(INFO) << "res size[" << size << "] larger than cache_size["
              << cache_size_ << "]";
    return -1;
  }
  while (size > GetFreeCacheSize()) {
    if (RemoveOne() != 0) {
      LOG(ERROR) << "RemoveOne failed so can not build entry";
      return -1;
    }
  }
  entry->buf_ = res.SerializeAsString();
  used_size_ += size;
  return 0;
}

void RequestCache::UpdateLru(uint64_t key) {
  std::lock_guard<std::recursive_mutex> lk(cache_mtx_);
  auto lru_iter = std::find(lru_.begin(), lru_.end(), key);
  if (lru_iter != lru_.end()) {
    lru_.erase(lru_iter);
  }
  lru_.push_front(key);
}

bool RequestCache::Enabled() { return cache_size_ > 0; }

int64_t RequestCache::GetFreeCacheSize() { return cache_size_ - used_size_; }

int RequestCache::RemoveOne() {
  std::lock_guard<std::recursive_mutex> lk(cache_mtx_);
  uint64_t lru_key = lru_.back();
  VLOG(1) << "Remove key[" << lru_key << "] from cache";
  auto iter = map_.find(lru_key);
  if (iter == map_.end()) {
    LOG(ERROR) << "Remove key[" << lru_key << "] not find in cache";
    return -1;
  }
  auto entry = iter->second;
  used_size_ -= entry.buf_.size();
  map_.erase(iter);
  lru_.pop_back();

  return 0;
}

void RequestCache::ThreadLoop() {
  std::queue<std::pair<uint64_t, std::shared_ptr<Response>>> exec_task_queue;
  for (;;) {
    {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      condition_.wait(
          lock, [this]() { return this->bstop_ || this->task_queue_.size(); });

      if (!task_queue_.size()) {
        if (bstop_) {
          return;
        }
        continue;
      }
      swap(exec_task_queue, task_queue_);
    }
    while (!exec_task_queue.empty()) {
      auto [key, res_ptr] = exec_task_queue.front();
      exec_task_queue.pop();
      PutImpl(*res_ptr, key);
    }
  }
}

int RequestCache::AddTask(uint64_t key, const Response& res) {
  std::unique_lock<std::mutex> lock(queue_mutex_);
  std::shared_ptr<Response> res_ptr = std::make_shared<Response>(res);
  task_queue_.push(std::make_pair(key, res_ptr));
  condition_.notify_one();
  return 0;
}

bool RequestCache::Empty() {
  std::lock_guard<std::recursive_mutex> lk(cache_mtx_);
  return lru_.empty();
}

int RequestCache::Clear() {
  {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    std::queue<std::pair<uint64_t, std::shared_ptr<Response>>> empty;
    swap(empty, task_queue_);
  }
  int count = 0;
  {
    std::lock_guard<std::recursive_mutex> lk(cache_mtx_);
    count = lru_.size();
    lru_.clear();
    map_.clear();
  }
  LOG(INFO) << "Clear " << count << " key!";
  return 0;
}

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu