/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_debug.h,v 1.1 2008/09/03 06:47:49 xiaowei Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file bsl_debug.h
 * @author xiaowei(com@baidu.com)
 * @date 2008/07/25 20:49:30
 * @version $Revision: 1.1 $ 
 * @brief 
 *  
 **/


#ifndef  __BSL_DEBUG_H_
#define  __BSL_DEBUG_H_

#include <stdio.h>
#include <assert.h>
#include <sys/time.h>

namespace bsl
{

#ifdef BSL_DEBUG_MOD

#define __BSL_DEBUG(fmt, arg...) \
{ \
	fprintf(stdout, "[----------debug--------][%s:%d]" fmt "\n", __FILE__, __LINE__, ##arg); \
}
#define __BSL_ASSERT(flag, fmt, arg...) \
{\
	bool ___bsl_flag = flag; \
	if (!(___bsl_flag)) { \
		fprintf(stdout, "[error][%s:%d][%s]" fmt "\n", __FILE__, __LINE__, #flag, ##arg); \
	}\
	assert (___bsl_flag); \
}


class auto_xtimer            
{
	char buf[64];  
	timeval s, e;
	public:             
	auto_xtimer(const char *str = NULL) {                 
		buf[0] = 0;
		if (str) snprintf(buf, sizeof(buf), "%s", str);
		gettimeofday(&s, NULL);
	}
	~auto_xtimer() {
		gettimeofday(&e, NULL);                                           
		long t = (long)((e.tv_sec - s.tv_sec) * 1000000 + (e.tv_usec - s.tv_usec));
		__BSL_DEBUG("%s %ld", buf, t);                               
	}    
};

#define AUTO_TIMER(str) bsl::auto_xtimer t(str)
#else
#define __BSL_DEBUG(fmt, arg...) 
#define __BSL_ASSERT(...) 
#define AUTO_TIMER(str)
#endif

#define __BSL_ERROR(fmt, arg...) \
{ \
	fprintf(stdout, "[error][%s:%d]" fmt "\n", __FILE__, __LINE__, ##arg); \
}



};
















#endif  //__BSL_DEBUG_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
