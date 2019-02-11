#!/usr/bin/env python
#coding:gbk
#copy[write] by dirlt(dirtysalt1987@gmail.com)

import string
import re
import os
import sys

#class declaration.[incluing the necessary namespace]
CLSDECL="""/* -*- c++ -*- */
#ifndef __BSL_KV_BTREE_XMEMCPY_H__
#define __BSL_KV_BTREE_XMEMCPY_H__
//不要编辑这个文件..zhangyan04@baidu.com
//借鉴xiaowei的xmemcpy.但是这个东西让我非常不爽的一点就是
//它有一个.cpp,你需要链接一个库..
#include <cstdio>
#include <cstdlib>
#include <cstring>
#ifdef __SSE2__
#include <emmintrin.h>
#endif
//实现放在名字空间下面..
namespace %(ns_name)s{
#ifdef __SSE2__
typedef __m128i _memcpy_unit_128_t;
#endif
typedef unsigned long long _memcpy_unit_64_t;
typedef int _memcpy_unit_32_t;
%(implement)s
//入口放在class作为静态方法..
//class %(cls_name)s{
//public:
static void *xmemcpy(void *dst,const void *src,size_t len){
//这里使用long是合理的.
long _offset=(long)dst-(long)src;
//位置重叠或者是len过长..
if(len>%(thres)d || (_offset<(long)len && _offset>(0-(long)len))){
return ::memcpy(dst,src,len);
}else{
return %(ns_name)s::got[len](dst,src);
}
}
//};
}
#endif
"""

def generator(ns_name,cls_name,SSE2):
    #泛化..
    GENERAL_TEMPLATE="""//这个部分为_memcpy的实现.
    %(_memcpy_implement)s
    //这个部分为跳转表
    typedef void* (*_memcpy_func_t)(void *dst,const void *src);
    static const _memcpy_func_t got[]={
    %(gototables)s
    };
    """

    #不同类型对应的block.
    BLOCK_TYPE="""//必须使用结构体,数组不允许copy.
    template<size_t size> struct _memcpy_block_%(aggtype)s_t{
    %(aggtype)s _data[size];
    };
    """
    
    #实例化实现.
    TEMPLATE="""static void * _memcpy_%(bsize)d(void *dst,const void *src){
    typedef _memcpy_block_%(aggtype)s_t<%(bsize_roundup)d> type_t;
    *(type_t*)dst=*(type_t*)src;
    %(extra)s
    return dst;
    }
    """

    #特殊的实例化实现..
    SPEC_TEMPLATE="""static void * _memcpy_%(bsize)d(void *dst,const void *src){    
    %(extra)s
    return dst;
    }
    """
    
    SPEC_TEMPLATE_ZERO="""static void * _memcpy_0(void *dst,const void */*src*/){    
    return dst;
    }
    """
    #在64位下面支持SSE2指令..
    if(SSE2):
        #大于512字节的话,那么不进行优化..
        thres=512
        conf_list=[['_memcpy_unit_32_t',4,0,128],
                   ['_memcpy_unit_128_t',16,129,512]]
    else:
        thres=128
        #大于128字节的话,就不进行优化.
        conf_list=[['_memcpy_unit_32_t',4,0,128]]
        
    #产生memcpy的实现.
    _memcpy_implement=""
    for conf in conf_list:
        aggtype=conf[0]
        sizeof_aggtype=conf[1]
        #首先需要定义block type..
        block_type=BLOCK_TYPE%(locals())
        _memcpy_implement+=block_type
        for i in range(conf[2],conf[3]+1):
            bsize=i
            bsize_roundup=i/sizeof_aggtype
            extra=""
            for j in range(1,sizeof_aggtype):
                if((i%sizeof_aggtype)>=j):
                    extra+="""((char *)dst)[%d] = ((char *)src)[%d];"""%(i-j,i-j)
            if(i==0):
                template=SPEC_TEMPLATE_ZERO
            elif(i<sizeof_aggtype):
                template=SPEC_TEMPLATE%(locals())
            else:
                template=TEMPLATE%(locals())
            _memcpy_implement+=template
                
    #生成goto tables.
    gototables=""
    for i in range(0,thres+1):
        bsize=i
        gototables+="_memcpy_%(bsize)d,\n"%(locals())
    #内部implement
    implement=GENERAL_TEMPLATE%(locals())
    clsdecl=CLSDECL%(locals())
    return clsdecl

open('bsl_kv_btree_xmemcpy.h','w').write(generator('ZY','xmemcpy',SSE2=True))
