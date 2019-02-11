/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: var_traits.h,v 1.2 2010/04/28 12:45:33 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file var_traits.h
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/09/24 01:32:42
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/
#ifndef  __BSL_VAR_TRAITS_H__
#define  __BSL_VAR_TRAITS_H__

#include <cstdlib>
#include <climits>
#include "bsl/var/IVar.h"

namespace bsl{ namespace var{
    /**
    * @brief 关于var的一些类型萃取特性
    *  
    *  
    */
    template<typename C>
        struct var_traits;
    /*
       signed char int8
       unsigned char   uint8
       signed short    int16
       unsigned short  uint16
       signed int  int32
       unsigned int    uint32
       signed long long    int64
       unsigned long long  uint64
       */
    template<>
        struct var_traits<signed char>{
            static const IVar::mask_type MASK       = IVar::IS_NUMBER|IVar::IS_SIGNED|IVar::IS_ONE_BYTE;
            static const IVar::mask_type ANTI_MASK  = IVar::IS_FLOATING;
        };
    template<>
        struct var_traits<unsigned char>{
            static const IVar::mask_type MASK       = IVar::IS_NUMBER|IVar::IS_ONE_BYTE;
            static const IVar::mask_type ANTI_MASK  = IVar::IS_FLOATING|IVar::IS_SIGNED;
        };
    template<>
        struct var_traits<signed short>{
            static const IVar::mask_type MASK       = IVar::IS_NUMBER|IVar::IS_SIGNED|IVar::IS_TWO_BYTE;
            static const IVar::mask_type ANTI_MASK  = IVar::IS_FLOATING;
        };
    template<>
        struct var_traits<unsigned short>{
            static const IVar::mask_type MASK       = IVar::IS_NUMBER|IVar::IS_TWO_BYTE;
            static const IVar::mask_type ANTI_MASK  = IVar::IS_FLOATING|IVar::IS_SIGNED;
        };
    template<>
        struct var_traits<signed int>{
            static const IVar::mask_type MASK       = IVar::IS_NUMBER|IVar::IS_SIGNED|IVar::IS_FOUR_BYTE;
            static const IVar::mask_type ANTI_MASK  = IVar::IS_FLOATING;
        };
    template<>
        struct var_traits<unsigned int>{
            static const IVar::mask_type MASK       = IVar::IS_NUMBER|IVar::IS_FOUR_BYTE;
            static const IVar::mask_type ANTI_MASK  = IVar::IS_FLOATING|IVar::IS_SIGNED;
        };
    template<>
        struct var_traits<signed long long>{
            static const IVar::mask_type MASK       = IVar::IS_NUMBER|IVar::IS_SIGNED|IVar::IS_EIGHT_BYTE;
            static const IVar::mask_type ANTI_MASK  = IVar::IS_FLOATING;
        };
    template<>
        struct var_traits<unsigned long long>{
            static const IVar::mask_type MASK       = IVar::IS_NUMBER|IVar::IS_EIGHT_BYTE;
            static const IVar::mask_type ANTI_MASK  = IVar::IS_FLOATING|IVar::IS_SIGNED;
        };
    template<>
        struct var_traits<float>{
            static const IVar::mask_type MASK       = IVar::IS_NUMBER|IVar::IS_FLOATING|IVar::IS_FOUR_BYTE;
            static const IVar::mask_type ANTI_MASK  = IVar::NONE_MASK;
        };
    template<>
        struct var_traits<double>{
            static const IVar::mask_type MASK       = IVar::IS_NUMBER|IVar::IS_FLOATING|IVar::IS_EIGHT_BYTE;
            static const IVar::mask_type ANTI_MASK  = IVar::NONE_MASK;
        };

}}   //namespace bsl::var

#endif  //__BSL_VAR_TRAITS_H__

/* vim: set ts=4 sw=4 sts=4 tw=100 */
