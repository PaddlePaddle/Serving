/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_wrappers.h,v 1.2 2009/03/09 04:56:41 xiaowei Exp $ 
 * 
 **************************************************************************/

/**
 * @file bsl_wrappers.h
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/08/06 00:24:12
 * @version $Revision: 1.2 $ 
 * @brief 该文件定义了大量的函数包裹器，把各种具有C++特色的函数，如构造函数、析构函数、new运算符、delete运算符、模版函数等，包装成简单的C风格函数的类型。从而可以用作函数指针，或向后兼容C风格的函数接口
 * 
 * 
 * “把一切龌龊的实现都包藏起来，从此C程序员也可以很开心地玩C++了”
 * 
 * 注：不建议对C++不熟悉者修改本文件的代码
 *  
 **/

#ifndef  __BSL_WRAPPERS_H_
#define  __BSL_WRAPPERS_H_

#include <memory>
#include <iostream>
#include <cstdio>
#include "bsl/ResourcePool/bsl_wrappers_config.h"

/**
 * @brief 定义这个宏可以跳过POD的初始化（清零）
 *  
 *  定义这个宏可以模仿malloc()和placement operator new的行为,并提高分配POD数组的速度
 *  不定义这个宏可以模仿calloc(), 普通operator new，STL容器的行为。
 */
//#define BSL_CONFIG_SKIP_POD_INITIALIAZION

/**
 * @brief 定义这个宏可以在构造对象数组时使用默认构造函数而非复制构造函数
 *  
 *  定义这个宏可以模仿operator new的行为
 *  不定义这个宏可以模仿STL容器的行为
 */
//#define BSL_CONFIG_DO_NOT_COPY_FOR_DEFAULT_CONSTRUCTION

namespace bsl{
    /**
     * @brief construct(void*)函数的辅助函数，当T类型拥有平凡的构造函数时被调用
     *
     * 注：内部使用，用户不应直接调用该函数。
     *
     * @param [in] p_object   : void*
     * @return  void 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2008/08/07 10:48:10
     **/
    template<typename T>
        inline void __construct_aux(void * p_object, __BSL_TRUE_TYPE__){
#ifndef BSL_CONFIG_SKIP_POD_INITIALIAZION
            *static_cast<T*>(p_object) = 0;
#endif
        }

    /**
     * @brief construct(void*)函数的辅助函数，当T类型拥有非平凡的构造函数时被调用
     *
     * 注：内部使用，用户不应直接调用该函数。
     *
     * @param [in/out] p_object   : void*
     * @return  template<typename T> inline void 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2008/08/07 10:48:30
     **/
    template<typename T>
        inline void __construct_aux(void * p_object, __BSL_FALSE_TYPE__){
            ::new(p_object) T();    
        }

    /**
     * @brief T类型默认构造函数的C风格包裹函数
     *  
     *  
     */
    template<typename T>
        inline void construct(void * p_object){
            typedef __BSL_HAS_TRIVIAL_DEFAULT_CONSTRUCTOR(T) __is_POD;
            __construct_aux<T>(p_object, __is_POD() );
        }
    /**
     * @brief T类型单参数构造函数（特别地，当T_arg = T时，为复制构造函数）的C风格包裹函数
     *  
     *  
     */
    template<typename T, typename T_arg>
        inline void construct(void * p_object, const void * p_arg ){
            new(p_object) T(*static_cast<const T_arg*>(p_arg));
        }

    /**
     * @brief T类型析构函数的C风格包裹函数
     *
     * @param [in] p_object   : void*
     * @return void 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2008/08/12 20:29:56
     **/
    template<typename T>
        inline void destroy(void * p_object){
            static_cast<T*>(p_object)->~T();
        }

    /**
     * @brief construct_array(void*,void*)函数的辅助函数，当T类型拥有非平凡的构造函数时被调用
     *
     * 注：内部使用，用户不应直接调用该函数。
     * @param [in] begin   : void*
     * @param [in] end   : void*
     * @return  template<typename T> inline void 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2008/08/12 20:29:47
     **/
    template<typename T>
        inline void __construct_array_aux( void * begin, void * end, __BSL_FALSE_TYPE__){
#ifndef BSL_CONFIG_DO_NOT_COPY_FOR_DEFAULT_CONSTRUCTION
            std::uninitialized_fill( static_cast<T*>(begin), static_cast<T*>(end), T() ); 
#else
            ::new(begin) T[static_cast<T*>(end) - static_cast<T*>(begin)];
#endif 
        }

    /**
     * @brief construct_array(void*,void*)函数的辅助函数，当T类型拥有平凡的构造函数时被调用
     *
     * 注：内部使用，用户不应直接调用该函数。
     * @param [in] begin   : void*
     * @param [in] end   : void*
     * @return  template<typename T> inline void 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2008/08/12 20:31:31
     **/
    template<typename T>
        inline void __construct_array_aux( void * begin, void * end, __BSL_TRUE_TYPE__){
#ifndef BSL_CONFIG_SKIP_POD_INITIALIAZION
            memset(begin, 0, static_cast<char*>(end) - static_cast<char*>(begin) );
#endif
        }

    /**
     * @brief 调用T类型默认构造函数构造对象数组的C风格包裹函数 
     *
     * @param [in] begin   : void*  对象数组的首址
     * @param [in] end   : void*    对象数组的尾址（＝首址＋数组大小）
     * @return  template<typename T> inline void 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2008/08/12 20:31:36
     **/
    template<typename T>
        inline void construct_array(void * begin, void * end ){
            typedef __BSL_HAS_TRIVIAL_DEFAULT_CONSTRUCTOR(T) __is_POD;
            __construct_array_aux<T>( begin, end, __is_POD() );
        }

    /**
     * @brief 调用T类型单参数构造函数（特别地，当T_arg = T时，为复制构造函数）构造对象数组的C风格包裹函数，
     *
     * @param [in] begin   : void*  对象数组的首址
     * @param [in] end   : void*    对象数组的尾址（＝首址＋数组大小）
     * @return  template<typename T> inline void 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2008/08/12 20:31:36
     **/
    template<typename T, typename T_arg >
        inline void construct_array(void * begin, void * end, const void * src ){
            std::uninitialized_fill( static_cast<T *>(begin), static_cast<T *>(end), *static_cast<const T_arg *>(src) ); // this method will be specially optimized to POD;
        }

    /**
     * @brief destroy_array(void *, void *)的辅助函数，当当前g++版本不支持type-traits，或未在bsl_wrappers_config.h中定义的时候使用
     *
     * @param [in] begin   : T* T类型对象数组的首址
     * @param [in] end   : T*   T类型对象数组的尾址（＝首址＋数组大小）
     * @return void  
     * @retval   
     * @see 
     * @author chenxm
     * @date 2008/08/12 20:38:18
     **/
    template<typename T>
        inline void default_destroy_array(T* begin, T* end){
            while( begin != end ){
                (begin ++) -> ~T();
            }
        }

    /**
     * @brief 销毁T类型对象数组的C风格包裹函数
     *
     * @param [in] begin   : void*  T类型对象数组的首址
     * @param [in] end   : void*    T类型对象数组的尾址（＝首址＋数组大小）
     * @return void  
     * @retval   
     * @see 
     * @author chenxm
     * @date 2008/08/12 20:42:11
     **/
    template<typename T>
        inline void destroy_array( void * begin, void * end ){
            __BSL_DESTROY(static_cast<T*>(begin), static_cast<T*>(end)); // this method will be specially optimized to the type which has trivial destructor;
        }

    /**
     * @brief 通过Alloc类型空间分配器释放内存的C风格包裹函数
     * 若该段内存不可由Alloc类对象释放或（ end - begin ) ％ sizeof(Alloc::value_type) != 0，则结果未定义
     * @param [in] begin   : void* 被释放的内存块的首址
     * @param [in] end     : void* 被释放的内存块的尾址
     * @return void 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2008/08/12 20:43:49
     **/
    template<typename Alloc >
        inline void deallocate( void * begin, void * end ){
            typedef typename Alloc::value_type value_t;
            value_t * vt_begin = static_cast<value_t *>(begin);
            value_t * vt_end   = static_cast<value_t *>(end);
            Alloc().deallocate( vt_begin, vt_end - vt_begin );
        }

    /**
     * @brief destroy(void*)和deallocate(void*)的组合 
     *
     * @param [in] p_object   : void*
     * @return  void 
     * @retval   
     * @see destroy(void*)
     * @see deallocate(void*)
     * @author chenxm
     * @date 2008/08/12 20:46:56
     **/
    template<typename T, typename Alloc>
        inline void destroy_and_deallocate( void * p_object ){
            static_cast<T*>(p_object)->~T();
            typedef typename Alloc::template rebind<T>::other T_Alloc;
            T_Alloc().deallocate(static_cast<T *>(p_object), 1);
        }

    /**
     * @brief destroy_array(void*, void*)和deallocate(void*)的组合 
     *
     * @param [in] begin   : void*
     * @param [in] end   : void*
     * @return  void 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2008/08/12 20:48:04
     **/
    template<typename T, typename Alloc>
        inline void destroy_and_deallocate_array( void * begin, void * end ){
            __BSL_DESTROY(static_cast<T*>(begin), static_cast<T*>(end)); // this method will be specially optimized to the type which has trivial destructor;
            typedef typename Alloc::template rebind<T>::other T_Alloc;
            T_Alloc().deallocate(static_cast<T *>(begin), static_cast<T*>(end) - static_cast<T*>(begin) );
        }

    /**
     * @brief ::operator delete的C风格包裹函数
     *
     * @param [in] object   : void* 对象地址
     * @return void  
     * @retval   
     * @see 
     * @author chenxm
     * @date 2008/08/12 20:49:02
     **/
    template<typename T>
        inline void bsl_delete(void * object ){
            delete static_cast<T*>(object);
        }

    /**
     * @brief ::operator delete[]的C风格包裹函数
     *
     * @param [in] array   : void* 对象数组首址
     * @return  void 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2008/08/12 20:50:10
     **/
    template<typename T>
        inline void bsl_delete_array( void * array ){
            delete[] static_cast<T*>(array);
        }

    /**
     * @brief 系统函数free(void*)的C风格包裹函数，提供调试支持
     *
     * 在BSL中，建议使用该函数，而非系统函数free(void*)
     *
     * @param [in] data   : void*
     * @return  void 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2008/08/12 20:51:32
     **/
    inline void bsl_free( void *data){
#ifdef BSL_DEBUG_FLAG
        // 输出调试信息
        std::cerr<<"memory "<<data<<" freeed"<<std::endl;
#endif
        free(data);
    }

    /**
     * @brief 系统函数fclose(FILE*)的C风格包裹函数（函数签名略有不同），提供调试支持
     *
     * 在BSL中，建议使用该函数，而非系统函数fclose(FILE*)
     * @param [in] data   : void*
     * @return  void 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2008/08/12 20:53:17
     **/
    inline void bsl_fclose( void *data){
#ifdef BSL_DEBUG_FLAG
        // 输出调试信息
        std::cerr<<"file "<<data<<" closed"<<std::endl;
#endif
        fclose(static_cast<FILE*>(data));
    }

}   //namespace bsl
#endif  //__BSL_WRAPPERS_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
