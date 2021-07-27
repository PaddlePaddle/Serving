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

#include "core/predictor/mempool/mempool.h"

namespace im {

__thread Mempool* g_mempool = NULL;

namespace fugue {

namespace memory {

void Region::init() {
  _big_mem_capacity = 32 * 1024 * 1024;
  _big_mem_start = new char[_big_mem_capacity];
}

void Region::reset() {
  // release memory allocate from GlobalMempool
  _free_blocks.unsafe_foreach<GlobalPut>();
  _free_blocks.reset();

  // release memory from malloc
  BigNode* head = _big_nodes.release();
  while (head) {
    BigNode* next = head->next;
    ::free(head);
    head = next;
  }
  _mlc_mem_size.store(0, butil::memory_order_relaxed);
  _mlc_mem_count.store(0, butil::memory_order_relaxed);

  // clear the large buffer
  _big_mem_size.store(0, butil::memory_order_relaxed);
  _big_mem_count.store(0, butil::memory_order_relaxed);
}

BlockReference* Region::get() {
  BlockReference* ref = _free_blocks.get();
  if (ref->block == NULL) {
    ref->offset = 0;
    ref->block = GlobalBlockFreeList::instance()->get();
  }
  return ref;
}

void Region::put(BlockReference* block) { _free_blocks.put(block); }

void* Region::malloc(size_t size) {
  if (size < MLC_MEM_THRESHOLD) {
    uint32_t offset =
        _big_mem_size.fetch_add(size, butil::memory_order_relaxed);
    if (offset + size < _big_mem_capacity) {
      _big_mem_count.fetch_add(1, butil::memory_order_relaxed);
      return _big_mem_start + offset;
    }
  }

  _mlc_mem_size.fetch_add(size, butil::memory_order_relaxed);
  _mlc_mem_count.fetch_add(1, butil::memory_order_relaxed);
  BigNode* node = reinterpret_cast<BigNode*>(::malloc(sizeof(BigNode) + size));
  _big_nodes.push(node);
  return node->data;
}

Region::Region() {
  _big_mem_size.store(0, butil::memory_order_relaxed);
  _big_mem_count.store(0, butil::memory_order_relaxed);

  _big_mem_start = NULL;
  _big_mem_capacity = 0;

  _mlc_mem_size.store(0, butil::memory_order_relaxed);
  _mlc_mem_count.store(0, butil::memory_order_relaxed);
}
}  // namespace memory
}  // namespace fugue
}  // namespace im
