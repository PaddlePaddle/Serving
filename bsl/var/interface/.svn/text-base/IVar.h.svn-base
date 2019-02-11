/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: IVar.h,v 1.4 2010/04/28 12:45:33 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file IVar.h
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/09/24 01:29:15
 * @version $Revision: 1.4 $ 
 * @brief 
 *  
 **/
#ifndef  __BSL_VAR_IVAR_H__
#define  __BSL_VAR_IVAR_H__

#include "bsl/containers/string/bsl_string.h"
#include "bsl/exception/bsl_exception.h"
#include "bsl/ResourcePool.h"
#include "bsl/var/ArrayIterator.h"
#include "bsl/var/DictIterator.h"

namespace bsl{
namespace var{

    /**
    * @brief 为IVar支持无类型二进制数据而定义的结构体
    *  
    * 相当于头指针与长度的简单集合，析构时不会回收data所指内存。
    */
    struct raw_t{
        /**
        * @brief 二进制数据头指针
        *  
        *  
        */
        const void  *data;
        /**
        * @brief 二进制数据长度　
        *  
        *  
        */
        size_t      length;

        /**
         * @brief 默认构造函数
         *
         * @see 
         * @author chenxm
         * @date 2010/03/17 16:46:28
        **/
        raw_t()
            :data(NULL), length(0) {}
        /**
         * @brief 常用的构造函数
         *
         * @param [in] data_   : const void*
         * @param [in] len   : size_t
         * @see 
         * @author chenxm
         * @date 2010/03/17 16:46:41
        **/
        raw_t( const void *data_, size_t len)
            :data(data_), length(len) {}
    };

	//forward declaration
    class IVar{
    public:
        /**
        * @brief 字符串类型
        *  
        *  
        */
        typedef bsl::string         string_type;

        /**
        * @brief 字段名类型
        *  
        *  
        */
        typedef bsl::string         field_type;

        /**
        * @brief 无类型二进制数据类型
        *  
        *  
        */
        typedef raw_t               raw_type;

        /**
        * @brief 数组迭代器类型
        *  
        *  
        */
        typedef ArrayIterator       array_iterator;

        /**
        * @brief 只读数组迭代器类型
        *  
        *  
        */
        typedef ArrayConstIterator  array_const_iterator;

        /**
        * @brief 字典迭代器类型
        *  
        *  
        */
        typedef DictIterator        dict_iterator;

        /**
        * @brief 只读字典迭代器类型
        *  
        *  
        */
        typedef DictConstIterator   dict_const_iterator;

        /**
        * @brief 掩码类型
        *  
        *  
        */
        typedef unsigned int mask_type;

        // constant definition
#ifdef PHP_COMLOG
        static const mask_type _IS_BOOL = 1 << 0;          /**< 是否布尔类型       */
        static const mask_type IS_NUMBER = 1 << 1;        /**< 是否数值类型       */
        static const mask_type _IS_STRING = 1 << 2;        /**< 是否字符串类型       */
        static const mask_type IS_RAW = 1 << 3;        /**< 是否二进制类型       */
        static const mask_type _IS_ARRAY = 1 << 4;        /**< 是否数组类型       */
        static const mask_type IS_DICT = 1 << 5;        /**< 是否字典类型       */
        static const mask_type IS_CALLABLE = 1 << 6;        /**< 是否可调用类型       */
        static const mask_type IS_OTHER = 1 << 7;        /**< 是否其它类型       */
        static const mask_type IS_REF = 1 << 8;        /**< 是否引用类型       */
        static const mask_type IS_MUTABLE = 1 << 9;        /**< 是否可变类型       */
        static const mask_type IS_FLOATING = 1 << 10;        /**< 是否浮点类型       */
        static const mask_type IS_SIGNED = 1 << 11;        /**< 是否有符号类型       */
        static const mask_type IS_ONE_BYTE = 1 << 12;        /**< 是否单字节类型       */
        static const mask_type IS_TWO_BYTE = 1 << 13;        /**< 是否双字节类型       */
        static const mask_type IS_FOUR_BYTE = 1 << 14;        /**< 是否四字节类型       */
        static const mask_type IS_EIGHT_BYTE = 1 << 15;        /**< 是否八字节类型       */
        static const mask_type IS_BIG_INT = 1 << 16;          /**< 是否大整数类型     */
        static const mask_type NONE_MASK = 0;        /**< 空掩码 */
        static const mask_type ALL_MASK = ~0;        /**< 满掩码 */
#else
        static const mask_type IS_BOOL = 1 << 0;          /**< 是否布尔类型       */
        static const mask_type IS_NUMBER = 1 << 1;        /**< 是否数值类型       */
        static const mask_type IS_STRING = 1 << 2;        /**< 是否字符串类型       */
        static const mask_type IS_RAW = 1 << 3;        /**< 是否二进制类型       */
        static const mask_type IS_ARRAY = 1 << 4;        /**< 是否数组类型       */
        static const mask_type IS_DICT = 1 << 5;        /**< 是否字典类型       */
        static const mask_type IS_CALLABLE = 1 << 6;        /**< 是否可调用类型       */
        static const mask_type IS_OTHER = 1 << 7;        /**< 是否其它类型       */
        static const mask_type IS_REF = 1 << 8;        /**< 是否引用类型       */
        static const mask_type IS_MUTABLE = 1 << 9;        /**< 是否可变类型       */
        static const mask_type IS_FLOATING = 1 << 10;        /**< 是否浮点类型       */
        static const mask_type IS_SIGNED = 1 << 11;        /**< 是否有符号类型       */
        static const mask_type IS_ONE_BYTE = 1 << 12;        /**< 是否单字节类型       */
        static const mask_type IS_TWO_BYTE = 1 << 13;        /**< 是否双字节类型       */
        static const mask_type IS_FOUR_BYTE = 1 << 14;        /**< 是否四字节类型       */
        static const mask_type IS_EIGHT_BYTE = 1 << 15;        /**< 是否八字节类型       */
        static const mask_type IS_BIG_INT = 1 << 16;           /**< 是否大整数类型     */ 
        static const mask_type NONE_MASK = 0;        /**< 空掩码 */
        static const mask_type ALL_MASK = ~0;        /**< 满掩码 */
#endif

        //methods for all
        /**
         * @brief 析构函数
         *
         * @see 
         * @author chenxm
         * @date 2009/02/04 18:49:09
        **/
        virtual ~IVar(){ } 

        /**
         * @brief 赋值运算符
         *
         * 所有IVar实现类都必须支持该方法。
         *
         * @return  IVar&  operator 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 18:49:21
        **/
        virtual IVar& operator = ( IVar& ) = 0;

        /**
         * @brief 打印重要的内部状态，及其子IVar对象的状态
         *
         * 所有IVar实现类都必须支持该方法。
         * 该方法仅用于调试与跟踪，其内容应该容易被肉眼识别。其格式可能经常变化，不应对其内容进行监控。
         * 
         * 可选的verbose_level参数表示递归深度。0表示不递归子IVar对象，实现类应保证该函数算法复杂度为O(1)；1表示递归所有直接子IVar对象，实现类应保证该函数算法复杂度为O(size())，余类推。
         *
         * @return  string_type 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 18:51:26
        **/
        virtual string_type dump(size_t verbose_level=0) const = 0;

        /**
         * @brief 清空函数
         *
         * 所有IVar实现类都必须支持该方法。
         *
         * @return  void 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 18:51:06
        **/
        virtual void clear() = 0;

        /**
         * @brief 转化为字符串。
         *
         * 所有IVar实现类都必须支持该方法。
         * 自BSL 1.0.5后，该函数只用于转化为字符串，调试/跟踪应使用dump()
         *
         * @return  string_type 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 18:51:26
        **/
        virtual string_type to_string() const = 0;

        /**
         * @brief 返回实现类型的字符串表示
         *
         * 所有IVar实现类都必须支持该方法。
         *
         * @return  string_type 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 18:54:18
        **/
        virtual string_type get_type() const = 0; 

        /**
         * @brief 返回实现类型的类型掩码
         *
         * 所有IVar实现类都必须支持该方法。
         * 目前返回值类型目前是unsigned short，以后可能会改变，但会保持与unsigned short兼容
         *
         * @return  mask_type 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/11 18:54:18
        **/
        virtual mask_type get_mask() const = 0;

        /**
         * @brief 复制一个var结点
         *
         * @return  IVar& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/01/29 16:20:41
        **/
        virtual IVar& clone(bsl::ResourcePool& /*rp*/) const = 0;

        /**
         * @brief 复制一个var结点, 由is_deep_copy参数控制是否深复制
         *
         * @return  IVar& 
         * @retval   
         * @see 
         * @author zhujianwei
         * @date 2011/03/22
        **/
        virtual IVar& clone(bsl::ResourcePool& /*rp*/, bool /*is_deep_copy*/) const{
            throw bsl::NotImplementedException()<<BSL_EARG<<"type["<<typeid(*this).name()
                <<"] this["<<dump(0)<<"]: clone interface has not been implemented.";
        }

        //methods for all, test methods
        /**
         * @brief 返回是否为空类型
         *
         * 所有IVar实现类都必须支持该方法。
         *
         * @return  bool 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 18:56:50
        **/
        virtual bool is_null() const {
            return (get_mask() & 0xFF) == 0;
        }

        /**
         * @brief 返回是否为引用类型
         *
         * 所有IVar实现类都必须支持该方法。
         *
         * @return  bool 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 18:57:06
        **/
        virtual bool is_ref() const {
            return get_mask() & IS_REF;
        }

        /**
         * @brief 返回是否为布尔类型
         * 所有IVar实现类都必须支持该方法。
         *
         * @return  bool 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/05/07 11:10:51
        **/
        virtual bool is_bool() const {
#ifdef PHP_COMLOG
            return get_mask() & _IS_BOOL;
#else
            return get_mask() & IS_BOOL;
#endif
        }
        /**
         * @brief 返回是否为数值类型
         *
         * 所有IVar实现类都必须支持该方法。
         *
         * @return  bool 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 18:57:14
        **/
        virtual bool is_number() const {
            return get_mask() & IS_NUMBER;
        }

        /**
         * @brief 返回是否为8位有符号整数类型
         *
         * 所有IVar实现类都必须支持该方法。
         *
         * @return  bool 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 17:05:41
        **/
        virtual bool is_int8() const {
            return check_mask( get_mask(), IS_NUMBER|IS_ONE_BYTE|IS_SIGNED, IS_FLOATING );
        }
        
        /**
         * @brief 返回是否为8位无符号整数类型
         *
         * 所有IVar实现类都必须支持该方法。
         *
         * @return  bool 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 17:05:48
        **/
        virtual bool is_uint8() const {
            return check_mask( get_mask(), IS_NUMBER|IS_ONE_BYTE, IS_SIGNED|IS_FLOATING );
        }

        /**
         * @brief 返回是否为16位有符号整数类型
         *
         * 所有IVar实现类都必须支持该方法。
         *
         * @return  bool 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 17:05:52
        **/
        virtual bool is_int16() const {
            return check_mask( get_mask(), IS_NUMBER|IS_TWO_BYTE|IS_SIGNED, IS_FLOATING );
        }

        /**
         * @brief 返回是否为16位无符号整数类型
         *
         * 所有IVar实现类都必须支持该方法。
         *
         * @return  bool 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 17:05:59
        **/
        virtual bool is_uint16() const {
            return check_mask( get_mask(), IS_NUMBER|IS_TWO_BYTE, IS_SIGNED|IS_FLOATING );
        }

        /**
         * @brief 返回是否为32位整数类型
         *
         * 所有IVar实现类都必须支持该方法。
         *
         * @return  bool 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 18:57:24
        **/
        virtual bool is_int32() const {
            return check_mask( get_mask(), IS_NUMBER|IS_FOUR_BYTE|IS_SIGNED, IS_FLOATING );
        }

        /**
         * @brief 返回是否为32位无符号整数类型
         *
         * 所有IVar实现类都必须支持该方法。
         *
         * @return  bool 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 17:06:04
        **/
        virtual bool is_uint32() const {
            return check_mask( get_mask(), IS_NUMBER|IS_FOUR_BYTE, IS_SIGNED|IS_FLOATING );
        }

        /**
         * @brief 返回是否为64位整数类型
         *
         * 所有IVar实现类都必须支持该方法。
         *
         * @return  bool 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 18:57:36
        **/
        virtual bool is_int64() const {
            return check_mask( get_mask(), IS_NUMBER|IS_EIGHT_BYTE|IS_SIGNED, IS_FLOATING );
        }

        /**
         * @brief 返回是否为64位无符号整数类型
         *
         * 所有IVar实现类都必须支持该方法。
         *
         * @return  bool 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 17:06:08
        **/
        virtual bool is_uint64() const {
            return check_mask( get_mask(), IS_NUMBER|IS_EIGHT_BYTE, IS_SIGNED|IS_FLOATING );
        }

		/**
		 * @brief 返回是否为大整数类型
		 *
		 * 所有IVar实现类都必须支持该方法
		 *
		 * @return 
		 *
		 * @version 1.1.24
		 * @author linjieqiong
		 * @date 2013/07/25 10:11:48
		 */
        virtual bool is_bigint() const {
            return check_mask( get_mask(), IS_NUMBER|IS_BIG_INT, IS_FLOATING );
        }

        /**
         * @brief 返回是否为单精度浮点数类型
         *
         * 所有IVar实现类都必须支持该方法。
         *
         * @return  bool 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 17:06:11
        **/
        virtual bool is_float() const {
            return check_mask( get_mask(), IS_NUMBER|IS_FOUR_BYTE|IS_FLOATING );
        }

        /**
         * @brief 返回是否为双精度浮点数类型
         *
         * 所有IVar实现类都必须支持该方法。
         *
         * @return  bool 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 18:57:47
        **/
        virtual bool is_double() const {
            return check_mask( get_mask(), IS_NUMBER|IS_EIGHT_BYTE|IS_FLOATING );
        }

        /**
         * @brief 返回是否为字符串类型
         *
         * 所有IVar实现类都必须支持该方法。
         *
         * @return  bool 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 18:58:11
        **/
        virtual bool is_string() const {
#ifdef PHP_COMLOG
            return get_mask() & _IS_STRING;
#else
            return get_mask() & IS_STRING;
#endif
        }

        /**
         * @brief 返回是否为数组类型
         *
         * 所有IVar实现类都必须支持该方法。
         *
         * @return  bool 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 18:58:21
        **/
        virtual bool is_array() const {
#ifdef PHP_COMLOG
            return get_mask() & _IS_ARRAY;
#else
            return get_mask() & IS_ARRAY;
#endif
        }

        /**
         * @brief 返回是否为字典类型
         *
         * 所有IVar实现类都必须支持该方法。
         *
         * @return  bool 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 18:58:28
        **/
        virtual bool is_dict() const {
            return get_mask() & IS_DICT;
        }

        /**
         * @brief 返回是否为可调用类型
         *
         * 所有IVar实现类都必须支持该方法。
         *
         * @return  bool 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 18:58:37
        **/
        virtual bool is_callable() const {
            return get_mask() & IS_CALLABLE;
        }

        /**
         * @brief 返回是否为无类型二进制数据类型
         *
         * 所有IVar实现类都必须支持该方法
         *
         * @return  bool 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/05/03 14:26:52
        **/
        virtual bool is_raw() const {
            return get_mask() & IS_RAW;
        }

        /**
         * @brief 所有is_bool()返回true的IVar实现类都必须支持该方法
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         *
         * @return  bool 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/05/07 11:10:19
        **/
        virtual bool to_bool() const {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"]";
        }
        //methods for value

        /**
         * @brief 转化为位有符号整数
         *
         * 所有is_number()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 若转化发生数值溢出，抛出bsl::UnderflowException / bsl::OverflowException
         *
         * @return  signed char 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/01/29 13:40:59
        **/
        virtual signed char to_int8() const {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"]";
        }

        /**
         * @brief 转化为位无符号整数
         *
         * 所有is_number()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 若转化发生数值溢出，抛出bsl::UnderflowException / bsl::OverflowException
         *
         * @return  unsigned char 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/01/29 13:41:03
        **/
        virtual unsigned char to_uint8() const {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"]";
        }

        /**
         * @brief 转化为位有符号整数
         *
         * 所有is_number()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 若转化发生数值溢出，抛出bsl::UnderflowException / bsl::OverflowException
         *
         * @return  signed short 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/01/29 13:41:09
        **/
        virtual signed short to_int16() const {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"]";
        }

        /**
         * @brief 转化为位无符号整数
         *
         * 所有is_number()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 若转化发生数值溢出，抛出bsl::UnderflowException / bsl::OverflowException
         *
         * @return  unsigned short 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/01/29 13:41:12
        **/
        virtual unsigned short to_uint16() const {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"]";
        }

        /**
         * @brief 转化为位有符号整数
         *
         * 所有is_number()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 若转化发生数值溢出，抛出bsl::UnderflowException / bsl::OverflowException
         *
         * @return  signed int 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/01/29 13:41:15
        **/
        virtual signed int to_int32() const {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"]";
        }

        /**
         * @brief 转化为位无符号整数
         *
         * 所有is_number()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 若转化发生数值溢出，抛出bsl::UnderflowException / bsl::OverflowException
         *
         * @return  unsigned int 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/01/29 13:41:18
        **/
        virtual unsigned int to_uint32() const {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"]";
        }

        /**
         * @brief 转化为long long
         *
         * 所有is_number()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 若转化发生数值溢出，抛出bsl::UnderflowException / bsl::OverflowException
         *
         * @return  signed long long 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/01/29 13:41:22
        **/
        virtual signed long long to_int64() const {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"]";
        }

        /**
         * @brief 转化为unsigned long long
         *
         * 所有is_number()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 若转化发生数值溢出，抛出bsl::UnderflowException / bsl::OverflowException
         *
         * @return  unsigned long long 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/01/29 13:41:25
        **/
        virtual unsigned long long to_uint64() const {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"]";
        }

        /**
         * @brief 转化为float类型
         *
         * 所有is_number()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         *
         * @return  float 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 17:10:08
        **/
        virtual float to_float() const {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"]";
        }

        /**
         * @brief 转化为双精度浮点数
         *
         * 所有is_number()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         *
         * @return  double 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:03:06
        **/
        virtual double to_double() const {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"]";
        }

        /**
         * @brief 转化为无类型二进制数据类型
         *
         * 所有is_raw()返回true的实现类都必须支持该方法
         *
         * @return  raw_type 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/05/03 14:28:31
        **/
        virtual raw_type to_raw() const {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"]";
        }

        /**
         * @brief 所有is_bool()返回true的IVar实现类都必须支持该方法
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         *
         * @return  IVar&  operator 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/05/07 11:09:34
        **/
        virtual IVar& operator = ( bool value_ ){
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] assign from["<<value_<<"]";
        }

        /**
         * @brief 用8位有符号整数赋值
         *
         * 所有is_number()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 若转化发生数值溢出，抛出bsl::UnderflowException / bsl::OverflowException
         *
         * @return  IVar&  operator 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:03:28
        **/
        virtual IVar& operator = ( signed char value_ ){
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] assign from["<<value_<<"]";
        }

        /**
         * @brief 用8位无符号整数赋值
         *
         * 所有is_number()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 若转化发生数值溢出，抛出bsl::UnderflowException / bsl::OverflowException
         *
         * @return  IVar&  operator 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:03:28
        **/
        virtual IVar& operator = ( unsigned char value_ ){
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] assign from["<<value_<<"]";
        }


        /**
         * @brief 用16位有符号整数赋值
         *
         * 所有is_number()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 若转化发生数值溢出，抛出bsl::UnderflowException / bsl::OverflowException
         *
         * @return  IVar&  operator 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:03:28
        **/
        virtual IVar& operator = ( signed short value_ ){
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] assign from["<<value_<<"]";
        }

        /**
         * @brief 用16位无符号整数赋值
         *
         * 所有is_number()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 若转化发生数值溢出，抛出bsl::UnderflowException / bsl::OverflowException
         *
         * @return  IVar&  operator 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:03:28
        **/
        virtual IVar& operator = ( unsigned short value_ ){
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] assign from["<<value_<<"]";
        }


        /**
         * @brief 用32位有符号整数赋值
         *
         * 所有is_number()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 若转化发生数值溢出，抛出bsl::UnderflowException / bsl::OverflowException
         *
         * @return  IVar&  operator 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:03:28
        **/
        virtual IVar& operator = ( signed int value_ ){
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] assign from["<<value_<<"]";
        }

        /**
         * @brief 用32位无符号整数赋值
         *
         * 所有is_number()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 若转化发生数值溢出，抛出bsl::UnderflowException / bsl::OverflowException
         *
         * @return  IVar&  operator 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:03:28
        **/
        virtual IVar& operator = ( unsigned int value_ ){
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] assign from["<<value_<<"]";
        }


        /**
         * @brief 用64位有符号整数赋值
         *
         * 所有is_number()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 若转化发生数值溢出，抛出bsl::UnderflowException / bsl::OverflowException
         *
         * @return  IVar&  operator 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:03:28
        **/
        virtual IVar& operator = ( signed long long value_ ){
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] assign from["<<value_<<"]";
        }

        /**
         * @brief 用64位无符号整数赋值
         *
         * 所有is_number()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 若转化发生数值溢出，抛出bsl::UnderflowException / bsl::OverflowException
         *
         * @return  IVar&  operator 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:03:28
        **/
        virtual IVar& operator = ( unsigned long long value_ ){
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] assign from["<<value_<<"]";
        }

        /**
         * @brief 用单精度浮点数赋值
         *
         * @param [in] value_   : float
         * @return  IVar&  operator 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 17:11:50
        **/
        virtual IVar& operator = ( float value_ ){
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] assign from["<<value_<<"]";
        }

        /**
         * @brief 用双精度浮点数赋值
         *
         * 所有is_number()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 若转化发生数值溢出，抛出bsl::UnderflowException / bsl::OverflowException
         *
         * @return  IVar&  operator 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:05:50
        **/
        virtual IVar& operator = ( double value_ ){
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] assign from["<<value_<<"]";
        }

        /**
         * @brief 用C风格字符串赋值
         *
         * 所有is_string()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 若转化发生数值溢出，抛出bsl::UnderflowException / bsl::OverflowException
         *
         * @return  IVar&  operator 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:06:16
        **/
        virtual IVar& operator = ( const char * value_ ){
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] assign from["<<value_<<"]";
        }

        /**
         * @brief 用字符串赋值
         *
         * 所有is_string()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 若转化发生数值溢出，抛出bsl::UnderflowException / bsl::OverflowException
         *
         * @return  IVar&  operator 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:06:16
        **/
        virtual IVar& operator = ( const string_type& value_ ){
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] assign from["<<value_<<"]";
        }

        /**
         * @brief 返回C风格字符串表示
         *
         * 所有is_string()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         *
         * @return  const char* 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/04/22 18:41:19
        **/
        virtual const char * c_str() const {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"]";
        }

        /**
         * @brief 返回C风格字符串表示的长度，不包括末尾的'\0'
         *
         * 所有is_string()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         *
         * @return  size_t 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/04/22 18:41:54
        **/
        virtual size_t c_str_len() const {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"]";
        }

        /**
         * @brief 模板拆箱(unboxing)函数
         *
         * 该函数不可以改写
         *
         * @return  template<typename T>             T 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 17:12:36
        **/
        template<typename T>
            T to() const;

        /**
         * @brief 用无类型二进制数据类型赋值
         *
         * 所有is_raw()返回true的实现类都必须实现该方法
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         *
         * @return  IVar&  operator 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/05/03 14:32:19
        **/
        virtual IVar& operator = ( const raw_type& value_ ){
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] assign from raw[data:"<<value_.data<<", length:"<<value_.length<<"]";
        }

        //methods for array and dict
        /**
         * @brief 返回集合的元素个数。
         *
         * is_array()返回true或者is_dict()返回true的实现类必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         *
         * @return  size_t 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 18:49:52
        **/
        virtual size_t size() const{
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"]";
        }

        //methods for array
        /**
         * @brief 获取下标index处的IVar对象的引用对象
         *
         * 所有is_array()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 若下标越界，返回bsl::var::Null::null;
         * 一般用于试探性获取
         *
         * @return  IVar& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:07:04
        **/
        virtual IVar& get( size_t idx ) {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] index["<<idx<<"]";
        }

        /**
         * @brief 获取下标index处的IVar对象的引用对象
         *
         * 所有is_array()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 若下标越界，返回默认值
         * 一般用于试探性获取，并且有特殊默认值要求
         *
         * @return  IVar& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:07:04
        **/
        virtual IVar& get( size_t idx, IVar& default_value ) {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] index["<<idx<<"] default["<<default_value.dump(0)<<"]";
        }

        /**
         * @brief 获取下标index处的IVar对象的引用对象
         *
         * 所有is_array()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 若index >= size()，返回bsl::var::Null::null;
         * 一般用于试探性获取
         *
         * @return  const IVar& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:12:42
        **/
        virtual const IVar& get( size_t idx ) const {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] index["<<idx<<"]";
        }

        /**
         * @brief 获取下标index处的IVar对象的引用对象
         *
         * 所有is_array()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 若下标越界，返回默认值
         * 一般用于试探性获取，并且有特殊默认值要求
         *
         * @return  IVar& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:07:04
        **/
        virtual const IVar& get( size_t idx, const IVar& default_value ) const {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] index["<<idx<<"] default["<<default_value.dump(0)<<"]";
        }

        /**
         * @brief 设置下标绑定
         *
         * 所有is_array()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 若index >= size()，则数组会自动增长到size() + 1
         *
         * @return  void
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:07:04
        **/
        virtual void set( size_t idx, IVar& value_ ){
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] index["<<idx<<"] value["<<value_.dump(0)<<"]";
        }

        /**
         * @brief 删除下标绑定
         *
         * 所有is_array()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 若绑定不存在，返回false；否则，返回true。
         *
         * @return  bool 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:14:29
        **/
        virtual bool del( size_t idx ){
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] index["<<idx<<"]";
        }

        /**
         * @brief 返回只读起始数组迭代器
         *
         * 所有is_array()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         *
         * @return  array_const_iterator 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:14:51
        **/
        virtual array_const_iterator array_begin() const {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"]";
        }

        /**
         * @brief 返回起始数组迭代器
         *
         * 所有is_array()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         *
         * @return  array_iterator 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:15:06
        **/
        virtual array_iterator array_begin() {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"]";
        }

        /**
         * @brief 返回只读末尾数组迭代器
         *
         * 所有is_array()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         *
         * @return  array_const_iterator 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:15:25
        **/
        virtual array_const_iterator array_end() const {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"]";
        }

        /**
         * @brief 返回数组末尾迭代器
         *
         * 所有is_array()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         *
         * @return  array_iterator 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:15:54
        **/
        virtual array_iterator array_end() {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"]";
        }

        /**
         * @brief 返回/设置下标绑定
         *
         * 所有is_array()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 若index >= size()，抛出bsl::OutOfBoundException异常
         * 一般用于"确定性获取"（获取不到直接抛异常）
         * 试验性支持：若index < 0，等价于size() + index
         *
         * @param [in] index   : int
         * @return  const IVar& [] 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:16:21
        **/
        virtual const IVar& operator []( int idx ) const {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] index["<<idx<<"]";
        }

        /**
         * @brief 返回/设置下标绑定
         *
         * 所有is_array()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 若index >= size()，数组会自动增长到index + 1
         * 一般用于设置下标绑定或者确定性获取（获取不到会导致数组自动增长）
         * 试验性支持：若index < 0，等价于size() + index
         *
         * @param [in] index   : int
         * @return  IVar& [] 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:23:03
        **/
        virtual IVar& operator []( int idx ){
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] index["<<idx<<"]";
        }

        //methods for dict
        /**
         * @brief 返回字段名绑定
         *
         * 所有is_dict()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 对于不存在的字段名绑定，返回bsl::var::Null::null。
         * 一般用于试探性获取。
         *
         * @return  IVar& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:23:15
        **/
        virtual IVar& get( const field_type& name ) {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] name["<<name<<"]";
        }

        /**
         * @brief 返回字段名绑定，若字段名不存在，返回默认值
         *
         * 所有is_dict()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 对于不存在的字段名绑定，返回指定的默认值。
         *
         * @return  IVar& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:23:15
        **/
        virtual IVar& get( const field_type& name, IVar& default_value ) {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] name["<<name<<"] default["<<default_value.dump(0)<<"]";
        }

        /**
         * @brief 返回字段名绑定
         *
         * 所有is_dict()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 若不存在该字段名绑定，返回bsl::var::Null::null;
         * 一般用于试探性获取
         *
         * @return  const IVar& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:24:41
        **/
        virtual const IVar& get( const field_type& name ) const {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] name["<<name<<"]";
        }

        /**
         * @brief 返回字段名绑定，若字段名不存在，返回默认值
         *
         * 所有is_dict()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 若不存在该字段名绑定，返回指定的默认值
         * 一般用于试探性获取
         *
         * @return  IVar& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:23:15
        **/
        virtual const IVar& get( const field_type& name, const IVar& default_value ) const {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] name["<<name<<"] default["<<default_value.dump(0)<<"]";
        }

        /**
         * @brief 设置字段名绑定
         *
         * 所有is_dict()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         *
         * @return  IVar& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:23:15
        **/
        virtual void set( const field_type& name, IVar& value_ ){
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] name["<<name<<"] value["<<value_.dump(0)<<"]";
        }

        /**
         * @brief 删除字段名绑定
         *
         * 所有is_dict()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 若绑定不存在，返回false；否则，返回true。
         *
         * @return  bool 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:26:44
        **/
        virtual bool del( const field_type& name ) {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] name["<<name<<"]";
        }

        /**
         * @brief 获取/设置字段名绑定
         *
         * 所有is_dict()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 若绑定不存在，抛出bsl::KeyNotFoundException异常
         * 一般用于"确定性获取"（获取不到直接抛异常）
         *
         * @param [in] name   : const field_type&
         * @return  const IVar& [] 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:26:59
        **/
        virtual const IVar& operator []( const field_type& name ) const {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] name["<<name<<"]";
        }

        /**
         * @brief 获取/设置字段名绑定
         *
         * 所有is_dict()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * 若绑定不存在，新建绑定。
         * 一般用于绑定或者确定性获取（获取不到会导致新建绑定）
         *
         * @param [in] name   : const field_type&
         * @return  IVar& [] 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:27:21
        **/
        virtual IVar& operator []( const field_type& name ){
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] name["<<name<<"]";
        }

        /**
         * @brief 返回只读起始字典迭代器
         *
         * 所有is_dict()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         *
         * @return  dict_const_iterator 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:27:52
        **/
        virtual dict_const_iterator dict_begin() const {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"]";
        }

        /**
         * @brief 返回起始字典迭代器
         *
         * 所有is_dict()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         *
         * @return  dict_iterator 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:29:20
        **/
        virtual dict_iterator dict_begin() {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"]";
        }

        /**
         * @brief 返回只读末尾字典迭代器
         *
         * 所有is_dict()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         *
         * @return  dict_const_iterator 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:31:51
        **/
        virtual dict_const_iterator dict_end() const {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"]";
        }

        /**
         * @brief 返回末尾字典迭代器
         *
         * 所有is_dict()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         *
         * @return  dict_iterator 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:32:30
        **/
        virtual dict_iterator dict_end() {
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"]";
        }

        /**
         * @brief 调用IVar对象 
         *
         * 所有is_callable()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * params应该是一个is_array()返回true的IVar对象
         * 注：这是一个试验性方法，bsl::ResourcePool有可能被bsl::var::IFactory代替
         *
         * @return  IVar& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:32:50
        **/
        virtual IVar& operator()(IVar& params, bsl::ResourcePool& /*rp*/ ){
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] params["<<params.dump(1)<<"]";
        }

        /**
         * @brief 调用IVar对象
         *
         * 所有is_callable()返回true的IVar实现类都必须支持该方法。
         * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
         * self可以是任意IVar对象
         * params应该是一个is_array()返回true的IVar对象
         * 注：这是一个试验性方法，bsl::ResourcePool有可能被bsl::var::IFactory代替
         *
         * @return  IVar& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/02/04 19:33:16
        **/
        virtual IVar& operator()(IVar& self, IVar& params, bsl::ResourcePool& /*rp*/ ){
            throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] self["<<self.dump(0)<<"] params["<<params.dump(1)<<"]";
        }

        /**
         * @brief 工具函数，测试mask是否已设set_bits中的所有位，并且没有设unset_bits的任一位
         *
         * @param [in] mask   : mask_type
         * @param [in] set_bits   : mask_type
         * @param [in] unset_bits   : mask_type
         * @return  inline bool 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 17:14:00
        **/
        static inline bool check_mask( mask_type mask, mask_type set_bits, mask_type unset_bits = 0 ){
            return ((mask & set_bits) == set_bits) && ((mask & unset_bits) == 0);
        }
    };

    /**
     * @brief IVar::to<T>的特化实现（该方法不能被改写）
     *
     * @return  template<>         inline bool 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2010/03/17 17:16:00
    **/
    template<>
        inline bool IVar::to<bool>() const{
            return this->to_bool();
        }
    /**
     * @brief IVar::to<T>的特化实现（该方法不能被改写）
     *
     * @return  template<>         inline signed char IVar::to<signed 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2010/03/17 17:16:05
    **/
    template<>
        inline signed char IVar::to<signed char>() const{
            return this->to_int8();
        }
    /**
     * @brief IVar::to<T>的特化实现（该方法不能被改写）
     *
     * @return  template<>         inline unsigned char IVar::to<unsigned 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2010/03/17 17:16:10
    **/
    template<>
        inline unsigned char IVar::to<unsigned char>() const{
            return this->to_uint8();
        }
    /**
     * @brief IVar::to<T>的特化实现（该方法不能被改写）
     *
     * @return  template<>         inline signed short IVar::to<signed 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2010/03/17 17:16:14
    **/
    template<>
        inline signed short IVar::to<signed short>() const{
            return this->to_int16();
        }
    /**
     * @brief IVar::to<T>的特化实现（该方法不能被改写）
     *
     * @return  template<>         inline unsigned short IVar::to<unsigned 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2010/03/17 17:16:17
    **/
    template<>
        inline unsigned short IVar::to<unsigned short>() const{
            return this->to_uint16();
        }
    /**
     * @brief IVar::to<T>的特化实现（该方法不能被改写）
     *
     * @return  template<>         inline signed int IVar::to<signed 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2010/03/17 17:16:26
    **/
    template<>
        inline signed int IVar::to<signed int>() const{
            return this->to_int32();
        }
    /**
     * @brief IVar::to<T>的特化实现（该方法不能被改写）
     *
     * @return  template<>         inline unsigned int IVar::to<unsigned 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2010/03/17 17:16:56
    **/
    template<>
        inline unsigned int IVar::to<unsigned int>() const{
            return this->to_uint32();
        }
    /**
     * @brief IVar::to<T>的特化实现（该方法不能被改写）
     *
     * @return  template<>         inline signed long long IVar::to<signed long 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2010/03/17 17:17:00
    **/
    template<>
        inline signed long long IVar::to<signed long long>() const{
            return this->to_int64();
        }
    /**
     * @brief IVar::to<T>的特化实现（该方法不能被改写）
     *
     * @return  template<>         inline unsigned long long IVar::to<unsigned long 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2010/03/17 17:17:12
    **/
    template<>
        inline unsigned long long IVar::to<unsigned long long>() const{
            return this->to_uint64();
        }
    /**
     * @brief IVar::to<T>的特化实现（该方法不能被改写）
     *
     * @return  template<>         inline float 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2010/03/17 17:17:15
    **/
    template<>
        inline float IVar::to<float>() const{
            return this->to_float();
        }
    /**
     * @brief IVar::to<T>的特化实现（该方法不能被改写）
     *
     * @return  template<>         inline double 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2010/03/17 17:17:17
    **/
    template<>
        inline double IVar::to<double>() const{
            return this->to_double();
        }
    /**
     * @brief IVar::to<T>的特化实现（该方法不能被改写）
     *
     * @return  template<>         inline bsl::string 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2010/03/17 17:17:19
    **/
    template<>
        inline bsl::string IVar::to<bsl::string>() const{
            return this->to_string();
        }
    /**
     * @brief IVar::to<T>的特化实现（该方法不能被改写）
     *
     * @return  template<>         inline raw_t 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2010/03/17 17:17:21
    **/
    template<>
        inline raw_t IVar::to<raw_t>() const{
            return this->to_raw();
        }
    /**
     * @brief IVar::to<T>的特化实现（该方法不能被改写）
     *
     * @return  template<>         inline const char*  IVar::to<const 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2010/03/17 17:17:23
    **/
    template<>
        inline const char * IVar::to<const char *>() const{
            return this->c_str();
        }

#if __WORDSIZE == 64 
    /**
     * @brief IVar::to<T>的特化实现（该方法不能被改写）
     *
     * @return signed long
     * @retval   
     * @see 
     * @author chenxm
     * @date 2010/03/17 17:17:23
    **/
    template<>
        inline signed long IVar::to<signed long>() const{
            return this->to_int64();
        }
    /**
     * @brief IVar::to<T>的特化实现（该方法不能被改写）
     *
     * @return unsigned long
     * @retval   
     * @see 
     * @author chenxm
     * @date 2010/03/17 17:17:23
    **/
    template<>
        inline unsigned long IVar::to<unsigned long>() const{
            return this->to_uint64();
        }
#else
    /**
     * @brief IVar::to<T>的特化实现（该方法不能被改写）
     *
     * @return signed long
     * @retval   
     * @see 
     * @author chenxm
     * @date 2010/03/17 17:17:23
    **/
    template<>
        inline signed long IVar::to<signed long>() const{
            return this->to_int32();
        }

    /**
     * @brief IVar::to<T>的特化实现（该方法不能被改写）
     *
     * @return unsigned long
     * @retval   
     * @see 
     * @author chenxm
     * @date 2010/03/17 17:17:23
    **/
    template<>
        inline unsigned long IVar::to<unsigned long>() const{
            return this->to_uint32();
        }
#endif

}}   //  namespace bsl::var

#endif  //__BSL_VAR_IVAR_H__

/* vim: set ts=4 sw=4 sts=4 tw=100 */
