/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_bin_buffer.cpp,v 1.0 2010/07/31 13:29:04 chenxm Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file bsl_bin_buffer.cpp
 * @author liaosb(liaoshangbin@baidu.com)
 * @date 2010/07/31 13:47:35
 * @version $Revision: 1.0 $ 
 * @brief 
 *  
 **/
#include "bsl/utils/bsl_memcpy.h"
#include "BinBuffer.h"

namespace bsl{

    BinBuffer& BinBuffer::push(const void* data_, size_t len ){
        if ( data_ != NULL && len != 0 ){
            if ( _str && (_size + len <= _capacity) ){
                xmemcpy( _str + _size, data_, len );
                _size += len;
                _truncated = false;
            }else{  //not enough space;
                if ( reserve( _size + len)  ){
                    //reserve succeed, copy succeed
                    xmemcpy( _str + _size, data_, len );
                    _size += len;
                    _truncated = false;
                }else if ( _size < _capacity ){
                    //reserve failed
                    //still has memory
                    //content is truncated
                    xmemcpy( _str + _size, data_, _capacity - _size );
                    _size = _capacity;
                    _truncated = true;
                    _ever_truncated = true;
                }else{
                    //is full or _str == NULL
                    _truncated = true;
                    _ever_truncated = true;
                    _size = _capacity;
                }
            }
        }
        return *this;
    }
                   
}   //namespace bsl
/* vim: set ts=4 sw=4 sts=4 tw=100 */
