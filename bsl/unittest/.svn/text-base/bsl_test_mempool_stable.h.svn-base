/***************************************************************************
 * 
 * Copyright (c) 2009 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_test_mempool_stable.h,v 1.1 2009/01/06 15:05:14 xiaowei Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file bsl_test_mempool_stable.h
 * @author xiaowei(com@baidu.com)
 * @date 2009/01/06 22:29:11
 * @version $Revision: 1.1 $ 
 * @brief 
 *  
 **/


#ifndef  __BSL_TEST_MEMPOOL_STABLE_H_
#define  __BSL_TEST_MEMPOOL_STABLE_H_


#include <bsl/pool/bsl_pool.h>
#include <bsl/pool/bsl_poolalloc.h>
#include <bsl/pool/bsl_xmempool.h>
#include <bsl/pool/bsl_xcompool.h>
#include <cxxtest/TestSuite.h>

#include <vector>
#include <set>
#include <map>

#define ALLOC(type) bsl::pool_allocator<type>
#define VECTOR(type) std::vector<type, ALLOC(type) >
#define MAP(key, value) \
	std::map<key, value, std::less<key>, ALLOC(value) >


class bsl_test_mempool_stable : public CxxTest::TestSuite
{
public:
	void test_normal()
	{
		bsl::xcompool stlp;
		stlp.create(1<<16);


		bsl::xcompool pool;
		pool.create();


		for (int idx=0; idx<3; ++idx) {
			{
				ALLOC(void *) ap(&stlp);
				VECTOR(void *) vec (ap);
				ALLOC(size_t) sp(&stlp);
				std::less<void *> eq;
				MAP(void *, size_t) vmp(eq, sp);

				long total = 0;
				for (int i=0; i<1<<16; ++i) {
					int lp = rand()%100;
					for (int j=0; j<lp; ++j) {
						int siz = rand()%(1<<17) + rand()%(1<<16);
						void *ptr = pool.malloc(siz);
						if (ptr == NULL) {
							std::cout<<"NONE "<<siz<<std::endl;
							break;
						}
						((char *)ptr)[0] = 'a';
						((char *)ptr)[siz-1] = 'a';
						vec.push_back(ptr);
						vmp[ptr] = siz;
						total += siz;
					}
					long rel = total / 2;
					while (rel > 0) {
						if (vec.size() == 0) break;
						rel -= vmp[vec.back()];
						pool.free (vec.back(), vmp[vec.back()]);
						vec.pop_back();
					}
				}
			}

			stlp.clear();
			pool.clear();

			std::cout<<idx<<std::endl;
		}
	}

	void test_normal2 ()
	{
		bsl::xcompool pool;
		pool.create(1<<16);


		for (int idx=0; idx<3; ++idx) {
			{
				ALLOC(void *) ap(&pool);
				VECTOR(void *) vec (ap);
				ALLOC(size_t) sp(&pool);
				std::less<void *> eq;
				MAP(void *, size_t) vmp(eq, sp);

				long total = 0;
				for (int i=0; i<1<<16; ++i) {
					int lp = rand()%100;
					for (int j=0; j<lp; ++j) {
						int siz = rand()%(1<<17) + rand()%(1<<16);
						void *ptr = pool.malloc(siz);
						if (ptr == NULL) {
							std::cout<<"NONE "<<siz<<std::endl;
							break;
						}
						((char *)ptr)[0] = 'a';
						((char *)ptr)[siz-1] = 'a';
						vec.push_back(ptr);
						vmp[ptr] = siz;
						total += siz;
					}
					long rel = total / 2;
					while (rel > 0) {
						if (vec.size() == 0) break;
						rel -= vmp[vec.back()];
						pool.free (vec.back(), vmp[vec.back()]);
						vec.pop_back();
					}
				}
			}

			std::cout<<idx<<std::endl;
		}
	}
};


#endif  //__BSL_TEST_MEMPOOL_STABLE_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
