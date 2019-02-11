// 'struct xhash' from SGI -*- C++ -*-

// Copyright (C) 2001, 2002, 2003 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING.  If not, write to the Free
// Software Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
// USA.

// As a special exception, you may use this file as part of a free software
// library without restriction.  Specifically, if other files instantiate
// templates or use macros or inline functions from this file, or you compile
// this file and link it with other files to produce an executable, this
// file does not by itself cause the resulting executable to be covered by
// the GNU General Public License.  This exception does not however
// invalidate any other reasons why the executable file might be covered by
// the GNU General Public License.

/*
 * Copyright (c) 1996-1998
 * Silicon Graphics Computer Systems, Inc.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Silicon Graphics makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Hewlett-Packard Company makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */

/** @file ext/hash_fun.h
 *  This file is a GNU extension to the Standard C++ Library (possibly
 *  containing extensions from the HP/SGI STL subset).  You should only
 *  include this header if you are using GCC 3 or later.
 */

#ifndef _BSL_HASH_FUN_H
#define _BSL_HASH_FUN_H 

#include <cstddef>
#include <string>


namespace bsl
{
  using std::size_t;

  template <class _Key> struct xhash { 
	  size_t operator () (const _Key & _k) const {
		  return (size_t)_k;
	  }
  };

  inline size_t
  __bsl_hash_string(const char* __s)
  {
	  if (__s == 0) return 0;
    unsigned long __h = 0;
    for ( ; *__s; ++__s)
      __h = 5*__h + *__s;
    return size_t(__h);
  }

  template <> struct xhash<std::string>  {
	  size_t operator () (const std::string & _k) const {
		  return __bsl_hash_string(_k.c_str());
	  }
  };

  template <> struct xhash<const std::string>  {
	  size_t operator () (const std::string & _k) const {
		  return __bsl_hash_string(_k.c_str());
	  }
  };

  template<> struct xhash<char*>
  {
    size_t operator()(const char* __s) const
    { return __bsl_hash_string(__s); }
  };

  template<> struct xhash<const char*>
  {
    size_t operator()(const char* __s) const
    { return __bsl_hash_string(__s); }
  };

  template<> struct xhash<char>
  { size_t operator()(char __x) const { return __x; } };

  template<> struct xhash<unsigned char>
  { size_t operator()(unsigned char __x) const { return __x; } };

  template<> struct xhash<signed char>
  { size_t operator()(unsigned char __x) const { return __x; } };

  template<> struct xhash<short>
  { size_t operator()(short __x) const { return __x; } };

  template<> struct xhash<unsigned short>
  { size_t operator()(unsigned short __x) const { return __x; } };

  template<> struct xhash<int>
  { size_t operator()(int __x) const { return __x; } };

  template<> struct xhash<unsigned int>
  { size_t operator()(unsigned int __x) const { return __x; } };

  template<> struct xhash<long>
  { size_t operator()(long __x) const { return __x; } };

  template<> struct xhash<unsigned long>
  { size_t operator()(unsigned long __x) const { return __x; } };
} // namespace __gnu_cxx

#endif
