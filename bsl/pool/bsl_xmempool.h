/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_xmempool.h,v 1.9 2009/06/15 06:29:04 chenxm Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file bsl_xmempool.h
 * @author xiaowei(com@baidu.com)
 * @date 2008/12/08 16:15:29
 * @version $Revision: 1.9 $ 
 * @brief 
 *  
 **/


#ifndef  __BSL_XMEMPOOL_H_
#define  __BSL_XMEMPOOL_H_

#include "bsl_pool.h"
#include <stdlib.h>
#include <stdio.h>

namespace bsl
{

union xmem_node_t
{
	xmem_node_t *next;
	char data[0];
};

class xfixmemg
{
public:
	xmem_node_t *_freelst;
	size_t _size;
public:
	inline xfixmemg() { create(0); }
	inline ~xfixmemg() { create(0); }
	inline void create (int size) {
		_size = size;
		_freelst = 0;
	}
	inline void destroy () {
		create (0);
	}
	inline void * malloc () {
		if (_freelst) {
			xmem_node_t *node = _freelst;
			_freelst = _freelst->next;
			return (void *)node->data;
		}
		return NULL;
	}

	inline void free (void *ptr) {
		((xmem_node_t *)ptr)->next = _freelst;
		_freelst = (xmem_node_t *) ptr;
	}
};

class xnofreepool : public mempool
{
public:
	char * _buffer;
	size_t _bufcap;
	size_t _bufleft;

	size_t _free;
public:
	/**
	 * @brief 创建pool
	 *
	 * @param [in/out] buf   : void* 托管的内存支持
	 * @param [in/out] bufsiz   : size_t	托管的内存大小
	 * @return  void 
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2009/01/05 19:54:30
	**/
	inline void create (void *buf, size_t bufsiz) {
		_buffer = (char *)buf;
		_bufcap = bufsiz;
		_bufleft = bufsiz;
		_free = 0;
	}
	inline void * malloc (size_t size) {
		if (size > _bufleft) return NULL;
		_bufleft -= size;
		return _buffer + _bufleft;
	}
	inline void free (void *, size_t size) {
		_free += size;
#ifdef BSL_NOFREEPOOL_AUTO_CLEAR
		if (_free == _bufcap) {
			clear();
		}
#endif
	}
	inline void clear () {
		_bufleft = _bufcap;
		_free = 0;
	}
	void destroy () {
		_buffer = NULL;
		_bufcap = 0;
		_bufleft = 0;
		_free = 0;
	}

};

class xmempool : public mempool
{
	static const int CNT = 10;

	xfixmemg *_pool;	//实际内存管理器
	int _poolsize;
	xnofreepool _mlc; //实际内存分配器
	size_t _minsiz;
	size_t _maxsiz;	//最大可分配内存
	float _rate;

public:
	xmempool ();
	~xmempool ();

	/**
	 * @brief 初始化内存池
	 *
	 * @param [in] buf   : void* 从哪片内存分配空间
	 * @param [in] bufsiz   : size_t	这片内存多大
	 * @param [in] bmin   : size_t	最小内存分配单元多大
	 * @param [in] bmax   : size_t	最大内存分配单元多大
	 * @param [in] rate   : float	slab 内存增长率
	 * @return  int 成功返回0, 其他-1
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/12/24 10:36:45
	**/
	int create (void * buf, size_t bufsiz, 
			size_t bmin = sizeof(void *), size_t bmax = (1<<20), float rate = 2.0f);
	
	/**
	 * @brief 销毁pool
	 *
	 * @return  int 成功返回0, 失败-1
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/12/24 10:40:43
	**/
	int destroy ();


	/**
	 * @brief 分配内存
	 *
	 * @param [in/out] size   : size_t
	 * @return  void* 分配size大小的内存, 失败返回NULL
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/12/24 10:41:16
	**/
	inline void * malloc (size_t size) {
		int idx = getidx (size);
		if (idx >= 0) {
			void * ret = _pool[idx].malloc();
			if (ret) return ret;
			
			return  _mlc.malloc (_pool[idx]._size);
		}
		return NULL;
	}

	/**
	 * @brief 释放内存
	 *
	 * @param [in/out] ptr   : void*
	 * @param [in/out] size   : size_t 释放的内存大小
	 * @return  void 
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/12/24 10:41:51
	**/
	inline void free (void *ptr, size_t size) {
		if (ptr == NULL) return;
		int idx = getidx(size);
		if (idx >= 0) {
			_pool[idx].free(ptr);
		}
	}

	/**
	 * @brief 允许分配的最大内存
	 *
	 * @return  size_t 
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/12/24 10:42:16
	**/
	inline size_t max_alloc_size() {
		return _maxsiz;
	}

	/**
	 * @brief 回收所有分配内存
	 *
	 * @return  int
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/12/24 10:43:27
	**/
	int clear ();


	/**
	 * @brief 用buffer替换当前内存,并清空
	 *
	 * @param [in/out] buffer   : void* 
	 * @param [in/out] size   : size_t
	 * @return  int 
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/12/24 15:05:27
	**/
	int clear (void *buffer, size_t size);


	/**
	 * @brief 这个接口相当危险, 不推荐使用
	 * 这个接口的功能是,让这个pool在管理新增的内存,
	 * 返回老内存的管理指针.
	 * 但是用户调用clear的时候,被替换的老内存将被无数,而不会重新分配
	 * 这样造成内存泄露 的假象
	 *
	 * 这个接口存在的意义, 是让懂实现的人,方便的利用这个pool的特性做二次开发
	 *
	 * @param [in/out] buf   : void*
	 * @param [in/out] size   : size_t
	 * @return  void* 
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/12/24 13:41:05
	**/
	void * addbuf (void *buf, size_t size);
private:

	/**
	 * @brief 获取分配的内存管理指针
	 *
	 * @param [in/out] size   : size_t
	 * @return  int 
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/12/24 10:43:48
	**/
	int getidx (size_t size);

public:
	/**
	 * @brief 根据size返回你如果要分配这个size，那么会分配给你多大的空间，主要用于调试
	 *
	 * @param [in/out] size   : size_t
	 * @return  size_t 
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2009/01/05 20:20:59
	**/
	size_t goodsize (size_t size) {
		int idx =  getidx(size);
		if (idx < 0 || idx >= _poolsize) return 0;
		return _pool[idx]._size;
	}
};

};

#endif  //__BSL_XMEMPOOL_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
