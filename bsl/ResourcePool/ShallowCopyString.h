/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: ShallowCopyString.h,v 1.3 2009/06/15 06:29:04 chenxm Exp $ 
 * 
 **************************************************************************/

/**
 * @file ShallowCopyString.h
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/08/08 09:08:18
 * @version $Revision: 1.3 $ 
 * @brief 
 *  
 **/

#ifndef  __SHALLOWCOPYSTRING_H_
#define  __SHALLOWCOPYSTRING_H_
#include <cstring>
#include <iostream>
#include "bsl/exception/bsl_exception.h"
#include "bsl/containers/string/bsl_string.h"

namespace bsl{

    class ShallowCopyString{
    public:
        typedef char            value_type;
        typedef size_t          size_type;
        typedef long            difference_type;
        typedef value_type &    reference;
        typedef const reference const_reference;
        typedef value_type *    pointer;
        typedef const pointer   const_pointer;
        typedef pointer         iterator;
        typedef const_pointer   const_iterator;
#if __GNUC__ <= 2
        typedef string_char_traits<value_type> traits_type;
#else
        typedef std::char_traits<value_type> traits_type;
#endif
        //ctors and dtors

        /**
         * @brief 默认构造函数
         *
         * 默认构造的ShallowCopyString表示空串。
         * @param [in] str   : const char*
         * @see 
         * @author chenxm
         * @date 2008/08/08 11:02:09
         **/
        ShallowCopyString() 
            : _str(_S_EMPTY_CS()), _length(0){}

        /**
         * @brief 隐式构造函数
         *
         * 该函数把C风格字符串转化为ShallowCopyString。
         *
         * 该方法时间复杂度为O(1)，长度被懒惰求值。（直到size()方法第一次被调用）
         *
         * @param [in] str   : const char*
         * @see 
         * @author chenxm
         * @date 2009/04/10 17:57:28
        **/
        ShallowCopyString( const char * str )  
            : _str(str), _length(npos) {
                if ( NULL == str ){
                    throw NullPointerException()<<BSL_EARG<<"str is NULL";
                }
            }

        /**
         * @brief 用C风格字符串与指定的长度构造ShallowCopyString
         *
         * 注意：len必须与strlen(str)相等，否则行为未定义。
         *
         * @param [in] str   : const char*
         * @param [in] len   : size_t
         * @see 
         * @author chenxm
         * @date 2009/04/14 21:28:49
        **/
        ShallowCopyString( const char * str, size_t len ) 
            : _str(str), _length(len){
                if ( NULL == str ){
                    throw NullPointerException()<<BSL_EARG;
                }
                if ( str[len] != '\0' ){
                    //给出的长度值有误？
                    throw bsl::BadArgumentException()<<BSL_EARG<<"wrong len:"<<len;
                }
            }

        /**
         * @brief 用使用任意allocator生成的bsl::basic_string构造ShallowCopyString
         *
         * @param [in] bsl::basic_string<char   : const
         * @param [in] bsl_str   : allocator_t>&
         * @return  template<class allocator_t> 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/04/14 21:44:58
        **/
        template<class allocator_t>
        ShallowCopyString( const bsl::basic_string<char, allocator_t>& bsl_str )
            : _str(bsl_str.c_str()), _length(bsl_str.length()) {};

        /**
         * @brief 使用AutoBuffer构造ShallowCopyString
         *
         * @param [in] buf   : const bsl::AutoBuffer&
         * @see 
         * @author chenxm
         * @date 2009/04/14 21:46:07
        **/
        ShallowCopyString( const bsl::AutoBuffer& buf )
            : _str(buf.c_str()), _length(buf.size()){}

        /**
         * @brief 复制构造函数
         *
         * @param [in] sstr   : const ShallowCopyString&
         * @see 
         * @author chenxm
         * @date 2009/04/14 21:46:22
        **/
        ShallowCopyString( const ShallowCopyString& sstr ) 
            : _str(sstr._str), _length(sstr._length) {}

        /**
         * @brief 析构函数
         *
         * @see 
         * @author chenxm
         * @date 2009/04/14 21:46:51
        **/
        ~ShallowCopyString() {
            //do nothing
        }

        void clear(){
            _str = _S_EMPTY_CS();
            _length = 0;
        }
        // getters
        /**
         * @brief 返回C风格字符串
         *
         * @return  const char* 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/04/14 21:47:17
        **/
        const char * c_str() const  {
            return _str;
        }

        /**
         * @brief 返回字符串长度
         *
         * 若字符串使用ShallowCopyString(const char *)构造并且本函数从未调用过，时间复杂为度O(strlen(this->c_str()))，否则为O(1)
         *
         * @return  size_t 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/04/14 21:55:58
        **/
        size_t size() const {
            if ( _length == npos ){
                const_cast<size_t&>(_length) = strlen(_str);
            }
            return _length;
        }

        /**
         * @brief 返回size()
         *
         * @return  size_t 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/04/14 21:59:39
        **/
        size_t length() const {
            return size();
        }

        /**
         * @brief 返回size()
         *
         * @return  size_t 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/04/14 21:59:48
        **/
        size_t capacity() const {
            return size();
        }
        
        /**
         * @brief 返回第idx个字符
         *
         * 注：本函数不检查idx的合法性
         * @param [in] idx   : size_t
         * @return  char[] 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/04/14 22:00:06
        **/
        char operator []( size_t idx ) const {
            return _str[idx];
        }

        /**
         * @brief 返回第idx个字符
         *
         * 本函数检查idx合法性，若idx>=this->size()，抛出bsl::OutOfBoundException
         * @param [in] idx   : size_t
         * @return  char 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/04/14 22:00:50
        **/
        char at( size_t idx ) const {
            if ( idx < this->size() ){
                return _str[idx];
            }
            throw bsl::OutOfBoundException()<<BSL_EARG<<"idx:"<<idx<<" size():"<<size();
        }
            
        /**
         * @brief 复制赋值
         *
         * 浅复制，时间复杂度为O(1)
         * @param [in] sstr   : const ShallowCopyString&
         * @return  ShallowCopyString&  operator 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/04/14 22:02:05
        **/
        ShallowCopyString& operator = ( const ShallowCopyString& sstr ){
            _str        = sstr._str;
            _length     = sstr._length;
            return *this;
        }

        ShallowCopyString& operator = ( const AutoBuffer& buf ){
            _str        = buf.c_str();
            _length     = buf.size();
            return *this;
        }

        ShallowCopyString& operator = ( const bsl::string& str ){
            _str        = str.c_str();
            _length     = str.size();
            return *this;
        }

        ShallowCopyString& operator = ( const char *cstr ){
            if ( cstr == NULL ){
                throw bsl::NullPointerException()<<BSL_EARG<<"cstr is NULL";
            }
            if ( cstr != _str ){
                _str        = cstr;
                _length     = npos;
            }
            return *this;
        }

        /**
         * @brief 字典序比较
         *
         * 若本字符串字典序小于、等于、大于other，则返回值<0、=0、>0
         * 若两字符串首字母不相同，或字符串地址相同，该函数时间复杂度为O(1)
         * 
         *
         * @param [in] other   : const ShallowCopyString&
         * @return  int 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/04/14 22:02:31
        **/
        int compare( const ShallowCopyString& other ) const {
            if ( _str[0] != other._str[0] ){
                return traits_type::lt(_str[0], other._str[0]) ? -1 : 1;
            }
            if ( _str == other._str ){
                return 0;   //对于ShallowCopyString来说这是有可能的。
            }
            size_t min_size = std::min( size() , other.size() );
            int ret = memcmp( _str, other._str, min_size );
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

        //comparers
        //因为const char *转换为ShallowCopyString是O(1)的，ShallowCopyString与一个随机的C字符串比较也是平均O(1)的（预判断第一字节），所以不专门对C字符串做比较函数了。
        friend inline bool operator == (const ShallowCopyString& sstr1, const ShallowCopyString& sstr2){
            return sstr1.compare( sstr2 ) == 0;
        }

        friend inline bool operator != (const ShallowCopyString& sstr1, const ShallowCopyString& sstr2){
            return sstr1.compare( sstr2 ) != 0;
        }

        friend inline bool operator < (const ShallowCopyString& sstr1, const ShallowCopyString& sstr2){
            return sstr1.compare( sstr2 ) < 0;
        }

        friend inline bool operator > (const ShallowCopyString& sstr1, const ShallowCopyString& sstr2){
            return sstr1.compare( sstr2 ) > 0;
        }

        friend inline bool operator <= (const ShallowCopyString& sstr1, const ShallowCopyString& sstr2){
            return sstr1.compare( sstr2 ) <= 0;
        }

        friend inline bool operator >= (const ShallowCopyString& sstr1, const ShallowCopyString& sstr2){
            return sstr1.compare( sstr2 ) >= 0;
        }

        /**
         * @brief 输出到输出流
         *
         * @param [in] os   : std::ostream&
         * @param [in] sstr   : const ShallowCopyString&
         * @return  friend inline std::ostream&  operator 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/04/14 22:14:58
        **/
        friend inline std::ostream& operator << ( std::ostream& os, const ShallowCopyString& sstr ){
            return os << sstr._str ;
        }
        
        static const size_t npos = static_cast<size_t>(-1);

    private:
        const char *    _str;
        size_t          _length;

        static const char *_S_EMPTY_CS(){
            static const char ret[] = "";
            return ret;
        };
    };
}   // namespace bsl

/**
 * @brief 定义bsl::ShallowCopyString追加到bsl::AutoBuffer的运算符
 *
 * @param [in] buf   : bsl::AutoBuffer&
 * @param [in] str   : bsl::ShallowCopyString&
 * @return  bsl::AutoBuffer&  operator 
 * @retval   
 * @see 
 * @author chenxm
 * @date 2009/05/09 01:59:57
**/
inline bsl::AutoBuffer& operator <<( bsl::AutoBuffer& buf, bsl::ShallowCopyString& str ){
    return buf.push(str.c_str(), str.length());
}

#if __GNUC__ >=3
namespace __gnu_cxx{
#else
namespace std{
#endif
    /**
    * @brief 针对__gnu_cxx::hash编写的hasher
    *  
    *  使用与const char*相同的hash算法。
    */
    template<>
    struct hash<bsl::ShallowCopyString>{
        size_t operator()(const bsl::ShallowCopyString& str ) const {
            return __stl_hash_string(str.c_str());
        }
    };
}   // namespace __gnu_cxx/std

namespace bsl{
    /**
    * @brief 针对bsl::xhash编写的hasher
    *  
    *  使用与const char*相同的hash算法。
    */
    template<>
    struct xhash<bsl::ShallowCopyString>{
        size_t operator()(const bsl::ShallowCopyString& str ) const {
            return __bsl_hash_string(str.c_str());
        }
    };
}   // namespace bsl

#endif  //__SHALLOWCOPYSTRING_H_
/* vim: set ts=4 sw=4 sts=4 tw=100 */
