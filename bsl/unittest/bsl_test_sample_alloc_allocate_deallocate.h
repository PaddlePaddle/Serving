/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_test_sample_alloc_allocate_deallocate.h,v 1.2 2008/12/15 09:57:00 xiaowei Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file bsl_test_sample_alloc_allocate_deallocate.h
 * @author xiaowei(com@baidu.com)
 * @date 2008/10/28 11:44:27
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/


#ifndef  __BSL_TEST_SAMPLE_ALLOC_ALLOCATE_DEALLOCATE_H_
#define  __BSL_TEST_SAMPLE_ALLOC_ALLOCATE_DEALLOCATE_H_


#include <cxxtest/TestSuite.h>
#include <bsl/alloc/allocator.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <list>
#include "define.rc"

template <class _Alloc>
void test_allocate_deallocate_1()
{
	typedef typename _Alloc::self _Self;  // bsl_sample_alloc
	typedef typename _Self::pointer pointer;

	_Self alloc;
	pointer p;
	int ret;

	ret = alloc.create();
	TSM_ASSERT(ret, ret == 0);
	
	for(int i = 0; i < 100000; i++){
	        p = alloc.allocate(1);
			if (p == NULL) {
				fprintf(stderr, "\n%d %m\n", i);
			}
        	TSM_ASSERT(p, p != NULL);
        	alloc.deallocate(p, 1);
	}

	ret = alloc.destroy();
        TSM_ASSERT(ret, ret == 0);
}

template <class _Alloc>
void test_allocate_deallocate_2()
{
        typedef typename _Alloc::self _Self;  // bsl_sample_alloc
        typedef typename _Self::pointer pointer;

        _Self alloc;
        int ret;

        ret = alloc.create();
        TSM_ASSERT(ret, ret == 0);

        alloc.deallocate(NULL, 1);

        ret = alloc.destroy();
        TSM_ASSERT(ret, ret == 0);
}

class test_bsl_sample_alloc_allocate_deallocate : public CxxTest::TestSuite
{
public:
	// 1)	¡Â?¡Ç£¤??£¤OEallocate¡Ò? deallocate
	void test_bsl_sample_alloc_allocate_deallocate_1(void)
	{
		typedef bsl::bsl_alloc<MyClass> alloc_t_1;
		typedef bsl::bsl_sample_alloc<alloc_t_1, 1000> cpsalloc_t_1;
		typedef bsl::bsl_alloc<std::string> alloc_t_2;
		typedef bsl::bsl_sample_alloc<alloc_t_2, 1000> cpsalloc_t_2;

		typedef bsl::bsl_alloc<std::list<double> > alloc_t_3;
		typedef bsl::bsl_sample_alloc<alloc_t_3, 1000> cpsalloc_t_3;

		typedef bsl::bsl_alloc<MyStruct> alloc_t_4;
		typedef bsl::bsl_sample_alloc<alloc_t_4, 1000> cpsalloc_t_4;

		test_allocate_deallocate_1<cpsalloc_t_1>();
		test_allocate_deallocate_1<cpsalloc_t_2>();
		test_allocate_deallocate_1<cpsalloc_t_3>();
		test_allocate_deallocate_1<cpsalloc_t_4>();
	}

	// 2)	¡ÜOE ?_Items¦¸oe£¤? ¡À¡ê¡§¦Ì~¡±¡Ìallocate
	void test_bsl_sample_alloc_allocate_deallocate_2(void)
        {
                typedef bsl::bsl_alloc<MyClass> alloc_t_1;
                typedef bsl::bsl_sample_alloc<alloc_t_1, 1000000> cpsalloc_t_1;

                typedef bsl::bsl_alloc<std::string> alloc_t_2;
                typedef bsl::bsl_sample_alloc<alloc_t_2, 1000000> cpsalloc_t_2;

                typedef bsl::bsl_alloc<std::list<double> > alloc_t_3;
                typedef bsl::bsl_sample_alloc<alloc_t_3, 1000000> cpsalloc_t_3;

		// _Items¡Ò¡´£¤?
                typedef bsl::bsl_alloc<MyStruct> alloc_t_4;
                typedef bsl::bsl_sample_alloc<alloc_t_4, 1000000> cpsalloc_t_4;

                test_allocate_deallocate_1<cpsalloc_t_1>();
                test_allocate_deallocate_1<cpsalloc_t_2>();
                test_allocate_deallocate_1<cpsalloc_t_3>();
                test_allocate_deallocate_1<cpsalloc_t_4>();
        }

	// 3)	¡ÜOE ?pointer ptrOE(TM)NULL ¡À¡ê¡§¦Ì~¡±¡Ìdeallocate
	void test_bsl_sample_alloc_allocate_deallocate_3(void)
        {
/*
                typedef bsl::bsl_alloc<MyClass> alloc_t_1;
                typedef bsl::bsl_sample_alloc<alloc_t_1, 1000> cpsalloc_t_1;
                typedef bsl::bsl_alloc<std::string> alloc_t_2;
                typedef bsl::bsl_sample_alloc<alloc_t_2, 1000> cpsalloc_t_2;
                typedef bsl::bsl_alloc<std::list<double> > alloc_t_3;
                typedef bsl::bsl_sample_alloc<alloc_t_3, 1000> cpsalloc_t_3;

                typedef bsl::bsl_alloc<MyStruct> alloc_t_4;
                typedef bsl::bsl_sample_alloc<alloc_t_4, 1000> cpsalloc_t_4;

                test_allocate_deallocate_2<cpsalloc_t_1>();
                test_allocate_deallocate_2<cpsalloc_t_2>();
                test_allocate_deallocate_2<cpsalloc_t_3>();
                test_allocate_deallocate_2<cpsalloc_t_4>();
*/
		std::cout << "test_bsl_sample_alloc_allocate_deallocate" << std::endl;
        }
};
















#endif  //__BSL_TEST_SAMPLE_ALLOC_ALLOCATE_DEALLOCATE_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
