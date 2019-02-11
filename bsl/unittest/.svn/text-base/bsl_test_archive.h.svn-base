/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_test_archive.h,v 1.2 2008/12/15 09:57:00 xiaowei Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file bsl_test_archive.h
 * @author xiaowei(com@baidu.com)
 * @date 2008/11/12 16:37:59
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/


#ifndef  __BSL_TEST_ARCHIVE_H_
#define  __BSL_TEST_ARCHIVE_H_


#include <cxxtest/TestSuite.h>
#include <bsl/archive/bsl_serialization.h>
#include <bsl/archive/bsl_filestream.h>
#include <bsl/archive/bsl_binarchive.h>

#define __XASSERT(flag, fmt, arg...)  \
{\
	bool ___bsl_flag = flag; \
	if (!(___bsl_flag)) { \
		fprintf(stdout, "\n[error][%s:%d][%s]"fmt"\n", __FILE__, __LINE__, #flag, ##arg); \
		return false; \
	}\
}

#define __XASSERT2(flag) __XASSERT(flag, "")

bool test_archive(void)
{
	bsl::filestream fs;
	fs.open ("test_archive.dat", "w");
	bsl::binarchive ar(fs);
	fs.start_trans();
	const char *ptr = "hello world";
	ar.push(ptr, strlen(ptr));
	fs.drop_trans(true);
	fs.close();
	return true;
}

class bsl_test_archive : public CxxTest::TestSuite
{
public:
	void test_archive_ (void) {
		TSM_ASSERT(0, test_archive());
	}
};
















#endif  //__BSL_TEST_ARCHIVE_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
