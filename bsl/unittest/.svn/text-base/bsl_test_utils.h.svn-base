/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_test_utils.h,v 1.3 2008/12/15 09:57:00 xiaowei Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file bsl_test_utils.h
 * @author xiaowei(com@baidu.com)
 * @date 2008/11/09 12:19:44
 * @version $Revision: 1.3 $ 
 * @brief 
 *  
 **/


#ifndef  __BSL_TEST_UTILS_H_
#define  __BSL_TEST_UTILS_H_

#include <cxxtest/TestSuite.h>
#include <bsl/utils/bsl_utils.h>
#include <iostream>
#include <stdio.h>

#define __XASSERT(flag, fmt, arg...)  \
{\
	bool ___bsl_flag = flag; \
	if (!(___bsl_flag)) { \
		fprintf(stdout, "\n[error][%s:%d][%s]"fmt"\n", __FILE__, __LINE__, #flag, ##arg); \
		return false; \
	}\
}

#define __XASSERT2(flag) __XASSERT(flag, "")

bool test_max () {
	int a = 10, b = 5, c = 10;
	int min = bsl::min<int>(a, b);
	int max = bsl::max<int>(a, b);
	if (min > max) { 
		max = min;
	}
	bsl::equal<int>()(a, c);
	bsl::more<int>()(a, b);
	bsl::less<int>()(b, a);

	printf("\nhello world\n");
	//std::cout<<bsl::high_bitpos<0x1234>::value<<std::endl;
	//std::cout<<bsl::moreq2<0x1234>::value<<std::endl;
	//-------13
	//-------0x2000
	//-------0x8000
	//-------32768
	printf("-------%d\n", bsl::high_bitpos<0x1234>::value);
	__XASSERT2(bsl::high_bitpos<0x1234>::value == 13);
	printf("-------0x%lx\n", bsl::moreq2<0x1234>::value);
	__XASSERT2(bsl::moreq2<0x1234>::value == 0x2000UL);
	printf("-------0x%lx\n", bsl::moreq2<0x5678>::value);
	__XASSERT2(bsl::moreq2<0x5678>::value == 0x8000UL);
	printf("-------%ld\n", bsl::moreq2<0x5678>::value);
	__XASSERT2(bsl::moreq2<0x5678>::value == 32768UL);

	return true;
}

class bsl_test_utils : public CxxTest::TestSuite
{
public:
	void test_max_() {
		TSM_ASSERT("", test_max());
	}
};

















#endif  //__BSL_TEST_UTILS_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
