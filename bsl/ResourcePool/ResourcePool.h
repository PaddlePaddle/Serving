/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: ResourcePool.h,v 1.6 2010/04/28 12:45:33 scmpf Exp $ 
 * 
 **************************************************************************/

/**
 * @file ResourcePool.h
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/08/06 00:25:57
 * @version $Revision: 1.6 $ 
 * @brief 
 *  
 **/

#ifndef  __RESOURCEPOOL_H_
#define  __RESOURCEPOOL_H_

#include "bsl/exception/bsl_exception.h"
#include "bsl/pool/bsl_pool.h"
#include "bsl/pool/bsl_poolalloc.h"
#include "bsl/utils/bsl_memcpy.h"
#include "bsl/ResourcePool/bsl_wrappers.h"

namespace bsl{
    /**
    * @brief ��Դ��
    *  
    * �û�����ͨ����Դ���йܰ����ڴ桢������ڵĸ�����Դ�������Խ��졢��¡���ֶ������顣
    *
    */
    class ResourcePool{
    public:
        
        typedef bsl::pool_allocator<char>   allocator_type;

        /**
        * @brief ��������Դ/����Ļص�����ָ��
        *  
        *  �ûص���������Դָ�루��FILE*��/�����ַ��Ϊ������û�з���ֵ
        * ng_wrappers.h����Գ�����Դ��ʵ�֣����龡��ʹ��ng_wrappers.h��ʵ��
        */
        typedef void( *object_destructor_t )(void * p_object);

        /**
        * @brief ������Դ���顢��������Ļص�����ָ��
        *  
        *  �ûص��������������ַ��βַ������ַ�������С����Ϊ������û�з���ֵ��
        */
        typedef void( *array_destructor_t )(void * begin, void * end);

        /**
         * @brief Ĭ�Ϲ��캯��
         *
         * @see 
         * @author chenxm
         * @date 2008/08/12 18:07:42
        **/
        ResourcePool()
            :_syspool(), 
            _mempool(_syspool), 
            _p_attach_object_list(NULL), 
            _p_attach_array_list(NULL), 
            _p_alloc_object_list(NULL), 
            _p_alloc_array_list(NULL){}

        /**
         * @brief ʹ��allocator�Ĺ��캯����
         *
         * @return  explicit 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/04/09 17:52:35
        **/
        explicit ResourcePool( mempool& pool )
            :_mempool(pool), 
            _p_attach_object_list(NULL), 
            _p_attach_array_list(NULL), 
            _p_alloc_object_list(NULL), 
            _p_alloc_array_list(NULL){}

        /**
         * @brief ��������
         *
         * ��Դ������ʱ�������destroy_all()������
         *
         * @see destroy_all()
         * @author chenxm
         * @date 2008/08/12 18:08:06
        **/
        ~ResourcePool(){
            reset();
        }

        allocator_type get_allocator() const {
            return allocator_type(&_mempool);
        }

        /**
         * @brief ����ResourcePool���ڴ�ض���
         *          
         * @param 
         * @return mempool&
         * @retval
         * @see
         * @author liaoshangbin
         * @data 2010/08/04 16:41:40
         **/

        mempool& get_mempool() const {
            return _mempool;
        }

        /**
         * @brief ʹ��Դ���й���Դ�ķ�����
         *
         * ������������destroy_all()�����û���Դ������ʱ���Զ����á�
         * 
         * ע��
         * ���ԵĻ�������ʹ��create_xxx()������clone_xxx()�������Խ��ͳ���Ŀ�����
         * ����Ѿ�����Դattach����Դ�أ��������Ҳ��Ҫ��ͼ�����ͷ���Դ��
         * ��destructor == NULL �������򵥵غ��Ը����󣨼�Ȼû����Ҫ�ɣ�Ҳû��Ҫ����¼�ˣ�
         * ��data == NULL   �����������ر���NULL�ᱻ���ݸ�destructor��
         * ����ڴ��������з������쳣���ڴ治�㣬�ǳ����е����п��ܣ�����Դ�����̱�����
         *
         * @param [in] data   : void* ��Դָ��/�����ַ
         * @param [in] destructor   : object_destructor_t ������Դ�Ļص�����/��������ķ���
         * @return  void 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2008/08/12 18:09:38
        **/
        void attach( void * p_object, object_destructor_t destructor ){
            if ( NULL != destructor ){
                try{
                    attach_object_info_t& info = _push_info(_p_attach_object_list); //throw
                    info.p_object   = p_object;
                    info.destructor = destructor;
                }catch(...){
                    destructor( p_object );
                    throw;
                }
            }
        }


        /**
         *
         * @brief ʹ��Դ���й���Դ����/��������ķ�����
         *
         * ������������destroy_all()�����û���Դ������ʱ���Զ����á�
         *
         * ע��
         * ���ԵĻ�������ʹ��create_xxx()������clone_xxx()�������Խ��ͳ���Ŀ�����
         * ����Ѿ�����Դattach����Դ�أ��������Ҳ��Ҫ��ͼ�����ͷ���Դ��
         * ��destructor == NULL �������򵥵غ��Ը����󣨼�Ȼû����Ҫ�ɣ�Ҳû��Ҫ����¼�ˣ�
         * ��begin == end   �����������ر���NULL�ᱻ���ݸ�destructor��
         * ����ڴ��������з������쳣���ڴ治�㣬�ǳ����е����п��ܣ�����Դ�����̱�����
         *
         * @param [in] begin   : void*
         * @param [in] end   : void*
         * @param [in] destructor   : array_destructor_t
         * @return  void 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2008/08/12 19:49:11
        **/
        void attach_array( void *begin, void *end, array_destructor_t destructor ){ 
            if ( NULL != destructor ){
                try{
                    attach_array_info_t& info = _push_info(_p_attach_array_list); //throw
                    info.begin      = begin;
                    info.end        = end;
                    info.destructor = destructor;
                }catch(...){
                    destructor( begin, end );
                    throw;
                }
            }
        }

        /**
         * @brief ����T���͵�Ĭ�Ϲ��캯��������һ��T���󣬸ö����ѱ���Դ���й�
         *
         * @return T& 
         * @retval ���������������
         * @see 
         * @author chenxm
         * @date 2008/08/12 19:51:31
        **/
        template<typename T>
            T& create(){
                return _create_object<T>( DefaultCtor<T>() );
            }

        template<typename T>
            T* createp(){
                try{
                    return &_create_object<T>( DefaultCtor<T>() );
                }catch(...){
                    return NULL;
                }
            }

        /**
         * @brief ����T���͵ĵ��������캯�����ر�أ���T_arg = Tʱ��Ϊ���ƹ��캯����������һ�����󣬸ö����ѱ���Դ���йܡ�
         *
         * @param [in] __arg   : const T_arg&
         * @return  T& 
         * @retval  ���������������
         * @see 
         * @author chenxm
         * @date 2008/08/12 19:53:25
         **/
        template<typename T, typename T_arg>
            T& create(const T_arg& arg_){
                return _create_object<T>( OneConstArgCtor<T, T_arg>(arg_) );
            }

        /**
         * @brief ����T���͵ĵ��������캯��������һ�����󣬸ö����ѱ���Դ���йܡ�
         *
         * @param [in] __arg   : const T_arg&
         * @return  T& 
         * @retval  ���������������
         * @see 
         * @author chenxm
         * @date 2008/08/12 19:53:25
         **/
        template<typename T, typename T_arg>
            T& createn(T_arg& arg_){
                return _create_object<T>( OneArgCtor<T, T_arg>(arg_) );
            }

            /**
             * @brief �����쳣������ָ��Ľӿ�
             *
             * NOTE�������쳣���ܸ�����C����Ա��ϰ�ߣ���
             *
             * @param [in] arg_   : const T_arg&
             * @return  T* 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2009/09/27 17:55:02
            **/
        template<typename T, typename T_arg>
            T* createp(const T_arg& arg_){
                try{
                    return &_create_object<T>( OneConstArgCtor<T, T_arg>(arg_) );
                }catch(...){
                    return NULL;
                }
            }
        /**
         * @brief ˫��������
         *
         * @param [in] arg1   : const T_arg1&
         * @param [in] arg2   : const T_arg2&
         * @return T& 
         * @retval �¹�������йܶ��������
         * @see 
         * @author chenxm
         * @date 2008/09/19 15:10:01
        **/
        template<typename T, typename T_arg1, typename T_arg2>
            T& create( const T_arg1& arg1, const T_arg2& arg2 ){
                return _create_object<T>( TwoConstArgCtor<T,T_arg1,T_arg2>(arg1,arg2) );
            }

        template<typename T, typename T_arg1, typename T_arg2>
            T* createp( const T_arg1& arg1, const T_arg2& arg2 ){
                try{
                    return &_create_object<T>( TwoConstArgCtor<T,T_arg1,T_arg2>(arg1,arg2) );
                }catch(...){
                    return NULL;
                }
            }

        /**
         * @brief ����������
         *
         * Ŀǰ������Ҫִ�����鹹�캯������ι����븴�ƹ��죩�����������ʱ�����Ż�һ��
         *
         * @param [in] arg1   : const T_arg1&
         * @param [in] arg2   : const T_arg2&
         * @param [in] arg3   : const T_arg3&
         * @return T& 
         * @retval �¹�������йܶ��������
         * @see 
         * @author chenxm
         * @date 2008/09/19 15:10:01
        **/
        template<typename T, typename T_arg1, typename T_arg2, typename T_arg3 >
            T& create( const T_arg1& arg1, const T_arg2& arg2, const T_arg3 arg3 ){
                return _create_object<T>( 
                        ThreeConstArgCtor<T,T_arg1,T_arg2,T_arg3>(arg1, arg2, arg3) 
                        );
            }

        template<typename T, typename T_arg1, typename T_arg2, typename T_arg3 >
            T* createp( const T_arg1& arg1, const T_arg2& arg2, const T_arg3 arg3 ){
                try{
                    return &_create_object<T>( 
                            ThreeConstArgCtor<T,T_arg1,T_arg2,T_arg3>(arg1, arg2, arg3) 
                            );
                }catch(...){
                    return NULL;
                }
            }
        /**
         * @brief �Ĳ�������
         *
         * Ŀǰ������Ҫִ�����鹹�캯������ι����븴�ƹ��죩�����������ʱ�����Ż�һ��
         *
         * @param [in] arg1   : const T_arg1&
         * @param [in] arg2   : const T_arg2&
         * @param [in] arg3   : const T_arg3&
         * @param [in] arg4   : const T_arg4&
         * @return T& 
         * @retval �¹�������йܶ��������
         * @see 
         * @author chenxm
         * @date 2008/09/19 15:10:01
        **/
        template<typename T, typename T_arg1, typename T_arg2, typename T_arg3, typename T_arg4 >
            T& create( 
                    const T_arg1& arg1, 
                    const T_arg2& arg2, 
                    const T_arg3 arg3, 
                    const T_arg4 arg4 
                    ){
                return _create_object<T>( 
                        FourConstArgCtor<T, T_arg1, T_arg2, T_arg3, T_arg4>(
                            arg1, 
                            arg2, 
                            arg3, 
                            arg4
                            ) );
            }

        template<typename T, typename T_arg1, typename T_arg2, typename T_arg3, typename T_arg4 >
            T* createp( 
                    const T_arg1& arg1, 
                    const T_arg2& arg2, 
                    const T_arg3 arg3, 
                    const T_arg4 arg4 
                    ){
                try{
                    return &_create_object<T>( 
                            FourConstArgCtor<T, T_arg1, T_arg2, T_arg3, T_arg4>(
                                arg1, 
                                arg2, 
                                arg3, 
                                arg4
                                ) );
                }catch(...){
                    return NULL;
                }
            }

        /**
         * @brief ����T���͵�Ĭ�Ϲ��캯��������һ������Ϊn��T�������飬�ö��������ѱ���Դ���й�
         *
         * @param [in] n   : int
         * @return  T* 
         * @retval  �������Ķ����������ַ
         * @see 
         * @author chenxm
         * @date 2008/08/12 19:55:45
        **/
        template<typename T>
            T* create_array( size_t n ){
                T *begin     = NULL;
                //���쳣��ʾinfo�����䲻�ˣ�ûʲô�ɻع��ġ�
                alloc_array_info_t& info = _push_info(_p_alloc_array_list); //throw
                try{
                    begin = static_cast<T*>(_mempool.malloc( n * sizeof(T) )); //throw
                    if ( !begin ){
                        throw bsl::BadAllocException()
                            <<BSL_EARG<<"_mempool["<<&_mempool<<"] size["<<n * sizeof(T)<<"]";
                    }
                    try{
                        construct_array<T>(begin, begin + n);   //throw (by user code)
                        info.begin      = begin;
                        info.end        = begin + n;
                        info.destructor = _s_destroy_and_deallocate_array<T>;
                        return begin;
                    }catch(...){
                        //�ع�Ϊ�������������ڴ�
                        _mempool.free( begin, n * sizeof(T) );
                        throw;
                    }
                }catch(...){
                    //�ع�info�ķ���
                    _pop_info(_p_alloc_array_list);
                    throw;
                }
            }

        /**
         * @brief ����T���͵ĵ��������캯�����ر�أ���T_arg = Tʱ��Ϊ���ƹ��캯����������һ������Ϊn�Ķ������飬�ö��������ѱ���Դ���йܡ�
         *
         * @param [in] n   : int
         * @param [in] __arg   : const T_arg&
         * @return  T* 
         * @retval  �������Ķ����������ַ
         * @see 
         * @author chenxm
         * @date 2008/08/12 19:56:58
         **/
        template<typename T, typename T_arg>
            T* create_array( size_t n, const T_arg& __arg ){
                T *begin     = NULL;
                //���쳣��ʾinfo�����䲻�ˣ�ûʲô�ɻع��ġ�
                alloc_array_info_t& info = _push_info(_p_alloc_array_list); //throw
                try{
                    begin = static_cast<T*>(_mempool.malloc( n * sizeof(T) ));   //throw
                    if ( !begin ){
                        throw bsl::BadAllocException()
                            <<BSL_EARG<<"_mempool["<<&_mempool<<"] size["<<n * sizeof(T)<<"]";
                    }
                    try{
                        construct_array<T, T_arg>(begin, begin + n, &__arg);   //throw (by user code)
                        info.begin      = begin;
                        info.end        = begin + n;
                        info.destructor = _s_destroy_and_deallocate_array<T>;
                        return begin;
                    }catch(...){
                        //�ع�Ϊ�������������ڴ�
                        _mempool.free( begin, n * sizeof(T) );
                        throw;
                    }
                }catch(...){
                    //�ع�info�ķ���
                    _pop_info(_p_alloc_array_list);
                    throw;
                }
            }

        /**
         * @brief ����һ����СΪbytes�ֽڵ�δ��ʼ���ռ䡣�ÿռ��ѱ���Դ���йܡ�
         *
         * @param [in] bytes   : int
         * @return  void* 
         * @retval  �������Ŀռ����ַ
         * @see 
         * @author chenxm
         * @date 2008/08/12 19:58:56
        **/
        void * create_raw( size_t bytes ){ 
            alloc_array_info_t& info = _push_info(_p_alloc_array_list); //throw
            try{
                char * res = static_cast<char*>(_mempool.malloc( bytes )); //throw
                if ( !res ){
                    throw bsl::BadAllocException()
                        <<BSL_EARG<<"_mempool["<<&_mempool<<"] size["<<bytes<<"]";
                }
                info.begin      = res;
                info.end        = res+bytes;
                info.destructor = _s_deallocate;
                return res;
            }catch(...){
                //���ڴ����ʧ�ܣ��ع�info�ķ���
                _pop_info(_p_alloc_array_list);
                throw;
            }
        }

        /**
         * @brief ͨ�����ƹ��캯��������һ��T����ĸ������ø����ѱ���Դ���йܡ�
         *
         * @param [in] src   : const T& �����ƵĶ���
         * @return  T& 
         * @retval  ��������������� 
         * @see 
         * @author chenxm
         * @date 2008/08/12 20:00:34
        **/
        template<typename T>
            T& clone(const T& src){
                return create<T,T>(src);
            }

        /**
         * @brief ͨ�����ƹ��캯��������һ��T��������ĸ������ø����ѱ���Դ���йܡ�
         *
         * ע�����˭�Ҹ���һ��void���黹����Ϊʲô���벻���Ļ�����һ���������PP
         *
         * @param [in] src   : const T* �����ƵĶ����������ַ
         * @param [in] n   : int �����ƵĶ�������ĳ���
         * @return T* 
         * @retval �������Ķ����������ַ  
         * @see 
         * @author chenxm
         * @date 2008/08/12 20:02:11
        **/
        template<typename T>
            T* clone_array(const T* src, size_t n){
                T *begin     = NULL;
                //���쳣��ʾinfo�����䲻�ˣ�ûʲô�ɻع��ġ�
                alloc_array_info_t& info = _push_info(_p_alloc_array_list); //throw
                try{
                    begin = static_cast<T*>(_mempool.malloc(n*sizeof(T)));  //throw
                    if ( !begin ){
                        throw bsl::BadAllocException()
                            <<BSL_EARG<<"_mempool["<<&_mempool<<"] size["<<n * sizeof(T)<<"]";
                    }
                    try{
                        std::uninitialized_copy( src, src + n, begin );   //throw (by user code )
                        info.begin      = begin;
                        info.end        = begin + n;
                        info.destructor = _s_destroy_and_deallocate_array<T>;
                        return begin;
                    }catch(...){
                        //�ع�Ϊ�������������ڴ�
                        _mempool.free(begin, n*sizeof(T));
                        throw;
                    }
                }catch(...){
                    //�ع�info�ķ���
                    _pop_info(_p_alloc_array_list);
                    throw;
                }
            }

        /**
         * @brief ʹ��ֱ���ڴ渴�Ʒ�ʽ����һ����СΪbytes�ֽڵĿռ�ĸ������ø����ѱ���Դ���йܡ�
         *
         * @param [in] p   : void*
         * @param [in] bytes   : int
         * @return  void* 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2008/08/13 11:15:20
        **/
        void * clone_raw( const void * src, size_t bytes ){
            void * res = create_raw( bytes );
            return xmemcpy( res, src, bytes );
        }

        /**
         * @brief ����һ��C����ַ����ĸ������ø����ѱ���Դ���йܡ�
         *
         * @param [in] src_str   : const char* �����Ƶ�C����ַ�����ַ
         * @return  char* 
         * @retval  ��������C����ַ�������ַ
         * @see 
         * @author chenxm
         * @date 2008/08/09 16:06:58
         **/
        char * clone_cstring(const char * src_str ){
            size_t size = strlen(src_str) + 1;
            void * res  = clone_raw( src_str, size ); //throw
            return static_cast<char *>(res);
        }

        /**
         * @brief ����һ��C����ַ��������Ӵ��ĸ������ø����ѱ���Դ���йܡ�
         *
         * ע�⣺��sub_str_len > strlen(src_str)����������Ϊδ���塣
         *
         * @param [in] src_str   : const char*
         * @param [in] sub_str_len   : size_t
         * @return  char* 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/04/14 21:26:43
        **/
        char * clone_cstring(const char * src_str, size_t sub_str_len ){
            char * res = static_cast<char*>(clone_raw( src_str, sub_str_len + 1 )); //throw
            res[sub_str_len] = 0;
            return res;
        }

        /**
         * @brief ͨ������printf()���﷨�������йܵ��ַ�����
         *
         * ��Ŀǰ��ʵ���crcprintf(...)�൱��crcprintf_hint( 63, ... )
         *
         * @param [in] format   : const char*
         * @return  const char* 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 15:30:58
        **/
        const char * crcprintf( const char * format, ... )__attribute__ ((format (printf, 2, 3) ));

        /**
         * @brief ͨ������printf()���﷨�������йܵ��ַ�����Ԥ���䲻����hint_capacity+1�ֽڵĿռ������Ч�ʡ�
         *
         * @param [in] hint_capacity   : size_t
         * @param [in] format   : const char*
         * @return  const char* 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 15:31:50
        **/
        const char * crcprintf_hint( 
                size_t hint_capacity, 
                const char * format, 
                ... 
                )__attribute__ ((format (printf, 3, 4) ));

        /**
         * @brief ͨ������vprintf()�﷨�������йܵ��ַ���
         *          
         * @param [in] format   : const char*
         * @param [in] ap       : va_list
         * @return const char*
         * @retval
         * @see
         * @author liaoshangbin
         * @data 2010/08/04 10:10:34
         **/
        const char * vcrcprintf( const char * format, va_list ap );

        /**
         * @brief ͨ������vprintf()���﷨�������йܵ��ַ�����Ԥ���䲻����hint_capacity+1�ֽڵĿռ������Ч�ʡ�
         *          
         * @param [in] format   : const char*
         * @param [in] ap       : va_list
         * @return const char*
         * @retval
         * @see
         * @author liaoshangbin
         * @data 2010/08/04 10:43:02
         **/
        const char * vcrcprintf_hint( size_t hint_capacity, const char * format, va_list ap );



        /**
         * @brief �����������йܵ���Դ
         *
         * @return  void 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 15:34:03
        **/
        void reset();

    private:
        static const size_t _S_ITEMS_PER_NODE = 64;

        /**
        * @brief ��������Դ/���󣬲�ͨ��alloc_�����ڴ�Ļص�����ָ��
        *  
        */
        typedef void( *alloc_object_destructor_t )(void * p_object, mempool& pool );

        /**
        * @brief ������Դ���顢�������飬��ͨ��alloc_�����ڴ�Ļص�����ָ��
        *  
        *  �ûص��������������ַ��βַ������ַ�������С����Ϊ������û�з���ֵ��
        */
        typedef void( *alloc_array_destructor_t )(void * begin, void * end, mempool& pool );

        /**
        * @brief ��¼ͨ��attach()�йܵĵ�����Դ/������Ϣ�Ľṹ�壨����dealloc��
        *  
        *  
        */
        struct attach_object_info_t{
            void *p_object;
            object_destructor_t destructor;
        };

        /**
        * @brief ��¼ͨ��attach()�йܵ���Դ����/����������Ϣ�Ľṹ�壨����dealloc��
        *  
        *  
        */
        struct attach_array_info_t{
            void *begin;
            void *end;
            array_destructor_t destructor;
        };

        /**
        * @brief ��¼��Դ���½��ĵ�����Դ/������Ϣ�Ľṹ�壨��Ҫdealloc��
        *  
        *  
        */
        struct alloc_object_info_t{
            void *p_object;
            alloc_object_destructor_t destructor;
        };

        /**
        * @brief ��¼��Դ���½�����Դ����/����������Ϣ�Ľṹ�壨��Ҫdealloc��
        *  
        *  
        */
        struct alloc_array_info_t{
            void *begin;
            void *end;
            alloc_array_destructor_t destructor;
        };

        template<typename info_t>
        struct block_list_node_t{
            block_list_node_t<info_t>  *p_next;
            size_t  current;
            info_t  data[_S_ITEMS_PER_NODE];
        };

        /**
         * @brief Ĭ�Ϲ��캯��
         *
         */
        //���캯���º������൱��lambda���ʽ
        template<typename T>
            class DefaultCtor{
            public:
                void operator ()( T* ptr ) const {
                    new(ptr) T();
                }
            };

        /**
         * @brief ���������캯��
         *
         */
        template<typename T, typename ArgT>
            class OneArgCtor{
            public:
                OneArgCtor( ArgT& arg )
                    :_arg(arg){}
                void operator ()( T* ptr ) const {
                    new(ptr) T(_arg);
                }
            private:
                ArgT& _arg;
            };

        /**
         * @brief  ���������캯��
         *
         */
        template<typename T, typename ArgT>
            class OneConstArgCtor{
            public:
                OneConstArgCtor( const ArgT& arg )
                    :_arg(arg) {}
                void operator ()( T* ptr ) const {
                    new(ptr) T(_arg);
                }
            private:
                const ArgT& _arg;
            };

        /**
         * @brief ˫��������
         *
         */
        template<typename T, typename Arg1T, typename Arg2T>
            class TwoConstArgCtor{
            public:
                TwoConstArgCtor( const Arg1T& arg1, const Arg2T& arg2 )
                    :_arg1(arg1), _arg2(arg2) {}
                void operator ()( T* ptr ) const {
                    new(ptr) T(_arg1, _arg2);
                }
            private:
                const Arg1T& _arg1;
                const Arg2T& _arg2;
            };
        /**
         * @brief ����������
         *
         */
        template<typename T, typename Arg1T, typename Arg2T, typename Arg3T>
            class ThreeConstArgCtor{
            public:
                ThreeConstArgCtor( const Arg1T& arg1, const Arg2T& arg2, const Arg3T& arg3 )
                    :_arg1(arg1), _arg2(arg2), _arg3(arg3) {}
                void operator ()( T* ptr ) const {
                    new(ptr) T(_arg1, _arg2, _arg3);
                }
            private:
                const Arg1T& _arg1;
                const Arg2T& _arg2;
                const Arg3T& _arg3;
            };
        /**
         * @brief �Ĳ�������
         *
         */
        template<typename T, typename Arg1T, typename Arg2T, typename Arg3T, typename Arg4T>
            class FourConstArgCtor{
            public:
                FourConstArgCtor( 
                        const Arg1T& arg1, 
                        const Arg2T& arg2, 
                        const Arg3T& arg3, 
                        const Arg4T& arg4 
                        )
                    :_arg1(arg1), _arg2(arg2), _arg3(arg3), _arg4(arg4) {}
                void operator ()( T* ptr ) const {
                    new(ptr) T(_arg1, _arg2, _arg3, _arg4);
                }
            private:
                const Arg1T& _arg1;
                const Arg2T& _arg2;
                const Arg3T& _arg3;
                const Arg4T& _arg4;
            };

    public:
        template<typename T, typename CtorT>
            T& _create_object( const CtorT& ctor ){
                T *p_object = NULL;
                //���쳣��ʾinfo�����䲻�ˣ�ûʲô�ɻع��ġ�
                alloc_object_info_t& info = _push_info(_p_alloc_object_list); //throw
                //���쳣��ʾ����ռ���䲻�ˣ��ع�info�ķ��䡣
                try{
                    p_object = static_cast<T*>(_mempool.malloc(sizeof(T)));
                    if ( !p_object ){
                        throw bsl::BadAllocException()
                            <<BSL_EARG<<"_mempool["<<&_mempool<<"] size["<<sizeof(T)<<"]";
                    }
                    try{
                        ctor(p_object); //throw (by user code)
                        info.p_object   = p_object;
                        info.destructor = _s_destroy_and_deallocate<T>;
                        return *p_object;
                    }catch(...){
                        //�ع�����ռ�ķ���
                        _mempool.free( p_object, sizeof(T) );
                        throw;
                    }
                }catch(...){
                    //�ع�����ռ�ķ���
                    _pop_info(_p_alloc_object_list);
                    throw;
                }
            }

    private:
        template<typename T, typename ArrayCtorT>
            T* _create_array( size_t n, const ArrayCtorT& ctor ){
                T *begin     = NULL;
                //���쳣��ʾinfo�����䲻�ˣ�ûʲô�ɻع��ġ�
                alloc_array_info_t& info = _push_info(_p_alloc_array_list); //throw
                try{
                    begin = static_cast<T*>(_mempool.malloc( n * sizeof(T) )); //throw
                    if ( !begin ){
                        throw bsl::BadAllocException()
                            <<BSL_EARG<<"_mempool["<<&_mempool<<"] size["<<n * sizeof(T)<<"]";
                    }
                    try{
                        //construct_array<T>(begin, begin + n);   
                        ctor(begin, begin+n);   //throw (by user code)
                        info.begin      = begin;
                        info.end        = begin + n;
                        info.destructor = _s_destroy_and_deallocate_array<T>;
                        return begin;
                    }catch(...){
                        //�ع�Ϊ�������������ڴ�
                        _mempool.free( begin, n * sizeof(T) );
                        throw;
                    }
                }catch(...){
                    //�ع�info�ķ���
                    _pop_info(_p_alloc_array_list);
                    throw;
                }
            }
        /**
         * @brief ׷��һ������
         *
         * ����ɹ��󣬷��ؿ���д������
         * ��������е�һ���鱻д�����ú���������һ���¿顣����֮��_p_list_head��ָ���¿顣
         * �������ڴ治�㣬���׳�mempoolָ�����쳣��
         * �ú���Ҫ��info_t����Ҫ���죨C���struct��
         *
         * @param [in/out] _p_list_head   : block_list_node_t<info_t>* &
         * @param [in] _alloc   : allocator_type&
         * @return  void 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/04/06 23:03:24
        **/
        template<typename info_t> 
        info_t& _push_info( block_list_node_t<info_t> *& p_list_head ){
            if ( NULL!=p_list_head && p_list_head->current < _S_ITEMS_PER_NODE - 1 ){
                ++ p_list_head->current; //�������δ����ֻ���Ӽ�����
                return p_list_head->data[p_list_head->current];   
            }else{
                typedef block_list_node_t<info_t> node_t;
                node_t* p_tmp = static_cast<node_t *>(_mempool.malloc(sizeof(node_t))); //throw
                if ( !p_tmp ){
                    throw bsl::BadAllocException()
                        <<BSL_EARG<<"_mempool["<<&_mempool<<"] size["<<sizeof(node_t)<<"]";
                }
                p_tmp->p_next = p_list_head;
                p_tmp->current= 0;
                p_list_head = p_tmp;
                return p_list_head->data[0];
            }
        }

        /**
         * @brief �ع�����׷�ӵ���
         *
         * @param [in] _p_list_head   : block_list_node_t<info_t>* &
         * @param [in] _alloc   : allocator_type&
         * @return  info_t& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/04/09 18:33:55
        **/
        template<typename info_t> 
        void _pop_info( block_list_node_t<info_t> *& p_list_head ){
            if ( p_list_head->current > 0 ){
                -- p_list_head->current; //���������Ϊ�գ�ֻ���ټ�����
            }else{
                block_list_node_t<info_t>* p_tmp = p_list_head;
                p_list_head = p_list_head->p_next;
                _mempool.free(p_tmp, sizeof(block_list_node_t<info_t>));
            }
        }

        /**
         * @brief �ͷ�����������
         * ����ִ�к�p_list_head�ᱻ��ΪNULL��
         * �ú���������������֤��
         *
         * @return  void 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/04/06 23:08:12
        **/
        template<typename info_t> 
        void _clear_info_list(block_list_node_t<info_t> *& p_list_head ){
            while( NULL != p_list_head ){
                block_list_node_t<info_t> *p_tmp = p_list_head;
                p_list_head = p_list_head->p_next;
                _mempool.free(p_tmp, sizeof(block_list_node_t<info_t>));
            }
        }
        
        /**
         * @brief ��Դ�ض��󲻿ɸ���
         *
         * �û���Ӧ���κη�ʽ������Դ�ض���ĸ�����
         *
         * @param [in] other   : const ResourcePool&
         * @see 
         * @author chenxm
         * @date 2008/08/06 20:32:53
         **/
        ResourcePool( const ResourcePool& other); 

        /**
         * @brief ResourcePool���ɸ�ֵ
         *
         * �û���Ӧ�����κη�ʽ��ResourcePool��ֵ
         *
         * @return  ResourcePool&  operator 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/04/07 00:06:26
        **/
        ResourcePool& operator = ( const ResourcePool& );

        /**
         * @brief ����rcprintf��ʵ�ֺ���
         *
         * @param [in] info   : alloc_array_info_t&
         * @param [in] hint_capacity   : size_t
         * @param [in] format   : const char*
         * @param [in] ap   : va_list
         * @return  const char* 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/04/12 00:47:34
        **/
        const char * _vprintf( 
                alloc_array_info_t& info, 
                size_t hint_capacity, 
                const char *format, 
                va_list ap 
                );
        /**
         * @brief ͨ���ռ�������ͷ��ڴ��C����������
         * @param [in] begin   : void* ���ͷŵ��ڴ�����ַ
         * @param [in] end     : void* ���ͷŵ��ڴ���βַ
         * @param [in] alloc_   : allocator_type& �ռ������
         * @return void 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2008/08/12 20:43:49
         **/
        static void _s_deallocate( void * begin, void * end, mempool& pool ){
            pool.free( begin, static_cast<char *>(end) - static_cast<char *>(begin) );
        }

        /**
         * @brief destroy(void*)��deallocate(void*, allocator_type&)����� 
         *
         * @param [in] p_object   : void*
         * @return  void 
         * @retval   
         * @see destroy(void*)
         * @see deallocate(void*)
         * @author chenxm
         * @date 2008/08/12 20:46:56
         **/
        template<typename T>
            static void _s_destroy_and_deallocate( void * p_object, mempool& pool ){
                static_cast<T*>(p_object)->~T();
                pool.free(p_object, sizeof(T));
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
        template<typename T>
            static void _s_destroy_and_deallocate_array( void * begin, void * end, mempool& pool ){
                __BSL_DESTROY(static_cast<T*>(begin), static_cast<T*>(end)); // this method will be specially optimized to the type which has trivial destructor;
                pool.free(begin, static_cast<char*>(end) - static_cast<char*>(begin) );
            }

        /**
        * @brief ��װ��::malloc()��::free()�ļ�mempool
        *  
        * Ĭ�Ϲ����ResourcePool ʹ�ø�Pool
        */
        syspool     _syspool;

        /**
        * @brief ��ǰʹ�õĿռ�������������
        *  
        */
        mempool&    _mempool;
        /**
        * @brief ά��ͨ��attach()�йܵĵ���������Ϣ�Ŀ�������������ڴ棩 
        *  
        *  
        */
        block_list_node_t<attach_object_info_t>    *_p_attach_object_list;
        /**
        * @brief ά��ͨ��attach()�йܵĶ���������Ϣ�Ŀ�������������ڴ棩
        *  
        *  
        */
        block_list_node_t<attach_array_info_t>     *_p_attach_array_list;
        /**
        * @brief ά������������Ϣ�Ŀ����� 
        *  
        *  
        */
        block_list_node_t<alloc_object_info_t>     *_p_alloc_object_list;
        /**
        * @brief ά������������Ϣ�Ŀ�����
        *  
        *  
        */
        block_list_node_t<alloc_array_info_t>      *_p_alloc_array_list;


    };

} // namespace bsl
#endif  //__RESOURCEPOOL_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
