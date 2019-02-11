/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_bin_buffer.h,v 1.0 2009/10/14 08:24:58 chenxm Exp $ 
 * 
 **************************************************************************/
 
 
 /**
 * @file bsl_bin_buffer.h
 * @author liaoshangbin(liaoshangbin@baidu.com)
 * @date 2010/07/31 13:46:02  / 2010/10/15 modified by zhujianwei
 * @version $Revision: 1.0 $ 
 * @brief 
 *  
 **/

#ifndef  __BSL_BIN_BUFFER_H_
#define  __BSL_BIN_BUFFER_H_
#include <cstring>
#include <cstdarg>
#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include "bsl/utils/bsl_memcpy.h"
#include "bsl/pool/bsl_pool.h"



namespace bsl{
    /**
    * @brief 读入数据，并按二进制进行存储
    *  
    *  该类的一大特点是不会抛出异常。在内存不足的时候，该类会截断字符串并置"被截断位"。
    *  BinBuffer对象通过字节对齐方式自行管理一片用于表示字符串的缓冲区，并提供方法追加各种类型对象。
    *  若内存不足时，内存容量将翻倍，若申请新内存失败，将填满旧内存，
    *  而且使truncated()方法返回true，ever_truncated()返回true。
    */
    class BinBuffer{
    public:
        /**
         * @brief 构造函数
         *
         * 可传入capacity参数指定预分配的内存空间。如capacity==0没有动态内存分配。
         * 可传入pack参数指定默认字节对齐值，pack要求为2的整数幂并且<=MAX_PACK
         *                                      否则_pack=DEFAULT_PACK
         * 
         * 注：如内存分配失败不会抛出异常。其结果相当于以0为参数构造。
         *
         * @param [in] capacity   : size_t
         * @param [in] pack       : size_t
         * @see 
         * @author liaoshangbin
         * @date 2010/7/29 12:05:10
        **/
        explicit BinBuffer( size_t cap = DEFAULT_CAPACITY, size_t pack = DEFAULT_PACK)
            :_size(0), _capacity(cap), _pack(pack), 
            _truncated(false), _ever_truncated(false), _str(NULL), _mempool(NULL) {
            if ( _capacity != 0 ) {
                _str = static_cast<char *>(_mempool == NULL ? 
                        malloc(_capacity) : _mempool->malloc(_capacity));
                if ( _str ){
                    _str[0] = '\0';
                }else{
                    _capacity = 0;
                }
            }
            if(!(pack > 0 && pack <= DEFAULT_MAX_PACK && is_power_two(pack))){
                _pack = DEFAULT_PACK;
            }
        }

        /**
         * @brief 使用allocator的构造函数
         *
         * 可传入capacity参数指定预分配的内存空间。如capacity==0没有动态内存分配。
         * 可传入pool参数指定使用的内存池
         * 可传入pack参数指定默认字节对齐值，pack要求为2的整数幂并且<=MAX_PACK
         *                                        否则_pack=DEFAULT_PACK
         *
         * 注：如内存分配失败不会抛出异常。其结果相当于以0为参数构造。
         * 
         * @param [in] pool       : mempool&
         * @param [in] capacity   : size_t
         * @param [in] pack       : size_t
         * @see
         * @author
         * @date 2010/7/29 12:06:16
        **/
        explicit BinBuffer( 
                mempool& pool, 
                size_t cap = DEFAULT_CAPACITY, 
                size_t pack = DEFAULT_PACK 
                )
            :_size(0), _capacity(cap), _pack(pack), 
            _truncated(false), _ever_truncated(false), _str(NULL), _mempool(&pool) {
            if ( _capacity != 0 ) {
                _str = static_cast<char *>(_mempool == NULL ? 
                        malloc(_capacity) : _mempool->malloc(_capacity));
                if ( _str ){
                    _str[0] = '\0';
                }else{
                    _capacity = 0;
                }
            }
            if(!(pack > 0 && pack <= DEFAULT_MAX_PACK && is_power_two(pack))) {
                _pack = DEFAULT_PACK;
            }
        }

        /**
         * @brief 把另一BinBuffer的内存转移到本BinBuffer（另一BinBuffer会被清空），O(1)复杂度
         *        因为不同BinBuffer使用的内存池会有不同
         *        先释放自身_str的内存，然后_str指向other._str浅复制字符串，
         *        自身的_mempool指向other._mempool，这样_mempool还是处理_str这块内存
         *        最后清空other._str，other成为空的BinBuffer还可以继续使用
         *
         * @param [in] other   : BinBuffer&
         * @return  BinBuffer& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 17:45:30
        **/
        BinBuffer& transfer_from ( BinBuffer& other ){
            if ( &other != this ){
                if ( _str ){
                    _mempool == NULL ? free( _str ) : _mempool->free( _str, _capacity);
                }
                _size           = other._size;              
                _capacity       = other._capacity;
                _pack           = other._pack;
                _truncated      = other._truncated; 
                _ever_truncated = other._ever_truncated;
                _str            = other._str; 
                _mempool        = other._mempool;
                other._size     = 0;                         
                other._capacity = 0;                             
                other._pack     = DEFAULT_PACK;                       
                other._truncated= false;                   
                other._ever_truncated = false;                             
                other._str      = NULL;
            }
            return *this;
        }

        /**
         * @brief 交换两BinBuffer内容，O(1)复杂度
         *
         * @param [in] other   : BinBuffer&
         * @return  void 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 18:14:15
        **/
        void swap( BinBuffer& other ){
            std::swap( _str,            other._str );
            std::swap( _size,           other._size );
            std::swap( _capacity,       other._capacity );
            std::swap( _pack,           other._pack );
            std::swap( _truncated,      other._truncated );
            std::swap( _ever_truncated, other._ever_truncated );
            std::swap( _mempool,        other._mempool );
        }

        /**
         * @brief 析构函数
         *
         * @see 
         * @author chenxm
         * @date 2009/02/04 18:14:47
        **/
        ~BinBuffer(){
            if ( _str ){
                _mempool == NULL ? free( _str ) : _mempool->free( _str, _capacity);
            }
        }

        /**
         * @brief BinBuffer长度
         *
         * @return  size_t 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 18:14:56
        **/
        size_t size() const{
            return _size;
        }
        
        /**
         * @brief BinBuffer当前容量。保证容量>=长度。当容量不足时，容量会自动增长。
         *
         * @return  size_t 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 18:15:23
        **/
        size_t capacity() const {
            return _capacity;
        }

        /**
         * @brief 返回BinBuffer是否为空
         *
         * @return  bool 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 18:16:26
        **/
        bool empty() const {
            return _size == 0;
        }

        /**
         * @brief 返回BinBuffer是否为满
         *
         * @return  bool 
         * @retval   
         * @see 
         * @author 
         * @date 
        **/
        bool full() const {
            return _size == _capacity;
        }

        /**
         * @brief 返回BinBuffer最近一次操作是否发生了截断
         *
         * @return  bool 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 18:16:58
        **/
        bool truncated() const {
            return _truncated;
        }
        
        /**
         * @brief 返回BinBuffer到目前为止是否发生了截断
         *
         * @return  bool 
         * @retval   
         * @see 
         * @author liaoshangbin
         * @date 2010/07/31 14:49:24
        **/
        bool ever_truncated() const {
            return _ever_truncated;
        }

        /**
         * @brief 返回BinBuffer内容的C风格字符串表示。O(1)复杂度
         *
         * @return  const char* 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 18:17:26
        **/
        const char * data() const {
            if ( _str ){
                return _str;
            }else{
                return "";
            }
        }

        /**
         * @brief 清空内容
         *
         * @return  void 
         * @retval   
         * @see 
         * @author liaoshangbin
         * @date 
        **/
        void clear() {
            if ( _size ){
                _str[0] = '\0';
            }
            _size = 0;
        }

        /**
         * @brief 手动扩大内存容量
         *
         * @param [in] __capacity   : size_t
         * @return  bool 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 18:18:34
        **/
        bool reserve( size_t cap ) {
            if ( cap > _capacity  ){
                if ( cap < _capacity * 2 ){ 
                    cap = _capacity * 2;
                }

                char * _new = static_cast<char*>(_mempool == NULL ? 
                        malloc(cap) :_mempool->malloc(cap));
                if ( !_new ){
                    return false;
                }

                if ( _str ){
                    xmemcpy( _new, _str, _size );
                    _mempool == NULL ? free( _str ) : _mempool->free( _str, _capacity);
                }

                _str        = _new;
                _capacity   = cap;
            }

            return true;
        }

        /**
         * @brief 追加另一个BinBuffer，other按照min(other.get_pack(), this->_pack)字节对齐
         *
         * @param [in] other   : const BinBuffer&
         * @return  BinBuffer&  operator 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/09/27 16:49:30
        **/
        BinBuffer& operator << (const BinBuffer& other){
            size_t pack = other.get_pack();
            pack = pack < _pack ? pack : _pack;
            _size = (_size + pack - 1) & (~(pack-1));
            return push( other.data(), other.size() );
        }

        /**
         * @brief 处理数值类型
         *          bool,signed char,unsigned char,short,unsigned short,int
         *          unsigned int,long int,unsigned long int,long long
         *          unsigned long lont,double,long double
         *          
         *          wchar_t:可输入任意的宽字符，包括'\0'
         *          char:可输入任意的字符，包括'\0'
         *
         * @param [in] value    : _Tp
         * @return BinBuffer& operator
         * @retval
         * @see
         * @author liaoshangbin
         * @data 2010/08/20 12:23:07
         **/
        template<typename _Tp>
            BinBuffer& operator << ( _Tp value ) {
                return push_bin_data( &value, sizeof(value) );
            }

               /**
         * @brief 追加另一个BinBuffer，新的BinBuffer按照min(other.get_pack(), this->_pack)字节对齐
         *
         * @param [in] other   : const BinBuffer&
         * @return  BinBuffer& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/09/27 16:54:00
        **/
        BinBuffer& push(const BinBuffer& other){
            size_t pack = other.get_pack();
            pack = pack > _pack ? pack : _pack;
            _size = (_size + pack - 1) & (~(pack-1));
            return push( other.data(), other._size );
        }

        /**
         * @brief 处理数值类型
         *          bool,signed char,unsigned char,short,unsigned short,int
         *          unsigned int,long int,unsigned long int,long long
         *          unsigned long lont,double,long double
         *
         *          wchar_t:可输入任意的宽字符，包括'\0'
         *          char:可输入任意的字符，包括'\0'    
         *
         * @param [in] value    : _Tp
         * @return BinBuffer& operator
         * @retval
         * @see
         * @author liaoshangbin
         * @data 2010/08/20 12:23:07
         **/
        template<typename _Tp>
            BinBuffer& push( _Tp value ) {
                return push_bin_data( &value, sizeof(value) );
            }

       /**
         * @brief 追加多个任意字符
         *
         * @param [in] count   : int
         * @param [in] c   : char
         * @return  BinBuffer& 
         * @retval   
         * @see 
         * @author chenxm / zhujianwei 
         * @date 2009/02/04 18:40:04 / mod. by zhjw at 2010/09/21
        **/
        BinBuffer& push( size_t count, char c){
            if ( count > _capacity - _size ){    //full
                count = (count <= size_t(-1) - _size) ? count : (size_t(-1) - _size); //limit the size
                if( !reserve( _size + count ) ){
                    //reserve fail
                    count = _capacity - _size;
                    _truncated = true;
                    _ever_truncated = true;
                }else{
                    _truncated = false;
                }
            }
            if ( count ){
                //str != NULL
                memset( _str + _size, c, count );
                _size += count;
            }
            return *this;
        }
        
        /**
         * @brief 追加len长度的数据
         *
         * 调用者必须保证data指向的数据长度不大于len，否则行为未定义
         *
         * @param [in] data_    : const void*
         * @param [in] len      : size_t
         * @return  BinBuffer& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 18:42:28
        **/
        BinBuffer& push(const void* data_, size_t len );

        /**
         * @brief 自定义字节对齐值
         *        返回值为true表示设置成功，false表示设置失败
         *          
         * @param [in] pack   : size_t
         * @return bool
         * @retval
         * @see
         * @author liaoshangbin
         * @data 2010/07/31 17:46:41
        **/
        bool set_pack( size_t pack ) {
            if ( pack > 0 && pack <= DEFAULT_MAX_PACK && is_power_two( pack ) ) {
                _pack = pack;
                return true;
            }
            return false;
        }

        /**
         * @brief 返回自定义字节对齐值
         *          
         * @param
         * @return size_t
         * @retval
         * @see
         * @author liaoshangbin
         * @data 2010/08/03 10:48:18
        **/
        size_t get_pack() const {
            return _pack;
        }

    public:
        /**
         * @brief 默认容量大小
         */
        static const size_t DEFAULT_CAPACITY   = 64;
        /**
         * @brief 默认字节对齐值
         */
        static const size_t DEFAULT_PACK       = 4;
        /**
         * @brief 默认最大字节对齐值
         */
        static const size_t DEFAULT_MAX_PACK   = 64;
    
    private:
        /**
         * @brief 复制构造函数
         */
        BinBuffer( const BinBuffer& other );
        /**
         * @brief 复制赋值运算符
         */
        BinBuffer& operator = ( const BinBuffer& );
        /**
         * @brief 判断一个整数是否为2的整数幂
         *          
         * @param [in] n    : int
         * @return bool
         * @retval
         * @see
         * @author liaoshangbin
         * @data 2010/07/31 15:55:57
        **/
        inline bool is_power_two(size_t n) {
            return (((n)&(n-1))==0);
        }
        /**
         * @brief 数值型数据通过此函数插入到buffer中
         *          
         * @param  [in] data  : const void*
         * @param  [in] len   : size_t
         * @return BinBuffer&
         * @retval
         * @see
         * @author liaoshangbin
         * @data 2010/07/31 17:50:09
        **/
        BinBuffer& push_bin_data( const void* data_, size_t len ) {
            // 根据_pack计算_size的起始位置
            size_t len_ = (len < _pack) ? len : _pack;
            _size = (_size + len_ - 1) & (~(len_ - 1));
            // push函数注意如果内存无法分配时调整_size大小
            return push( data_, len );
        }

        /**
         * @brief BinBuffer的长度
         */
        size_t  _size;
        /**
         * @brief BinBuffer的容量
         */
        size_t  _capacity;
        /**
         * @brief 自定义字节对齐值
         */
        size_t  _pack;
        /**
         * @brief 最近一次操作是否发生截断
         */
        bool    _truncated;
        /**
         * @brief 到目前为止是否发现截断
         */
        bool    _ever_truncated;
        /**
         * @brief BinBuffer的内部字符串缓冲区
         */
        char *  _str;
       
        /**
         * @brief 当前使用的内存池的指针
         *
        **/
        mempool* _mempool;

    };

}   //namespace bsl;


#endif  //__BSL_AUTO_BUFFER_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
