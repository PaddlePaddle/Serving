#ifndef _BSL_CONSTRUCT_H
#define _BSL_CONSTRUCT_H

#include <algorithm>
#include <new>

#if __GNUC__ >= 4
#include <bsl/utils/bsl_type_traits.h>
#endif

namespace bsl
{
  /**
   * @if maint
   * Constructs an object in existing memory by invoking an allocated
   * object's constructor with an initializer.
   * @endif
   */
  template<typename _T1, typename _T2>
    inline void
    bsl_construct(_T1* __p, const _T2& __value)
    {
      // _GLIBCXX_RESOLVE_LIB_DEFECTS
      // 402. wrong new expression in [some_]allocator::construct
      ::new(static_cast<void*>(__p)) _T1(__value);
    }

  /**
   * @if maint
   * Constructs an object in existing memory by invoking an allocated
   * object's default constructor (no initializers).
   * @endif
   */
  template<typename _T1>
    inline void
    bsl_construct(_T1* __p)
    {
      // _GLIBCXX_RESOLVE_LIB_DEFECTS
      // 402. wrong new expression in [some_]allocator::construct
      ::new(static_cast<void*>(__p)) _T1();
    }

  /**
   * @if maint
   * Destroy the object pointed to by a pointer type.
   * @endif
   */
  template<typename _Tp>
    inline void
    bsl_destruct(_Tp* __pointer)
    { __pointer->~_Tp(); }

  /**
   * @if maint
   * Destroy a range of objects with nontrivial destructors.
   *
   * This is a helper function used only by destruct().
   * @endif
   */
  template<typename _ForwardIterator>
    inline void
    __bsl_destroy_aux(_ForwardIterator __first, _ForwardIterator __last,
		  __false_type)
    { for ( ; __first != __last; ++__first) bsl::bsl_destruct(&*__first); }

  /**
   * @if maint
   * Destroy a range of objects with trivial destructors.  Since the destructors
   * are trivial, there's nothing to do and hopefully this function will be
   * entirely optimized away.
   *
   * This is a helper function used only by destruct().
   * @endif
   */
  template<typename _ForwardIterator>
    inline void
    __bsl_destroy_aux(_ForwardIterator, _ForwardIterator, __true_type)
    { }

  /**
   * @if maint
   * Destroy a range of objects.  If the value_type of the object has
   * a trivial destructor, the compiler should optimize all of this
   * away, otherwise the objects' destructors must be invoked.
   * @endif
   */
  template<typename _ForwardIterator>
    inline void
    bsl_destruct(_ForwardIterator __first, _ForwardIterator __last)
    {
      typedef typename std::iterator_traits<_ForwardIterator>::value_type _Value_type;
      typedef typename __type_traits < _Value_type >::has_trivial_destructor
                       _Has_trivial_destructor;
      bsl::__bsl_destroy_aux(__first, __last, _Has_trivial_destructor());
    }
} // namespace bsl

#endif /* _BSL_CONSTRUCT_H */

