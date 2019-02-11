/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: ResourcePool.cpp,v 1.5 2010/04/28 12:45:33 scmpf Exp $ 
 * 
 **************************************************************************/

/**
 * @file ResourcePool.cpp
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/08/06 11:14:09
 * @version $Revision: 1.5 $ 
 * @brief 
 *  
 **/

#include <cstdarg>
#include "bsl/ResourcePool.h"

namespace bsl{

    /**
     * @brief 清理所有受托管的资源
     *
     * @return  void 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2009/02/04 15:34:03
    **/
    void ResourcePool::reset(){
        for (block_list_node_t<alloc_object_info_t> *p_node = _p_alloc_object_list; 
                NULL != p_node; 
                p_node = p_node->p_next ){
            for( alloc_object_info_t* p_item = p_node->data + p_node->current; 
                    p_item >= p_node->data; 
                    --p_item ){
                p_item->destructor(p_item->p_object, _mempool);
            }
        }
        _clear_info_list(_p_alloc_object_list);
        for (block_list_node_t<alloc_array_info_t> *p_node = _p_alloc_array_list; 
                NULL != p_node; 
                p_node = p_node->p_next ){
            for( alloc_array_info_t* p_item = p_node->data + p_node->current; 
                    p_item >= p_node->data; 
                    --p_item ){
                p_item->destructor(p_item->begin, p_item->end, _mempool);
            }
        }
        _clear_info_list(_p_alloc_array_list);

        for (block_list_node_t<attach_object_info_t> *p_node = _p_attach_object_list; 
                NULL != p_node; 
                p_node = p_node->p_next ){
            for( attach_object_info_t* p_item = p_node->data + p_node->current; 
                    p_item >= p_node->data; 
                    --p_item ){
                p_item->destructor(p_item->p_object);
            }
        }
        _clear_info_list(_p_attach_object_list);
        for (block_list_node_t<attach_array_info_t> *p_node = _p_attach_array_list; 
                NULL != p_node; 
                p_node = p_node->p_next ){
            for( attach_array_info_t* p_item = p_node->data + p_node->current; 
                    p_item >= p_node->data; 
                    --p_item ){
                p_item->destructor(p_item->begin, p_item->end);
            }
        }
        _clear_info_list(_p_attach_array_list);
    }

    const char * ResourcePool::_vprintf( 
            alloc_array_info_t& info, 
            size_t hint_capacity, 
            const char *format, 
            va_list ap 
            ){
        //参数检查
        size_t size = hint_capacity + 1;
        char* str   = static_cast<char*>(_mempool.malloc(size));
        if ( !str ){
            throw bsl::BadAllocException()<<BSL_EARG<<"_mempool["<<&_mempool<<"] size["<<size<<"]";
        }

        //第一次尝试
        va_list aq;
        va_copy(aq, ap);
        int len = vsnprintf( str, size, format, aq );
        va_end(aq);
        try{
            if ( len < 0 ){
                throw bsl::BadFormatStringException()<<BSL_EARG<<"format:"<<format;
            }else if ( size_t(len) < size ){
                //成功了
                info.begin      = str;
                info.end        = str + size;
                info.destructor = _s_deallocate;
                return str;
            }else{
                //第一次尝试失败，回滚str的分配
                _mempool.free(str, size);
            }
        }catch(...){
            //回滚str的分配
            _mempool.free(str, size);
            throw;
        }

        //第二次尝试
        //len保存了真正需要的长度 
        //这次一定成功
        return _vprintf( info, len, format, ap );
    }
            
    const char *  ResourcePool::crcprintf(const char * format, ... ){
        //以60为默认值去尝试
        const size_t DEFAULT_STRING_SIZE = 60;
        alloc_array_info_t &info = _push_info( _p_alloc_array_list );
        va_list ap;
        va_start(ap, format);
        try{
            const char * res = _vprintf( info, DEFAULT_STRING_SIZE, format, ap );   //throw
            va_end(ap);
            return res;
        }catch(...){
            va_end(ap);
            //回滚info的分配
            _pop_info( _p_alloc_array_list );
            throw;
        }
    }    

     const char *  ResourcePool::crcprintf_hint(size_t hint_capacity, const char * format, ... ){
        alloc_array_info_t &info = _push_info( _p_alloc_array_list );
        va_list ap;
        va_start(ap, format);
        try{
            const char * res = _vprintf( info, hint_capacity, format, ap );   //throw
            va_end(ap);
            return res;
        }catch(...){
            va_end(ap);
            //回滚info的分配
            _pop_info( _p_alloc_array_list );
            throw;
        }
    }

     const char * ResourcePool::vcrcprintf(const char * format, va_list ap) {
         //以60为默认值去尝试
         const size_t DEFAULT_STRING_SIZE = 60;
         alloc_array_info_t &info = _push_info( _p_alloc_array_list );
         va_list aq;
         va_copy( aq, ap );
         try {
             const char * res = _vprintf( info, DEFAULT_STRING_SIZE, format, aq ); // throw
             va_end(aq);
             return res;
         } catch (...) {
             va_end( aq );
             //回滚info的分配
             _pop_info( _p_alloc_array_list );
             throw;
         }
     }


     const char * ResourcePool::vcrcprintf_hint(
             size_t hint_capacity, 
             const char * format, 
             va_list ap
             ) {
         alloc_array_info_t &info = _push_info( _p_alloc_array_list );
         va_list aq;
         va_copy( aq, ap );
         try {
             const char * res = _vprintf( info, hint_capacity, format, aq ); // throw
             va_end( aq );
             return res;
         } catch (...) {
             va_end( aq );
             //回滚info的分配
             _pop_info( _p_alloc_array_list );
             throw;
         }
     }
}   //namespace bsl

/* vim: set ts=4 sw=4 sts=4 tw=100 */
