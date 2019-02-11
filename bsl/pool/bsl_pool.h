/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_pool.h,v 1.7 2009/03/09 04:56:42 xiaowei Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file idl.h
 * @author yufan(com@baidu.com)
 * @date 2008/11/26 11:52:08
 * @version $Revision: 1.7 $ 
 * @brief 
 *  
 **/


#ifndef  __BSL_POOL_H_
#define  __BSL_POOL_H_
#include <stdint.h>
#include <stdlib.h>
#include <stdexcept>
#include <assert.h>

namespace bsl
{
	class mempool
	{
	public:
		virtual void * malloc (size_t size) = 0;
		virtual void free (void *p, size_t size) = 0;
		inline virtual ~mempool()
		{
		}
	};
	class syspool : public mempool
	{
	public:
		inline virtual	void * malloc (size_t size) {
			return ::malloc(size);
		}
		inline virtual void free (void *p, size_t) {
			::free(p);
		}
	};


} //namespace bsl















#endif  //__IDL_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
