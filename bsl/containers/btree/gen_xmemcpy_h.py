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
//��Ҫ�༭����ļ�..zhangyan04@baidu.com
//���xiaowei��xmemcpy.��������������ҷǳ���ˬ��һ�����
//����һ��.cpp,����Ҫ����һ����..
#include <cstdio>
#include <cstdlib>
#include <cstring>
#ifdef __SSE2__
#include <emmintrin.h>
#endif
//ʵ�ַ������ֿռ�����..
namespace %(ns_name)s{
#ifdef __SSE2__
typedef __m128i _memcpy_unit_128_t;
#endif
typedef unsigned long long _memcpy_unit_64_t;
typedef int _memcpy_unit_32_t;
%(implement)s
//��ڷ���class��Ϊ��̬����..
//class %(cls_name)s{
//public:
static void *xmemcpy(void *dst,const void *src,size_t len){
//����ʹ��long�Ǻ����.
long _offset=(long)dst-(long)src;
//λ���ص�������len����..
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
    #����..
    GENERAL_TEMPLATE="""//�������Ϊ_memcpy��ʵ��.
    %(_memcpy_implement)s
    //�������Ϊ��ת��
    typedef void* (*_memcpy_func_t)(void *dst,const void *src);
    static const _memcpy_func_t got[]={
    %(gototables)s
    };
    """

    #��ͬ���Ͷ�Ӧ��block.
    BLOCK_TYPE="""//����ʹ�ýṹ��,���鲻����copy.
    template<size_t size> struct _memcpy_block_%(aggtype)s_t{
    %(aggtype)s _data[size];
    };
    """
    
    #ʵ����ʵ��.
    TEMPLATE="""static void * _memcpy_%(bsize)d(void *dst,const void *src){
    typedef _memcpy_block_%(aggtype)s_t<%(bsize_roundup)d> type_t;
    *(type_t*)dst=*(type_t*)src;
    %(extra)s
    return dst;
    }
    """

    #�����ʵ����ʵ��..
    SPEC_TEMPLATE="""static void * _memcpy_%(bsize)d(void *dst,const void *src){    
    %(extra)s
    return dst;
    }
    """
    
    SPEC_TEMPLATE_ZERO="""static void * _memcpy_0(void *dst,const void */*src*/){    
    return dst;
    }
    """
    #��64λ����֧��SSE2ָ��..
    if(SSE2):
        #����512�ֽڵĻ�,��ô�������Ż�..
        thres=512
        conf_list=[['_memcpy_unit_32_t',4,0,128],
                   ['_memcpy_unit_128_t',16,129,512]]
    else:
        thres=128
        #����128�ֽڵĻ�,�Ͳ������Ż�.
        conf_list=[['_memcpy_unit_32_t',4,0,128]]
        
    #����memcpy��ʵ��.
    _memcpy_implement=""
    for conf in conf_list:
        aggtype=conf[0]
        sizeof_aggtype=conf[1]
        #������Ҫ����block type..
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
                
    #����goto tables.
    gototables=""
    for i in range(0,thres+1):
        bsize=i
        gototables+="_memcpy_%(bsize)d,\n"%(locals())
    #�ڲ�implement
    implement=GENERAL_TEMPLATE%(locals())
    clsdecl=CLSDECL%(locals())
    return clsdecl

open('bsl_kv_btree_xmemcpy.h','w').write(generator('ZY','xmemcpy',SSE2=True))
