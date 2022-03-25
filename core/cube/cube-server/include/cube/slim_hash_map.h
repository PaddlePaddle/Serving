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

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <map>
#include <utility>
#include <vector>

#ifdef BCLOUD
#include "base/logging.h"
#else
#include "butil/logging.h"
#endif

/**
 * hash node
 */
#pragma pack(push, 1)
template <typename key_t, typename value_t>
struct slim_hash_node_t {
  std::pair<key_t, value_t> data;
  uint32_t next;
};
#pragma pack(pop)
/**
 * hash map
 */
template <typename key_t, typename value_t>
class slim_hash_map {
 public:
  typedef key_t key_type;
  typedef value_t mapped_type;
  typedef std::pair<key_t, value_t> value_type;
  typedef size_t size_type;

 private:
  typedef slim_hash_node_t<key_t, value_t> hash_node_t;
  static const size_type MAX_BLOCK_NUM = 512;
  static const size_type BLOCK_SIZE = 8388608;
  static const uint32_t DUMP_GRANULARITY =
      1000; /**< the granularity of every save job */
  static const int iterm_size = sizeof(key_t) + sizeof(value_t);
  hash_node_t* m_blockAddr[MAX_BLOCK_NUM];
  uint32_t m_nFreeEntries;
  uint32_t m_nNextEntry;
  size_type m_nBlockNum;
  uint32_t* m_hashTable;
  size_type m_nHashSize;
  size_type m_nSize;
  std::vector<uint64_t> m_fileLens;

  struct DumpBar {
    FILE* fp;     /**< the fp of the file to dump */
    char* buf;    /**< buffer to serialize the date */
    int buf_size; /**< the size of buf */
    // int startIndex;   /**< the begin index of the last dump job */
    int lastIndex; /**< the index where last dump job ended */

    DumpBar() : fp(NULL), buf(NULL) {}

    DumpBar(FILE* _fp, char* _buf, int _buf_size, int _lastIndex)
        : fp(_fp), buf(_buf), buf_size(_buf_size), lastIndex(_lastIndex) {}
    ~DumpBar() { reset(); }

    void reset() {
      if (fp) {
        fclose(fp);
      }

      fp = NULL;
      free(buf);
      buf = NULL;
      buf_size = 0;
      lastIndex = 0;

      return;
    }
  };

  DumpBar dumpBar;

 public:
  struct iterator {
    friend class slim_hash_map;

   private:
    slim_hash_map* container;
    hash_node_t* node;
    uint32_t index;

   public:
    iterator(slim_hash_map* container, uint32_t index, hash_node_t* node) {
      this->container = container;
      this->index = index;
      this->node = node;
    }
    hash_node_t* get_node() { return node; }
    uint32_t cur_index() { return index; }
    std::pair<key_t, value_t>* operator->() { return &node->data; }
    std::pair<key_t, value_t>& operator*() { return node->data; }
    iterator& operator++() {
      if (node == NULL) {
        return *this;
      }

      if (node->next == 0) {
        ++index;

        while (index < container->m_nHashSize) {
          if (container->m_hashTable[index] != 0) {
            break;
          }

          ++index;
        }

        if (index == container->m_nHashSize) {
          index = -1;
          node = NULL;
          return *this;
        }

        node = container->get_node(container->m_hashTable[index]);
      } else {
        node = container->get_node(node->next);
      }

      return *this;
    }
    iterator operator++(int) {
      iterator it_bak = *this;
      this->operator++();
      return it_bak;
    }

    enum IteratorStatus { END = 0, OneBlockEND = 1, CONTINUE = 2 };

    IteratorStatus IncrIterSenseBlock() {
      if (NULL == node) {
        return END;
      }

      if (0 == node->next) {
        ++index;

        while (index < container->m_nHashSize) {
          if (container->m_hashTable[index] != 0) {
            break;
          }

          ++index;
        }

        if (index == container->m_nHashSize) {
          index = -1;
          node = NULL;
          return END;
        }

        node = container->get_node(container->m_hashTable[index]);
        return OneBlockEND;
      }

      node = container->get_node(node->next);
      return CONTINUE;
    }
    bool operator==(const iterator& it) const {
      return container == it.container && index == it.index && node == it.node;
    }
    bool operator!=(const iterator& it) const { return !operator==(it); }
  };

  slim_hash_map() {
    m_nFreeEntries = 0;
    m_nNextEntry = 1;
    memset(m_blockAddr, 0, sizeof(m_blockAddr));
    m_nBlockNum = 0;
    m_hashTable = 0;
    m_nSize = 0;
    m_nHashSize = 0;
  }

  void destroy() {
    delete[] m_hashTable;
    m_hashTable = NULL;
    m_nHashSize = 0;

    for (size_type i = 0; i < m_nBlockNum; ++i) {
      delete[] m_blockAddr[i];
      m_blockAddr[i] = NULL;
    }
    m_nBlockNum = 0;
  }

  ~slim_hash_map() { destroy(); }

  int copy_data_from(const slim_hash_map& rhs) {
    destroy();
    LOG(INFO) << "start copy data, rhs info, mHashSize: " << rhs.m_nHashSize;
    if (rhs.m_nHashSize > 0) {
      m_hashTable = new (std::nothrow) uint32_t[rhs.m_nHashSize];
      if (!m_hashTable) {
        LOG(ERROR) << "new m_hashTable failed, size "
                   << sizeof(uint32_t) * rhs.m_nHashSize;
        return -1;
      }
      memcpy(m_hashTable, rhs.m_hashTable, sizeof(uint32_t) * rhs.m_nHashSize);
    }
    m_nHashSize = rhs.m_nHashSize;

    for (m_nBlockNum = 0; m_nBlockNum < rhs.m_nBlockNum; ++m_nBlockNum) {
      m_blockAddr[m_nBlockNum] = new (std::nothrow) hash_node_t[BLOCK_SIZE];
      if (!m_blockAddr[m_nBlockNum]) {
        LOG(ERROR) << "new m_blockAddr[" << m_nBlockNum << "] failed, size "
                   << sizeof(hash_node_t) * BLOCK_SIZE;
        return -1;
      }
      LOG(INFO) << "copy data, m_nBlockNum: " << m_nBlockNum << " , copy size:" << sizeof(hash_node_t) * BLOCK_SIZE;
      memcpy(m_blockAddr[m_nBlockNum],
             rhs.m_blockAddr[m_nBlockNum],
             sizeof(hash_node_t) * BLOCK_SIZE);
    }

    m_nFreeEntries = rhs.m_nFreeEntries;
    m_nNextEntry = rhs.m_nNextEntry;
    m_nSize = rhs.m_nSize;
    m_fileLens = rhs.m_fileLens;

    return 0;
  }

  size_type size() const { return m_nSize; }
  size_type max_size() const { return (size_type)-1; }
  bool empty() const { return m_nSize == 0; }
  size_type bucket_count() const { return m_nHashSize; }
  iterator begin(uint32_t index = 0) {
    for (size_type i = index; i < m_nHashSize; ++i) {
      if (m_hashTable[i] != 0) {
        return iterator(this, i, get_node(m_hashTable[i]));
      }
    }

    return end();
  }
  iterator end() { return iterator(this, -1, NULL); }
  iterator find(const key_type& key) {
    if (m_nHashSize == 0) {
      return iterator(this, 0, NULL);
    }
    size_type index = key % m_nHashSize;
    hash_node_t* node = get_node(m_hashTable[index]);
    int node_cnt = 0;
    while (node != NULL && node->data.first != key) {
      LOG(INFO) << "node link get:" << node->data.first;
      node_cnt++;
      node = get_node(node->next);
    }
    LOG(INFO) << "key: " << key << " , found count: " << node_cnt;  
    if (node == NULL) {
      return end();
    }

    return iterator(this, index, node);
  }
  size_type erase(const key_type& key) {
    size_type index = key % m_nHashSize;
    uint32_t* last_node_pointer = &m_hashTable[index];
    uint32_t addr = m_hashTable[index];
    hash_node_t* node = get_node(addr);

    while (node != NULL && node->data.first != key) {
      last_node_pointer = &node->next;
      addr = node->next;
      node = get_node(addr);
    }

    if (node == NULL) {
      return 0;
    }

    *last_node_pointer = node->next;
    release_node(addr, node);
    return 1;
  }
  void erase(const iterator& it) {
    uint32_t* last_node_pointer = &m_hashTable[it.index];
    uint32_t addr = m_hashTable[it.index];
    hash_node_t* node = get_node(addr);

    while (node != it.node) {
      last_node_pointer = &node->next;
      addr = node->next;
      node = get_node(addr);
    }

    *last_node_pointer = node->next;
    release_node(addr, node);
    return;
  }
  static uint64_t next_prime(uint64_t n) {
    static const uint64_t s_prime_list[] = {
        53ul,        97ul,         193ul,        389ul,       769ul,
        1543ul,      3079ul,       6151ul,       12289ul,     24593ul,
        49157ul,     98317ul,      196613ul,     393241ul,    786433ul,
        1572869ul,   3145739ul,    6291469ul,    12582917ul,  25165843ul,
        50331653ul,  100663319ul,  200000033ul,  201326611ul, 210000047ul,
        220000051ul, 230000059ul,  240000073ul,  250000103ul, 260000137ul,
        270000161ul, 280000241ul,  290000251ul,  300000277ul, 310000283ul,
        320000287ul, 330000371ul,  340000387ul,  350000411ul, 360000451ul,
        370000489ul, 380000519ul,  390000521ul,  400000543ul, 402653189ul,
        805306457ul, 1610612741ul, 3221225473ul, 4294967291ul};
    const size_t s_num_primes = sizeof(s_prime_list) / sizeof(s_prime_list[0]);
    const uint64_t* first = s_prime_list;
    const uint64_t* last = s_prime_list + static_cast<int>(s_num_primes);
    const uint64_t* pos = std::lower_bound(first, last, n);
    return (pos == last) ? *(last - 1) : *pos;
  }

  void resize(size_type size) {
    uint32_t* hashTable;
    size = next_prime(size);

    if (size <= m_nHashSize) {
      LOG(INFO) << "next prime[" << size << "] <= m_nHashSize[" << m_nHashSize
                << "], no need to resize";
      return;
    }
    LOG(INFO) << "resize m_nHashSize[" << m_nHashSize << "] to next prime["
              << size << "]";

    try {
      hashTable = new uint32_t[size];
    } catch (std::exception& e) {
      LOG(ERROR) << "std::exception[" << e.what() << "] was thrown!";
      return;
    } catch (...) {
      LOG(ERROR) << "unkown exception was thrown!";
      return;
    }

    memset(hashTable, 0, sizeof(uint32_t) * size);

    if (m_hashTable == NULL) {
      m_hashTable = hashTable;
    } else {
      // rehash
      for (size_type bucket = 0; bucket < m_nHashSize; ++bucket) {
        uint32_t first_addr = m_hashTable[bucket];
        hash_node_t* first = get_node(first_addr);

        while (first) {
          size_type new_bucket = first->data.first % size;
          uint32_t next_addr = first->next;
          m_hashTable[bucket] = next_addr;
          first->next = hashTable[new_bucket];
          hashTable[new_bucket] = first_addr;
          first_addr = next_addr;
          first = get_node(first_addr);
        }
      }

      delete[] m_hashTable;
      m_hashTable = hashTable;
    }

    m_nHashSize = size;
  }
  mapped_type& operator[](const key_type& key) {
    uint32_t index = key % m_nHashSize;
    hash_node_t* node = get_node(m_hashTable[index]);

    while (node != NULL && node->data.first != key) {
      node = get_node(node->next);
    }

    if (node != NULL) {
      return node->data.second;
    }
    return add_node(index, key)->data.second;
  }
  void clear() {
    memset(m_hashTable, 0, sizeof(uint32_t) * m_nHashSize);
    m_nNextEntry = 1;
    m_nFreeEntries = 0;
    m_nSize = 0;
  }
  bool load(const char* file, uint32_t block_id) {
    // clear();
    // bias = 0 means base mode, bias = K means patch mode, and base dict has size K
    int size = sizeof(key_t) + sizeof(value_t);
    FILE* fp = fopen(file, "rb");
    char* buf = reinterpret_cast<char*>(malloc(size * 100000));
    LOG(INFO) << "current block id: " << block_id;
    if (fp == NULL || buf == NULL) {
      return false;
    }
    size_t read_count;
    bool err = false;
    key_t key;
    value_t value;

    while ((read_count = fread(buf, size, 100000, fp)) != 0) {
      if (ferror(fp) != 0) {
        err = true;
        break;
      }

      for (int i = 0; i < static_cast<int>(read_count); ++i) {
        key = *(reinterpret_cast<key_t*>(buf + i * size));
        value = *(reinterpret_cast<value_t*>(buf + i * size + sizeof(key_t)));
        value = ((uint64_t)block_id << 32) | value;
        LOG(INFO) << "slim map key: " << key << " , value: " << value; 
        (*this)[key] = value;
      }
    }

    if (err) {
      clear();
    }

    fclose(fp);
    free(buf);
    return !err;
  }

  /**
  * @brief    dump data in memory into file.
  *           DUMP_GRANULARITYs entry will dump every execution.
  *
  * @param    file    the file name to store the data
  *
  * @return   -1: failed to dump data;
  *           0 : complete the dumping;
  *           greater than 0: the index of the entry in hashtable where dump job
  * goes
  */
  int save(const char* file) {
    static const int FACTOR = 1000;
    bool writerErr = false;
    hash_node_t* node = NULL;

    if (NULL == file && NULL == dumpBar.fp) {
      return -1;
    }

    if (file) {  // the begin of the dump job
      dumpBar.fp = fopen(file, "wb");

      if (NULL == dumpBar.fp) {
        goto ERR_RET;
      }

      dumpBar.buf_size = iterm_size * DUMP_GRANULARITY * FACTOR;
      dumpBar.buf = reinterpret_cast<char*>(malloc(dumpBar.buf_size));

      if (NULL == dumpBar.buf) {
        goto ERR_RET;
      }
    }

    for (uint32_t i = 0;
         i < DUMP_GRANULARITY && (size_type)dumpBar.lastIndex < m_nHashSize;
         i++, dumpBar.lastIndex++) {
      node = get_node(m_hashTable[dumpBar.lastIndex]);

      for (uint32_t j = 0; NULL != node;) {
        *(reinterpret_cast<key_t*>(dumpBar.buf + j * iterm_size)) =
            node->data.first;
        *(reinterpret_cast<value_t*>(dumpBar.buf + j * iterm_size +
                                     sizeof(key_t))) = node->data.second;
        ++j;

        node = get_node(node->next);

        if (j == DUMP_GRANULARITY * FACTOR || NULL == node) {
          if (fwrite(dumpBar.buf, iterm_size, j, dumpBar.fp) != j) {
            writerErr = true;
            goto ERR_RET;
          }

          j = 0;
        }
      }  // end of for
    }    // end of for

    if ((size_type)dumpBar.lastIndex == m_nHashSize) {  // mission complete
      dumpBar.reset();
      return 0;
    }

    // not complete yet
    return dumpBar.lastIndex;

  ERR_RET:

    if (writerErr) {
      remove(file);
    }

    dumpBar.reset();
    return -1;
  }

  void add_file_len(uint64_t len) { m_fileLens.push_back(len); }

  // return true if m_fileLens is prefix of lens
  bool check_file_len(const std::vector<uint64_t>& lens,
                      size_t* old_size) const {
    *old_size = m_fileLens.size();

    if (*old_size > lens.size()) {
      LOG(WARNING) << "old file_len size[" << *old_size
                   << "] > new file_len size[" << lens.size()
                   << "] in patch mode";
      return false;
    }

    for (size_t i = 0; i < *old_size; ++i) {
      if (m_fileLens[i] != lens[i]) {
        LOG(WARNING) << "old file_len[" << m_fileLens[i] << "] != new file_len["
                     << lens[i] << "] of pos " << i << " in patch mode";
        return false;
      }
    }

    return true;
  }

 protected:
  hash_node_t* get_node(uint32_t addr) {
    if (addr == 0) {
      return NULL;
    }

    uint32_t block = ((addr & 0xFF800000) >> 23);
    uint32_t index = (addr & 0x7FFFFF);
    return &m_blockAddr[block][index];
  }
  void release_node(uint32_t addr, hash_node_t* node) {
    --m_nSize;
    node->next = m_nFreeEntries;
    m_nFreeEntries = addr;
    return;
  }
  hash_node_t* add_node(uint32_t index, const key_type& key) {
    ++m_nSize;
    if (m_nFreeEntries) {
      uint32_t addr = m_nFreeEntries;
      hash_node_t* node = get_node(addr);
      m_nFreeEntries = node->next;
      node->next = m_hashTable[index];
      m_hashTable[index] = addr;
      node->data.first = key;
      return node;
    }

    uint32_t block = ((m_nNextEntry & 0xFF800000) >> 23);
    //LOG(INFO) << "key: " << key << " here. index: " << index << " , m_nNextEntry: "<< m_nNextEntry << " , block:" << block<< ", m_nBlockNum:" << m_nBlockNum;
    if (block >= m_nBlockNum) {
      try {
        m_blockAddr[m_nBlockNum++] = new hash_node_t[BLOCK_SIZE];
      } catch (std::exception& e) {
        LOG(ERROR) << "std::exception[" << e.what() << "] was thrown!";
        return NULL;
      } catch (...) {
        LOG(ERROR) << "unkown exception was thrown!";
        return NULL;
      }
    }
    uint32_t addr = m_nNextEntry;
    ++m_nNextEntry;
    hash_node_t* node = get_node(addr);
    node->next = m_hashTable[index];
    m_hashTable[index] = addr;
    node->data.first = key;
    return node;
  }
};
