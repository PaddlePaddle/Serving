/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_xmempool.cpp,v 1.4 2008/12/25 05:44:10 yufan Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file bsl_xmempool.cpp
 * @author xiaowei(com@baidu.com)
 * @date 2008/12/08 16:27:15
 * @version $Revision: 1.4 $ 
 * @brief 
 *  
 **/

#include "bsl_xmempool.h"

namespace bsl
{

#define DELSIZE(rsize, size) \
	size_t rsize = (((size) + sizeof(int) - 1) / sizeof(int) * sizeof(int))

#define RSIZE(rsize, size) \
	rsize = (((size) + sizeof(int) - 1) / sizeof(int) * sizeof(int))

xmempool::xmempool()
{
	_maxsiz = 0;
	_pool = 0;
	_rate = 0;
}

xmempool::~xmempool()
{
	destroy();
}


int xmempool::create(void *buf, size_t bufsiz,
		size_t bmin, size_t bmax, float rate)
{
	DELSIZE(__bmin, bmin);
	DELSIZE(__bmax, bmax);
	if (__bmin < sizeof(void *)) __bmin = sizeof(void *);
	if (__bmin > __bmax) return -1;

	if (buf == NULL || bufsiz == 0 || rate < 1.01f) return -1;

	size_t __b = __bmin;
	int lvl = 1;
	while (__b < __bmax) {
		++ lvl;
		RSIZE(__b, (size_t)(__b * rate));
	}

	_mlc.create(buf, bufsiz);
	_poolsize = lvl;
	_minsiz = __bmin;
	_rate = rate;
	return clear();
}

int xmempool::clear()
{
	return clear (_mlc._buffer, _mlc._bufcap);
}

int xmempool::clear (void *buffer, size_t size)
{
	_mlc.create (buffer, size);
	_pool = (xfixmemg *)_mlc.malloc(sizeof(xfixmemg) * _poolsize);
	if (_pool == NULL) return -1;

	_maxsiz = 0;
	size_t m = _minsiz;
	for (int i=0; i<_poolsize; ++i) {
		_pool[i].create(m);
		_maxsiz = m;
		RSIZE(m, (size_t)(m * _rate));
	}
	return 0;
}

int xmempool::destroy()
{
	_mlc.destroy();
	_pool = 0;
	_maxsiz = 0;
	_minsiz = 0;
	_poolsize = 0;
	_rate = 0;
	return 0;
}

int xmempool::getidx(size_t size)
{
	if (size > _pool[_poolsize-1]._size) return -1;
	int len = _poolsize, half=0, mid=0, first=0;

	while (len > 0) {
		half = len >> 1;
		mid = first + half;
		if (_pool[mid]._size < size) {
			first = mid + 1;
			len = len - half - 1;
		} else {
			len = half;
		}
	}
	return first;
}

void * xmempool::addbuf (void *buf, size_t size)
{
	void * oldbuf = _mlc._buffer;
	for (int i=_poolsize-1; i>=0; --i) {
		void * ret = NULL;
		while ( (ret = _mlc.malloc(_pool[i]._size)) != NULL) {
			_pool[i].free (ret);
		}
	}
	_mlc.create (buf, size);
	return oldbuf;
}

};


/* vim: set ts=4 sw=4 sts=4 tw=100 */
