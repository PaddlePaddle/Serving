/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file pool.cpp
 * @author yufan(com@baidu.com)
 * @date 2008/12/08 14:44:56
 * @brief 
 *  
 **/

#include "bsl_cachedpool.h"

namespace bsl
{
void * cached_mempool::malloc(size_t size)
{
	size = (size + 7 ) & ( -1 << 3);
	if(size < seg_size - sizeof(void *))
	{
		if(size > buf_size)
		{
			void *tmp = ::malloc(seg_size);
			if(tmp)
			{
				*(void **)tmp = memlist;
				memlist = tmp;
				free_space = (char *)tmp + sizeof(void *);
				buf_size = seg_size - sizeof(void *);
			}
			else
				return 0;
		}
		buf_size -= size;
		void * p = free_space;
		free_space += size;
		return p;
	}
	else 
	{
		void *tmp = ::malloc(size + sizeof(void *));
		if(tmp)
		{
			*(void **)tmp = memlist;
			memlist = tmp;
			return ((char *)tmp + sizeof(void *));
		}
		else
			return 0;

	}
}

}


















/* vim: set expandtab ts=4 sw=4 sts=4 tw=100 */
