/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_string.h,v 1.26 2009/10/14 08:24:58 chenxm Exp $ 
 * 
 **************************************************************************/



/**
 * @file bsl_basic_string.h
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/09/10 19:41:57
 * @version $Revision: 1.26 $ 
 * @brief 
 *  
 **/
#ifndef  __BSL_STRING_H__
#define  __BSL_STRING_H__
#include <cstdarg>
#include <cstring>
#include <string>   //for char_traits
#include <cstdlib>
#include <iostream>
#if __GNUC__ >= 4 // 4.3 or greater
#include <backward/hash_fun.h>
#elif __GNUC__ >= 3
#include <ext/hash_fun.h>
#else
#include <stl_hash_fun.h>
#endif
#include "bsl/exception/bsl_exception.h"
#include "bsl/pool/bsl_poolalloc.h"
#include "bsl/archive/bsl_serialization.h" 
#include "bsl/utils/bsl_memcpy.h"
#include "bsl/containers/hash/bsl_hash_fun.h"


namespace bsl{
    template<class, class> class basic_string;
    typedef basic_string<char, pool_allocator<char> > string;
    /**
     * @brief 字符串类
     *  
     *  这是一个比较简单的字符串类。为解决std::string在g++2.96下的多线程bug而设计，接口尽量与std::string保持一致。复制使用立即深复制算法，目前没有使用引用计数。（以后可能改进）
     *  初始化时_str保证指向一个值为0的静态空间（没有malloc的开销），因此_str总是一个合法的字符串，但是这样basic_string就没有trivial default constructor了。
     *  另一种替代算法是构造时_str被初始化为NULL，从而使string拥有trivial default constructor，但在很多的函数调用里都需要检查_str是否指向NULL，考虑到多数string对象被构造后都会被多次调用，因此效率上可能得不偿失
     *  
     *  注意！该类不接受NULL作为合法的C风格字符串(与std::string行为相同)，对于const_pointer(NULL)参数，一律抛出bsl::NullPointerException
     *
     * 目前bsl::string与std::string接口的主要不同点：
     *   模板参数：
     *     bsl::string比std::string少了一个模板参数：_TraitsT，原因为：
     *         该参数很少使用；
     *         该参数不能兼容g++2.96和g++3.4.5，若加上参数，则无论是自定义_TraitsT还是自定义_Alloc，都必须条件编译；
     *         使编译后的类名短一点。
     *
     *   C++风格字符串的子串操作：
     *     std::string对不合法的子串操作（如begin_pos过大）会抛出std::out_of_range异常，而bsl::string会修正参数
     *
     *   类C风格的扩展API
     *     考虑到百度内大部分程序员习惯C风格的API，bsl::string引入以下扩展：
     *     bsl::string& setf( const char * format, ... );
     *     bsl::string& vsetf( const char * format, va_list );
     *     bsl::string& appendf( const char * format, ... );
     *     bsl::string& vappendf( const char * format, va_list );
     *   与直接在Ｃ风格字符串上操作相比，这套API能自动扩展string的长度，而不会有访问越界问题
     *
     *   迭代器,insert操作, replace操作，更多的运算符重载
     *     bsl::string目前不提供这些操作，日后将会逐步加上
     *
     *  
     *
     */
    template<class _CharT, class _Alloc>
        class basic_string{
        public:
            typedef _CharT              value_type;
            typedef size_t              size_type;
            typedef long                difference_type;
            typedef value_type &        reference;
            typedef const value_type&   const_reference;
            typedef value_type *        pointer;
            typedef const value_type *  const_pointer;
            typedef _Alloc              allocator_type;

            typedef pointer             iterator;
            typedef const_pointer       const_iterator;

#if __GNUC__ <= 2
            typedef string_char_traits<_CharT> traits_type;
#else
            typedef std::char_traits<_CharT> traits_type;
#endif



        //define shallow_copy
#ifndef _BSL_STRING_SUPPORT_SHALLOW_COPY
#define _BSL_STRING_SUPPORT_SHALLOW_COPY
#endif
            /**
             * @brief 浅拷贝构造函数,将cstr作为自己的成员.shallow_copy==false:深拷贝构造.shallow_copy==true:浅拷贝构造
             * @param [in] shallow_copy : false:深拷贝.true:浅拷贝
             * @throw NullPointerException()<<BSL_EARG<<"cstr";当cstr为NULL时
             */

            basic_string( const_pointer cstr,size_type len,const allocator_type& alloc_,bool shallow_copy )
                    :_dataplus(NULL,alloc_), _length(0), _capacity(0),_shallow_copy(shallow_copy)
            {
                if(cstr==NULL)
                {
                    throw NullPointerException()<<BSL_EARG<<"cstr";
                }
                _capacity=_length=len;
                if( shallow_copy==false )
                {
                    _dataplus._str = _s_clone_cstring( cstr, _capacity, alloc_ );
                }
                else
                {
                    _dataplus._str = const_cast<pointer>(cstr);
                }
            }


            
            //ctors and dtors

            /**
             * @brief 默认构造函数，无动态内存分配，无抛掷保证。
             *
             * @see 
             * @author chenxm
             * @date 2008/09/16 11:27:38
            **/
            basic_string() 
                : _dataplus(_s_create_cstring(0,allocator_type())), _length(0), _capacity(0),_shallow_copy(false){ }

            /**
             * @brief 根据内存分配器构造string对象
             *
             * 这个方法很少用到。主要为了兼容std::string的接口
             * @return  explicit 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 11:28:27
            **/
            explicit basic_string( const allocator_type& alloc_ )
                : _dataplus(_s_create_cstring(0, alloc_), alloc_ ), _length(0), _capacity(0),_shallow_copy(false){}

            /**
             * @brief 复制构造函数
             * 当other.length() == 0时，没有动态内存分配（考虑到STL及类似容器经常使用一个默认构造的对象去复制构造一大堆对象，这样做是很有意义的）
             * 
             * @param [in] other   : const basic_string&
             * @see 
             * @author chenxm
             * @date 2008/09/13 23:35:51
             * @modified by chenyanling@baidu.com 2011/10/17
             **/
            basic_string( const basic_string& other ) 
                    : _dataplus(NULL,other._dataplus), 
                    _length(other._length), _capacity(other._length),_shallow_copy(other._shallow_copy) {
                if(false == _shallow_copy){
                    _dataplus._str = _s_clone_cstring( other._dataplus._str, _length, _dataplus );
                }
                else{
                    _dataplus._str = other._dataplus._str;
                }
            }

            /**
             * @brief 复制构造other的子串
             *
             * 若sub_str_len未给出或等于npos
             * 则*this被构造为other上的[begin_pos, other.length())区间上的子串
             * 否则*this被构造为other上的[begin_pos, begin_pos+sub_str_len)上子串，即第begin_pos个(0-based)字符开头，长度最多为sub_str_len的子串
             * 
             *
             * @param [in] other   : const basic_string&
             * @param [in] begin_pos   : size_type
             * @param [in] sub_str_len   : size_type
             * @see 
             * @author chenxm
             * @date 2008/09/16 11:32:18
            **/
            basic_string( const basic_string& other, size_type begin_pos, size_type sub_str_len = npos, const allocator_type& alloc_ = allocator_type() ) 
                : _dataplus(NULL, alloc_), _length(0), _capacity(0),_shallow_copy(false){
                    if ( begin_pos > other._length ){
                        begin_pos = other._length;
                    }
                    if ( sub_str_len > other._length - begin_pos ){
                        sub_str_len = other._length - begin_pos;
                    }
                    _length = _capacity = sub_str_len;
                    _dataplus._str = _s_clone_cstring( other._dataplus._str + begin_pos, _capacity, alloc_ );
                }

            /**
             * @brief 复制构造C风格字符串的子串
             *
             * 本函数直接复制区间[cstr, cstr+len)之间的内容，不考虑是否存在\0
             * 
             * @param [in] cstr   : const_pointer
             * @param [in] sub_str_len   : size_type
             * @see 
             * @author chenxm
             * @date 2008/09/16 11:38:53
            **/
            basic_string( const_pointer cstr, size_type len, const allocator_type& alloc_= allocator_type() ) 
                : _dataplus(NULL,alloc_), _length(0), _capacity(0),_shallow_copy(false) {
                    _length = _capacity = len;
                    _dataplus._str      = _s_clone_cstring( cstr, _length, alloc_ );
                }

            /**
             * @brief 复制构造C风格字符串
             *
             * 若cstr为空串或NULL，则构造结果为空串，并且没有动态内存分配。
             * @param [in] cstr   : const_pointer
             * @see 
             * @author chenxm
             * @date 2008/09/16 11:41:25
            **/
            basic_string( const_pointer cstr, const allocator_type& alloc_ = allocator_type() ) 
                : _dataplus(NULL,alloc_), _length(0), _capacity(0),_shallow_copy(false) {
                    _capacity = _length = _s_cstring_len(cstr);
                    _dataplus._str = _s_clone_cstring(cstr, _length, alloc_);
                }

            /**
             * @brief 构造一个由n个chr字符组成的字符串
             *
             * @param [in] n   : size_type
             * @param [in] chr   : value_type
             * @see 
             * @author chenxm
             * @date 2008/09/16 11:41:55
            **/
            basic_string( size_type n, value_type chr, const allocator_type& alloc_ = allocator_type() )
                : _dataplus(_s_create_cstring(n, alloc_), alloc_), _length(n), _capacity(n),_shallow_copy(false){
                    std::fill_n(_dataplus._str,n,chr);    //会有多种特化版本来优化性能
                    _dataplus._str[_length] = 0;
                }

            /**
             * @brief 根据区间[__begin, __end)构造字符串
             *
             * 注：不检查区间内是否有'\0'值！（与std::string行为相同）
             * @param [in] __begin   : InputIterator
             * @param [in] __end   : InputIterator
             * @see 
             * @author chenxm
             * @date 2008/09/16 11:43:05
            **/
            template<class InputIterator>
                basic_string( InputIterator __begin, InputIterator __end, const allocator_type& alloc_= allocator_type() )
                :_dataplus(NULL,alloc_), _length(0), _capacity(0),_shallow_copy(false) {
                    difference_type dis = std::distance(__begin, __end);
                    if ( dis > 0 ){
                        pointer tmp = _dataplus._str = _s_create_cstring( dis, alloc_ );
                        _length = _capacity = size_type(dis);
                        for(; __begin != __end; ++ __begin ){
                            *tmp++ = *__begin;
                        }
                        *tmp = 0;
                    }else{
                        _dataplus._str = _s_create_cstring(0, alloc_);
                    }
                }

            /**
             * @brief basic_string( InputIterator __begin, InputIterator __end, const allocator_type& alloc_= allocator_type() ) 在InputIterator == const_pointer时的特化 
             *
             * @param [in] __begin   : const_pointer
             * @param [in] __end   : const_pointer
             * @param [in] alloc_   : const allocator_type&
             * @see 
             * @author chenxm
             * @date 2009/06/02 12:47:52
            **/
            basic_string( const_pointer __begin, const_pointer __end, const allocator_type& alloc_= allocator_type() )
                :_dataplus(NULL, alloc_), _length(0), _capacity(0),_shallow_copy(false){
                    if ( __begin == NULL || __end == NULL ){
                        throw NullPointerException()<<BSL_EARG<<"__begin["<<static_cast<const void*>(__begin)<<"] > __end["<<static_cast<const void*>(__end)<<"]";
                    }
                    if ( __begin <= __end ){
                        _length = _capacity = __end - __begin;
                        _dataplus._str = _s_clone_cstring( __begin, _length, alloc_ );
                    }else{
                        _dataplus._str = _s_create_cstring(0, alloc_);
                    }
                }

            /**
             * @brief basic_string( InputIterator __begin, InputIterator __end, const allocator_type& alloc_= allocator_type() ) 在InputIterator == pointer时的特化 
             *
             * @param [in] __begin   : pointer
             * @param [in] __end   : pointer
             * @param [in] alloc_   : const allocator_type&
             * @see 
             * @author chenxm
             * @date 2009/06/02 12:48:22
            **/
            basic_string( pointer __begin, pointer __end, const allocator_type& alloc_= allocator_type() )
                :_dataplus(NULL, alloc_), _length(0), _capacity(0),_shallow_copy(false){
                    if ( __begin == NULL || __end == NULL ){
                        throw NullPointerException()<<BSL_EARG<<"__begin["<<static_cast<const void*>(__begin)<<"] > __end["<<static_cast<const void*>(__end)<<"]";
                    }
                    if ( __begin <= __end ){
                        _length = _capacity = __end - __begin;
                        _dataplus._str = _s_clone_cstring( __begin, _length, alloc_ );
                    }else{
                        _dataplus._str = _s_create_cstring(0, alloc_);
                    }
                }

            /**
             * @brief 析构函数
             *
             * @see 
             * @author chenxm
             * @date 2008/09/16 11:44:02
            **/
            ~basic_string() {
                _s_destroy_cstring(_dataplus._str, _capacity, _dataplus);
            }

            //substr
            basic_string substr(size_t pos = 0, size_t n = npos ) const {
                return basic_string(*this, pos, n);
            }

            //serialization
            
            /**
             * @brief const版本的串行化函数
             *
             * 这个是正常版本的串行化函数
             * @param [in] ar   : _Archive&
             * @return int 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 11:45:22
            **/
            template <class _Archive>
                int serialization(_Archive &ar) const {
                    if (ar.push(&_length, sizeof(_length)) != 0) {
                        __BSL_ERROR("push siz[%lu] err", _length);
                        return -1;
                    }
                    if ( _length && ar.push(_dataplus._str, _length*sizeof(value_type)) != 0) {
                        __BSL_ERROR("push str[%p] err", _dataplus._str);
                        return -1;
                    }
                    return 0;
                }

            /**
             * @brief 反串行化函数
             *
             * 当反串行化失败时，该函数返回-1，强异常安全保证
             * @param [in] ar   : _Archive&
             * @return int 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 11:46:22
            **/
            template <class _Archive>
                int deserialization(_Archive &ar) {
                    size_type siz = 0;
                    if (ar.pop(&siz, sizeof(siz)) != 0) {
                        __BSL_ERROR("pop siz err");
                        return -1;
                    }
                    pointer ptr = _s_create_cstring(siz, _dataplus);   //throw
                    if (ar.pop(ptr, sizeof(value_type)*siz) != 0 ) {
                        __BSL_ERROR("pop str err");
                        _s_destroy_cstring(ptr, siz, _dataplus);
                        return -1;
                    }

                    ptr[siz] = 0;
                    _s_destroy_cstring( _dataplus._str, _length, _dataplus );
                    _dataplus._str = ptr;
                    _length = _capacity = siz;

                    return 0;
                }

            // getters
            /**
             * @brief 返回C风格字符串，O(1)
             *
             * @return  const_pointer 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 11:48:59
            **/
            const_pointer c_str() const  {
                return _dataplus._str; 
            }

            /**
             * @brief 返回该字符串是否为空
             *
             * @return  bool 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 11:49:24
            **/
            bool empty() const {
                return _length == 0;
            }

            /**
             * @brief 返回字符串的长度
             *
             * @return  size_type 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 11:49:39
            **/
            size_type size() const {
                return _length;
            }

            /**
             * @brief 返回字符串的长度
             *
             * @return  size_type 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 11:49:59
            **/
            size_type length() const {
                return _length;
            }

            /**
             * @brief 返回字符串的容量
             *
             * 容量的定义为：在不重新进行内存分配的情况下，本字符串最多能容纳的字符数（不包括'\0'）
             * @return  size_type 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/10 21:38:24
             **/
            size_type capacity() const {
                return _capacity;
            }

            /**
             * @brief 返回第index个字符（const 版本）
             *
             * 若0 <= index < (int)length()
             * 则返回顺数第index个字符；
             * 若-(int)length() < index < 0
             * 则返回倒数第index个字符；
             * 否则
             * 结果未定义。
             * @param [in] index   : int
             * @return  value_type[] 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 11:51:12
            **/
            const_reference operator []( int idx ) const {
                if (idx >= 0 && idx < int(_length)){
                    return _dataplus._str[idx];
                }
                if (idx < 0 && idx >= -int(_length) ){
                    return _dataplus._str[int(_length)+idx];
                }
                throw OutOfBoundException()<<BSL_EARG<<"index["<<idx<<"] bound["<<-int(_length)<<"~"<<int(_length)-1<<"]";
            }
            /**
             * @brief 返回第index个字符（const 版本）
             *
             * 若0 <= index < (int)length()
             * 则返回顺数第index个字符；
             * 若-(int)length() < index < 0
             * 则返回倒数第index个字符；
             * 否则
             * 结果未定义。
             * @param [in] index   : int
             * @return  value_type[] 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 11:51:12
            **/
            reference operator []( int idx ) {
                if (idx >= 0 && idx < int(_length)){
                    return _dataplus._str[idx];
                }
                if (idx < 0 && idx >= -int(_length) ){
                    return _dataplus._str[int(_length)+idx];
                }
                throw OutOfBoundException()<<BSL_EARG<<"index["<<idx<<"] bound["<<-int(_length)<<"~"<<int(_length)-1<<"]";
            }

            
            size_type find(const _CharT* s, size_type pos, size_type n) const{
                if ( s == NULL ){
                    throw bsl::NullPointerException()<<BSL_EARG<<"s can't be NULL!";
                }
                const_pointer tb = _dataplus._str + pos;
                const_pointer te = _dataplus._str + _length - n;
                for(; tb <= te; ++tb ){
                    if ( traits_type::compare( tb, s, n ) == 0 ){
                        return tb - _dataplus._str;
                    }
                }
                return npos;
            }

            size_type find(const basic_string& str, size_type pos = 0) const { 
                return this->find(str._dataplus._str, pos, str._length); 
            }

            size_type find(const _CharT* s, size_type pos = 0) const {
                return this->find(s, pos, _s_cstring_len(s));
            }

            size_type find(_CharT c, size_type pos = 0) const{
                const_pointer b = _dataplus._str + pos;
                const_pointer e = _dataplus._str + _length;
                for(; b < e; ++b){
                    if ( *b == c ){
                        return b - _dataplus._str;
                    }
                }
                return npos;
            }
            

            size_type rfind(const basic_string& str, size_type pos = npos) const { 
                return this->rfind(str._dataplus._str, pos, str._length); 
            }

            size_type rfind(const _CharT* s, size_type pos, size_type n) const{
                if ( s == NULL ){
                    throw bsl::NullPointerException()<<BSL_EARG<<"s can't be NULL!";
                }
                if ( _length >= n ){
                    const_pointer tb = _dataplus._str + std::min(size_t(_length - n), pos);
                    for(; tb >= _dataplus._str; --tb ){
                        if ( traits_type::compare( tb, s, n ) == 0 ){
                            return tb - _dataplus._str;
                        }
                    }
                }
                return npos;
            }

            size_type rfind(const _CharT* s, size_type pos = npos) const {
                return this->rfind(s, pos, _s_cstring_len(s));
            }

            size_type rfind(_CharT c, size_type pos = npos) const{
                const_pointer b = _dataplus._str + (pos >= _length ? _length - 1 : pos);
                const_pointer e = _dataplus._str;
                for(; b >= e; --b){
                    if ( *b == c ){
                        return b - _dataplus._str;
                    }
                }
                return npos;
            }

            /**
             * @brief 追加另外一个bsl::string
             *
             * @param [in] other   : const basic_string&
             * @return  basic_string& 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/11/08 23:35:28
            **/
            basic_string& append( const basic_string& other ){
                reserve( _length + other._length );
                xmemcpy( _dataplus._str + _length, other._dataplus._str, sizeof(value_type) * other._length );    // 1 for '\0'
                _length += other._length;
                _dataplus._str[_length] = 0;
                return *this;
            }

            /**
             * @brief 追加另一个bsl::string的子串
             *
             * 若begin_pos >= other._length，*this不会被修改
             * 若begin_pos + sub_str_len >= other.length()，则sub_str_len被调整为other.length() - begin_pos
             *
             * @param [in] other   : const basic_string&
             * @param [in] begin_pos   : size_type
             * @param [in] sub_str_len   : size_type
             * @return  basic_string& 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/11/08 23:36:42
            **/
            basic_string& append( const basic_string& other, size_type begin_pos, size_type sub_str_len = npos ){
                if ( begin_pos >= other._length ){
                    return *this;
                }
                if ( sub_str_len > other._length - begin_pos ){
                    sub_str_len = other._length - begin_pos;
                }
                reserve( _length + sub_str_len );
                xmemcpy( _dataplus._str + _length, other._dataplus._str + begin_pos, sizeof(value_type) * sub_str_len ); 
                _length += sub_str_len;
                _dataplus._str[_length] = 0;
                return *this;
            }

            /**
             * @brief 追加一个C风格字符串
             *
             * 如果已经确切知道cstr的长度，使用append( cstr, len)可以快4~8倍
             *
             * @param [in] cstr   : const_pointer
             * @return  basic_string& 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/11/08 23:53:09
            **/
            basic_string& append( const_pointer cstr ){
                size_t cstr_len = _s_cstring_len( cstr );
                reserve( _length + cstr_len );
                xmemcpy( _dataplus._str + _length, cstr, sizeof(value_type) * cstr_len );    
                _length += cstr_len;
                _dataplus._str[_length] = 0;
                return *this;
            }

            /**
             * @brief 追加C风格字符串的子串
             *
             * 本函数直接复制追加区间[cstr, cstr+len)之间的内容，不考虑是否存在\0
             *
             * @param [in] cstr   : const_pointer
             * @param [in] sub_str_len   : size_type
             * @return  basic_string& 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/11/08 23:55:50
            **/
            basic_string& append( const_pointer cstr, size_type len ){
                if ( cstr == NULL ){
                    throw bsl::NullPointerException()<<BSL_EARG<<"cstr";
                }
                reserve( _length + len );
                xmemcpy( _dataplus._str + _length, cstr, sizeof(value_type) * len );    
                _length += len;
                _dataplus._str[_length] = 0;
                return *this;
            }

            basic_string& append( pointer cstr, size_type len ){
                if ( cstr == NULL ){
                    throw bsl::NullPointerException()<<BSL_EARG<<"cstr";
                }
                reserve( _length + len );
                xmemcpy( _dataplus._str + _length, cstr, sizeof(value_type) * len );    
                _length += len;
                _dataplus._str[_length] = 0;
                return *this;
            }
            /**
             * @brief 追加n个chr字符。
             *
             *
             * @param [in] n   : size_type
             * @param [in] chr   : value_type
             * @return  basic_string& 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/11/08 23:57:04
            **/
            basic_string& append( size_type n, value_type chr ){
                reserve( _length + n );
                std::fill_n(_dataplus._str + _length, n, chr);    //会有多种特化版本来优化性能
                _length += n;
                _dataplus._str[_length] = 0;
                return *this;
            }

            /**
             * @brief 把区间[__begin, __end)的内容追加到字符串尾
             *
             * @param [in] __begin   : InputIterator
             * @param [in] __end   : InputIterator
             * @return  template<class InputIterator>                 basic_string& 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/11/20 14:30:05
            **/
            template<class InputIterator>
                basic_string& append( InputIterator __begin, InputIterator __end ){ 
                    difference_type dis = std::distance(__begin, __end);
                    if ( dis > 0 ){
                        reserve( _length + dis );
                        for( pointer p = _dataplus._str + _length; __begin != __end; ++p, ++ __begin ){
                            *p = *__begin;
                        }
                        _length += size_type(dis);
                        _dataplus._str[_length] = 0;
                    }else{
                        //pass
                    }
                    return *this;
                }

            /**
             * @brief basic_string& append( InputIterator __begin, InputIterator __end )在InputIterator == const_pointer时的特化
             *
             * @param [in] __begin   : const_pointer
             * @param [in] __end   : const_pointer
             * @return  basic_string& 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2009/06/02 12:49:35
            **/
            basic_string& append( const_pointer __begin, const_pointer __end ){
                if ( __begin == NULL || __end == NULL ){
                    throw NullPointerException()<<BSL_EARG<<"__begin["<<static_cast<const void*>(__begin)<<"] > __end["<<static_cast<const void*>(__end)<<"]";
                }
                if ( __begin < __end ){
                    return append( __begin, __end - __begin );
                }else{
                    //pass
                    return *this;
                }
            }

            /**
             * @brief basic_string& append( InputIterator __begin, InputIterator __end )在InputIterator == pointer时的特化
             *
             * @param [in] __begin   : pointer
             * @param [in] __end   : pointer
             * @return  basic_string& 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2009/06/02 12:49:35
            **/
            basic_string& append( pointer __begin, pointer __end ){
                if ( __begin == NULL || __end == NULL ){
                    throw NullPointerException()<<BSL_EARG<<"__begin["<<static_cast<const void*>(__begin)<<"] > __end["<<static_cast<const void*>(__end)<<"]";
                }
                if ( __begin < __end ){
                    return append( __begin, __end - __begin );
                }else{
                    //pass
                    return *this;
                }
            }

            void push_back( value_type chr ){
                if ( _length >= _capacity ){
                    reserve( _length + 1 );
                }
                _dataplus._str[_length] = chr;
                _dataplus._str[++_length] = 0;
            }

            // setters
            /**
             * @brief 把本字符串置为空串
             *
             * 注：若本字符串使用的动态分配的内存，该内存不会被释放。若确实需要释放内存，可使用swap惯用法：big_str.swap(string());
             *
             * @return  void 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 11:54:44
            **/
            void clear(){
                _length = 0;
                _dataplus._str[0] = 0;
            }

            /**
             * @brief 扩充字符串的容量至__capacity或以上
             *
             * 容量的定义参见capacity()
             * 若__capacity > capacity() 则内存将被重新分配以保证capacity()>=__capacity，但内容保持不变；
             * 否则，本方法无操作。
             *
             * @param [in] __capacity   : size_type
             * @return  void 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 11:57:46
            **/
            void reserve( size_type __capacity ){
                if ( __capacity > _capacity ){
                    if ( __capacity < _capacity + _capacity ){
                        __capacity = _capacity + _capacity;
                    }
                    pointer tmp = _s_create_cstring( __capacity, _dataplus );   //throw
                    xmemcpy( tmp, _dataplus._str, (_length+1)*sizeof(value_type) );
                    _s_destroy_cstring( _dataplus._str, _capacity, _dataplus );
                    _dataplus._str = tmp;
                    _capacity = __capacity;
                }
            }

            /**
             * @brief 交互两个字符串，O(1)，无抛掷保证
             *
             * 平凡但非常有用的一个函数。灵活使用“swap惯用法“可以完成很多有用的功能，如编写强异常安全的上层函数，或把字符串“trim to size“等 
             *
             * @param [in] other   : basic_string&
             * @return  void 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 18:46:43
            **/
            void swap( basic_string& other ) {
                std::swap(_dataplus._str, other._dataplus._str);
                std::swap(_length, other._length);
                std::swap(_capacity, other._capacity);
            }

            /**
             * @brief 为本字符串赋值
             *
             * 若other.length() <= this->capacity，则内存不会被重新分配
             *
             * @param [in] other   : const basic_string&
             * @return  basic_string&  operator 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 12:00:42
            **/
            basic_string& operator = ( const basic_string& other ){
                if ( other._dataplus._str[0] == 0 ){
                    _dataplus._str[0] = 0;
                    _length = 0;
                }else if ( &other != this ){
                    if ( other._length > _capacity ){
                        pointer tmp = _s_create_cstring( other._length, _dataplus );    //throw
                        _s_destroy_cstring( _dataplus._str, _capacity, _dataplus );
                        _dataplus._str = tmp;
                        _capacity = other._length;
                    }
                    _length = other._length;
                    xmemcpy( _dataplus._str, other._dataplus._str, (_length+1)*sizeof(value_type) ); 
                }
                return *this;
            }

            /**
             * @brief 使用C风格字符串为本字符串赋值
             *
             * 若cstr的长度 <= capacity()，则内存不会被重新分配
             * @param [in] cstr   : const_pointer
             * @return  basic_string&  operator 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 12:02:31
            **/
            basic_string& operator = ( const_pointer cstr ){
                if ( cstr == NULL ){
                    _dataplus._str[0] = 0;
                    _length = 0;
                }else if ( cstr[0] == 0 ){
                    _dataplus._str[0] = 0;
                    _length = 0;
                }else if ( cstr != _dataplus._str){
                    size_type len = _s_cstring_len( cstr );
                    if ( len > _capacity ){
                        pointer tmp = _s_create_cstring( len, _dataplus );    //throw
                        _s_destroy_cstring( _dataplus._str, _capacity, _dataplus );
                        _dataplus._str = tmp;
                        _capacity = len;
                    }
                    _length = len;
                    xmemcpy( _dataplus._str, cstr, (len+1)*sizeof(value_type) );
                }
                return *this;
            }


            //comparers
            int compare( const basic_string& other ) const {
                const size_type len = _length < other._length ? _length : other._length;
                int ret = traits_type::compare( _dataplus._str, other._dataplus._str, len );
                if ( ret == 0 ){
                    if (_length > other._length) {
                        return 1;
                    } else if (_length == other._length) {
                        return 0;
                    } else {
                        return -1;
                    }
                }else{
                    return ret;
                }
            }

            int compare( const_pointer cstr ) const {
                const size_type cstr_len = _s_cstring_len(cstr);
                const size_type len = _length < cstr_len ? _length : cstr_len;
                int ret = traits_type::compare( _dataplus._str, cstr, len );
                if ( ret == 0 ){
                    if (_length > cstr_len) {
                        return 1;
                    } else if (_length == cstr_len) {
                        return 0;
                    } else {
                        return -1;
                    }
                }else{
                    return ret;
                }
            }
            /**
             * @brief 判断两字符串字典序的大小关系
             *
             * @param [in] sstr1   : const basic_string&
             * @param [in] sstr2   : const basic_string&
             * @return  friend inline bool operator 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 12:03:49
            **/
            friend inline bool operator == (const basic_string& sstr1, const basic_string& sstr2){
                return sstr1._length == sstr2._length && sstr1.compare( sstr2 ) == 0;
            }

            /**
             * @brief 判断两字符串字典序的大小关系
             *
             * @param [in] sstr   : const basic_string&
             * @param [in] cstr   : const_pointer
             * @return  friend inline bool operator 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 12:14:24
            **/
            friend inline bool operator == (const basic_string& sstr, const_pointer cstr ){
                return sstr.compare( cstr ) == 0;
            }

            /**
             * @brief 判断两字符串字典序的大小关系
             *
             * @param [in] cstr   : const_pointer
             * @param [in] sstr   : const basic_string&
             * @return  friend inline bool operator 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 12:14:28
            **/
            friend inline bool operator == (const_pointer cstr, const basic_string& sstr ){ 
                return sstr.compare( cstr ) == 0;
            }

            /**
             * @brief 判断两字符串字典序的大小关系
             *
             * @param [in] sstr1   : const basic_string&
             * @param [in] sstr2   : const basic_string&
             * @return  friend inline bool operator 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 12:14:32
            **/
            friend inline bool operator != (const basic_string& sstr1, const basic_string& sstr2){
                return sstr1._length != sstr2._length || sstr1.compare( sstr2 ) != 0;
            }

            /**
             * @brief 判断两字符串字典序的大小关系
             *
             * @param [in] sstr   : const basic_string&
             * @param [in] cstr   : const_pointer
             * @return  friend inline bool operator 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 12:14:36
            **/
            friend inline bool operator != (const basic_string& sstr, const_pointer cstr ){
                return sstr.compare( cstr ) != 0;
            }

            /**
             * @brief 判断两字符串字典序的大小关系
             *
             * @param [in] cstr   : const_pointer
             * @param [in] sstr   : const basic_string&
             * @return  friend inline bool operator 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 12:14:39
            **/
            friend inline bool operator != (const_pointer cstr, const basic_string& sstr ){ 
                return sstr.compare( cstr ) != 0;
            }

            /**
             * @brief 判断两字符串字典序的大小关系
             *
             * @param [in] sstr1   : const basic_string&
             * @param [in] sstr2   : const basic_string&
             * @return  friend inline bool operator 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 12:14:47
            **/
            friend inline bool operator < (const basic_string& sstr1, const basic_string& sstr2){
                return sstr1.compare( sstr2 ) < 0;
            }

            /**
             * @brief 判断两字符串字典序的大小关系
             *
             * @param [in] sstr   : const basic_string&
             * @param [in] cstr   : const_pointer
             * @return  friend inline bool operator 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 12:14:51
            **/
            friend inline bool operator < (const basic_string& sstr, const_pointer cstr ){
                return sstr.compare( cstr ) < 0;
            }

            /**
             * @brief 判断两字符串字典序的大小关系
             *
             * @param [in] cstr   : const_pointer
             * @param [in] sstr   : const basic_string&
             * @return  friend inline bool operator 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 12:15:00
            **/
            friend inline bool operator < (const_pointer cstr, const basic_string& sstr ){ 
                return sstr.compare( cstr ) > 0;
            }

            /**
             * @brief 判断两字符串字典序的大小关系
             *
             * @param [in] sstr1   : const basic_string&
             * @param [in] sstr2   : const basic_string&
             * @return  friend inline bool operator 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 12:15:04
            **/
            friend inline bool operator > (const basic_string& sstr1, const basic_string& sstr2){
                return sstr1.compare( sstr2 ) > 0;
            }

            /**
             * @brief 判断两字符串字典序的大小关系
             *
             * @param [in] sstr   : const basic_string&
             * @param [in] cstr   : const_pointer
             * @return  friend inline bool operator 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 12:15:09
            **/
            friend inline bool operator > (const basic_string& sstr, const_pointer cstr ){
                return sstr.compare( cstr ) > 0;
            }

            /**
             * @brief 判断两字符串字典序的大小关系
             *
             * @param [in] cstr   : const_pointer
             * @param [in] sstr   : const basic_string&
             * @return  friend inline bool operator 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 12:15:12
            **/
            friend inline bool operator > (const_pointer cstr, const basic_string& sstr ){ 
                return sstr.compare( cstr ) < 0;
            }

            /**
             * @brief 判断两字符串字典序的大小关系
             *
             * @param [in] sstr1   : const basic_string&
             * @param [in] sstr2   : const basic_string&
             * @return  friend inline bool operator 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 12:15:16
            **/
            friend inline bool operator <= (const basic_string& sstr1, const basic_string& sstr2){
                return sstr1.compare( sstr2 ) <= 0;
            }

            /**
             * @brief 判断两字符串字典序的大小关系
             *
             * @param [in] sstr   : const basic_string&
             * @param [in] cstr   : const_pointer
             * @return  friend inline bool operator 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 12:15:21
            **/
            friend inline bool operator <= (const basic_string& sstr, const_pointer cstr ){
                return sstr.compare( cstr ) <= 0;
            }

            /**
             * @brief 判断两字符串字典序的大小关系
             *
             * @param [in] cstr   : const_pointer
             * @param [in] sstr   : const basic_string&
             * @return  friend inline bool operator 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 12:15:25
            **/
            friend inline bool operator <= (const_pointer cstr, const basic_string& sstr ){ 
                return sstr.compare( cstr ) >= 0;
            }

            /**
             * @brief 判断两字符串字典序的大小关系
             *
             * @param [in] sstr1   : const basic_string&
             * @param [in] sstr2   : const basic_string&
             * @return  friend inline bool operator 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 12:15:28
            **/
            friend inline bool operator >= (const basic_string& sstr1, const basic_string& sstr2){
                return sstr1.compare( sstr2 ) >= 0;
            }

            /**
             * @brief 判断两字符串字典序的大小关系
             *
             * @param [in] sstr   : const basic_string&
             * @param [in] cstr   : const_pointer
             * @return  friend inline bool operator 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 12:15:33
            **/
            friend inline bool operator >= (const basic_string& sstr, const_pointer cstr ){
                return sstr.compare( cstr ) >= 0;
            }

            /**
             * @brief 判断两字符串字典序的大小关系
             *
             * @param [in] cstr   : const_pointer
             * @param [in] sstr   : const basic_string&
             * @return  friend inline bool operator 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 12:15:36
            **/
            friend inline bool operator >= (const_pointer cstr, const basic_string& sstr ){ 
                return sstr.compare( cstr ) <= 0;
            }

#if __GNUC__ <= 2
            /**
             * @brief 把本字符串输出到流
             *
             * 本函数使bsl::string可以使用cout、cerr或者stringstream等类型来输出
             * @param [in] os   : std::ostream&
             * @param [in] sstr   : const basic_string&
             * @return  friend inline std::ostream&  operator 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 12:15:41
            **/
            friend inline std::ostream& 
                operator << ( std::ostream& os, const basic_string& sstr ){
                //TODO: implementation for value_type = ushort or uint required
                for( const_pointer p = sstr._dataplus._str; *p; ++ p){
                    os << *p;
                }
                return os;
            }
#else
            /**
             * @brief 把本字符串输出到流
             *
             * 本函数使bsl::string可以使用cout、cerr或者stringstream等类型来输出
             * @param [in] os   : std::ostream&
             * @param [in] sstr   : const basic_string&
             * @return  friend inline std::ostream&  operator 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/09/16 12:15:41
            **/
            friend inline std::basic_ostream<value_type>& 
                operator << ( std::basic_ostream<value_type>& os, const basic_string& sstr ){
                //TODO: implementation for value_type = ushort or uint required
                for( const_pointer p = sstr._dataplus._str; *p; ++ p){
                    os << *p;
                }
                return os;
            }
#endif
        public:
            //非标准扩展
            /**
             * @brief 接口与标准C函数printf()相同，输出值作为*this的新值，若容量不够，会自动扩充
             *
             * @param [in] format   : const char*
             * @return  basic_string& 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/11/09 15:44:39
            **/
            basic_string& setf( const char * format, ... )__attribute__ ((format (printf, 2, 3) ));

            /**
             * @brief 接口与标准C函数printf()相同，输出值追加到原字符串尾，若容量不够，会自动扩充
             *
             * @param [in] format   : const char*
             * @return  basic_string& 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/11/09 16:37:26
            **/
            basic_string& appendf( const char * format, ... )__attribute__ ((format (printf, 2, 3) ));

            /**
             * @brief 
             *
             * @brief this function is removed because g++2.96 and the corresponding glibc doesn't support vsscanf function
             * @param [in/out] format   : const char*
             * @return  int 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/11/07 16:36:44
            **/
            /*
            int getf( const char * format, ... ) const {
                va_list ap;
                va_start( ap, format );
                int res = vsscanf( _dataplus._str, format, ap );
                va_end( ap );
                return res;
            }*/

            
            /**
             * @brief this function is removed because g++2.96 and the corresponding glibc doesn't support vsscanf function
             *
             * @param [in/out] format   : const char*
             * @param [in/out] ap   : va_list
             * @return  int 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/11/07 16:37:45
            **/
            /*
            int vgetf( const char * format, va_list ap ) const {
                return vsscanf( _dataplus._str, format, ap );
            }
            */

            /**
             * @brief 接口与标准C函数vprintf()相同，输出值作为*this的新值，若容量不够，会自动扩充
             *
             * @param [in] format   : const char*
             * @param [in] ap   : va_list
             * @return  basic_string& 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/11/09 16:38:21
            **/
            basic_string& vsetf( const char * format, va_list ap ){
                clear();
                return vappendf( format, ap );
            }

            /**
             * @brief 接口与标准C函数vprintf()相同，输出值追加到原字符串尾，若容量不够，会自动扩充
             *
             * 无论_CharT是什么，format总是const char *
             *
             * @param [in] format   : const char*
             * @param [in] ap   : va_list
             * @return  basic_string& 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/11/09 16:39:15
            **/
            basic_string& vappendf( const char * format, va_list ap ){
                _do_vappendf( format, ap, _CharT() );   //特化_CharT == char 以改善性能
                return *this;
            }

        
        private:

            static size_t _s_cstring_len( const_pointer cstr ) {
                if ( !cstr ){
                    throw NullPointerException()<<BSL_EARG<<"cstr";
                }
                const_pointer tmp = cstr;
                while( *tmp ){
                    ++ tmp;
                }
                return tmp - cstr;
            }

            static pointer _s_create_cstring( size_type __capacity, const allocator_type& alloc_ ) {
                if ( __capacity == 0 ){
                    // (Scott) Meyers Singleton
                    static value_type EMPTY_CSTRING[] = { '\0' };
                    return EMPTY_CSTRING;
                }
                pointer res = allocator_type(alloc_).allocate( __capacity + 1 );
                if ( res == NULL ){
                    throw BadAllocException()<<BSL_EARG<<"__capacity["<<__capacity<<"]";
                }
                res[0] = 0;
                return res;
            }

            static pointer _s_clone_cstring( const_pointer cstr, size_type len, const allocator_type& alloc_ ) {
                if ( cstr == NULL ){
                    throw NullPointerException()<<BSL_EARG<<"cstr";
                }else if ( len == 0 ){
                    static value_type EMPTY_CSTRING[] = { '\0' };
                    return EMPTY_CSTRING;
                }
                pointer res = _s_create_cstring( len, alloc_ );
                xmemcpy( res, cstr, len * sizeof(value_type) );//cstr[len] can be non-null 
                res[len] = 0;
                return res;
            }

            void _s_destroy_cstring( pointer cstr, size_type __capacity, const allocator_type& alloc_ ) {
                if(_shallow_copy==true)
                {
                    return;
                }
                if ( __capacity ){
                    allocator_type(alloc_).deallocate( cstr, __capacity + 1 );
                }
            }

            /**
             * @brief vappendf在_CharT = char时的特化
             *
             * @param [in] format   : const char*
             * @param [in] ap   : va_list
             * @return  void 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/11/10 20:46:22
            **/
            void _do_vappendf( const char * format, va_list ap, char /*type tag*/ ){
                va_list aq;
                va_copy( aq, ap );
                int len = vsnprintf( _dataplus._str + _length, _capacity - _length + 1, format, aq ); // 1 for the trailing '\0'
                va_end( aq );
                if ( len > int(_capacity - _length) ){
                    reserve( _length + len ); //扩充容量 _capacity = _length + len 
                    va_copy( aq, ap );
                    vsnprintf( _dataplus._str + _length, _capacity - _length + 1, format, aq );    //重试
                    va_end( aq );
                    _length += len;
                }else if ( len >= 0 ){
                    //pass
                    _length += len;
                }else{  //len < 0
                    //pass
                }
            }



            /**
             * @brief vappendf的泛化函数
             *
             * _CharType == wchar_t时也不能使用vswprintf()，这是因为vswprintf()与vsnprintf()接口不一致造成的
             *
             * @param [in] format   : const _CharT*
             * @param [in] ap   : va_list
             * @return  template<typename _CharType>             static int 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2008/11/10 18:20:00
            **/
            template<typename _CharType>
            void _do_vappendf( const char * format, va_list ap, _CharType /*type tag*/ ){
                va_list aq;
                va_copy( aq, ap );
                int append_len = vsnprintf( NULL, 0, format, aq ); //just get the length;
                va_end( aq );
                if ( append_len < 0 ){ //bad format
                    return; //pass
                }
                if ( append_len > int(_capacity - _length) ){
                    reserve( _length + append_len );
                }
                // 先抄到tmp中去
                typedef typename _Alloc::template rebind<char>::other char_alloc_t;
                basic_string<char, char_alloc_t> tmp;
                tmp.reserve(append_len);
                va_copy( aq, ap );
                tmp.vappendf( format, aq ); //只会写一次
                va_end( aq );

                //再逐字抄回来
                const char * tmp_str = tmp.c_str();
                for( int i = 0; i< append_len; ++ i ){  
                    _dataplus._str[_length+i]  = tmp_str[i];
                }
                _length += append_len;
                _dataplus._str[_length] = 0;
            }


        public:
            static const size_type npos = static_cast<size_type>(-1);

        private:

            // Use empty-base optimization: http://www.cantrip.org/emptyopt.html
            struct _alloc_hider_t: allocator_type{
                _alloc_hider_t( pointer p)
                    :allocator_type(), _str(p) {}

                _alloc_hider_t( pointer p, allocator_type alloc_)
                    :allocator_type(alloc_), _str(p) {}

                pointer  _str;
            };
            _alloc_hider_t _dataplus;           
            size_type  _length;
            size_type  _capacity;
            bool _shallow_copy;
        };

    template<class _CharT, class _Alloc>
        basic_string<_CharT,_Alloc>& basic_string<_CharT,_Alloc>::setf( const char * format, ... ){
            va_list ap;
            va_start( ap, format );
            clear();
            vappendf( format, ap );
            va_end( ap );
            return *this;
        }

    template<class _CharT, class _Alloc>
        basic_string<_CharT,_Alloc>& basic_string<_CharT,_Alloc>::appendf( const char * format, ... ){
            va_list ap;
            va_start( ap, format );
            vappendf( format, ap );
            va_end( ap );
            return *this;
        }

    /**
     * @brief 把bsl::string的内容追加到bsl::AutoBuffer
     *
     * 暂不支持其它_CharT及_AllocT
     *
     * @param [in] buf   : bsl::AutoBuffer&
     * @param [in] str   : bsl::string&
     * @return  bsl::AutoBuffer&  operator 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2009/05/07 16:07:16
    **/
    inline bsl::AutoBuffer& operator <<( bsl::AutoBuffer& buf, bsl::string& str ){
        return buf << str.c_str();
    }
}   // namespace bsl

#if __GNUC__ >= 3
namespace __gnu_cxx{
#else
namespace std{
#endif
    /**
    * @brief 针对__gnu_cxx::hash编写的hasher
    *  
    *  目前只支持char类型，但allocator类型为任意类型。
    *  使用与const char*相同的hash算法。
    */
    template<class allocator_t> 
        struct hash<bsl::basic_string<char, allocator_t> >{
            size_t operator()(const bsl::basic_string<char, allocator_t>& bs ) const {
                return __stl_hash_string(bs.c_str());
            }
        };
}   // namespace __gnu_cxx/std

namespace bsl{
    /**
    * @brief 针对bsl::xhash编写的hasher
    *  
    *  目前只支持char类型，但allocator类型为任意类型。
    *  使用与const char*相同的hash算法。
    */
    template<class allocator_t> 
        struct xhash<bsl::basic_string<char, allocator_t> >{
            size_t operator()(const bsl::basic_string<char, allocator_t>& bs ) const {
                return __bsl_hash_string(bs.c_str());
            }
        };
}   // namespace bsl

#endif // __BSL_STRING_H__

/* vim: set ts=4 sw=4 sts=4 tw=100 */
