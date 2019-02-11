/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_sample_alloc.h,v 1.5 2009/04/07 06:35:53 xiaowei Exp $ 
 * 
 **************************************************************************/
 
/**
 * @file bsl_sample_alloc.h
 * @author xiaowei(com@baidu.com)
 * @date 2008/07/12 14:24:41  / 2010/10/18 modified by zhujianwei
 * @version $Revision: 1.5 $ 
 * @brief 
 *  
 **/

/*
 *
 * 用来优化分定长块
 *
 */


#ifndef  __BSL_SAMPLE_ALLOC_H_
#define  __BSL_SAMPLE_ALLOC_H_

#include <bsl/utils/bsl_utils.h>

namespace bsl
{
/*
 * 这个是容器内部，如果需要优化大块内存分配
 * 可以调用这个模版重新获取内存
 * 属于容器内部数据
 * 链表和hash可能用得到
 *
 */

/**
 * 只能管理内存
 */

template <class _Alloc, size_t _Items>
class bsl_sample_alloc
{
public:
	typedef _Alloc _Base;
	typedef typename _Base::pool_type pool_type;
	typedef typename _Base::size_type size_type;
	typedef typename _Base::difference_type difference_type;
	typedef typename _Base::pointer pointer;
	typedef typename _Base::const_pointer const_pointer;
	typedef typename _Base::reference reference;
	typedef typename _Base::const_reference const_reference;
	typedef typename _Base::value_type value_type;	
	typedef bsl_sample_alloc<_Alloc, _Items> self;
	
	union node_type
	{
		char value[sizeof(value_type)];
		node_type *next;
	};

	struct large_type
	{
		large_type *next;
		node_type data[_Items];
	};

	typedef  typename _Base::template rebind<large_type>::other large_alloc;
	template <typename _Tp1>
	struct rebind {
	private:
		typedef typename _Base::template rebind<_Tp1>::other other_alloc;
	public:
		typedef bsl_sample_alloc<other_alloc, _Items> other;
	};

	static const bool recycle_space;// = true;
	static const bool thread_safe;// = false;

public:

	bsl_sample_alloc() { 
        create(); 
    }

	bsl_sample_alloc(const bsl_sample_alloc<_Alloc, _Items> & ) { 
        create(); 
    }

	~bsl_sample_alloc() { 
		destroy();
	}

	inline pointer allocate(size_type, void * = 0) {
		//if (__n < 2) {
			if (_freelst) {
				node_type *tmp = _freelst;
				_freelst = _freelst->next;
				return (pointer)tmp->value;
			}
			if (_blknum == 0 || _lg == 0) {
				large_type *lg = _alloc.allocate(1);
				if (lg == 0) return 0;
				_blknum = _Items;
				lg->next = _lg;
				_lg = lg;
			}
			return (pointer)_lg->data[--_blknum].value;	
		//} 
		//return _pool.allocate(__n, ptr);
		return 0;
	}

	inline void deallocate(pointer ptr, size_type ) {
		//if (__n < 2) {
			((node_type *)ptr)->next = _freelst;
			_freelst = (node_type *)ptr;
		//} else {
		//	_pool.deallocate(ptr, __n);
		//}
	}

private:
	large_alloc _alloc;
	//pool_type _pool;
	large_type * _lg;
	node_type *_freelst;
	size_type _blknum;

public:
	void swap(self & __other) {
		_alloc.swap(__other._alloc);
		std::swap(_lg, __other._lg);
		std::swap(_freelst, __other._freelst);
		std::swap(_blknum, __other._blknum);
	}

	void merge( self & __other){
		node_type *tmp = (node_type*)&_freelst;
		while(tmp->next)
			tmp = tmp->next;
		if(__other._lg)
		{
			for(size_type i = 0; i < __other._blknum; i++)
			{
				tmp->next = &(__other._lg->data[i]);
				tmp = tmp->next;
			}
			large_type *lg = (large_type*)&_lg;
			while(lg->next)
				lg = lg->next;
			lg->next = __other._lg;
		}
		tmp->next = __other._freelst;
		__other._lg = 0;
		__other._blknum = 0;
		__other._freelst = 0;		
	}

	int create() {
		_lg = 0;
		_freelst = 0;
		_blknum = 0;
		_alloc.create();
		//_pool.create();
        return 0;
	}

	int destroy() {
		large_type *ptr = _lg;
		while (_lg) {
			_lg = _lg->next;
			free (ptr);
			ptr = _lg;
		}
		_alloc.destroy();
		//_pool.destroy();
		return 0;
	}

	value_type * getp(pointer __p) const { return __p; }
};

template <class _Alloc, size_t _Items>
inline bool operator == (const bsl_sample_alloc < _Alloc, _Items > &, 
		const bsl_sample_alloc < _Alloc, _Items > &) {
	return false;
}
template <class _Alloc, size_t _Items, class _Alloc2>
inline bool operator == (const bsl_sample_alloc <_Alloc, _Items> &, const _Alloc2 &) {
	return false;
}

template < typename _Alloc, size_t _Items >
inline bool operator != (const bsl_sample_alloc < _Alloc, _Items > &, 
		const bsl_sample_alloc < _Alloc, _Items > &) {
	return true;
}

template <class _Alloc, size_t _Items, class _Alloc2>
inline bool operator != (const bsl_sample_alloc <_Alloc, _Items> &, const _Alloc2 &) {
	return true;
}

template <class _Alloc, size_t _Items>
const bool bsl_sample_alloc<_Alloc, _Items>::recycle_space = true;
template <class _Alloc, size_t _Items>
const bool bsl_sample_alloc<_Alloc, _Items>::thread_safe = false;

}

#endif  //__BSL_SAMPLE_ALLOC_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
