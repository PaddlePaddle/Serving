/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_test_pool.h,v 1.14 2009/03/09 04:56:42 xiaowei Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file bsl_test_pool.h
 * @author xiaowei(com@baidu.com)
 * @date 2008/12/08 19:52:09
 * @version $Revision: 1.14 $ 
 * @brief 
 *  
 **/


#ifndef  __BSL_TEST_POOL_H_
#define  __BSL_TEST_POOL_H_


#include <bsl/pool/bsl_pool.h>
#include <bsl/pool/bsl_poolalloc.h>
#include <bsl/pool/bsl_xmempool.h>
#include <bsl/pool/bsl_xcompool.h>
#include <bsl/pool/bsl_debugpool.h>
#include <vector>
#include <cxxtest/TestSuite.h>
#include <vector>
#include <set>

#define __XASSERT(flag, fmt, arg...)  \
{\
	bool ___bsl_flag = flag; \
	if (!(___bsl_flag)) { \
		fprintf(stdout, "\n[error][%s:%d][%s]"fmt"\n", __FILE__, __LINE__, #flag, ##arg); \
		return false; \
	}\
}

#define __XASSERT2(flag) __XASSERT(flag, "")


typedef bool (*fun_t)(bsl::mempool *);

bool test_normal_pool (bsl::mempool *pool)
{
	std::vector<void *> vec;
	std::vector<size_t> vec2;
	//size_t sizv[] = {0, 4, 16, 1<<21, 1<<19};
	size_t vsize = 1000;
	size_t cnt = 0;
	for (size_t i=0; i<vsize; ++i) {
		size_t size = rand()%(1<<20)*2;//sizv[i];
		void * ptr = pool->malloc(size);
		if (ptr) {
			memset(ptr, 0, size);
			cnt += size;
			vec.push_back(ptr);
			vec2.push_back(size);
			void *ptr2 = vec.back();
			size_t ptr2siz = vec2.back();
		    pool->free (ptr2, ptr2siz);
		}
	}
	std::cout<<(cnt>>20)<<std::endl;
	vec.clear();
	vec2.clear();

	for (size_t i=0; i<vsize; ++i) {
		size_t size = rand()%(1<<20) * 2;
		void *ptr = pool->malloc (size);
		if (ptr) {
			memset (ptr, 0, size);
			cnt += size;
			vec.push_back(ptr);
			vec2.push_back(size);
		}
	}
	for (size_t i=0; i<vec.size(); ++i) {
		pool->free (vec[i], vec2[i]);
	}
	return true;
}

bool test_xmempool (fun_t op)
{
	bsl::xmempool *pool = new bsl::xmempool;
	size_t size = 1<<24;
	void *dat = malloc (size);
	pool->create(dat, size, 2, 1<<16, 1.8);
	__XASSERT2(op((bsl::mempool *)pool));

	pool->clear();
	for (int i=0; i<1<<16; i = int(float(i)*1.5) + 1) 
		std::cout<<i<<" "<< pool->goodsize(i) <<" "<<pool->goodsize(pool->goodsize(i))<<" "
			<<pool->goodsize(pool->goodsize(i)+1)<<std::endl;

	//pool->destroy();
	delete pool;
	free (dat);
	{
		bsl::xmempool pool2;
	}
	return true;
}

bool test_xcompool (fun_t op)
{
	bsl::xcompool *pool = new bsl::xcompool;
	pool->create(1<<20, 2,  1<<16, 2.0f);
	__XASSERT2(op((bsl::mempool *)pool));
	pool->destroy();
	delete pool;
	{
		bsl::xcompool pool2;
	}
	return true;
};

bool test_debugpool (fun_t op)
{
	bsl::debugpool *pool = new bsl::debugpool;
	__XASSERT2(op((bsl::mempool *)pool));
	pool->free(pool->malloc(10), 10);
	delete pool;
	return true;
}

bool g_exp = true;
template <typename Tp>
bool test_stl (bsl::mempool *pool)
{
#ifndef __i386
	typedef bsl::pool_allocator<Tp> alloc;
	alloc a(pool);
	typedef std::vector<Tp, alloc> vector;
	vector v(a);
	typedef std::set<Tp, std::less<Tp>, alloc> set;
	set s(std::less<Tp>(), a);
	size_t num = 10000;
	try {
		for (size_t i=0; i<num; ++i) {
			v.push_back((Tp)(i));
			s.insert((Tp)(i));
		}
	} catch (...) {
		__XASSERT(g_exp, "exp");
	}
	std::sort(v.begin(), v.end(), std::greater<Tp>());
	for (size_t i=0; i<v.size(); ++i) {
		if (s.find((Tp)i) == s.end()) {
			__XASSERT(false, "not find");
		}
	}
#endif
	return true;
}

bool test_comp (bsl::mempool *pool)
{
#ifndef __i386
	typedef bsl::pool_allocator<char> alloc_c;
	typedef std::basic_string<char, std::char_traits<char>, alloc_c> string;
	typedef bsl::pool_allocator<string> alloc_s;
	typedef std::vector<string, alloc_s> vector;
	typedef std::set<string, std::less<string>, alloc_s> set;

	alloc_c ca(pool);
	alloc_s cs(pool);

	string a(ca);
	a = "helloa";
	string b(ca);
	b = "hellob";
	string c(ca);
	c = "helloc";

	size_t slen = 1000;
	vector v(cs);
	for (size_t i=0; i<slen; ++i) {
		v.push_back(a);
		v.push_back(b);
		v.push_back(c);
	}
	std::sort(v.begin(), v.end(), std::greater<string>());
	set s(std::less<string>(), cs);
	for (size_t i=0; i<v.size(); ++i) {
		s.insert(v[i]);
	}
#endif

	return true;
}

bool test_compool (bsl::mempool *pool)
{
	for (int i=0; i<1<<24; ++i) {
		__XASSERT2(pool->malloc(rand()%16+1) != NULL);
	}
	return true;
}

class bsl_test_pool : public CxxTest::TestSuite
{
public:
	void test_normal_pool_ () {
		TSM_ASSERT(0, test_xmempool(test_normal_pool));
		TSM_ASSERT(0, test_xcompool(test_normal_pool));
		TSM_ASSERT(0, test_xcompool(test_compool));
	}
	void test_stl_pool_ () {
		g_exp = true;
		TSM_ASSERT(1, test_xmempool(test_stl<int>));
		TSM_ASSERT(1, test_xmempool(test_stl<char>));
		TSM_ASSERT(1, test_xmempool(test_stl<float>));
		TSM_ASSERT(1, test_xmempool(test_stl<double>));

		g_exp = false;
		TSM_ASSERT(1, test_xcompool(test_stl<int>));
		TSM_ASSERT(1, test_xcompool(test_stl<char>));
		TSM_ASSERT(1, test_xcompool(test_stl<float>));
		TSM_ASSERT(1, test_xcompool(test_stl<double>));

		TSM_ASSERT(1, test_debugpool(test_stl<int>));
	}
	void test_stl_string_ () {
		TSM_ASSERT(2, test_xmempool(test_comp));
		TSM_ASSERT(2, test_xcompool(test_comp));
	}

	void test_pool_allocator_() {
		{
			bsl::syspool pool;
			bsl::pool_allocator<char> alloc((bsl::mempool *)&pool);
			try {
				alloc.allocate(1<<30);
				alloc.allocate(1<<30);
				alloc.allocate(1<<30);
				alloc.allocate(1<<30);
			} catch (...) {
				std::cout<<"Exception"<<std::endl;
			}
		}

		{
			bsl::xcompool pool;
			pool.create ();
			bsl::pool_allocator<char> alloc((bsl::mempool *) &pool);
			try {
				alloc.allocate(1<<30);
				alloc.allocate(1<<30);
				alloc.allocate(1<<30);
				alloc.allocate(1<<30);
			} catch(bsl::BadAllocException &e){
				std::cout<<"Exception"<<std::endl;
			}
		}
		{
			try{
				bsl::xcompool *pool = new bsl::xcompool;
				pool->create();
				bsl::pool_allocator<double>  alloc(pool);
				int size = 1<<30;
				alloc.allocate(size);
				delete pool;
			}
			catch(bsl::BadAllocException &e){
				std::cout << "bad alloc!\n" << std::endl;
			}
		}
		{
			bsl::xcompool pool;
		}
		{
			typedef bsl::pool_allocator<char> _Alloc;
			typedef  _Alloc::pointer pointer;
			typedef  _Alloc::const_reference const_reference;
			int size = 1000;
			int size2 = 1073741824;
			char *buf = (char*)malloc(sizeof(char)*size);
			bsl::xmempool *pool = new bsl::xmempool;
			int ret = pool->create(buf, size);
			if (ret != 0) return;
			_Alloc alloc(pool);
			pointer p;
			try{
				p = alloc.allocate(size2);
				TSM_ASSERT(p, p != NULL);
				delete pool;
			}
			catch(bsl::BadAllocException &e){
				std::cout << "bad alloc!\n" << std::endl;
				free(buf);
			}
		}
		{
			bsl::xcompool pool;
			assert (pool.create() == 0);
			assert (pool.create(0) != 0);
			assert (pool.create(1<<10) == 0);
			assert (pool.malloc(1<<11) != 0);
			assert (pool.create(1<<16) == 0);
			assert (pool.malloc(1<<17) != 0);
			assert (pool.create(1<<17) == 0);
		}

		{
			bsl::xcompool pool;
			assert (pool.create(1<<10) == 0);
			for (int k=0; k<2; ++k) {
				for (int i=0; i<(1<<22); ++i) {
					void * ret = (void *)pool.malloc(i);
					assert (ret != 0);
					pool.free (ret, i);
					if (i%(1<<20) == 0) {
						std::cout<<i<<std::endl;
					}
				}
				pool.clear();
			}
		}

		{
			bsl::xcompool pool;
			pool.create(0);
		}
	}
};




#endif  //__BSL_TEST_POOL_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
