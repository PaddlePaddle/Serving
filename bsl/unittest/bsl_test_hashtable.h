
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

class bsl_test_hashtable : public CxxTest::TestSuite
{
public:

	typedef std::string key;
	typedef int value;
	typedef bsl::bsl_hashtable<key,key,hash_func,std::equal_to<key>,
				bsl::param_select<key>,bsl::bsl_sample_alloc<bsl::bsl_alloc<key>, 256> > hash_type;
	 
	void test_hashtable() {
		
		{
			hash_type ht;
			TS_ASSERT( 0 == ht.create(100));
			TS_ASSERT( true == ht.is_created() );
			
			hash_type::const_iterator cit = ht.begin();		
			++cit;
			cit++;
			
			hash_type::iterator it = ht.begin();
			++it;
			it++;
			
			const hash_type cht;
			TS_ASSERT(false == cht.is_created());
			hash_type::const_iterator cht_cit= cht.begin();
			++cht_cit;
			cht_cit++;
		}
		{
			try {
				hash_type ht;
				hash_type ht1;
				ht = ht1;
			} catch (bsl::Exception& e) {
				printf("\n==\n%s\n==\n",e.all());
			}
		}
		{
			try {
				hash_type ht;
				hash_type ht1(ht);
			} catch (bsl::Exception& e) {
				printf("\n==\n%s\n==\n",e.all());
			}
		}

                //added 2011/11/15
                {
                    try{
                        hash_type ht;
                        hash_type ht1(ht);
                        std::string key1("what");
                        ht1.set(ht1._hashfun(key1),key1,key1);
                    } catch (bsl::Exception& e) {
                        printf("-------------------new\n\n==\n%s\n==\n",e.all());
                    }
                }

                //added 2011/11/15
                {
                    try{
                        hash_type ht;
                        ht.create(10);
                        hash_type ht1(ht);
                        
                        std::string key1("what");
                        ht1.set(ht1._hashfun(key1),key1,key1);
                    } catch (bsl::Exception& e) {
                        printf("-------------------new\n\n==\n%s\n==\n",e.all());
                    }
                }

                //added 2011/11/15
                {
                    try{
                        hash_type ht;
                        ht.create(10);
                        hash_type ht1;
                        ht1.create(5);
                        ht1 = ht;
                        
                        std::string key1("what");
                        ht1.set(ht1._hashfun(key1),key1,key1);
                    } catch (bsl::Exception& e) {
                        printf("-------------------new\n\n==\n%s\n==\n",e.all());
                    }
                }
                
		{
			try {
				printf("hello\n");
				hash_type ht(0);
			} catch (bsl::Exception& e) {
				printf("\n==\n%s\n==\n",e.all());
				printf("world\n");
			}
		}
		{
			try {
				hash_type ht(1000);
				hash_type ht1(ht);
				hash_type ht2 = ht;
			} catch (bsl::Exception& e) {
				printf("\n==\n%s\n==\n",e.all());
			}
		}
		{
			hash_type ht(100);
			std::set<key> st;
			ht.assign(st.begin(),st.end());
		}
		{
			std::set<key> st;
			for (size_t i = 0; i < N; i ++) {
				st.insert(s[i]);
			}
			hash_type ht(100);
			ht.assign(st.begin(),st.end());
			TS_ASSERT( ht.size() == N );

			hash_type ht2 = ht;
			TS_ASSERT( ht2.size() == N );
			hash_type ht3;
			ht3 = ht;
			TS_ASSERT( ht3.size() == N );

			for (size_t i = 0; i < N; i ++) {
				TS_ASSERT( ht.find( ht._hashfun( s[i] ), s[i] ) != NULL );
				TS_ASSERT( ht2.find( ht2._hashfun( s[i] ), s[i] ) != NULL );
				TS_ASSERT( ht3.find( ht3._hashfun( s[i] ), s[i] ) != NULL );
			}
		}
		
	}
	
	void test_create() {
		hash_type ht;
		for (int i = 10; i < 100; i ++) {
			ht.create(i);
		}
		hash_type ht2;
		for (int i = 0; i < 100; i ++) {
			ht2 = ht;
		}
	}	
};

#endif
/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
