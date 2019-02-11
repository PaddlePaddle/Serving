/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_cachedpool.h,v 1.5 2009/01/04 09:03:34 yufan Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file /home/xiaowei/libsrc/bsl/pool/bsl_cachedpool.h
 * @author xiaowei(com@baidu.com)
 * @date 2008/12/08 18:33:15
 * @version $Revision: 1.5 $ 
 * @brief 
 *  
 **/


#ifndef  _BSL_CACHEDPOOL_H_
#define  _BSL_CACHEDPOOL_H_

#include "bsl_pool.h"

namespace bsl
{
class cached_mempool : public mempool
{
	static const int seg_size = 4096;
	size_t buf_size;
	char * free_space;
	void * memlist;
	public:
	inline cached_mempool()
	{
		buf_size = 0;
		free_space = 0;
		memlist = 0;
	}
	virtual void * malloc(size_t size);
	virtual	inline void free( void *, size_t)
	{
	}
	inline void clear()
	{
		if(0 == memlist)
			return;
		while(*(void **)memlist)
		{
			void * tmp = *(void **)memlist;
			::free(memlist);
			memlist = tmp;
		}
                // 这里空间上可能会有一些浪费
                // 因为我们不知道最后一块大小内存为多少
                // 但是我们这里可以保守估计为seg_size这么大小
                // 因为每一个memlist的大小至少为seg_size.
                free_space=(char*)memlist+sizeof(void*);
                buf_size=seg_size-sizeof(void*);                
	}
	inline virtual ~cached_mempool()
	{
		while(memlist)
		{
			void * tmp = *(void **)memlist;
			::free(memlist);
			memlist = tmp;
		}
	}
};


}















#endif  //__/HOME/XIAOWEI/LIBSRC/BSL/POOL/BSL_CACHEDPOOL_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
