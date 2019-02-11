/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_test_cpalloc_address.h,v 1.2 2008/12/15 09:57:00 xiaowei Exp $ 
 * 
 **************************************************************************/



/**
 * @file bsl_test_cpalloc_address.h
 * @author xiaowei(com@baidu.com)
 * @date 2008/10/28 11:25:32
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/


#ifndef  __BSL_TEST_CPALLOC_ADDRESS_H_
#define  __BSL_TEST_CPALLOC_ADDRESS_H_

#include <cxxtest/TestSuite.h>
#include <bsl/alloc/allocator.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <list>
#include "define.rc"


template <class _Alloc>
void test_address()
{
	typedef typename _Alloc::_Self _Self;  // cpsalloc
	typedef typename _Alloc::_Base _Base;  // 米⊙≒?alloc

	typedef typename _Self::pointer cpsalloc_pointer;
	typedef typename _Base::pointer alloc_pointer;
	typedef typename _Self::pointer cpsalloc_pointer;
	typedef typename _Base::pointer alloc_pointer;
	typedef typename _Base::reference reference;

	_Base alloc;
	_Self cpsalloc;

	int ret;
	ret = cpsalloc.create();
	TSM_ASSERT(ret, ret == 0);

	cpsalloc_pointer p1 = cpsalloc.allocate(1);
	TSM_ASSERT(p1, p1 != 0);

	alloc_pointer p2 = alloc.allocate(1);
	TSM_ASSERT(p2, p2 != NULL);

	//	cpsalloc_pointer p3 = cpsalloc.address(*p2);
	//	TSM_ASSERT(p3, p3 != 0);


	ret = cpsalloc.destroy();
	TSM_ASSERT(ret, ret == 0);
}


	template <class _Alloc>
void test_address_const()
{
	/*
	   typedef typename _Alloc::pointer pointer;
	   typedef typename _Alloc::const_pointer const_pointer;
	   typedef typename _Alloc::const_reference const_reference;

	   _Alloc alloc;
	   pointer p = alloc.allocate(100);
	   TSM_ASSERT(p, p != NULL);

	   const_pointer p2 = alloc.address((const_reference)*p);
	   TSM_ASSERT(p2, p2 == p);

	   alloc.deallocate(p, 1);
	   */
}

class test_bsl_cpsalloc_address : public CxxTest::TestSuite
{
	public:
		// 1)	﹉米??¯米 ▲pointer??每?米?address次§?/
		void test_bsl_cpsalloc_address_1(void)
		{
			typedef bsl::bsl_alloc<MyClass> alloc_t_1;
			typedef bsl::bsl_cpsalloc<alloc_t_1> cpsalloc_t_1;
			//typedef bsl::bsl_cpsalloc<MyClass> alloc_t_1;
			//		typedef bsl::bsl_cpsalloc<std::string> alloc_t_2;
			//typedef bsl::bsl_cpsalloc<std::list> alloc_t_3;
			//typedef bsl::bsl_cpsalloc<MyStruct> alloc_t_4;

			test_address<cpsalloc_t_1>();
			//test_address<alloc_t_2>();
			//test_address<alloc_t_3>();
			//test_address<alloc_t_4>();
		}

		// 2)	﹉米??¯米 ▲const pointer??每?米?address次§?/
		void test_bsl_cpsalloc_address_2(void)
		{
			/*
			   typedef bsl::bsl_cpalloc<MyClass> alloc_t_1;
			   typedef bsl::bsl_cpalloc<std::string> alloc_t_2;
			//typedef bsl::bsl_cpalloc<std::list> alloc_t_3;
			typedef bsl::bsl_cpalloc<MyStruct> alloc_t_4;

			test_address_const<alloc_t_1>();
			test_address_const<alloc_t_2>();
			//test_address_const<alloc_t_3>();
			test_address_const<alloc_t_4>();

			std::cout << "test_bsl_alloc_address" << std::endl;
			*/
		}
};

















#endif  //__BSL_TEST_CPALLOC_ADDRESS_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
