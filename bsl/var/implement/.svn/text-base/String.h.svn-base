/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: String.h,v 1.4 2010/04/28 12:45:33 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file String.h
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/09/24 01:34:12
 * @version $Revision: 1.4 $ 
 * @brief 
 *  
 **/
#ifndef  __BSL_VAR_STRING_H__
#define  __BSL_VAR_STRING_H__
#include "bsl/var/IVar.h"
#include "bsl/var/Ref.h"
#include "bsl/check_cast.h"
#include "bsl/pool/bsl_pool.h"
#include "bsl/pool/bsl_poolalloc.h"
#include "BigInt.h"

namespace bsl{
namespace var{
    
    // forward declarations & typedefs
    template<typename implement_t>
        class BasicString;


    /**
     * @brief 封装了IVar::string_type(bsl::string)
     *
    **/
    typedef BasicString<IVar::string_type> String;
    
    /**
     * @brief var::String字符串类，与var::Dict保持类似的接口
     *          implement_t为BasicString内部实现
     *          
     **/
    template<typename implement_t>
        class BasicString: public IVar{
        public:
            
            /**
             * @brief Var::String的string类型
             *
             **/
            typedef IVar::string_type   string_type;
            /**
             * @brief var::String的字段类型
             *
             **/
            typedef IVar::field_type    field_type;
            /**
             * @brief 得到实现类的allocator
             *
             **/
            typedef typename implement_t::allocator_type allocator_type;        
            
        public:
            //special methods
            BasicString( ): _value() {}

            /**
             * @brief 复制构造函数
             *          
             * @param  [in] other   : BasicString&
             * @return
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/09 18:33:34
             **/
            BasicString( const BasicString& other )
                :IVar(other), _value(other._value) { }
            
            /**
             * @brief 带allocator的构造函数
             *          
             * @param  [in] alloc_  : const allocator_type&
             * @return
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/09 18:21:44
             **/
            explicit BasicString( const allocator_type& alloc_ ): _value( alloc_ ) {}
            
            /**
             * @brief 传入字符串和allocator的构造函数
             *          
             * @param  [in] cstr    : const char*
             * @param  [in] alloc_  : const allocator_type&
             * @return
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/09 19:41:24
             **/
            BasicString( const char * cstr, const allocator_type& alloc_ = allocator_type()  )
                : _value(cstr,alloc_) { }

            /**
             * @brief 传入字符串和长度和allocator的构造函数
             *          
             * @param  [in] cstr    : const char*
             * @param  [in] len     : size_t
             * @param  [in] alloc_  : const allocator_type&
             * @return
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/09 19:42:19
             **/
            BasicString( 
                    const char *cstr, size_t len, 
                    const allocator_type& alloc_ = allocator_type() 
                    )
                : _value(cstr, len, alloc_) {}

            /**
             * @brief 与复制构造函数不同，value_是implement_t类型
             *          
             * @param  [in] value_  : implement_t&
             * @return
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/09 18:35:10
             **/
            BasicString( const implement_t& value_ ): _value(value_){}

            /**
             * @brief 重载operator=运算符
             *          
             * @parami [in] other   : const BasicString&
             * @return
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/09 19:43:45
             **/
            BasicString& operator = ( const BasicString& other ){
                _value = other._value;
                return *this;
            }
            
            virtual ~BasicString(){
                //pass
            }
            
            /**
             * @brief IVar所有派生类都必须实现的接口
             *          
             * @param  [in] var     : IVar&
             * @return
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/09 19:49:54
             **/
            //methods for all
            virtual BasicString& operator = ( IVar& var ){
                _value = var.to_string();
                return *this;
            }

            /**
             * @brief 输出调试信息（仅用于调试）可通过verbose_level控制递归层数
             *
             * @return  implement_t 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 19:44:20
            **/
            virtual implement_t dump(size_t /*verbose_level*/=0) const {
                return  implement_t("[bsl::var::String]").append(_value);
            }

            /**
             * @brief 转化为字符串
             *
             * @return  implement_t 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 19:44:20
            **/
            virtual implement_t to_string() const {
                return _value;            
            }

            /**
             * @brief 获取类型字符串（所有var类型都支持，仅用于调试使用，勿用于类型判断）
             *
             * @return  implement_t 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 19:44:20
            **/
            virtual implement_t get_type() const {
                return "bsl::var::String";
            }

            /**
             * @brief 获取类型掩码（所有var类型都支持）
             *
             * @return  IVar::mask_type 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 19:44:20
            **/
            virtual IVar::mask_type get_mask() const {
#ifdef PHP_COMLOG
                return IVar::_IS_STRING;
#else
                return IVar::IS_STRING;
#endif
            }

            /**
             * @brief 清理函数
             *
             * @return  void 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 17:48:46
             **/
            virtual void clear(){
                _value.clear();
            }

            /**
             * @brief 克隆函数
             *
             * 该函数只克隆本身结点，不克隆子结点，对引用类型，克隆指向的结点
             *
             * @param [in] rp   : bsl::ResourcePool&
             * @return  String& 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 17:58:07
            **/
            virtual BasicString& clone( bsl::ResourcePool& rp ) const {
                return rp.clone(*this);
            }

            /**
             * @brief 克隆函数
             * 
             * 由is_deep_copy参数控制是否深复制
             * 若为false, 只克隆本身结点，不克隆子结点
             * 若为true, 克隆本身结点，并且递归克隆子结点
             * 对引用类型，克隆指向的结点
             * 
             **@param [in] rp   : bsl::ResourcePool& 
             * @param [in] is_deep_copy : bool
             * @return  BasicString& 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 17:58:07 / modified by zhujianwei at 2011/03/22
            **/
            virtual BasicString& clone( bsl::ResourcePool& rp, bool /*is_deep_copy*/ ) const {
                return rp.clone(*this);
            }

            //testers
            /**
             * @brief 判断是否是字符串类型
             *
             * @return  bool 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 18:05:44
            **/
            virtual bool is_string() const {
                return true;
            }

            //methods for value
            /**
             * @brief 转化为布尔类型
             *
             * @return  bool 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 18:09:14
            **/
            virtual bool to_bool() const {
                return _value.c_str()[0] != '\0';
            }

            /**
             * @brief 转化为8位有符号整数类型（所有is_number()返回真的类型都支持）
             *
             * @return  signed char 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 18:09:14
            **/
            virtual signed char to_int8() const {
                return check_cast<signed char>(_value.c_str());
            }

            /**
             * @brief 转化为8位无符号整数类型（所有is_number()返回真的类型都支持）
             *
             * @return  unsigned char 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 18:09:14
            **/
            virtual unsigned char to_uint8() const {
                return check_cast<unsigned char>(_value.c_str());
            }

            /**
             * @brief 转化为16位有符号整数类型（所有is_number()返回真的类型都支持）
             *
             * @return  signed short 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 18:09:14
            **/
            virtual signed short to_int16() const {
                return check_cast<signed short>(_value.c_str());
            }

            /**
             * @brief 转化为16位无符号整数类型（所有is_number()返回真的类型都支持）
             *
             * @return  unsigned short 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 18:09:14
            **/
            virtual unsigned short to_uint16() const {
                return check_cast<unsigned short>(_value.c_str());
            }

            /**
             * @brief 转化为32位有符号整数类型（所有is_number()返回真的类型都支持）
             *
             * @return  signed int 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 18:09:14
            **/
            virtual signed int to_int32() const {
                return check_cast<signed int>(_value.c_str());
            }

            /**
             * @brief 转化为32位无符号整数类型（所有is_number()返回真的类型都支持）
             *
             * @return  unsigned int 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 18:09:14
            **/
            virtual unsigned int to_uint32() const {
                return check_cast<unsigned int>(_value.c_str());
            }

            /**
             * @brief 转化为64位有符号整数类型（所有is_number()返回真的类型都支持）
             *
             * @return  signed long long 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 18:09:14
            **/
            virtual signed long long to_int64() const {
                return check_cast<signed long long>(_value.c_str());
            }

            /**
             * @brief 转化为64位无符号整数类型（所有is_number()返回真的类型都支持）
             *
             * @return  unsigned long long 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 18:09:14
            **/
            virtual unsigned long long to_uint64() const {
                return check_cast<unsigned long long>(_value.c_str());
            }

			/**
			 * @brief conversion to bigint, not a virtual function derived
			 *
			 * @tparam bits
			 *
			 * @return 
			 *
			 * @version 1.1.24
			 * @author linjieqiong
			 * @date 2013/07/25 15:51:13
			 */
			template<size_t bits, bool sign>
			BigInt<bits> to_bigint() {
				BigInt<bits, sign> tmp = _value;
				return tmp;
			}

            /**
             * @brief 转化为单精度浮点数类型（所有is_number()返回真的类型都支持）
             *
             * @return  float 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 18:09:14
            **/
            virtual float to_float() const {
                return check_cast<float>(_value.c_str());
            }

            /**
             * @brief 转化为双精度浮点数类型（所有is_number()返回真的类型都支持）
             *
             * @return  double 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 18:09:14
            **/
            virtual double to_double() const {
                return check_cast<double>(_value.c_str());
            }

            /**
             * @brief 转化为布尔类型
             *          
             * @param
             * @return
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/09 19:52:26
             **/
            virtual BasicString& operator = ( bool b ){
                if ( b ){
                    _value = "true";
                }else{
                    _value = "false";
                }
                return *this;
            }

            /**
             * @brief 使用signed char类型赋值
             *
             * @param [in] val   : signed char
             * @return  String& 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 19:23:16
            **/
            virtual BasicString& operator = ( signed char val ){
                _value.setf( "%hhd", val );
                return *this;
            }

            /**
             * @brief 使用unsigned char类型赋值
             *
             * @param [in] val   : unsigned char
             * @return  String& 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 19:23:16
            **/
            virtual BasicString& operator = ( unsigned char val ){
                _value.setf( "%hhu", val );
                return *this;
            }
            /**
             * @brief 使用signed short类型赋值
             *
             * @param [in] val   : signed short
             * @return  String& 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 19:23:16
            **/
            virtual BasicString& operator = ( signed short val ){
                _value.setf( "%hd", val );
                return *this;
            }
            /**
             * @brief 使用unsigned short类型赋值
             *
             * @param [in] val   : unsigned short
             * @return  String& 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 19:23:16
            **/
            virtual BasicString& operator = ( unsigned short val ){
                _value.setf( "%hu", val );
                return *this;
            }
            /**
             * @brief 使用signed int类型赋值
             *
             * @param [in] val   : signed int
             * @return  String& 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 19:23:16
            **/
            virtual BasicString& operator = ( signed int val ){
                _value.setf( "%d", val );
                return *this;
            }
            /**
             * @brief 使用unsigned int类型赋值
             *
             * @param [in] val   : unsigned int
             * @return  String& 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 19:23:16
            **/
            virtual BasicString& operator = ( unsigned int val ){
                _value.setf( "%u", val );
                return *this;
            }
            /**
             * @brief 使用signed long long类型赋值
             *
             * @param [in] val   : signed long long
             * @return  String& 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 19:23:16
            **/
            virtual BasicString& operator = ( signed long long val ){
                _value.setf( "%lld", val );
                return *this;
            }
            /**
             * @brief 使用unsigned long long类型赋值
             *
             * @param [in] val   : unsigned long long
             * @return  String& 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 19:23:16
            **/
            virtual BasicString& operator = ( unsigned long long val ){
                _value.setf( "%llu", val );
                return *this;
            }

			template<size_t bits, bool sign>
			BasicString& operator = ( BigInt<bits, sign> val){
				_value = val.to_string();
				return *this;
			}

            /**
             * @brief 使用float类型赋值
             *
             * @param [in] val   : float
             * @return  String& 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 19:23:16
            **/
            virtual BasicString& operator = ( float f ){
                _value.setf( "%f", f );
                return *this;
            }

            /**
             * @brief 使用double类型赋值
             *
             * @param [in] val   : double
             * @return  String& 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 19:23:16
            **/
            virtual BasicString& operator = ( double d ){
                _value.setf( "%lf", d );
                return *this;
            }

            /**
             * @brief 使用const char *类型赋值
             *
             * @param [in] val   : const char *
             * @return  String& 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 19:23:16
            **/
            virtual BasicString& operator = ( const char * cstr ){
                _value = cstr;
                return *this;
            }

            /**
             * @brief 使用字符串类型赋值
             *
             * @param [in] val   : const implement_t&
             * @return  String& 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 19:23:16
            **/
            virtual BasicString& operator = ( const implement_t& str ){
                _value = str;
                return *this;
            }

            //use default version for bool、raw
            using IVar::operator =;

            /**
             * @brief 返回C风格字符串表示
             *
             * 所有is_string()返回true的IVar实现类都必须支持该方法。
             * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
             * @return  const char* 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2009/04/22 18:41:19
            **/
            virtual const char * c_str() const {
                return _value.c_str();
            }

            /**
             * @brief 返回C风格字符串表示的长度，不包括末尾的'\0'
             *
             * 所有is_string()返回true的IVar实现类都必须支持该方法。
             * 若实现类不支持该操作，抛出bsl::InvalidOperationException异常
             * @return  size_t 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2009/04/22 18:41:54
            **/
            virtual size_t c_str_len() const {
                return _value.size();
            }

        private:
            /**
             * BaseString内部封装的string类型
             *
             **/
            implement_t _value;
        };

}}//namespace bsl::var

#endif  //__BSL_VAR_STRING_H__

/* vim: set ts=4 sw=4 sts=4 tw=100 */
