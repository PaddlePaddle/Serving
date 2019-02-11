/***************************************************************************
 * 
 * Copyright (c) 2009 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_test_memcpy.h,v 1.2 2009/04/07 06:35:53 xiaowei Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file bsl_test_memcpy.h
 * @author xiaowei(com@baidu.com)
 * @date 2009/01/06 19:01:23
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/


#ifndef  __BSL_TEST_MEMCPY_H_
#define  __BSL_TEST_MEMCPY_H_

#include <cassert>
#include <stdlib.h>
#include "bsl/utils/bsl_memcpy.h"
#include <stdio.h>
#include <sys/time.h>

int test1()
{
	{
		char buf[1024], ds[1024];
		for (int i=0; i<(int)sizeof(buf); ++i) {
			buf[i] = 'a';
			ds[i] = 'z'-i%26;
		}
		for (int i=0; i<1024; ++i) {
			bsl::xmemcpy(buf, ds, i);
			for (int j=0; j<i; ++j) {
				if (buf[j] != ds[j]) {
					printf("error\n");
					assert (0);
				}
			}
		}

	}

	{
		char buf[1024], ds[1024];
		for (int i=0; i<(int)sizeof(buf); ++i) {
			buf[i] = rand()%256;
			ds[i] = rand()%256;
		}
		int lefts = 11;
		int mcp = 100;
		for (int i=0; i<(int)sizeof(buf)-mcp; ++i) {
			bsl::xmemcpy(buf+i, buf+lefts+i, mcp);
			//assert (memcmp(buf+i, buf+lefts+i, mcp) == 0);
		}
	}

	{
		int buf[1024], ds[1024];
		((char *)buf)[0] = 'a';
		((char *)ds)[0] = 'b';
		int len = 9;
		((char *)buf)[len+1] = 'c';
		((char *)ds)[len+1] = 'd';
		bsl::xmemcpy(((char *)buf)+1, ((char *)ds)+1, len);
		assert ( ((char *)buf)[0] != ((char *)ds)[0] );
		assert ( ((char *)buf)[len+1] != ((char *)ds)[len+1]);
	}
	return 0;
}

int test2()
{
	char buf1[1024+1], ds1[1024+1];
	char *buf = buf1+1;
	char *ds = ds1+1;
	int loop = 1<<20;
	int siz = 130;
	for (int i=1; i<siz; ++i) {
		timeval s, e;
		gettimeofday (&s, NULL);
		for (int j=0; j<loop; ++j) {
			bsl::xmemcpy(buf, ds, i);
		}
		gettimeofday (&e, NULL);

		printf("|\t%d\t|\t%ld\t|",i, (e.tv_sec-s.tv_sec)*1000 + (e.tv_usec-s.tv_usec)/1000);

		gettimeofday (&s, NULL);
		for (int j=0; j<loop; ++j) {
			memcpy(buf, ds, i);
		}
		gettimeofday (&e, NULL);
		printf("\t%ld\t|\n", (e.tv_sec-s.tv_sec)*1000 + (e.tv_usec-s.tv_usec)/1000);
#if 0
		gettimeofday (&s, NULL);
		for (int j=0; j<loop; ++j) {
			com::copy(buf, ds, i);
		}
		gettimeofday (&e, NULL);

		printf("\t%ld\t|\n", (e.tv_sec-s.tv_sec)*1000 + (e.tv_usec-s.tv_usec)/1000);
#endif
	}

	return 0;
}





class bsl_test_fun : public CxxTest::TestSuite
{
public:
	void test_normal()
	{
		test1();
		test2();
	}
};











#endif  //__BSL_TEST_MEMCPY_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
