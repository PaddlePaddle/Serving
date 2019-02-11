/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: Number.h,v 1.2 2010/04/28 12:45:33 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file Number.h
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/09/24 01:32:42
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/
#ifndef  __BSL_VAR_NUMBER_H__
#define  __BSL_VAR_NUMBER_H__

#include <cstdlib>
#include <climits>
#include "bsl/var/IVar.h"
#include "bsl/check_cast.h"
#include "bsl/var/var_traits.h"

namespace bsl{
namespace var{
    template<typename T>
    class Number: public IVar{
    public:
        typedef IVar::string_type   string_type;
        typedef IVar::field_type    field_type;

    public:
        //special methods
        Number( T __value = 0 ) :_value(__value){}

        Number( const Number& other )
            :IVar(other), _value( other._value ) {}

        Number& operator = ( const Number& other ){
            _value = other._value;
            return *this;
        }

        //methods for all
        virtual Number& operator = ( IVar& var ) {
            try{
                if ( var.is_double() ){
                    _value = check_cast<T>( var.to_double() );
                }else if ( var.is_uint64() ){
                    _value = check_cast<T>( var.to_uint64() );
                }else{
                    _value = check_cast<T>( var.to_int64() );
                }
                
            }catch(bsl::Exception& e){
                e<<"{"<<__PRETTY_FUNCTION__<<"("<<var.dump()<<")}";
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
         * @return  Number& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 17:58:07
        **/
        virtual Number& clone( bsl::ResourcePool& rp ) const {
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
         * @return  Number& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 17:58:07 / modified by zhujianwei at 2011/03/22
        **/
        virtual Number& clone( bsl::ResourcePool& rp, bool /*is_deep_copy*/ ) const {
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
            return  string_type("[bsl::var::Number<").append(typeid(T).name()).append(">]").append(check_cast<string_type>(_value));
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
            return  check_cast<string_type>(_value);
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
            return "bsl::var::Number";
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
            return var_traits<T>::MASK;
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
            return _value != 0;
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
            return check_cast<signed int>(_value);
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
         * @brief 转化为64位有符号整数类型（所有is_number()返回真的类型都支持）
         *
         * @return  signed long long 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 18:09:14
        **/
        virtual signed long long to_int64() const {
            return check_cast<signed long long>(_value);
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
         * @brief 转化为单精度浮点数类型（所有is_number()返回真的类型都支持）
         *
         * @return  float 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 18:09:14
        **/
        virtual float to_float() const {
            #ifdef __arm__
                if (typeid(T) == typeid(float) || typeid(T) == typeid(const float)) {
                    volatile uint32_t tmp = *(volatile uint32_t *)&_value;
                    return check_cast<float>(*(float *)&tmp);
                } else if (typeid(T) == typeid(double) || typeid(T) == typeid(const double)) {
                    volatile uint64_t tmp = *(volatile uint64_t *)&_value;
                    return check_cast<float>(*(double *)&tmp);
                } else {
                    return check_cast<float>(_value);
                }
            #else
            return check_cast<float>(_value);
            #endif
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
            #ifdef __arm__
                if (typeid(T) == typeid(float) || typeid(T) == typeid(const float)) {
                    volatile uint32_t tmp = *(volatile uint32_t *)&_value;
                    return check_cast<double>(*(float *)&tmp);
                } else if (typeid(T) == typeid(double) || typeid(T) == typeid(const double)) {
                    volatile uint64_t tmp = *(volatile uint64_t *)&_value;
                    return check_cast<double>(*(double *)&tmp);
                } else {
                    return check_cast<double>(_value);
                }
            #else
            return check_cast<double>(_value);
            #endif
        }

        virtual Number& operator = ( bool b ){
            _value = check_cast<T>(b);
            return *this;
        }

        /**
         * @brief 使用signed char类型赋值
         *
         * @param [in] val   : signed char
         * @return  Number& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Number& operator = ( signed char i ){
            _value = check_cast<T>(i);
            return *this;
        }

        /**
         * @brief 使用unsigned char类型赋值
         *
         * @param [in] val   : unsigned char
         * @return  Number& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Number& operator = ( unsigned char i ){
            _value = check_cast<T>(i);
            return *this;
        }

        /**
         * @brief 使用signed short类型赋值
         *
         * @param [in] val   : signed short
         * @return  Number& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Number& operator = ( signed short i ){
            _value = check_cast<T>(i);
            return *this;
        }

        /**
         * @brief 使用unsigned short类型赋值
         *
         * @param [in] val   : unsigned short
         * @return  Number& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Number& operator = ( unsigned short i ){
            _value = check_cast<T>(i);
            return *this;
        }

        /**
         * @brief 使用signed int类型赋值
         *
         * @param [in] val   : signed int
         * @return  Number& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Number& operator = ( signed int i ){
            _value = check_cast<T>(i);
            return *this;
        }

        /**
         * @brief 使用unsigned int类型赋值
         *
         * @param [in] val   : unsigned int
         * @return  Number& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Number& operator = ( unsigned int i ){
            _value = check_cast<T>(i);
            return *this;
        }

        /**
         * @brief 使用signed long long类型赋值
         *
         * @param [in] val   : signed long long
         * @return  Number& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Number& operator = ( signed long long i ){
            _value = check_cast<T>(i);
            return *this;
        }

        /**
         * @brief 使用unsigned long long类型赋值
         *
         * @param [in] val   : unsigned long long
         * @return  Number& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Number& operator = ( unsigned long long i ){
            _value = check_cast<T>(i);
            return *this;
        }

        /**
         * @brief 使用float类型赋值
         *
         * @param [in] val   : float
         * @return  Number& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Number& operator = ( float f ){
            _value = check_cast<T>(f);
            return *this;
        }

        /**
         * @brief 使用double类型赋值
         *
         * @param [in] val   : double
         * @return  Number& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Number& operator = ( double d ){
            _value = check_cast<T>(d);
            return *this;
        }

        /**
         * @brief 使用const char *类型赋值
         *
         * @param [in] val   : const char *
         * @return  Number& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Number& operator = ( const char * cstr ){
            _value = check_cast<T>(cstr);
            return *this;
        }

        /**
         * @brief 使用字符串类型赋值
         *
         * @param [in] val   : const string_type&
         * @return  Number& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Number& operator = ( const string_type& str ){
            _value = check_cast<T>(str.c_str());
            return *this;
        }

        //use default version for raw
        using IVar::operator =;

        //testers
        virtual bool is_number() const {
            return true;
        }

        virtual bool is_int8() const {
            return typeid(T) == typeid(signed char);
        }

        virtual bool is_uint8() const {
            return typeid(T) == typeid(unsigned char);
        }

        virtual bool is_int16() const {
            return typeid(T) == typeid(signed short);
        }

        virtual bool is_uint16() const {
            return typeid(T) == typeid(unsigned short);
        }

        virtual bool is_int32() const {
            return typeid(T) == typeid(signed int);
        }

        virtual bool is_uint32() const {
            return typeid(T) == typeid(unsigned int);
        }

        virtual bool is_int64() const {
            return typeid(T) == typeid(signed long long);
        }

        virtual bool is_uint64() const {
            return typeid(T) == typeid(unsigned long long);
        }

        virtual bool is_float() const {
            return typeid(T) == typeid(float);
        }

        virtual bool is_double() const {
            return typeid(T) == typeid(double);
        }

    private:
        T _value;
    };

}}   //namespace bsl::var

#endif  //__BSL_VAR_NUMBER_H__

/* vim: set ts=4 sw=4 sts=4 tw=100 */
