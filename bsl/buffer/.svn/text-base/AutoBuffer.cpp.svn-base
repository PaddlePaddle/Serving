/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_auto_buffer.cpp,v 1.6 2009/06/15 06:29:04 chenxm Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file bsl_auto_buffer.cpp
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/12/12 15:31:45
 * @version $Revision: 1.6 $ 
 * @brief 
 *  
 **/
#include "bsl/utils/bsl_memcpy.h"
#include "AutoBuffer.h"

namespace bsl{
    const char * const AutoBuffer::TRUE_LITERAL = "true";
    const char * const AutoBuffer::FALSE_LITERAL= "false";
    const size_t       AutoBuffer::TRUE_LITERAL_LEN = strlen(AutoBuffer::TRUE_LITERAL);
    const size_t       AutoBuffer::FALSE_LITERAL_LEN= strlen(AutoBuffer::FALSE_LITERAL);

    AutoBuffer& AutoBuffer::push(const char* cstr, size_t sub_str_len ){
        if ( cstr != NULL && sub_str_len != 0 ){
            if ( _str && (_size + sub_str_len <= _capacity) ){
                xmemcpy( _str + _size, cstr, sub_str_len );
                _size += sub_str_len;
                _str[_size] = 0;
                _truncated = false;

            }else{  //not enough space;
                if ( reserve( _size + sub_str_len)  ){
                    //reserve succeed, copy succeed
                    xmemcpy( _str + _size, cstr, sub_str_len );
                    _size += sub_str_len;
                    _str[_size] = 0;
                    _truncated = false;

                }else if ( _size < _capacity ){
                    //reserve failed
                    //still has memory
                    //content is truncated
                    xmemcpy( _str + _size, cstr, _capacity - _size );
                    _size = _capacity;
                    _str[_size] = 0;
                    _truncated = true;
                }else{
                    //is full or _str == NULL
                    _truncated = true;
                }
            }
        }
        return *this;
    }

    AutoBuffer& AutoBuffer::vpushf( const char *format, va_list ap ){
        va_list aq;
        int len = 0;
        if ( !format ){
            return *this;
        }
        if ( _str ){
            va_copy( aq, ap );
            len = vsnprintf( _str + _size, _capacity - _size + 1, format, aq );
            va_end( aq );
            if ( len >= 0 && _size + len <= _capacity ){
                _size += len;
                _truncated = false;
                return *this;
            }
        }else{
            va_copy( aq, ap );
            len = vsnprintf( NULL, 0, format, aq );
            va_end( aq );
        }

        if ( len > 0 ){
            if ( reserve( _size + len)  ){
                //reserve succeed, vsnprintf succeed
                va_copy( aq, ap );
                vsnprintf( _str + _size, _capacity - _size + 1, format, aq );
                va_end( aq );

                _size += len;
                _truncated = false;

            }else{
                //reserve failed
                //content is truncated
                _size = _capacity;
                _truncated = true;
            }

        }
        return *this;
    }

    //write this way because of a bug of g++ 2.96
    //varargs function cannot be inline
    AutoBuffer& AutoBuffer::pushf( const char *format, ... ){ 
        va_list ap;
        va_start( ap, format );
        vpushf( format, ap );
        va_end( ap );
        return *this;
    }
}   //namespace bsl
/* vim: set ts=4 sw=4 sts=4 tw=100 */
