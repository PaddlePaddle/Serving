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
    * @brief 资源池
    *  
    * 用户可以通过资源池托管包括内存、句柄在内的各种资源，还可以建造、克隆各种对象、数组。
    *
    */
    class ResourcePool{
    public:
        
        typedef bsl::pool_allocator<char>   allocator_type;

        /**
        * @brief 清理单个资源/对象的回调函数指针
        *  
        *  该回调函数以资源指针（如FILE*）/对象地址作为参数，没有返回值
        * ng_wrappers.h有针对常见资源的实现，建议尽量使用ng_wrappers.h的实现
        */
        typedef void( *object_destructor_t )(void * p_object);

        /**
        * @brief 清理资源数组、对象数组的回调函数指针
        *  
        *  该回调函数以数组的首址、尾址（＝首址＋数组大小）作为参数，没有返回值。
        */
        typedef void( *array_destructor_t )(void * begin, void * end);

        /**
         * @brief 默认构造函数
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
         * @brief 使用allocator的构造函数。
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
         * @brief 析构函数
         *
         * 资源池析构时，会调用destroy_all()方法。
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
         * @brief 返回ResourcePool的内存池对象
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
         * @brief 使资源池托管资源的方法。
         *
         * 清理函数将会在destroy_all()被调用或资源池析构时被自动调用。
         * 
         * 注：
         * 可以的话，尽量使用create_xxx()方法或clone_xxx()方法，以降低出错的可能性
         * 如果已经把资源attach到资源池，无论如何也不要试图自行释放资源！
         * 若destructor == NULL 本函数简单地忽略该请求（既然没事情要干，也没必要做记录了）
         * 若data == NULL   本函数不做特别处理（NULL会被传递给destructor）
         * 如果在创建过程中发生了异常（内存不足，非常罕有但仍有可能），资源会立刻被清理
         *
         * @param [in] data   : void* 资源指针/对象地址
         * @param [in] destructor   : object_destructor_t 清理资源的回调函数/析构对象的方法
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
         * @brief 使资源池托管资源数组/对象数组的方法。
         *
         * 清理函数将会在destroy_all()被调用或资源池析构时被自动调用。
         *
         * 注：
         * 可以的话，尽量使用create_xxx()方法或clone_xxx()方法，以降低出错的可能性
         * 如果已经把资源attach到资源池，无论如何也不要试图自行释放资源！
         * 若destructor == NULL 本函数简单地忽略该请求（既然没事情要干，也没必要做记录了）
         * 若begin == end   本函数不做特别处理（NULL会被传递给destructor）
         * 如果在创建过程中发生了异常（内存不足，非常罕有但仍有可能），资源会立刻被清理
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
         * @brief 调用T类型的默认构造函数，创建一个T对象，该对象已被资源池托管
         *
         * @return T& 
         * @retval 被创建对象的引用
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
         * @brief 调用T类型的单参数构造函数（特别地，当T_arg = T时，为复制构造函数），创建一个对象，该对象已被资源池托管。
         *
         * @param [in] __arg   : const T_arg&
         * @return  T& 
         * @retval  被创建对象的引用
         * @see 
         * @author chenxm
         * @date 2008/08/12 19:53:25
         **/
        template<typename T, typename T_arg>
            T& create(const T_arg& arg_){
                return _create_object<T>( OneConstArgCtor<T, T_arg>(arg_) );
            }

        /**
         * @brief 调用T类型的单参数构造函数，创建一个对象，该对象已被资源池托管。
         *
         * @param [in] __arg   : const T_arg&
         * @return  T& 
         * @retval  被创建对象的引用
         * @see 
         * @author chenxm
         * @date 2008/08/12 19:53:25
         **/
        template<typename T, typename T_arg>
            T& createn(T_arg& arg_){
                return _create_object<T>( OneArgCtor<T, T_arg>(arg_) );
            }

            /**
             * @brief 不抛异常，返回指针的接口
             *
             * NOTE：不抛异常可能更符合C程序员的习惯，但
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
         * @brief 双参数构造
         *
         * @param [in] arg1   : const T_arg1&
         * @param [in] arg2   : const T_arg2&
         * @return T& 
         * @retval 新构造的受托管对象的引用
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
         * @brief 三参数构造
         *
         * 目前构造需要执行两遍构造函数（多参构造与复制构造），将来需求大时可以优化一下
         *
         * @param [in] arg1   : const T_arg1&
         * @param [in] arg2   : const T_arg2&
         * @param [in] arg3   : const T_arg3&
         * @return T& 
         * @retval 新构造的受托管对象的引用
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
         * @brief 四参数构造
         *
         * 目前构造需要执行两遍构造函数（多参构造与复制构造），将来需求大时可以优化一下
         *
         * @param [in] arg1   : const T_arg1&
         * @param [in] arg2   : const T_arg2&
         * @param [in] arg3   : const T_arg3&
         * @param [in] arg4   : const T_arg4&
         * @return T& 
         * @retval 新构造的受托管对象的引用
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
         * @brief 调用T类型的默认构造函数，创建一个长度为n的T对象数组，该对象数组已被资源池托管
         *
         * @param [in] n   : int
         * @return  T* 
         * @retval  被创建的对象数组的首址
         * @see 
         * @author chenxm
         * @date 2008/08/12 19:55:45
        **/
        template<typename T>
            T* create_array( size_t n ){
                T *begin     = NULL;
                //抛异常表示info都分配不了，没什么可回滚的。
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
                        //回滚为对象数组分配的内存
                        _mempool.free( begin, n * sizeof(T) );
                        throw;
                    }
                }catch(...){
                    //回滚info的分配
                    _pop_info(_p_alloc_array_list);
                    throw;
                }
            }

        /**
         * @brief 调用T类型的单参数构造函数（特别地，当T_arg = T时，为复制构造函数），创建一个长度为n的对象数组，该对象数组已被资源池托管。
         *
         * @param [in] n   : int
         * @param [in] __arg   : const T_arg&
         * @return  T* 
         * @retval  被创建的对象数组的首址
         * @see 
         * @author chenxm
         * @date 2008/08/12 19:56:58
         **/
        template<typename T, typename T_arg>
            T* create_array( size_t n, const T_arg& __arg ){
                T *begin     = NULL;
                //抛异常表示info都分配不了，没什么可回滚的。
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
                        //回滚为对象数组分配的内存
                        _mempool.free( begin, n * sizeof(T) );
                        throw;
                    }
                }catch(...){
                    //回滚info的分配
                    _pop_info(_p_alloc_array_list);
                    throw;
                }
            }

        /**
         * @brief 创建一个大小为bytes字节的未初始化空间。该空间已被资源池托管。
         *
         * @param [in] bytes   : int
         * @return  void* 
         * @retval  被创建的空间的首址
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
                //若内存分配失败，回滚info的分配
                _pop_info(_p_alloc_array_list);
                throw;
            }
        }

        /**
         * @brief 通过复制构造函数，创建一个T对象的副本，该副本已被资源池托管。
         *
         * @param [in] src   : const T& 被复制的对象
         * @return  T& 
         * @retval  被创建对象的引用 
         * @see 
         * @author chenxm
         * @date 2008/08/12 20:00:34
        **/
        template<typename T>
            T& clone(const T& src){
                return create<T,T>(src);
            }

        /**
         * @brief 通过复制构造函数，创建一个T对象数组的副本，该副本已被资源池托管。
         *
         * 注：如果谁敢复制一个void数组还问我为什么编译不过的话，我一定会打他的PP
         *
         * @param [in] src   : const T* 被复制的对象数组的首址
         * @param [in] n   : int 被复制的对象数组的长度
         * @return T* 
         * @retval 被创建的对象数组的首址  
         * @see 
         * @author chenxm
         * @date 2008/08/12 20:02:11
        **/
        template<typename T>
            T* clone_array(const T* src, size_t n){
                T *begin     = NULL;
                //抛异常表示info都分配不了，没什么可回滚的。
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
                        //回滚为对象数组分配的内存
                        _mempool.free(begin, n*sizeof(T));
                        throw;
                    }
                }catch(...){
                    //回滚info的分配
                    _pop_info(_p_alloc_array_list);
                    throw;
                }
            }

        /**
         * @brief 使用直接内存复制方式创建一个大小为bytes字节的空间的副本。该副本已被资源池托管。
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
         * @brief 创建一个C风格字符串的副本，该副本已被资源池托管。
         *
         * @param [in] src_str   : const char* 被复制的C风格字符串首址
         * @return  char* 
         * @retval  被创建的C风格字符串的首址
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
         * @brief 创建一个C风格字符串或其子串的副本，该副本已被资源池托管。
         *
         * 注意：若sub_str_len > strlen(src_str)，本方法行为未定义。
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
         * @brief 通过类似printf()的语法生成受托管的字符串。
         *
         * 在目前的实现里，crcprintf(...)相当于crcprintf_hint( 63, ... )
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
         * @brief 通过类似printf()的语法生成受托管的字符串。预分配不少于hint_capacity+1字节的空间以提高效率。
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
         * @brief 通过类似vprintf()语法生成受托管的字符串
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
         * @brief 通过类似vprintf()的语法生成受托管的字符串。预分配不少于hint_capacity+1字节的空间以提高效率。
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
         * @brief 清理所有受托管的资源
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
        * @brief 清理单个资源/对象，并通过alloc_回收内存的回调函数指针
        *  
        */
        typedef void( *alloc_object_destructor_t )(void * p_object, mempool& pool );

        /**
        * @brief 清理资源数组、对象数组，并通过alloc_回收内存的回调函数指针
        *  
        *  该回调函数以数组的首址、尾址（＝首址＋数组大小）作为参数，没有返回值。
        */
        typedef void( *alloc_array_destructor_t )(void * begin, void * end, mempool& pool );

        /**
        * @brief 记录通过attach()托管的单个资源/对象信息的结构体（不用dealloc）
        *  
        *  
        */
        struct attach_object_info_t{
            void *p_object;
            object_destructor_t destructor;
        };

        /**
        * @brief 记录通过attach()托管的资源数组/对象数组信息的结构体（不用dealloc）
        *  
        *  
        */
        struct attach_array_info_t{
            void *begin;
            void *end;
            array_destructor_t destructor;
        };

        /**
        * @brief 记录资源池新建的单个资源/对象信息的结构体（需要dealloc）
        *  
        *  
        */
        struct alloc_object_info_t{
            void *p_object;
            alloc_object_destructor_t destructor;
        };

        /**
        * @brief 记录资源池新建的资源数组/对象数组信息的结构体（需要dealloc）
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
         * @brief 默认构造函数
         *
         */
        //构造函数仿函数，相当于lambda表达式
        template<typename T>
            class DefaultCtor{
            public:
                void operator ()( T* ptr ) const {
                    new(ptr) T();
                }
            };

        /**
         * @brief 单参数构造函数
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
         * @brief  单参数构造函数
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
         * @brief 双参数构造
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
         * @brief 三参数构造
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
         * @brief 四参数构造
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
                //抛异常表示info都分配不了，没什么可回滚的。
                alloc_object_info_t& info = _push_info(_p_alloc_object_list); //throw
                //抛异常表示对象空间分配不了，回滚info的分配。
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
                        //回滚对象空间的分配
                        _mempool.free( p_object, sizeof(T) );
                        throw;
                    }
                }catch(...){
                    //回滚对象空间的分配
                    _pop_info(_p_alloc_object_list);
                    throw;
                }
            }

    private:
        template<typename T, typename ArrayCtorT>
            T* _create_array( size_t n, const ArrayCtorT& ctor ){
                T *begin     = NULL;
                //抛异常表示info都分配不了，没什么可回滚的。
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
                        //回滚为对象数组分配的内存
                        _mempool.free( begin, n * sizeof(T) );
                        throw;
                    }
                }catch(...){
                    //回滚info的分配
                    _pop_info(_p_alloc_array_list);
                    throw;
                }
            }
        /**
         * @brief 追加一个新项
         *
         * 插入成功后，返回可以写入的新项。
         * 如果块链中的一个块被写满，该函数会申请一个新块。调用之后_p_list_head会指向新块。
         * 若发生内存不足，会抛出mempool指定的异常。
         * 该函数要求info_t不需要构造（C风格struct）
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
                ++ p_list_head->current; //如果块链未满，只增加计数器
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
         * @brief 回滚最新追加的项
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
                -- p_list_head->current; //如果块链不为空，只减少计数器
            }else{
                block_list_node_t<info_t>* p_tmp = p_list_head;
                p_list_head = p_list_head->p_next;
                _mempool.free(p_tmp, sizeof(block_list_node_t<info_t>));
            }
        }

        /**
         * @brief 释放整个块链表。
         * 函数执行后，p_list_head会被置为NULL。
         * 该函数具有无抛掷保证。
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
         * @brief 资源池对象不可复制
         *
         * 用户不应以任何方式创建资源池对象的副本。
         *
         * @param [in] other   : const ResourcePool&
         * @see 
         * @author chenxm
         * @date 2008/08/06 20:32:53
         **/
        ResourcePool( const ResourcePool& other); 

        /**
         * @brief ResourcePool不可赋值
         *
         * 用户不应该以任何方式对ResourcePool赋值
         *
         * @return  ResourcePool&  operator 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/04/07 00:06:26
        **/
        ResourcePool& operator = ( const ResourcePool& );

        /**
         * @brief 各种rcprintf的实现函数
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
         * @brief 通过空间分配器释放内存的C风格包裹函数
         * @param [in] begin   : void* 被释放的内存块的首址
         * @param [in] end     : void* 被释放的内存块的尾址
         * @param [in] alloc_   : allocator_type& 空间分配器
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
         * @brief destroy(void*)和deallocate(void*, allocator_type&)的组合 
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
        template<typename T>
            static void _s_destroy_and_deallocate_array( void * begin, void * end, mempool& pool ){
                __BSL_DESTROY(static_cast<T*>(begin), static_cast<T*>(end)); // this method will be specially optimized to the type which has trivial destructor;
                pool.free(begin, static_cast<char*>(end) - static_cast<char*>(begin) );
            }

        /**
        * @brief 封装了::malloc()和::free()的简单mempool
        *  
        * 默认构造的ResourcePool 使用该Pool
        */
        syspool     _syspool;

        /**
        * @brief 当前使用的空间适配器的引用
        *  
        */
        mempool&    _mempool;
        /**
        * @brief 维护通过attach()托管的单个对象信息的块链表（不需回收内存） 
        *  
        *  
        */
        block_list_node_t<attach_object_info_t>    *_p_attach_object_list;
        /**
        * @brief 维护通过attach()托管的对象数组信息的块链表（不需回收内存）
        *  
        *  
        */
        block_list_node_t<attach_array_info_t>     *_p_attach_array_list;
        /**
        * @brief 维护单个对象信息的块链表 
        *  
        *  
        */
        block_list_node_t<alloc_object_info_t>     *_p_alloc_object_list;
        /**
        * @brief 维护对象数组信息的块链表
        *  
        *  
        */
        block_list_node_t<alloc_array_info_t>      *_p_alloc_array_list;


    };

} // namespace bsl
#endif  //__RESOURCEPOOL_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
