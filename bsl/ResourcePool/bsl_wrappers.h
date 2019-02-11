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
 * @brief ���ļ������˴����ĺ������������Ѹ��־���C++��ɫ�ĺ������繹�캯��������������new�������delete�������ģ�溯���ȣ���װ�ɼ򵥵�C����������͡��Ӷ�������������ָ�룬��������C���ĺ����ӿ�
 * 
 * 
 * ����һ��������ʵ�ֶ������������Ӵ�C����ԱҲ���Ժܿ��ĵ���C++�ˡ�
 * 
 * ע���������C++����Ϥ���޸ı��ļ��Ĵ���
 *  
 **/

#ifndef  __BSL_WRAPPERS_H_
#define  __BSL_WRAPPERS_H_

#include <memory>
#include <iostream>
#include <cstdio>
#include "bsl/ResourcePool/bsl_wrappers_config.h"

/**
 * @brief ����������������POD�ĳ�ʼ�������㣩
 *  
 *  ������������ģ��malloc()��placement operator new����Ϊ,����߷���POD������ٶ�
 *  ��������������ģ��calloc(), ��ͨoperator new��STL��������Ϊ��
 */
//#define BSL_CONFIG_SKIP_POD_INITIALIAZION

/**
 * @brief �������������ڹ����������ʱʹ��Ĭ�Ϲ��캯�����Ǹ��ƹ��캯��
 *  
 *  ������������ģ��operator new����Ϊ
 *  ��������������ģ��STL��������Ϊ
 */
//#define BSL_CONFIG_DO_NOT_COPY_FOR_DEFAULT_CONSTRUCTION

namespace bsl{
    /**
     * @brief construct(void*)�����ĸ�����������T����ӵ��ƽ���Ĺ��캯��ʱ������
     *
     * ע���ڲ�ʹ�ã��û���Ӧֱ�ӵ��øú�����
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
     * @brief construct(void*)�����ĸ�����������T����ӵ�з�ƽ���Ĺ��캯��ʱ������
     *
     * ע���ڲ�ʹ�ã��û���Ӧֱ�ӵ��øú�����
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
     * @brief T����Ĭ�Ϲ��캯����C����������
     *  
     *  
     */
    template<typename T>
        inline void construct(void * p_object){
            typedef __BSL_HAS_TRIVIAL_DEFAULT_CONSTRUCTOR(T) __is_POD;
            __construct_aux<T>(p_object, __is_POD() );
        }
    /**
     * @brief T���͵��������캯�����ر�أ���T_arg = Tʱ��Ϊ���ƹ��캯������C����������
     *  
     *  
     */
    template<typename T, typename T_arg>
        inline void construct(void * p_object, const void * p_arg ){
            new(p_object) T(*static_cast<const T_arg*>(p_arg));
        }

    /**
     * @brief T��������������C����������
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
     * @brief construct_array(void*,void*)�����ĸ�����������T����ӵ�з�ƽ���Ĺ��캯��ʱ������
     *
     * ע���ڲ�ʹ�ã��û���Ӧֱ�ӵ��øú�����
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
     * @brief construct_array(void*,void*)�����ĸ�����������T����ӵ��ƽ���Ĺ��캯��ʱ������
     *
     * ע���ڲ�ʹ�ã��û���Ӧֱ�ӵ��øú�����
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
     * @brief ����T����Ĭ�Ϲ��캯��������������C���������� 
     *
     * @param [in] begin   : void*  �����������ַ
     * @param [in] end   : void*    ���������βַ������ַ�������С��
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
     * @brief ����T���͵��������캯�����ر�أ���T_arg = Tʱ��Ϊ���ƹ��캯����������������C������������
     *
     * @param [in] begin   : void*  �����������ַ
     * @param [in] end   : void*    ���������βַ������ַ�������С��
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
     * @brief destroy_array(void *, void *)�ĸ�������������ǰg++�汾��֧��type-traits����δ��bsl_wrappers_config.h�ж����ʱ��ʹ��
     *
     * @param [in] begin   : T* T���Ͷ����������ַ
     * @param [in] end   : T*   T���Ͷ��������βַ������ַ�������С��
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
     * @brief ����T���Ͷ��������C����������
     *
     * @param [in] begin   : void*  T���Ͷ����������ַ
     * @param [in] end   : void*    T���Ͷ��������βַ������ַ�������С��
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
     * @brief ͨ��Alloc���Ϳռ�������ͷ��ڴ��C����������
     * ���ö��ڴ治����Alloc������ͷŻ� end - begin ) �� sizeof(Alloc::value_type) != 0������δ����
     * @param [in] begin   : void* ���ͷŵ��ڴ�����ַ
     * @param [in] end     : void* ���ͷŵ��ڴ���βַ
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
     * @brief destroy(void*)��deallocate(void*)����� 
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
     * @brief destroy_array(void*, void*)��deallocate(void*)����� 
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
     * @brief ::operator delete��C����������
     *
     * @param [in] object   : void* �����ַ
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
     * @brief ::operator delete[]��C����������
     *
     * @param [in] array   : void* ����������ַ
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
     * @brief ϵͳ����free(void*)��C�������������ṩ����֧��
     *
     * ��BSL�У�����ʹ�øú���������ϵͳ����free(void*)
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
        // ���������Ϣ
        std::cerr<<"memory "<<data<<" freeed"<<std::endl;
#endif
        free(data);
    }

    /**
     * @brief ϵͳ����fclose(FILE*)��C����������������ǩ�����в�ͬ�����ṩ����֧��
     *
     * ��BSL�У�����ʹ�øú���������ϵͳ����fclose(FILE*)
     * @param [in] data   : void*
     * @return  void 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2008/08/12 20:53:17
     **/
    inline void bsl_fclose( void *data){
#ifdef BSL_DEBUG_FLAG
        // ���������Ϣ
        std::cerr<<"file "<<data<<" closed"<<std::endl;
#endif
        fclose(static_cast<FILE*>(data));
    }

}   //namespace bsl
#endif  //__BSL_WRAPPERS_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
