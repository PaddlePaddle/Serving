/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: Int32.h,v 1.4 2010/04/28 12:45:33 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file Int32.h
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/09/24 01:32:42
 * @version $Revision: 1.4 $ 
 * @brief 
 *  
 **/
#ifndef  __BSL_VAR_INT32_H__
#define  __BSL_VAR_INT32_H__

#include <cstdlib>
#include <climits>
#include "bsl/var/IVar.h"
#include "bsl/var/Ref.h"
#include "bsl/check_cast.h"
#include "BigInt.h"

namespace bsl{
namespace var{

    class Int32: public IVar{
    public:
        typedef IVar::string_type   string_type;
        typedef IVar::field_type    field_type;

    public:
        //special methods
        Int32( int __value = 0 ) 
            :_value(__value){}

        Int32( const Int32& other )
            :IVar(other), _value( other._value ) {}

        Int32& operator = ( const Int32& other ){
            _value = other._value;
            return *this;
        }

        //methods for all
        virtual Int32& operator = ( IVar& var ) {
            try{
                _value = var.to_int32();    //throw
            }catch(bsl::Exception& e){
                e<<"{bsl::var::Int32::operator =("<<var.dump()<<")}";
                throw;
            }
            return *this;
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
            _value = 0;
        }
    
        /**
         * @brief 克隆函数
         *
         * 该函数只克隆本身结点，不克隆子结点，对引用类型，克隆指向的结点
         *
         * @param [in] rp   : bsl::ResourcePool&
         * @return  Int32& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 17:58:07
        **/
        virtual Int32& clone( bsl::ResourcePool& rp ) const {
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
         * @param [in] rp   : bsl::ResourcePool& 
         * @param [in] is_deep_copy : bool
         * @return  Int32& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 17:58:07 / modified by zhujianwei at 2011/03/22
        **/
        virtual Int32& clone( bsl::ResourcePool& rp, bool /*is_deep_copy*/ ) const {
            return rp.clone(*this);
        }

        /**
         * @brief 输出调试信息（仅用于调试）可通过verbose_level控制递归层数
         *
         * @return  string_type 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:44:20
        **/
        virtual string_type dump(size_t /*verbose_level*/ = 0) const {
            return  string_type("[bsl::var::Int32]").appendf("%d", _value);
        }

        /**
         * @brief 转化为字符串
         *
         * @return  string_type 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:44:20
        **/
        virtual string_type to_string() const {
            return  string_type().appendf("%d", _value);
        }

        /**
         * @brief 获取类型字符串（所有var类型都支持，仅用于调试使用，勿用于类型判断）
         *
         * @return  string_type 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:44:20
        **/
        virtual string_type get_type() const {
            return "bsl::var::Int32";
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
            return IVar::IS_NUMBER | IVar::IS_SIGNED | IVar::IS_FOUR_BYTE;
        }

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
            return _value;
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
            return check_cast<signed char>(_value);
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
            return check_cast<unsigned char>(_value);
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
            return check_cast<signed short>(_value);
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
            return check_cast<unsigned short>(_value);
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
            return _value;
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
            return check_cast<unsigned int>(_value);
        }

        /**
         * @brief 
         *
         * @return  long long 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 18:09:14
        **/
        virtual long long to_int64() const {
            return _value;
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
            return check_cast<unsigned long long>(_value);
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
        BigInt<bits, sign> to_bigint() {
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
            return static_cast<float>(_value);
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
            return static_cast<double>(_value);
        }

        virtual Int32& operator = ( bool b ){
            _value = b;
            return *this;
        }

        /**
         * @brief 使用signed char类型赋值
         *
         * @param [in] val   : signed char
         * @return  Int32& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Int32& operator = ( signed char i ){
            _value = i;
            return *this;
        }

        /**
         * @brief 使用unsigned char类型赋值
         *
         * @param [in] val   : unsigned char
         * @return  Int32& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Int32& operator = ( unsigned char i ){
            _value = i;
            return *this;
        }

        /**
         * @brief 使用signed short类型赋值
         *
         * @param [in] val   : signed short
         * @return  Int32& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Int32& operator = ( signed short i ){
            _value = i;
            return *this;
        }

        /**
         * @brief 使用unsigned short类型赋值
         *
         * @param [in] val   : unsigned short
         * @return  Int32& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Int32& operator = ( unsigned short i ){
            _value = i;
            return *this;
        }

        /**
         * @brief 使用signed int类型赋值
         *
         * @param [in] val   : signed int
         * @return  Int32& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Int32& operator = ( signed int i ){
            _value = i;
            return *this;
        }

        /**
         * @brief 使用unsigned int类型赋值
         *
         * @param [in] val   : unsigned int
         * @return  Int32& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Int32& operator = ( unsigned int ll ){
            _value = check_cast<int>(ll);
            return *this;
        }

        virtual Int32& operator = ( long long ll ){
            _value = check_cast<int>(ll);
            return *this;
        }

        /**
         * @brief 使用unsigned long long类型赋值
         *
         * @param [in] val   : unsigned long long
         * @return  Int32& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Int32& operator = ( unsigned long long ll ){
            _value = check_cast<int>(ll);
            return *this;
        }

		/**
		 * @brief 
		 *
		 * @tparam bits
		 * @param val
		 *
		 * @return 
		 *
		 * @version 1.1.24
		 * @author linjieqiong
		 * @date 2013/07/25 16:26:45
		 */
		template<size_t bits, bool sign>
        Int32& operator = ( BigInt<bits, sign> val){
			_value = val.to_int32();
            return *this;
        }

        /**
         * @brief 使用float类型赋值
         *
         * @param [in] val   : float
         * @return  Int32& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Int32& operator = ( float f ){
            _value = check_cast<int>(f);
            return *this;
        }

        /**
         * @brief 使用double类型赋值
         *
         * @param [in] val   : double
         * @return  Int32& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Int32& operator = ( double d ){
            _value = check_cast<int>(d);
            return *this;
        }

        /**
         * @brief 使用const char *类型赋值
         *
         * @param [in] val   : const char *
         * @return  Int32& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Int32& operator = ( const char *cstr ){
            _value = check_cast<int>(cstr);
            return *this;
        }

        /**
         * @brief 使用字符串类型赋值
         *
         * @param [in] val   : const string_type&
         * @return  Int32& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Int32& operator = ( const string_type& str ){
            _value = check_cast<int>(str.c_str());
            return *this;
        }

        //use default version for raw
        using IVar::operator =;

        //testers
        virtual bool is_number() const {
            return true;
        }

        virtual bool is_int32() const {
            return true;
        }

    private:
        int _value;
    };

}}   //namespace bsl::var

#endif  //__BSL_VAR_INT32_H__

/* vim: set ts=4 sw=4 sts=4 tw=100 */
