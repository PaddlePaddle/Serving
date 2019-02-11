/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: Bool.h,v 1.3 2010/04/28 12:45:33 scmpf Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file Bool.h
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/09/24 20:53:41
 * @version $Revision: 1.3 $ 
 * @brief 
 *  
 **/
#ifndef  __BSL_VAR_BOOL_H__
#define  __BSL_VAR_BOOL_H__
#include "bsl/var/IVar.h"
#include "BigInt.h"

namespace bsl{
namespace var{
    class Bool: public IVar{
    public:
        typedef IVar::string_type           string_type;
        typedef IVar::field_type            field_type;

        //special methods
        Bool()
            :_value(false){}

        Bool( const Bool& other )
            :IVar(other), _value(other._value){ }

        Bool( bool value_ )
            :IVar(), _value(value_){}

        virtual ~Bool(){ } 

        Bool& operator = ( const Bool& other ){
            _value = other._value;
            return *this;
        }

        //methods for all
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
            _value = false;
        }

        /**
         * @brief 克隆函数
         * 
         * 该函数只克隆本身结点，不克隆子结点，对引用类型，克隆指向的结点
         * 
         **@param [in] rp   : bsl::ResourcePool&
         * @return  Bool&
         * @retval
         * @see 
         * @author chenxm
         * @date 2010/03/17 17:58:07
        **/
        virtual Bool& clone( bsl::ResourcePool& rp) const {
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
         * @return  Bool& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 17:58:07 / modified by zhujianwei at 2011/03/22
        **/
        virtual Bool& clone( bsl::ResourcePool& rp, bool /*is_deep_copy*/) const {
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
            if ( _value ){
                return "[bsl::var::Bool]true";
            }else{
                return "[bsl::var::Bool]false";
            }
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
            if ( _value ){
                return "true";
            }else{
                return "false";
            }
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
            return "bsl::var::Bool";
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
            return IVar::_IS_BOOL;
#else
            return IVar::IS_BOOL;
#endif
        }

        virtual Bool& operator = ( IVar& other ){
            _value = other.to_bool();
            return *this;
        }

        virtual bool is_bool() const {
            return true;
        }

        //all other is_xxx() return false;

        //methods for bool
        virtual Bool& operator = ( bool val ){
            _value = val;
            return *this;
        }

        virtual Bool& operator = ( int val ){
            _value = val;
            return *this;
        }

        virtual Bool& operator = ( long long val ){
            _value = val;
            return *this;
        }

		template<size_t bits, bool sign>
        Bool& operator = ( BigInt<bits, sign> val){
			_value = val.to_bool();
            return *this;
        }

        /**
         * @brief 使用float类型赋值
         *
         * @param [in] val   : float
         * @return  Bool& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Bool& operator = ( float f ){
            _value = f;
            return *this;
        }

        /**
         * @brief 使用double类型赋值
         *
         * @param [in] val   : double
         * @return  Bool& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Bool& operator = ( double val ){
            _value = val;
            return *this;
        }

        /**
         * @brief 使用const char *类型赋值
         *
         * @param [in] val   : const char *
         * @return  Bool& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Bool& operator = ( const char * val ){
            _value = (NULL != val && '\0' != val[0]);
            return *this;
        }

        /**
         * @brief 使用字符串类型赋值
         *
         * @param [in] val   : const string_type&
         * @return  Bool& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Bool& operator = ( const string_type& val ){
            _value = (val.c_str()[0] != '\0') ;
            return *this;
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
            return static_cast<signed char>(_value);
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
            return static_cast<unsigned char>(_value);
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
            return static_cast<signed short>(_value);
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
            return static_cast<unsigned short>(_value);
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
            return static_cast<signed int>(_value);
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
            return static_cast<unsigned int>(_value);
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
            return static_cast<signed long long>(_value);
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
            return static_cast<unsigned long long>(_value);
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

        //using default version for raw
        using IVar::operator =;
    private:
        bool _value;
    };

}}   //namespace bsl::var
#endif  //__BSL_VAR_BOOL_H__

/* vim: set ts=4 sw=4 sts=4 tw=100 */
