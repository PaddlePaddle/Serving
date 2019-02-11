/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_hashutils.h,v 1.3 2008/12/15 09:56:59 xiaowei Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file bsl_hashutils.h
 * @author xiaowei(com@baidu.com)
 * @date 2008/07/13 17:40:15
 * @version $Revision: 1.3 $ 
 * @brief 
 *  
 **/


#ifndef  __BSL_HASHUTILS_H_
#define  __BSL_HASHUTILS_H_
#include <algorithm>

#include "bsl_hash_fun.h"

namespace bsl
{

enum {
	HASH_EXIST = 0xffff,	//hash值存在
	HASH_NOEXIST,	//hash值不存在
	HASH_OVERWRITE,	//覆盖原有的hash值
	HASH_INSERT_SEC,	//插入成功
};

#if 0
template <class _Tp>
struct xhash
{
	inline size_t operator () (const _Tp &_1) const {
		return size_t(_1);
	}
};
#endif

enum { __bsl_num_primes = 28 };

static const unsigned long __bsl_prime_list[__bsl_num_primes] =
{
	53ul,         97ul,         193ul,       389ul,       769ul,
	1543ul,       3079ul,       6151ul,      12289ul,     24593ul,
	49157ul,      98317ul,      196613ul,    393241ul,    786433ul,
	1572869ul,    3145739ul,    6291469ul,   12582917ul,  25165843ul,
	50331653ul,   100663319ul,  201326611ul, 402653189ul, 805306457ul, 
	1610612741ul, 3221225473ul, 4294967291ul
};

inline unsigned long __bsl_next_prime(unsigned long __n)
{
	const unsigned long* __first = __bsl_prime_list;
	const unsigned long* __last = __bsl_prime_list + (int)__bsl_num_primes;
	const unsigned long* pos = std::lower_bound(__first, __last, __n);
	return pos == __last ? *(__last - 1) : *pos;
}

};


#ifndef BSL_USER_LOCK
#define BSL_RWLOCK_T pthread_rwlock_t
#define BSL_RWLOCK_INIT(lock) pthread_rwlock_init(lock, NULL)
#define BSL_RWLOCK_DESTROY(lock) pthread_rwlock_destroy(lock)
#define BSL_RWLOCK_RDLOCK(lock) pthread_rwlock_rdlock(lock)
#define BSL_RWLOCK_WRLOCK(lock) pthread_rwlock_wrlock(lock)
#define BSL_RWLOCK_UNLOCK(lock) pthread_rwlock_unlock(lock)
#endif











#endif  //__BSL_HASHUTILS_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
