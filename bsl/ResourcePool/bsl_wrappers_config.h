/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_wrappers_config.h,v 1.2 2009/03/09 04:56:41 xiaowei Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file bsl_wrappers_config.h
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/08/11 11:32:53
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/

#ifndef  __BSL_WRAPPERS_CONFIG_H_
#define  __BSL_WRAPPERS_CONFIG_H_

#if     __GNUC__ == 2 && __GNUC_MINOR__ == 96
    #define __BSL_TRUE_TYPE__   __true_type
    #define __BSL_FALSE_TYPE__  __false_type
    #define __BSL_HAS_TRIVIAL_DEFAULT_CONSTRUCTOR(_TYPE_) typename __type_traits<_TYPE_>::has_trivial_default_constructor
    #define __BSL_DESTROY       std::destroy

#elif   __GNUC__ == 3 && __GNUC_MINOR__ == 4
    #define __BSL_TRUE_TYPE__   __true_type
    #define __BSL_FALSE_TYPE__  __false_type
    #define __BSL_HAS_TRIVIAL_DEFAULT_CONSTRUCTOR(_TYPE_) typename __type_traits<_TYPE_>::has_trivial_default_constructor
    #define __BSL_DESTROY       std::_Destroy

#elif   __GNUC__ >= 4
    //enjoy std::tr1!
    #include <tr1/type_traits>
    #define __BSL_TRUE_TYPE__   std::tr1::true_type
    #define __BSL_FALSE_TYPE__  std::tr1::false_type
    #define __BSL_HAS_TRIVIAL_DEFAULT_CONSTRUCTOR(_TYPE_) typename std::tr1::has_trivial_constructor<_TYPE_>
    #define __BSL_DESTROY       std::_Destroy

#else   //使用最保守的做法
    #ifndef BSL_CONFIG_NO_TYPE_TRAITS_WARNIBSL
    #warning type-traits support not implemented for the current g++ version. Default implementation is used. Add related configuration to bsl_wrappers_config.h to add type-traits support. Disable this wanring by defining marco BSL_CONFIG_NO_TYPE_TRAITS_WARNIBSL.
    #endif  //BSL_CONFIG_NO_TYPE_TRAITS_WARNIBSL

    namespace bsl{
        typedef struct __bsl_true_type__{}     __bsl_true_type__;
        typedef struct __bsl_false_type__{}    __bsl_false_type__;
    }

    #define __BSL_TRUE_TYPE__   __bsl_true_type__
    #define __BSL_FALSE_TYPE__  __bsl_false_type__
    #define __BSL_HAS_TRIVIAL_DEFAULT_CONSTRUCTOR(_TYPE_)  __BSL_FALSE_TYPE__ 
    #define __BSL_DESTROY       default_destroy_array

#endif // #if __GNUC__

#endif  //__BSL_WRAPPERS_CONFIG_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
