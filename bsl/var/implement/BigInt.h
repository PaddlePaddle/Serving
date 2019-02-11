/***************************************************************************
 * 
 * Copyright (c) 2013 Baidu.com, Inc. All Rights Reserved
 * $Id: BigInt.h,v 1.1.24 2013/07/19 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file BigInt.h
 * @brief BigInt in var
 * @author linjieqiong
 * @version 1.1.24
 * @date 2013-07-14
 */
#ifndef  __BSL_VAR_BIGINT_H__
#define  __BSL_VAR_BIGINT_H__

#include <cstdlib>
#include <climits>
#include <vector>
#include <deque>
#include <string.h>
#include "bsl/var/IVar.h"
#include "bsl/var/Ref.h"
#include "bsl/check_cast.h"

namespace bsl{
namespace var{

namespace {

#define HEX_CHARSET "1234567890abcdefABCDEF"

enum hex_machine_state{
	MSTART,
	MPREFIX,
	MDIGIT,
	MEND,
	MERROR,
};

enum base_type{
	BHEX,
	BDECIMAL,
	BBINARY,
	BUNREC,
};

}

template<size_t, bool>class BigInt;
typedef BigInt<1024, true> BigIntk;

template<size_t bits=1024, bool sign=true>
class BigInt : public IVar{
public:
    typedef IVar::string_type   string_type;
    typedef IVar::field_type    field_type;
	typedef unsigned char 		byte;
	typedef BigInt				Self;
	typedef std::deque<byte>	implement_t;
	typedef implement_t::const_iterator	const_imitr;
	template<size_t bits2, bool sign2>
	struct rebind {typedef BigInt<bits2, sign2> Other;};

public:
	BigInt(){
		clear();
	}
	BigInt(bool __value){
		clear();
		operator=(__value);
	}
	BigInt(int __value){
		clear();
		operator=(__value);
	}
	BigInt(long long __value){
		clear();
		operator=(__value);
	}
	BigInt(const char * __value){
		clear();
		operator=(__value);
	}
	BigInt(string_type __value){
		clear();
		operator=(__value);
	}
	BigInt(const BigInt & other)
		: IVar(other)
		, _value(other._value){
	}

	/**
	 * @brief copy constructor
	 *
	 * @param other
	 *
	 * @return 
	 *
	 * @version 1.1.24
	 * @author linjieqiong
	 * @date 2013/07/27 21:51:26
	 */
	BigInt & operator=(const BigInt & other){
		_value = other._value;
		return *this;
	}
	
	/**
	 * @brief 转换其他BigInt类型，本函数为check_cast
	 *
	 * @tparam bits2
	 * @tparam sign2
	 * @param other
	 *
	 * @version 1.1.24
	 * @author linjieqiong
	 * @date 2013/07/27 21:50:49
	 */
	template<size_t bits2, bool sign2>
	BigInt(const BigInt<bits2, sign2> & other)
		: IVar(other){
		operator=(other.to_string());
		if(sign == true && sign2 == false && (_value[0] & 0x80)){
			_value.push_front(0);
		}
		else if(sign == false && sign2 == true && (_value[0] & 0x80)){
			throw bsl::UnderflowException()<<BSL_EARG
				<<"signed to unsigned value["<<dump(); 
		}
	}
	
	/**
	 * @brief 	将其他IVar类型转换为BigInt
	 * @note	该函数使用有限，BigInt目前只能识别16进制形式的字符串
	 * 			其他情况将抛异常
	 *			并且，该转换是reinterpret_cast
	 *
	 * @param var
	 *
	 * @return 
	 *
	 * @version 1.1.24
	 * @author linjieqiong
	 * @date 2013/07/24 15:02:54
	 */
	BigInt & operator=(IVar & var){
    	try{
            string_type tmp = var.to_string();    //throw
			operator=(tmp); 
        }catch(bsl::Exception& e){
            e<<"{bsl::var::BigInt::operator =("
			 <<var.dump()<<"): can not be supported}";
            throw;
        }
        return *this;
    }

	/**
	 * @brief 
	 *
	 * @version 1.1.24
	 * @author linjieqiong
	 * @date 2013/07/19 16:19:02
	 */
	virtual void clear(){
		_value.clear();
	}

	/**
	 * @brief 
	 *
	 * @param rp
	 *
	 * @return 
	 *
	 * @version 1.1.24
	 * @author linjieqiong
	 * @date 2013/07/19 16:21:09
	 */
	virtual BigInt& clone(bsl::ResourcePool & rp) const {
		return rp.clone(*this);
	}

	/**
	 * @brief 
	 *
	 * @param rp
	 * @param bool
	 *
	 * @return 
	 *
	 * @version 1.1.24
	 * @author linjieqiong
	 * @date 2013/07/19 16:22:18
	 */
	virtual BigInt& clone(bsl::ResourcePool & rp, bool /*is_deep_copy*/) const {
		return rp.clone(*this);
	}

	/**
	 * @brief 
	 *
	 * @param 
	 *
	 * @return 
	 *
	 * @version 1.1.24
	 * @author linjieqiong
	 * @date 2013/07/19 16:28:50
	 */
    virtual string_type dump(size_t /*verbose_level*/ = 0) const {
		string_type val;
		val.setf("[%s]", get_type().c_str()).append(to_string());
		return val;
    }

	/**
	 * @brief 
	 *
	 * @return 
	 *
	 * @version 1.1.24
	 * @author linjieqiong
	 * @date 2013/07/19 16:30:02
	 */
    virtual string_type to_string() const {
		string_type tmp;
		tmp.append("0x");
		for(const_imitr itr=_value.begin(); itr!=_value.end(); ++itr){
			tmp.appendf("%02x", *itr);
		}
		return tmp;
    }

	/**
	 * @brief 
	 *
	 * @return 
	 *
	 * @version 1.1.24
	 * @author linjieqiong
	 * @date 2013/07/19 16:30:39
	 */
    virtual string_type get_type() const {
		string_type type;
		type.setf("bsl::var::BigInt<%lu, %s>", bits, sign?"signed":"unsigned");
		return type;
    }

	/**
	 * @brief 
	 *
	 * @return 
	 *
	 * @version 1.1.24
	 * @author linjieqiong
	 * @date 2013/07/19 16:32:01
	 */
    virtual IVar::mask_type get_mask() const {
		if(sign == true){
        	return IVar::IS_NUMBER | IVar::IS_BIG_INT | IVar::IS_SIGNED;
		}
		return IVar::IS_NUMBER | IVar::IS_BIG_INT;
    }

	/**
	 * @brief 
	 *
	 * @return 
	 *
	 * @version 1.1.24
	 * @author linjieqiong
	 * @date 2013/07/19 16:34:36
	 */
    virtual bool to_bool() const {
		size_t sz = _value.size();
		if(sz == 0){
			return false;
		}
		size_t idx = 0;
		if(sign == true){
			if((_value[0] & 0x7f) != 0){
				return true;
			}
			idx ++;
		}
		for(; idx<sz; ++idx){
			if(_value[idx] != 0){
				return true;
			}
		}
		return false;
    }
	
	/**
	 * @brief 
	 *
	 * @return 
	 *
	 * @version 1.1.24
	 * @author linjieqiong
	 * @date 2013/07/19 16:35:53
	 */
    virtual signed char to_int8() const {
		return check_cast_signed<signed char>();
    }

	/**
	 * @brief 
	 *
	 * @return 
	 *
	 * @version 1.1.24
	 * @author linjieqiong
	 * @date 2013/07/19 16:35:53
	 */
    virtual unsigned char to_uint8() const {
		return check_cast_unsigned<unsigned char>();
    }

	/**
	 * @brief 
	 *
	 * @return 
	 *
	 * @version 1.1.24
	 * @author linjieqiong
	 * @date 2013/07/19 16:36:01
	 */
    virtual signed short to_int16() const {
		return check_cast_signed<signed short>();
    }

	/**
	 * @brief 
	 *
	 * @return 
	 *
	 * @version 1.1.24
	 * @author linjieqiong
	 * @date 2013/07/19 16:36:01
	 */
    virtual unsigned short to_uint16() const {
		return check_cast_unsigned<unsigned short>();
    }

	/**
	 * @brief 
	 *
	 * @return 
	 *
	 * @version 1.1.24
	 * @author linjieqiong
	 * @date 2013/07/19 16:36:57
	 */
    virtual signed int to_int32() const {
		return check_cast_signed<signed int>();
    }

	/**
	 * @brief 
	 *
	 * @return 
	 *
	 * @version 1.1.24
	 * @author linjieqiong
	 * @date 2013/07/19 16:37:08
	 */
    virtual unsigned int to_uint32() const {
        return check_cast_unsigned<unsigned int>();
    }

	/**
	 * @brief 
	 *
	 * @return 
	 *
	 * @version 1.1.24
	 * @author linjieqiong
	 * @date 2013/07/19 16:37:21
	 */
    virtual signed long long to_int64() const {
		return check_cast_signed<signed long long>();
    }

	/**
	 * @brief 
	 *
	 * @return 
	 *
	 * @version 1.1.24
	 * @author linjieqiong
	 * @date 2013/07/19 16:37:41
	 */
    virtual unsigned long long to_uint64() const {
        return check_cast_unsigned<unsigned long long>();
    }

	///**
	// * @brief 
	// *
	// * @return 
	// *
	// * @version 1.1.24
	// * @author linjieqiong
	// * @date 2013/07/19 16:37:50
	// */
    //virtual float to_float() const {
    //    return static_cast<float>(_value);
    //}

	///**
	// * @brief 
	// *
	// * @return 
	// *
	// * @version 1.1.24
	// * @author linjieqiong
	// * @date 2013/07/19 16:38:04
	// */
    //virtual double to_double() const {
    //    return check_cast<double>(_value);
    //}

    virtual BigInt& operator = ( bool b ){
		clear();
		(b == true) ? _value.push_back(1) : _value.push_back(0);
        return *this;
    }

	/**
	 * @brief 
	 *
	 * @param i
	 *
	 * @return 
	 *
	 * @version 1.1.24
	 * @author linjieqiong
	 * @date 2013/07/19 17:01:08
	 */
    virtual BigInt& operator = ( signed char i ){
		assign_signed(i);
        return *this;
    }

	/**
	 * @brief 
	 *
	 * @param i
	 *
	 * @return 
	 *
	 * @version 1.1.24
	 * @author linjieqiong
	 * @date 2013/07/19 17:01:26
	 */
    virtual BigInt& operator = ( unsigned char i ){
		assign_unsigned(i);
        return *this;
    }

	/**
	 * @brief 
	 *
	 * @param i
	 *
	 * @return 
	 *
	 * @version 1.1.24
	 * @author linjieqiong
	 * @date 2013/07/19 17:01:37
	 */
    virtual BigInt& operator = ( signed short i ){
		assign_signed(i);
        return *this;
    }

	/**
	 * @brief 
	 *
	 * @param i
	 *
	 * @return 
	 *
	 * @version 1.1.24
	 * @author linjieqiong
	 * @date 2013/07/19 17:01:48
	 */
    virtual BigInt& operator = ( unsigned short i ){
		assign_unsigned(i);
        return *this;
    }

	/**
	 * @brief 
	 *
	 * @param i
	 *
	 * @return 
	 *
	 * @version 1.1.24
	 * @author linjieqiong
	 * @date 2013/07/19 17:02:00
	 */
    virtual BigInt& operator = ( signed int i ){
		assign_signed(i);
        return *this;
    }

	/**
	 * @brief 
	 *
	 * @param i
	 *
	 * @return 
	 *
	 * @version 1.1.24
	 * @author linjieqiong
	 * @date 2013/07/19 17:02:09
	 */
    virtual BigInt& operator = ( unsigned int i ){
		assign_unsigned(i);
        return *this;
    }

    virtual BigInt& operator = ( long long ll ){
	 	assign_signed(ll);
        return *this;
    }

	/**
	 * @brief 
	 *
	 * @param d
	 *
	 * @return 
	 *
	 * @version 1.1.24
	 * @author linjieqiong
	 * @date 2013/07/19 17:02:23
	 */
    virtual BigInt& operator = ( unsigned long long d ){
		assign_unsigned(d);
        return *this;
    }

	///**
	// * @brief 
	// *
	// * @param f
	// *
	// * @return 
	// *
	// * @version 1.1.24
	// * @author linjieqiong
	// * @date 2013/07/19 17:02:33
	// */
    //virtual BigInt& operator = ( float f ){
    //    _value = check_cast<long long>(f);
    //    return *this;
    //}

	///**
	// * @brief 
	// *
	// * @param d
	// *
	// * @return 
	// *
	// * @version 1.1.24
	// * @author linjieqiong
	// * @date 2013/07/19 17:02:47
	// */
    //virtual BigInt& operator = ( double d ){
    //    _value = check_cast<long long>(d);
    //    return *this;
    //}
	
	/**
	 * @brief 将数字字符串转成BigInt
	 * @note 目前只有十六进制形式的字符串能被识别
	 * 			e.g.: "0x01234fad", "0XFFFFF12"
	 *
	 * @param cstr
	 *
	 * @return 
	 *
	 * @version 1.1.24
	 * @author linjieqiong
	 * @date 2013/07/19 17:03:14
	 */
    virtual BigInt& operator = ( const char * cstr ){
		if(cstr == NULL){
			throw bsl::NullPointerException()<<BSL_EARG
					<<"can not convert a null pointer into BigInt.";
		}
		clear();
		int base = BUNREC;
		const char *digit_pos = GetBase(cstr, base);
		switch(base){
			case BHEX:
				check_hex_convert(digit_pos);
				break;
			case BDECIMAL:
			case BBINARY:
			case BUNREC:
			default:
				throw bsl::BadFormatStringException()
						<<BSL_EARG<<"unrecognizable digit string: "<<cstr;
		}
        return *this;
    }

	/**
	 * @brief 
	 *
	 * @param str
	 *
	 * @return 
	 *
	 * @version 1.1.24
	 * @author linjieqiong
	 * @date 2013/07/19 17:03:25
	 */
    virtual BigInt& operator = ( const string_type& str ){
        return operator=(str.c_str());
    }

    //use default version for bool、raw
    using IVar::operator =;

    //testers
    virtual bool is_number() const {
        return true;
    }

    virtual bool is_bigint() const {
        return true;
    }

	bool is_overflow() const {
		return _value.size() * 8 > bits;
	}

protected:
	template<class T>
	T check_cast_signed()const{
		const size_t byte_limit = sizeof(T);
		const size_t value_size = _value.size();
		if(value_size == 0){
			throw bsl::BadCastException()<<BSL_EARG<<get_type()<<" has no value"; 
		}
		bool nsign = static_cast<bool>(_value[0] & 0x80);
		if( value_size > byte_limit ||
			(value_size == byte_limit && sign == false && nsign == true)){
			throw bsl::OverflowException()<<BSL_EARG<<"value["<<to_string()
					<<"] bound["<<byte_limit<<"-byte signed integer]"; 
		}
		T tmp = _value[0];
		for(size_t i=1; i<value_size; ++i){
			tmp = (tmp << 8) | _value[i];
		}
		return tmp;
	}

	template<class T>
	T check_cast_unsigned()const{
		const size_t byte_limit = sizeof(T);
		const size_t value_size = _value.size();
		if(value_size == 0){
			throw bsl::BadCastException()<<BSL_EARG<<get_type()<<" has no value"; 
		}
		bool nsign = static_cast<bool>(_value[0] & 0x80);
		if(sign == true && nsign == true){
			throw bsl::UnderflowException()<<BSL_EARG<<"value["<<to_string()
					<<"] bound["<<byte_limit<<"-byte unsigned integer]"; 
		}
		if(value_size > byte_limit){
			throw bsl::OverflowException()<<BSL_EARG<<"value["<<to_string()
					<<"] bound["<<byte_limit<<"-byte unsigned integer]"; 
		}
		T tmp = _value[0];
		for(size_t i=1; i<value_size; ++i){
			tmp = (tmp << 8) | _value[i];
		}
		return tmp;
	}

	template<class T>
	void assign_signed(T num){
		clear();
		size_t byte_size = sizeof(num);
		for(size_t i=0; i<byte_size-1; ++i){
			_value.push_front(num & 0xff);
			num >>= 8;
		}
		bool nsign = static_cast<bool>(num & 0x80);
		if(sign == false && nsign == true){
			clear();
			throw bsl::UnderflowException()<<BSL_EARG<<"value["<<num
					<<"] bound[unsigned bigint]"; 
		}
		_value.push_front(num & 0xff);
	}

	template<class T>
	void assign_unsigned(T num){
		clear();
		size_t byte_size = sizeof(num);
		for(size_t i=0; i<byte_size-1; ++i){
			_value.push_front(num & 0xff);
			num >>= 8;
		}
		_value.push_front(num & 0xff);
		bool highest_bit = static_cast<bool>(num & 0x80);
		if(highest_bit == true && sign == true){
			_value.push_front(0);
		}
	}

	/**
	 * @brief get the base of a digit string, 
	 *     only hex string will be recognized currently
	 *
	 * @param str the input string
	 * @param base the base of the digit string
	 *
	 * @return if recognized, the prefix of the @str will be trimed
	 *
	 * @version 1.1.24
	 * @author linjieqiong
	 * @date 2013/07/24 10:38:05
	 */
	const char * GetBase(const char *str, int &base){
		const char * dpos = str;
		int state = MSTART;
		while(*dpos != 0){
			switch(state){
			case MSTART:
				state = MPREFIX;
				break;
			case MPREFIX:
				if(*dpos == '0'){
					if(tolower(*(dpos+1)) == 'x'){
						dpos += 2;
						state = MEND;
					}
					else{
						state = MERROR;
					}
				}
				else{
					state = MERROR;
				}
				break;
			case MEND:
				base = BHEX;
				return dpos;
			case MERROR:
			default:
				base = BUNREC;
				return NULL;
			}
		}
		base = BUNREC;
		return NULL;
	}

	/**
	 * @brief convert a hex string to values and push into container
	 *
	 * @param digits
	 *
	 * @version 1.1.24
	 * @author linjieqiong
	 * @date 2013/07/24 10:37:05
	 */
	void check_hex_convert(const char * digits){
		int idx = strlen(digits) - 1;
		while(idx >= 0){
			int hhalf=0, lhalf=0;
			lhalf = check_hex_value(digits, idx);
			-- idx;
			if(idx >= 0){
				hhalf = check_hex_value(digits, idx);
				-- idx;
			}
			_value.push_front(hhalf<<4 | lhalf);	
		}
	}

	/**
	 * @brief 
	 *
	 * @param digits a hex string, eg: 0abc13df
	 * @param idx the index of a hex char in @digits
	 *
	 * @return 
	 *
	 * @version 1.1.24
	 * @author linjieqiong
	 * @date 2013/07/24 10:35:10
	 */
	int check_hex_value(const char * digits, const int idx){
		char digit = digits[idx];
		char *pos = strchr(HEX_CHARSET, digit);
		if(pos != NULL){
			return (pos < HEX_CHARSET + 10) ? 
					digit - '0' : tolower(digit) - 'a' + 10;
		}
		else{
			throw bsl::BadFormatStringException()
					<<BSL_EARG<<"unrecognizable hex string: "<<digits;
		}
	}


private:
	implement_t _value;	//in binary format, not readable

};

}
}

#endif
