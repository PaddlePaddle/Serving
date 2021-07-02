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

// `g_mempool` this is not used at all
__thread Mempool* g_mempool = NULL;

namespace fugue {

namespace memory {

void Region::init() {
  _big_mem_capacity = 128 * 1024 * 1024;  // 128MB
  _big_mem_start = new char[_big_mem_capacity];
}

void Region::reset() {
  // return the Block memory borrow from BlockFreeList
  _blockReference_FreeList.unsafe_foreach<PutBlockByReference>();
  _blockReference_FreeList.reset();

  // release BigNode memory
  BigNode* head = _bigNode_Stack.releaseAndGetHeadPtr();
  while (head) {
    BigNode* next = head->next;
    ::free(head);
    head = next;
  }
  _total_bigNode_size.store(0, butil::memory_order_relaxed);
  _total_bigNode_count.store(0, butil::memory_order_relaxed);

  // clear the large buffer, but don`t release it.
  // it will be deleted in the deconstruction.
  _big_mem_size.store(0, butil::memory_order_relaxed);
  _big_mem_count.store(0, butil::memory_order_relaxed);
}

BlockReference* Region::get() {
  // the first time, it will be null
  // after you call put(), it won`t be null.
  BlockReference* ref = _blockReference_FreeList.get();
  if (ref->block == NULL) {
    ref->offset = 0;
    ref->block = BlockFreeList::instance()->get();
  }
  return ref;
}

// this will not return the Block to the BlockFreeList
// it just return to the _blockReference_FreeList.
// next time when you call get(), you will get the BlockReference* head (which
// is just put by yourself)
void Region::put(BlockReference* blockReference) {
  _blockReference_FreeList.put(blockReference);
}

void* Region::malloc(size_t size) {
  if (size < BIGNODE_MEM_THRESHOLD) {
    uint32_t offset =
        _big_mem_size.fetch_add(size, butil::memory_order_relaxed);
    if (offset + size < _big_mem_capacity) {
      _big_mem_count.fetch_add(1, butil::memory_order_relaxed);
      return _big_mem_start + offset;
    }
  }

  // if size>= BIGNODE_MEM_THRESHOLD or the _big_mem_capacity is used up.
  _total_bigNode_size.fetch_add(size, butil::memory_order_relaxed);
  _total_bigNode_count.fetch_add(1, butil::memory_order_relaxed);
  BigNode* node = reinterpret_cast<BigNode*>(::malloc(sizeof(BigNode) + size));
  _bigNode_Stack.push(node);
  return node->data;
}

Region::Region() {
  _big_mem_size.store(0, butil::memory_order_relaxed);
  _big_mem_count.store(0, butil::memory_order_relaxed);
  _big_mem_start = NULL;
  _big_mem_capacity = 0;

  _total_bigNode_size.store(0, butil::memory_order_relaxed);
  _total_bigNode_count.store(0, butil::memory_order_relaxed);
}
}  // namespace memory
}  // namespace fugue
}  // namespace im
