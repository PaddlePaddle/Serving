/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_memcpy.cpp,v 1.3 2009/10/14 08:24:59 chenxm Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file xmemcpy.cpp
 * @author xiaowei(com@baidu.com)
 * @date 2008/11/07 17:20:48
 * @version $Revision: 1.3 $ 
 * @brief 
 *  
 **/

#include "bsl_memcpy.h"

namespace bsl
{

class xmembase
{
public:
	virtual void * copy (void *dest, const void *src) = 0;
    virtual ~xmembase(){}
};

static const size_t XMEM_MAXSIZE = 120;

template <size_t size>
struct xmemcpy_t
{
	int data[size];
};

template <>
struct xmemcpy_t<0>
{
};

template <size_t size>
class xmemcopy : public xmembase
{
public:
	void * copy (void *dest, const void *src)
	{
		typedef xmemcpy_t<size/sizeof(int)> type_t;
		//if (size/sizeof(int) > 0) {
			*((type_t *)dest) = *((type_t *)src);
		//}
		//编译的时候是常数,if不存在判断性能损失
		if ((size%sizeof(int)) > 0) {
			((char *)dest)[size-1] = ((char *)src)[size-1];
		}
		if ((size%sizeof(int)) > 1) {
			((char *)dest)[size-2] = ((char *)src)[size-2];
		}
		if ((size%sizeof(int)) > 2) {
			((char *)dest)[size-3] = ((char *)src)[size-3];
		}
		return dest;
	}
};

template <>
class xmemcopy<0> : public xmembase
{
public:
    void * copy (void *dest, const void *src) {
        return dest;
        (void)src;
    }
};


static xmembase *g_base[XMEM_MAXSIZE] = {NULL};

template <size_t len>
void init() {
	if (g_base[len] == NULL) {
		g_base[len] = new xmemcopy<len>;
	}
	init<len-1>();
}

template <>
void init<0> () {
	if (g_base[0] == NULL) {
		g_base[0] = new xmemcopy<0>;
	}
}


class xmem_monitor
{
public:
	xmem_monitor() {
		init<XMEM_MAXSIZE-1>();
	}
	~xmem_monitor() {
        for (size_t i = 0; i < XMEM_MAXSIZE; ++i) {
			if (g_base[i]) {
				delete g_base[i];
				g_base[i] = NULL;
			}
		}
	}
};

static xmem_monitor g_monitor;



void *xmemcpy (void *dest, const void *src, size_t len)
{
	//防止重叠
	long dst = ((long)dest - (long)src);
	if ( (dst < (long)len && dst > 0 - (long)len) ||
			len >= XMEM_MAXSIZE) {
		return ::memcpy(dest, src, len);
	}

	if (g_base[len] == NULL) {
		return ::memcpy(dest, src, len);
	}

	return g_base[len]->copy(dest, src);
}

}



/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
