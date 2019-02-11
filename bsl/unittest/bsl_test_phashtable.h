
#ifndef __BSL_TEST_HASHTABLE_H
#define __BSL_TEST_HASHTABLE_H

#include <cxxtest/TestSuite.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <bsl/containers/hash/bsl_hashmap.h>
#include <bsl/containers/string/bsl_string.h>
#include <bsl/containers/hash/bsl_phashmap.h>
#include <bsl/containers/hash/bsl_hashtable.h>
#include <bsl/containers/hash/bsl_phashtable.h>

#include <bsl/archive/bsl_filestream.h>
#include <bsl/archive/bsl_serialization.h>
#include <bsl/archive/bsl_binarchive.h>
#include <bsl/alloc/allocator.h>
#include <vector>
#include <map>
#include <set>

char s[][32]={"yingxiang", "yufan", "wangjiping", "xiaowei", "yujianjia", "maran", "baonenghui",
	"gonglei", "wangyue", "changxinglong", "chenxiaoming", "guoxingrong", "kuangyuheng"
};

const size_t N = sizeof(s) / sizeof(s[0]);


std::string randstr()
{
	char buf[130] = {0};
	int len = rand()%64 + 1;
	for (int i=0; i<len; ++i) {
		buf[i] = rand()%26 + 'a';
	}
	return buf;
};

/**
 *key µÄhash·Âº¯Êý
 */
struct hash_func
{
	size_t operator () (const std::string &__key) const {
		size_t key = 0;
		for (size_t i=0; i<__key.size(); ++i) {
			key = (key<<8)+__key[i];
		}
		return key;
	}
};

template<typename _T, int _I>
void inc_node(_T* v, void* /*args = NULL*/){
	v->second += _I;
}

template<typename _T, int _I>
class IncNode{
public:
	void operator()(_T* v, void* /*args = NULL*/){
		v->second += _I;
	}
};

template<typename HASH>
void* update_func(void* args){
	HASH *ht = (HASH *)args;
	for(int i = 0; i < 1000; ++i){
		for(int j = 0; j < 100; ++j){
			TS_ASSERT(ht->get(i, NULL, inc_node<typename HASH::value_type, 1>) == bsl::HASH_EXIST);
		}
		for(int j = 0; j < 100; ++j){
			TS_ASSERT(ht->get(i, NULL, IncNode<typename HASH::value_type, 1>()) == bsl::HASH_EXIST);
		}
	}
	return NULL;
}

class bsl_test_phashtable : public CxxTest::TestSuite
{
public:

	void test_hashtable() {
		typedef std::string key;
		typedef int value;
		typedef bsl::bsl_phashtable<key,key,hash_func,std::equal_to<key>,
				bsl::param_select<key>,bsl::bsl_sample_alloc<bsl::bsl_alloc<key>, 256> > hash_type;

		{
			hash_type ht;
			TS_ASSERT( 0 == ht.create(100));
		}
		{
			bool ex_flag = false;
			try {
				hash_type ht(0);
			} catch (bsl::Exception& e) {
				ex_flag = true;	
			}
			TS_ASSERT(ex_flag == true);
		}
		{
			hash_type ht(100);
			std::set<key> st;
			ht.assign(st.begin(),st.end());
			TS_ASSERT(ht.size() == 0);
		}
		{
			std::set<key> st;
			for (size_t i = 0; i < N; i ++) {
				st.insert(s[i]);
			}
			hash_type ht(100);
			ht.assign(st.begin(),st.end());
			TS_ASSERT( ht.size() == N );
		}
	}
	
	void test_multi_create() {
		typedef std::string key;
		typedef int value;
		typedef bsl::bsl_phashtable<key,key,hash_func,std::equal_to<key>,
				bsl::param_select<key>,bsl::bsl_sample_alloc<bsl::bsl_alloc<key>, 256> > hash_type;
		hash_type ht;
		for (int i = 0; i < 100; i ++) {
			ht.create(i);
		}
	}
	
	void test_get_callback(){	
		typedef int _Key;
		typedef std::pair<_Key, int> _Value;
		typedef bsl::bsl_phashtable<
			_Key, 
			_Value, 
			bsl::xhash<_Key>,
			std::equal_to<_Key>,
			bsl::pair_first<_Value>, 
			bsl::bsl_sample_alloc<bsl::bsl_alloc<_Key>, 256>
		> hash_type;

		hash_type ht;
		int hash_num = 1333;
		int n = 1000;
		const int INC = 1000;
		
		TS_ASSERT(ht.create(hash_num) == 0);
		for(int i = 0; i < n; ++i){
			ht.set(i, std::make_pair<_Key, int>(i, 0));
		}
		TS_ASSERT((int)ht.size() == n);

		for(int i = 0; i < n; ++i){
			_Value old_val;
			TS_ASSERT(ht.get(i, &old_val, inc_node<hash_type::value_type, INC>) == bsl::HASH_EXIST);
			TS_ASSERT(old_val.second == 0);
			TS_ASSERT(ht.get(i, &old_val) == bsl::HASH_EXIST);
			TS_ASSERT(old_val.second == INC);
			TS_ASSERT((int)ht.size() == n);
		}
	
		ht.clear();
		TS_ASSERT(ht.create(hash_num) == 0);
		for(int i = 0; i < n; ++i){
			ht.set(i, std::make_pair<_Key, int>(i, 0));
		}
		TS_ASSERT((int)ht.size() == n);

		for(int i = 0; i < n; ++i){
			_Value old_val;
			TS_ASSERT(ht.get(i, &old_val, IncNode<hash_type::value_type, INC>()) == bsl::HASH_EXIST);
			TS_ASSERT(old_val.second == 0);
			TS_ASSERT(ht.get(i, &old_val) == bsl::HASH_EXIST);
			TS_ASSERT(old_val.second == INC);
			TS_ASSERT((int)ht.size() == n);
		}
		return;
	}
	
	void test_get_callback_multithread(){
		typedef int _Key;
		typedef std::pair<_Key, int> _Value;
		typedef bsl::bsl_phashtable<
			_Key, 
			_Value, 
			bsl::xhash<_Key>,
			std::equal_to<_Key>,
			bsl::pair_first<_Value>, 
			bsl::bsl_sample_alloc<bsl::bsl_alloc<_Key>, 256>
		> hash_type;

		int rep = 20;
		while(rep--){
			int hash_num = 1333;
			hash_type ht;
			TS_ASSERT(ht.create(hash_num) == 0);
			int n = 1000;
			for(int i = 0; i < n; ++i){
				ht.set(i, std::make_pair<_Key,int>(i, 0));
			}
			//check
			TS_ASSERT((int)ht.size() == n);
			for(int i = 0; i < n; ++i){
				hash_type::value_type val;
				TS_ASSERT(ht.get(i, &val) == bsl::HASH_EXIST);
				TS_ASSERT(val.second == 0);
			}
			int pn = 50;
			std::vector<pthread_t> pids(pn);
			for(int i = 0; i < pn; ++i){
				TS_ASSERT(pthread_create(&pids[i], NULL, update_func<hash_type>, &ht) == 0);
			}
			for(int i = 0; i < pn; ++i){
				pthread_join(pids[i], NULL);
			}
			//check
			TS_ASSERT((int)ht.size() == n);
			for(int i = 0; i < n; ++i){
				hash_type::value_type val;
				TS_ASSERT(ht.get(i, &val) == bsl::HASH_EXIST);
				TS_ASSERT(val.second == pn * 100 * 2);
			}
		}
		return;
	}
};

#endif
/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
