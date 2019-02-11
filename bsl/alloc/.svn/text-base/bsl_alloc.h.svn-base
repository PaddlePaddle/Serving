/********************************************************************
created:	2008/06/27
created:	27:6:2008   14:05
filename: 	xalloc.h

2010/10/15 modified by zhujianwei

author:		xiaowei
mail:	xiaowei@baidu.com

purpose:	
*********************************************************************/

#ifndef BSL_ALLOC_H__
#define BSL_ALLOC_H__

#include <cstdio>
#include <cstdlib>
#include <new>
#include <algorithm>
#include <cstddef>
#include <stddef.h>
namespace bsl
{
/**
*  @brief  An allocator that uses malloc.
*
*  This is precisely the allocator defined in the C++ Standard.
*    - all allocation calls malloc
*    - all deallocation calls free
*/

template < typename _Tp >
class bsl_alloc
{
public:
	typedef size_t size_type;
    #if __GNUC__ >= 4 && __GNUC_MINOR__ >= 4
	typedef std::ptrdiff_t difference_type;
    #else
	typedef ptrdiff_t difference_type;
	#endif
    typedef _Tp    *pointer;
	typedef const _Tp *const_pointer;
	typedef         _Tp & reference;
	typedef const   _Tp & const_reference;
	typedef _Tp     value_type;
	typedef bsl_alloc<_Tp> pool_type;

	template < typename _Tp1 >
	struct rebind {
		typedef         bsl_alloc < _Tp1 > other;
	};

	static const bool recycle_space;// = false;	//在alloc析构的时候,是否释放空间
	static const bool thread_safe;// = true;	//空间分配器是否线程安全

	bsl_alloc()  {
        create();
    }

	bsl_alloc(const bsl_alloc &)  {
        create();
    }

	template < typename _Tp1 >
	bsl_alloc(const bsl_alloc < _Tp1 > &)  {
        create();
    }

	~bsl_alloc()  {
        destroy();
    }

	pointer address(reference __x) const {
		return &__x;
	}

	const_pointer address(const_reference __x) const {
		return &__x;
	}

	//NB:__n is permitted to be 0. The C++ standard says nothing
	// about what the return value is when __n == 0.
	pointer allocate(size_type __n, const void * = 0) {
		pointer __ret = static_cast < _Tp * >(malloc(__n * sizeof(_Tp)));
		return __ret;
	}

	pointer reallocate(size_type __n, void * ptr = 0) {
		if (ptr == NULL) {
			return allocate(__n, ptr);
		}
		return static_cast < _Tp * >(realloc(ptr, __n * sizeof(_Tp)));
	}

	//__p is not permitted to be a null pointer.
	void deallocate(pointer __p, size_type) {
		free(static_cast < void *>(__p));
	}

	size_type max_size() const  {
		return size_t(-1) / sizeof(_Tp);
	}

	//_GLIBCXX_RESOLVE_LIB_DEFECTS
	//402. wrong new expression in[some_] allocator: :                construct
	void construct(pointer __p, const _Tp & __val) {
		::new(__p) value_type(__val);
	}

	void destroy(pointer __p) {
		__p->~ _Tp();
	}

	int create() {
        return 0;
    }

	int destroy() {
        return 0;
    }

	void swap (bsl_alloc<_Tp> &) {
	}

	int merge(bsl_alloc<_Tp> &){
		return 0;
	}

	value_type * getp(pointer __p) const { return __p; }
};

template < typename _Tp >
inline bool operator == (const bsl_alloc < _Tp > &, const bsl_alloc < _Tp > &) {
	return true;
}

template < typename _Tp, class _Alloc2 > 
inline bool operator == (const bsl_alloc <_Tp > &, const _Alloc2 &) {
	return false;
}

template < typename _Tp >
inline bool operator != (const bsl_alloc < _Tp > &, const bsl_alloc < _Tp > &) {
	return false;
}

template < typename _Tp, class _Alloc2 > 
inline bool operator != (const bsl_alloc <_Tp > &, const _Alloc2 &) {
	return true;
}

template <typename _Tp>
const bool bsl_alloc<_Tp>::recycle_space = false;
template <typename _Tp>
const bool bsl_alloc<_Tp>::thread_safe = true;

}

#endif // xalloc_h__
