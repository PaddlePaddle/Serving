/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_test_alloc.h,v 1.2 2008/12/15 09:57:00 xiaowei Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file bsl_test_alloc.h
 * @author xiaowei(com@baidu.com)
 * @date 2008/08/22 17:38:52
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/


#ifndef  __BSL_TEST_ALLOC_H_
#define  __BSL_TEST_ALLOC_H_
#include <cxxtest/TestSuite.h>
#include <bsl/alloc/allocator.h>
#include <vector>


#define __XASSERT(flag, fmt, arg...)  \
{\
	bool ___bsl_flag = flag; \
	if (!(___bsl_flag)) { \
		fprintf(stdout, "\n[error][%s:%d][%s]"fmt"\n", __FILE__, __LINE__, #flag, ##arg); \
		return false; \
	}\
}

#define __XASSERT2(flag) __XASSERT(flag, "")



template <class _Alloc>
bool test_normal()
{
	typedef typename _Alloc::value_type value_type;
	typedef typename _Alloc::pointer pointer;
	typedef std::vector<value_type> vec_t;
	typedef std::vector<pointer> vecp_t;

	_Alloc alloc;
	__XASSERT2(alloc.create() == 0);
	vec_t vecs;
	vecp_t vecp;
	int loop = 100000;
	for (int i=0; i<loop; ++i) {
		pointer p = alloc.allocate(1);
		__XASSERT2(p != 0);
		value_type v = rand();
		(*alloc.getp(p)) = v;
		vecs.push_back(v);
		vecp.push_back(p);
	}
	for (int i=0; i<loop; ++i) {
		__XASSERT2(*alloc.getp(vecp[i]) == vecs[i]);
	}
	for (int i=0; i<loop; ++i) {
		alloc.deallocate(vecp[i], 1);
	}

	for (int i=0; i<loop; ++i) {
		pointer p = alloc.allocate(1);
		__XASSERT2(p != 0);
		alloc.deallocate(p, 1);
	}

	vecs.clear();
	vecp.clear();

	for (int i=0; i<loop; ++i) {
		pointer p = alloc.allocate(1);
		__XASSERT2(p != 0);
		value_type v = rand();
		(*alloc.getp(p)) = v;
		vecs.push_back(v);
		vecp.push_back(p);
	}
	for (int i=0; i<loop; ++i) {
		__XASSERT2(*alloc.getp(vecp[i]) == vecs[i]);
		alloc.deallocate(vecp[i], 1);
	}
	return true;
}

class bsl_test_fun : public CxxTest::TestSuite
{
public:
	typedef bsl::bsl_alloc<int> intalloc_t;
	void test_normal_int(void) {   
		TSM_ASSERT (0, test_normal<intalloc_t>());
	}   

	typedef bsl::bsl_alloc<double> dballoc_t;
	void test_normal_double() {
		TSM_ASSERT (0, test_normal<dballoc_t>());
	}

	typedef bsl::bsl_sample_alloc<intalloc_t, 256 > intsalloc_t;
	void test_salloc_int(void) {
		TSM_ASSERT (0, test_normal<intsalloc_t>());
	}

	typedef bsl::bsl_sample_alloc<dballoc_t, 256> dbsalloc_t;
	void test_salloc_double() {
		TSM_ASSERT (0, test_normal<dbsalloc_t>());
	}

	typedef bsl::bsl_cpsalloc<intalloc_t> intcpalloc_t;
	void test_cpalloc_int() {
		TSM_ASSERT (0, test_normal<intcpalloc_t>());
	}

	typedef bsl::bsl_cpsalloc<dballoc_t> dbcpalloc_t;
	void test_cpalloc_double() {
		TSM_ASSERT (0, test_normal<dbcpalloc_t>());
	}
};




#endif  //__BSL_TEST_ALLOC_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
