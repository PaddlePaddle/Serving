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

/*
struct BigNode {
    BigNode* next;
    char data[0];
  };
*/
// template T is BigNode
// which is a node of variable length memory linked list
// _head is a BigNode* ptr, always points to the head node of the Stack.
// so PushOnlyStack is the head node of the Stack with some member function.
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

  T* releaseAndGetHeadPtr() {
    return _head.exchange(NULL, butil::memory_order_relaxed);
  }

 private:
  butil::atomic<T*> _head;
};

// T can be class Block or class BlockReference
// class Block is 2M bytes memory
// class BlockReference is class Block* ptr.
// so the main member of FreeListNode is 2M bytes or class Block* ptr
// int 'id' is the index of itself in a FreeList.
// int 'next' is the index of next FreeListNode<T> in a FreeList.
template <class T>
struct FreeListNode {
  uint64_t id;
  uint64_t next;
  T data;
};

// T can be class Block or class BlockReference
// CAP means capicity
// the main member of FreeList is FreeListNode<T>* [CAP].
// FreeList doesn`t realse the block data, it`s only an array of
// FreeListNode<T>* ptr.
template <class T, int CAP>
class FreeList {
 public:
  typedef FreeListNode<T> Node;
  static const uint64_t EMPTY = 0xFFFFFFFFFFFFFFFF;

  // get the head Node`s member data ptr(T*)
  T* get() {
    uint64_t head = _head.load(butil::memory_order_acquire);
    if (head == EMPTY) {
      return new_node();
    }

    // _head is atomic<int>, which means the head index.
    // head is the tempValue of _head.
    // maybe _head is not equals head anymore.
    // cause other thread may change the _head.
    /*compare_exchange_weak
    When the current value is equal to the expected value, modify the current
    value to the set value and return true
    When the current value is not equal to the expected value, modify the
    expected value to the current value and return false
    */
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
    /*
    container_of
    according to the member(pointer type) of a Class
    to get the class Pointer
    for example
    T is the member of class Node, T data, 'data' is the name.
    T* value is the member(pointer type) of class Node
    so we can get the Node* by calling container_of(value, Node, data)
    */
    Node* node = container_of(value, Node, data);

    uint64_t head = _head.load(butil::memory_order_acquire);

    // node->id is int64. slot index is int32.
    // address(): slot = static_cast<uint32_t>(node->id)
    // will this be wrong?
    // add version? maybe this is different from new node?
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

  // F is callable class, class PutBlockByReference.
  // actually, F is the function put.
  // this function put the reuse the used block or blockReference
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
    // used_bytes/1024 = KB
    return used_bytes >> 10;
  }

  uint32_t get_number_of_allocate_blocks() const {
    return _slot_index.load(butil::memory_order_relaxed);
  }

  uint32_t get_number_of_free_blocks() const {
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

// Memory is 2M bytes
struct Block {
  static const int BLOCK_SIZE = 2 * 1024 * 1024;  // 2MB
  char data[BLOCK_SIZE];
};

// Block* and offset
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

// This is a real singleton class FreeList<Block,MAX_BLOCK_COUNT>
// FreeList is always an array of FreeListNode<Block>* ptr.
// Block(2MB) is created when get() is called.

// because BlockFreeList is a threal-safe Singleton.
// so we don`t release Block, it is global memory.
// total number is 256*1024.
// the MAX_BLOCK_COUNT of Region(one thread one Region) is 1024.
// so BlockFreeList allow 256 Region(means 256 thread).
// the memory used by BlockFreeListType is sizeof(void*)*256*1024.
// Block(2MB) memory is created only when get() is called.
class BlockFreeList {
 public:
  static const int MAX_BLOCK_COUNT = 256 * 1024;
  typedef lockfree::FreeList<Block, MAX_BLOCK_COUNT> BlockFreeListType;
  static BlockFreeListType* instance() {
    static BlockFreeListType singleton;
    return &singleton;
  }
};

// _big_mem_capacity: a large memory is owned by Region.
// _bigNode_Stack: A list of bigNode(variable length memory)is owned by
// Region,the number is unlimit.
// _blockReference_FreeList: a FreeList of Block(2MB) is owned by singleton
// BlockFreeList, which is global.
// we can borrow 1024*Block from BlockFreeList.
class Region {
 public:
  struct PutBlockByReference {
    void operator()(BlockReference* block_ref) {
      if (block_ref->block != NULL) {
        BlockFreeList::instance()->put(block_ref->block);
      }
      block_ref->reset();
    }
  };

  // this is a variable length memory node.
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
    uint32_t alloc_blocks =
        _blockReference_FreeList.get_number_of_allocate_blocks();
    uint32_t free_blocks = _blockReference_FreeList.get_number_of_free_blocks();
    uint32_t used_mem_mb = _blockReference_FreeList.real_used_size();
    uint32_t big_buf_size = _big_mem_size.load(butil::memory_order_relaxed);
    uint32_t big_buf_count = _big_mem_count.load(butil::memory_order_relaxed);
    uint32_t mlc_mem_size =
        _total_bigNode_size.load(butil::memory_order_relaxed);
    uint32_t mlc_mem_count =
        _total_bigNode_count.load(butil::memory_order_relaxed);

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

  void put(BlockReference* blockReference);

  static const int MAX_BLOCK_COUNT = 1024;  // each Block is 2MB
  static const int BIG_MEM_THRESHOLD =
      2 * 1024 *
      1024;  // 2MB,means when you need less than 2M, get memory from Block.

  // 128MB,means when you need less than 128MB, get memory from BigMemory
  // instead
  // of BigNode
  static const int BIGNODE_MEM_THRESHOLD = (128 * 1024 * 1024 + 1);
  static const int COUNTER_SIZE =
      BIGNODE_MEM_THRESHOLD / BIG_MEM_THRESHOLD + 1;  // this is not used

 private:
  lockfree::FreeList<BlockReference, MAX_BLOCK_COUNT> _blockReference_FreeList;

  // _total_bigNode_size is the total size of BigNodeStack.
  // _total_bigNode_count is the total count of BigNodeStack.
  // BigNode is variable length memory.
  lockfree::PushOnlyStack<BigNode> _bigNode_Stack;
  butil::atomic<uint32_t> _total_bigNode_size;
  butil::atomic<uint32_t> _total_bigNode_count;

  // '_big_mem_start' points to a single big memory belong to Region.
  // _big_mem_capacity is the size of single big memory.
  // _big_mem_size is the already used size.
  // _big_mem_count is the used count.
  char* _big_mem_start;
  uint32_t _big_mem_capacity;  // 32M
  butil::atomic<uint32_t> _big_mem_size;
  butil::atomic<uint32_t> _big_mem_count;
};
}  // namespace memory
}  // namespace fugue

class Mempool {
 public:
  void* malloc(size_t size) {
    size = _align(size);
    // It does not enter the if statement the first time.
    // The if statement may enter after the block is created.
    // If the block has not been used up, it will enter.
    if (size <= _free_size) {
      void* p = _free_cursor;
      _free_size -= size;
      _free_cursor += size;
      return p;
    }

    return malloc_from_region(size);
  }

  void free(void* p, size_t size) {
    // size>Block(2M)
    // other memory is managed by Region，no need to release here.
    if (size > fugue::memory::Region::BIG_MEM_THRESHOLD) {
      return;
    }

    // memory in _block，update the pointer.
    if (_free_cursor - size == static_cast<char*>(p)) {
      // for example, you need to release -(8+1)bytes
      // you can only release -8bytes，cause -(8+2)byte is used by other.
      size_t down_aligned = _down_align(size);
      _free_cursor -= down_aligned;
      _free_size += down_aligned;
    }
  }

  void* realloc(void* old_data, size_t old_size, size_t new_size) {
    // Return the pointer directly and reuse it without expansion.
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
        // old_data will copy to other structure
        // so _free_cursor rollback，means the memory used by old_data can be
        // used.
        _free_cursor = static_cast<char*>(old_data);
        _free_size += old_size;
      }
    }

    // 可能返回的是单独Region中malloc的内存。
    // 也可能是Block，例如new_size=1M, old_data原本的指针头就在1.2M处
    // old_size = 0.5M
    // 此时,_free_size = 0.3M，new_size<2M,但是required = 1-0.5 >0.3
    // 分配出来的就是Block，但是该Block没有并很完美的利用完全。
    void* p = this->malloc_from_region(new_size);
    if (p != NULL) {
      memcpy(p, old_data, old_size);
      return p;
    }

    return NULL;
  }

  explicit Mempool(fugue::memory::Region* region)
      : _free_size(0), _free_cursor(NULL), _region(region) {
    _blockReference = NULL;
  }

  ~Mempool() { release_block(); }

  void release_block() {
    if (_blockReference) {
      _blockReference->offset = fugue::memory::Block::BLOCK_SIZE - _free_size;
      _region->put(_blockReference);
    }

    _free_size = 0;
    _free_cursor = NULL;
    _blockReference = NULL;
  }

 private:
  void* malloc_from_region(size_t size) {
    // if greater than BIG_MEM_THRESHOLD, _region->malloc
    // else get the memory from the Block.
    if (size > fugue::memory::Region::BIG_MEM_THRESHOLD) {
      return _region->malloc(size);
    }

    while (true) {
      fugue::memory::BlockReference* blockReference = _region->get();
      if (blockReference == NULL) {
        return NULL;
      }

      uint32_t free_size =
          fugue::memory::Block::BLOCK_SIZE - blockReference->offset;
      // 若未能满足要求，则while下一次循环，那么上次的block必然成为野值。
      if (size <= free_size) {
        // 试图更新该节点的offset，但是该结点已经变成了野值，无法被再次使用了，只有等待归还。
        // 应将该节点put进队列,进入该判断语句内，证明本来get已肯定可以return.
        // 此时，上次没用完的值，应该更新offset后，还回去，下次没准还能用，不至于浪费。
        if (_blockReference) {
          _blockReference->offset =
              fugue::memory::Block::BLOCK_SIZE - _free_size;
          _region->put(_blockReference);
        }

        char* p = blockReference->block->data + blockReference->offset;
        _free_size = free_size - size;
        _free_cursor = p + size;
        _blockReference = blockReference;
        return p;
      }
    }
    // It's not executed at all, for the sake of syntax.
    return _region->malloc(size);
  }

  static const int ALIGN_SIZE = sizeof(void*);

  // align to the 8bytes, if (8+1), it will be (8+8)bytes.
  inline size_t _align(size_t size) const {
    return (size + (ALIGN_SIZE - 1)) & ~(ALIGN_SIZE - 1);
  }

  // down_align to 8bytes, if (8+1), it will be (8+0)bytes.
  inline size_t _down_align(size_t size) const {
    return size & ~(ALIGN_SIZE - 1);
  }

  size_t _free_size;
  char* _free_cursor;

  fugue::memory::Region* _region;
  fugue::memory::BlockReference* _blockReference;
};

// use threal-local key instead of __thread.
// it`s not referenced.
/*
extern __thread Mempool* g_mempool;
*/

// class mempool is a Interface.
// it`s not necessary at all.
/*
class mempool {
 public:
  virtual void* malloc(size_t size) = 0;
  virtual void free(void* p, size_t size) = 0;
  inline virtual ~mempool() {}
};
*/

// GlobalMempool is a Singleton-RAII class.
// It`s propose is to manage the thread-local pointer 'g_mempool'(class
// Mempool*)
// It`s not referenced, so it`s useless.
/*
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
*/

// MempoolGuard is a RAII class.
// It`s propose is to manage the thread-local pointer 'g_mempool'(class
// Mempool*)
// It`s not referenced, so it`s useless.
/*
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
*/
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
