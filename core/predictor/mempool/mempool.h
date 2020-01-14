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

#pragma once

#ifdef BCLOUD
#include <base/atomicops.h>
#include <base/logging.h>
#else
#include <butil/atomicops.h>
#include <butil/logging.h>
#endif

#include <execinfo.h>
#include <pthread.h>
#include <iostream>
#include <new>
#include <sstream>
#include <string>

namespace im {
namespace fugue {

#ifdef BCLOUD
namespace butil = base;
#endif

namespace lockfree {

template <class T>
class PushOnlyStack {
 public:
  PushOnlyStack() { _head.store(NULL, butil::memory_order_relaxed); }

  void push(T* node) {
    T* head = _head.load(butil::memory_order_relaxed);
    node->next = head;
    while (
        !_head.compare_exchange_weak(head, node, butil::memory_order_relaxed)) {
      node->next = head;
    }
  }

  T* release() { return _head.exchange(NULL, butil::memory_order_relaxed); }

 private:
  butil::atomic<T*> _head;
};

template <class T>
struct FreeListNode {
  uint64_t id;
  uint64_t next;
  T data;
};

template <class T, int CAP>
class FreeList {
 public:
  typedef FreeListNode<T> Node;
  static const uint64_t EMPTY = 0xFFFFFFFFFFFFFFFF;

  T* get() {
    uint64_t head = _head.load(butil::memory_order_acquire);
    if (head == EMPTY) {
      return new_node();
    }

    Node* node = address(head);
    while (!_head.compare_exchange_weak(
        head, node->next, butil::memory_order_acquire)) {
      if (head == EMPTY) {
        return new_node();
      }
      node = address(head);
    }
    return &node->data;
  }

  void put(T* value) {
    Node* node = container_of(value, Node, data);

    uint64_t head = _head.load(butil::memory_order_acquire);
    // add version
    node->id += (1UL << 32);
    node->next = head;

    // NOTE: we MUST use a temp var *head* to call compare_exchange_weak
    // because Boost.Atomic will update the *expected* even success
    // std::atomic do not have this limitation
    while (!_head.compare_exchange_weak(
        head, node->id, butil::memory_order_release)) {
      node->next = head;
    }
  }

  template <class F>
  void unsafe_foreach() {
    uint32_t used_blk_cnt = _slot_index.load(butil::memory_order_relaxed);
    for (uint32_t i = 0; i < used_blk_cnt; ++i) {
      F()(&_node[i]->data);
    }
  }

  uint32_t real_used_size() const {
    uint32_t used_blk_cnt = _slot_index.load(butil::memory_order_relaxed);
    uint64_t used_bytes = 0;
    for (uint32_t i = 0; i < used_blk_cnt; ++i) {
      used_bytes += _node[i]->data.offset;
    }
    return used_bytes >> 10;
  }

  uint32_t allocate_blocks() const {
    return _slot_index.load(butil::memory_order_relaxed);
  }

  uint32_t free_blocks() const {
    uint64_t head = _head.load(butil::memory_order_relaxed);
    uint32_t size = 0;
    while (head != FreeList::EMPTY) {
      const Node* head_ptr = address(head);
      head = head_ptr->next;
      ++size;
    }
    return size;
  }

  void reset() {
    _head.store(FreeList::EMPTY, butil::memory_order_relaxed);
    _slot_index.store(0, butil::memory_order_relaxed);
  }

  FreeList() {
    for (int i = 0; i < CAP; ++i) {
      _node[i] = NULL;
    }
    reset();
  }

 private:
  uint32_t slot(uint64_t id) const { return static_cast<uint32_t>(id); }

  T* new_node() {
    uint32_t index = _slot_index.fetch_add(1, butil::memory_order_relaxed);
    if (index >= CAP) {
      return NULL;
    }

    if (_node[index] != NULL) {
      return &(_node[index]->data);
    }

    Node* node = reinterpret_cast<Node*>(malloc(sizeof(Node)));
    new (node) Node;

    node->id = index;
    _node[index] = node;

    return &node->data;
  }

  Node* address(uint64_t id) { return _node[slot(id)]; }

  const Node* address(uint64_t id) const { return _node[slot(id)]; }

  butil::atomic<uint64_t> _head;
  butil::atomic<uint32_t> _slot_index;
  Node* _node[CAP];
};
}  // namespace lockfree

namespace memory {

struct Block {
  static const int BLOCK_SIZE = 2 * 1024 * 1024;
  char data[BLOCK_SIZE];
};

class GlobalBlockFreeList {
 public:
  static const int MAX_BLOCK_COUNT = 32 * 1024;
  typedef lockfree::FreeList<Block, MAX_BLOCK_COUNT> type;
  static type* instance() {
    static type singleton;
    return &singleton;
  }
};

struct BlockReference {
  BlockReference() : offset(0), block(NULL) {
    // do nothing
  }

  void reset() {
    offset = 0;
    block = NULL;
  }

  uint32_t offset;
  Block* block;
};

class Region {
 public:
  struct GlobalPut {
    void operator()(BlockReference* block_ref) {
      if (block_ref->block != NULL) {
        GlobalBlockFreeList::instance()->put(block_ref->block);
      }
      block_ref->reset();
    }
  };

  struct BigNode {
    BigNode* next;
    char data[0];
  };

  ~Region() {
    reset();
    delete[] _big_mem_start;
    _big_mem_start = NULL;
  }

  char const* debug_str() const {
    uint32_t alloc_blocks = _free_blocks.allocate_blocks();
    uint32_t free_blocks = _free_blocks.free_blocks();
    uint32_t used_mem_mb = _free_blocks.real_used_size();
    uint32_t big_buf_size = _big_mem_size.load(butil::memory_order_relaxed);
    uint32_t big_buf_count = _big_mem_count.load(butil::memory_order_relaxed);
    uint32_t mlc_mem_size = _mlc_mem_size.load(butil::memory_order_relaxed);
    uint32_t mlc_mem_count = _mlc_mem_count.load(butil::memory_order_relaxed);

    std::ostringstream oss;
    oss << "[alloc_blks:" << alloc_blocks << ",free_blks:" << free_blocks
        << ",used_mem_kb:" << used_mem_mb
        << ",big_mem_kb:" << (big_buf_size >> 10)
        << ",big_buf_cnt:" << big_buf_count
        << ",mlc_mem_kb:" << (mlc_mem_size >> 10)
        << ",mlc_cnt:" << mlc_mem_count << "]";

    return oss.str().c_str();
  }

  Region();

  void init();

  void reset();

  BlockReference* get();

  void* malloc(size_t size);

  void put(BlockReference* block);

  static const int MAX_BLOCK_COUNT = 1024;
  static const int BIG_MEM_THRESHOLD = 256 * 1024;
  static const int MLC_MEM_THRESHOLD = 4 * 1024 * 1024;
  static const int COUNTER_SIZE = MLC_MEM_THRESHOLD / BIG_MEM_THRESHOLD + 1;

 private:
  lockfree::FreeList<BlockReference, MAX_BLOCK_COUNT> _free_blocks;
  lockfree::PushOnlyStack<BigNode> _big_nodes;

  butil::atomic<uint32_t> _big_mem_size;
  butil::atomic<uint32_t> _big_mem_count;

  char* _big_mem_start;
  uint32_t _big_mem_capacity;

  butil::atomic<uint32_t> _mlc_mem_size;
  butil::atomic<uint32_t> _mlc_mem_count;
};
}  // namespace memory
}  // namespace fugue

class Mempool {
 public:
  void* malloc(size_t size) {
    size = _align(size);
    if (size <= _free_size) {
      void* p = _free_cursor;
      _free_size -= size;
      _free_cursor += size;
      return p;
    }

    return malloc_from_region(size);
  }

  void free(void* p, size_t size) {
    if (size >= fugue::memory::Region::BIG_MEM_THRESHOLD) {
      return;
    }

    if (_free_cursor - size == static_cast<char*>(p)) {
      size_t down_aligned = _down_align(size);
      _free_cursor -= down_aligned;
      _free_size += down_aligned;
    }
  }

  void* realloc(void* old_data, size_t old_size, size_t new_size) {
    if (old_size >= new_size) {
      return old_data;
    }

    size_t required = new_size - old_size;
    if (_free_cursor == static_cast<char*>(old_data) + old_size) {
      if (_free_size >= required) {
        _free_cursor += required;
        _free_size -= required;
        return old_data;
      } else {
        _free_cursor = static_cast<char*>(old_data);
        _free_size += old_size;
      }
    }

    void* p = this->malloc_from_region(new_size);
    if (p != NULL) {
      memcpy(p, old_data, old_size);
      return p;
    }

    return NULL;
  }

  explicit Mempool(fugue::memory::Region* blocks)
      : _free_size(0), _free_cursor(NULL), _blocks(blocks) {
    _block = NULL;
  }

  ~Mempool() { release_block(); }

  void release_block() {
    if (_block) {
      _block->offset = fugue::memory::Block::BLOCK_SIZE - _free_size;
      _blocks->put(_block);
    }

    _free_size = 0;
    _free_cursor = NULL;
    _block = NULL;
  }

 private:
  void* malloc_from_region(size_t size) {
    if (size >= fugue::memory::Region::BIG_MEM_THRESHOLD) {
      return _blocks->malloc(size);
    }

    while (true) {
      fugue::memory::BlockReference* block = _blocks->get();
      if (block == NULL) {
        return NULL;
      }

      uint32_t free_size = fugue::memory::Block::BLOCK_SIZE - block->offset;
      if (size <= free_size) {
        if (_block) {
          _block->offset = fugue::memory::Block::BLOCK_SIZE - _free_size;
        }

        char* p = block->block->data + block->offset;
        _free_size = free_size - size;
        _free_cursor = p + size;
        _block = block;
        return p;
      }
    }
    return _blocks->malloc(size);
  }

  static const int ALIGN_SIZE = sizeof(void*);

  inline size_t _align(size_t size) const {
    return (size + (ALIGN_SIZE - 1)) & ~(ALIGN_SIZE - 1);
  }

  inline size_t _down_align(size_t size) const {
    return size & ~(ALIGN_SIZE - 1);
  }

  size_t _free_size;
  char* _free_cursor;

  fugue::memory::Region* _blocks;
  fugue::memory::BlockReference* _block;
};

extern __thread Mempool* g_mempool;
class mempool {
 public:
  virtual void* malloc(size_t size) = 0;
  virtual void free(void* p, size_t size) = 0;
  inline virtual ~mempool() {}
};

class GlobalMempool : public mempool {
 public:
  GlobalMempool() {
    // do nothing;
  }

  virtual ~GlobalMempool() {
    // do nothing;
  }

  static GlobalMempool* instance() {
    static GlobalMempool singleton;
    return &singleton;
  }

  void reset(Mempool* mempool) { g_mempool = mempool; }

  void* malloc(size_t size) { return g_mempool->malloc(size); }

  void* realloc(void* old_data, size_t old_size, size_t new_size) {
    return g_mempool->realloc(old_data, old_size, new_size);
  }

  void free(void* p, size_t s) { g_mempool->free(p, s); }

  void clear() { g_mempool->release_block(); }

  Mempool* get() { return g_mempool; }
};

class MempoolGuard {
 public:
  explicit MempoolGuard(fugue::memory::Region* region) : _mempool(region) {
    acquire();
  }

  void acquire() {
    _saved_mempool = g_mempool;
    g_mempool = &_mempool;
  }

  void release() {
    _mempool.release_block();
    g_mempool = _saved_mempool;
  }

  ~MempoolGuard() { release(); }

 private:
  Mempool _mempool;
  Mempool* _saved_mempool;
};

inline std::string print_trace() {
  static const int BT_BUF_SIZE = 400;
  std::stringstream debug_stream;

  void* buffer[BT_BUF_SIZE];
  int nptrs = backtrace(buffer, BT_BUF_SIZE);
  char** strings = backtrace_symbols(buffer, nptrs);

  for (int j = 0; j < nptrs; j++) {
    debug_stream << strings[j] << "\t";
  }

  return debug_stream.str();
}
}  // namespace im
