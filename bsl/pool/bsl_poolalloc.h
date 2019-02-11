/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_poolalloc.h,v 1.7 2009/06/15 06:29:04 chenxm Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file bsl_poolalloc.h
 * @author xiaowei(com@baidu.com)
 * @date 2008/12/08 18:30:29
 * @version $Revision: 1.7 $ 
 * @brief 
 *  
 **/


#ifndef  __BSL_POOLALLOC_H_
#define  __BSL_POOLALLOC_H_
#include <bsl/exception/bsl_exception.h>
#include "bsl_pool.h"
#include <cstddef>

namespace bsl
{

    template<typename _Tp>
        class pool_allocator;

    /**
    * @brief pool_allocator对void的特化
    *  
    *  没有分配内存作用，只可以rebind到其它类型
    */
    template<>
        class pool_allocator<void>
        {
        public:
            typedef size_t      size_type;
            typedef ptrdiff_t   difference_type;
            typedef void*       pointer;
            typedef const void* const_pointer;
            typedef void        value_type;

            template<typename _Tp1>
                friend class pool_allocator;

            template<typename _Tp1>
                struct rebind
                { typedef pool_allocator<_Tp1> other; };

            pool_allocator() throw()
                :_p_pool(NULL)  { }

            pool_allocator(mempool *p_pool) throw() 
                :_p_pool(p_pool) { }

            pool_allocator(const pool_allocator & a) throw()
                :_p_pool(a._p_pool){ }

            template<typename _Tp1>
                pool_allocator( const pool_allocator<_Tp1>& a ) throw()
                :_p_pool(a._p_pool){ }

            ~pool_allocator() throw() { }
        private:
            mempool *_p_pool;
            
        };

    template<typename _Tp>
        class pool_allocator
        {
        public:
            typedef size_t     size_type;
            typedef ptrdiff_t  difference_type;
            typedef _Tp*       pointer;
            typedef const _Tp* const_pointer;
            typedef _Tp&       reference;
            typedef const _Tp& const_reference;
            typedef _Tp        value_type;

            template<typename _Tp1>
                friend class pool_allocator;

            template<typename _Tp1>
                struct rebind { typedef pool_allocator<_Tp1> other; };

            pool_allocator() throw()
                :_p_pool(NULL)  { }

            pool_allocator(mempool *p_pool) throw() 
                :_p_pool(p_pool) { }

            pool_allocator(const pool_allocator & a) throw()
                :_p_pool(a._p_pool){ }

            template<typename _Tp1>
                pool_allocator( const pool_allocator<_Tp1>& a ) throw()
                :_p_pool(a._p_pool){ }

            ~pool_allocator() throw() { }

            pointer
                address(reference __x) const { return &__x; }

            const_pointer
                address(const_reference __x) const { return &__x; }

            pointer
                allocate(size_type __n, const void* = 0)
                {
                    size_t  size_  = __n * sizeof(_Tp);
                    pointer __ret = static_cast<_Tp*>( _p_pool ? _p_pool->malloc(size_) : malloc(size_) );
                    if (!__ret){
                        throw bsl::BadAllocException() << BSL_EARG << "size["<<size_<<"] _p_pool["<<_p_pool<<"]";
                    }
                    return __ret;
                }

            // __p is not permitted to be a null pointer.
            void
                deallocate(pointer __p, size_type size_) 
                { 
                    if ( _p_pool ){
                        _p_pool->free (static_cast<void *>(__p), sizeof(_Tp) * size_);
                    }else{
                        free( __p );
                    }
                }

            size_type
                max_size() const throw() { 
                    return size_t(-1) / sizeof(_Tp); 
                }

            void 
                construct(pointer __p, const _Tp& __val) 
                { ::new(__p) value_type(__val); }

            void 
                destroy(pointer __p) { __p->~_Tp(); }

            bool
                operator==(const pool_allocator& other) const  { 
                    return _p_pool == other._p_pool;
                }

            bool
                operator!=(const pool_allocator& other) const  { 
                    return _p_pool != other._p_pool;
                }

        private:
            mempool *_p_pool;
        };


}





#endif  //__BSL_POOLALLOC_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
