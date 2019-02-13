/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: ConfigError.h,v 1.11 2010/04/13 09:59:41 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file ConfigUnit.h
 * @author yingxiang(com@baidu.com)
 * @date 2008/11/26 20:53:09
 * @version $Revision: 1.11 $ 
 * @brief 
 *  
 **/


#ifndef  __CONFIGERROR_H_
#define  __CONFIGERROR_H_

#include "utils/cc_utils.h"
#include "ConfigUnit.h"
namespace comcfg{

	class ConfigError : public ConfigUnit{
		public:

			virtual ~ConfigError(){};

			virtual char to_char(ErrCode * errCode = NULL)const;
			virtual unsigned char to_uchar(ErrCode * errCode = NULL)const;
			virtual int16_t to_int16(ErrCode * errCode = NULL)const;
			virtual u_int16_t to_uint16(ErrCode * errCode = NULL)const;
			virtual int to_int32(ErrCode * errCode = NULL)const;
			virtual u_int32_t to_uint32(ErrCode * errCode = NULL)const;
			virtual long long to_int64(ErrCode * errCode = NULL)const;
			virtual unsigned long long to_uint64(ErrCode * errCode = NULL)const;
			virtual float to_float(ErrCode * errCode = NULL)const;
			virtual double to_double(ErrCode * errCode = NULL)const;
			virtual bsl_string to_bsl_string(ErrCode * errCode = NULL)const;
			//virtual std_string to_std_string(ErrCode * errCode = NULL)const;
			virtual str_t to_raw_string(ErrCode * errCode = NULL)const;
			virtual const char * to_cstr(ErrCode * errCode = NULL)const;

			/**
			 * @brief 带有默认值的to_XXX系列函数
			 *        返回值与to_xxx系列完全相同。
			 *        唯一的不同是：当errCode不为0（即发生错误）, 返回def默认值
			 *
			 * @param [out] errCode   : ErrCode* 出错信息
			 * @param [in] def   : const &  默认值
			 * @return  
			**/
			virtual char to_char(ErrCode * errCode, const char & def)const;
			virtual unsigned char to_uchar(ErrCode * errCode, const unsigned char & def)const;
			virtual int16_t to_int16(ErrCode * errCode, const int16_t & def)const;
			virtual u_int16_t to_uint16(ErrCode * errCode, const u_int16_t & def)const;
			virtual int to_int32(ErrCode * errCode, const int & def)const;
			virtual u_int32_t to_uint32(ErrCode * errCode, const u_int32_t & def)const;
			virtual long long to_int64(ErrCode * errCode, const long long & def)const;
			virtual unsigned long long to_uint64(ErrCode * errCode, const unsigned long long & def)const;
			virtual float to_float(ErrCode * errCode, const float & def)const;
			virtual double to_double(ErrCode * errCode, const double & def)const;
			virtual bsl_string to_bsl_string(ErrCode * errCode, const bsl_string & def)const;
			virtual str_t to_raw_string(ErrCode * errCode, const str_t & def)const;
			//获取C风格的字符串（常量）
			virtual const char * to_cstr(ErrCode * errCode, const char * def)const;

			virtual ErrCode get_char(char * valueBuf)const;
			virtual ErrCode get_uchar(unsigned char * valueBuf)const;
			virtual ErrCode get_int16(int16_t * valueBuf)const;
			virtual ErrCode get_uint16(u_int16_t * valueBuf)const;
			virtual ErrCode get_int32(int * valueBuf)const;
			virtual ErrCode get_uint32(u_int32_t * valueBuf)const;
			virtual ErrCode get_int64(long long * valueBuf)const;
			virtual ErrCode get_uint64(unsigned long long * valueBuf)const;
			virtual ErrCode get_float(float * valueBuf)const;
			virtual ErrCode get_double(double * valueBuf)const;
			virtual ErrCode get_bsl_string(bsl_string * valueBuf)const;
			//virtual ErrCode get_std_string(std_string * valueBuf)const;
			virtual ErrCode get_raw_string(str_t * valueBuf) const;
			virtual ErrCode get_cstr(char * valueBuf, size_t len) const;

			/**
			 * @brief 带有默认值的ge_XXX系列函数
			 *        返回值与get_xxx系列完全相同。
			 *        唯一的不同是：当返回值不为0（即发生错误）valueBuf中使用def默认值
			 *
			 * @param [out] valueBuf   : char*  传出的值
			 * @param [in] def   : const char&  默认值
			 * @return  ErrCode 
			 * @retval   
			 * @see 
			**/
			virtual ErrCode get_char(char * valueBuf, const char & def)const;
			virtual ErrCode get_uchar(unsigned char * valueBuf, const unsigned char & def)const;
			virtual ErrCode get_int16(int16_t * valueBuf, const int16_t & def)const;
			virtual ErrCode get_uint16(u_int16_t * valueBuf, const u_int16_t & def)const;
			virtual ErrCode get_int32(int * valueBuf, const int & def)const;
			virtual ErrCode get_uint32(u_int32_t * valueBuf, const u_int32_t & def)const;
			virtual ErrCode get_int64(long long * valueBuf, const long long & def)const;
			virtual ErrCode get_uint64(unsigned long long * valueBuf, const unsigned long long & def)const;
			virtual ErrCode get_float(float * valueBuf, const float & def)const;
			virtual ErrCode get_double(double * valueBuf, const double & def)const;
			virtual ErrCode get_bsl_string(bsl_string * valueBuf, const bsl_string & def)const;
			virtual ErrCode get_raw_string(str_t * valueBuf, const str_t & def)const;
			virtual ErrCode get_cstr(char * valueBuf, size_t len, const char * def)const;

			virtual bsl::var::IVar& to_IVar(bsl::ResourcePool* vpool, ErrCode* errCode = NULL)const;

			virtual int selfType() const{
				return CONFIG_ERROR_TYPE;
			}
			virtual size_t size()const{
				return 0;
			}
 
			virtual str_t info() const{
				return str_t("[Error: No such key.]");
			}
			/**
			 * @brief 比较是否相等
			 *
			 * @return  int 0表示相等 -1不相等
			 * @author zhang_rui
			**/
			virtual int equals(const ConfigUnit & conf) const;
			/**
			 * @brief 删除一个unit
			 *
			 * @return  int 0 成功，其他是错误号
			 * @author zhang_rui
			**/
			virtual int set_value(const bsl_string & value, int except=0);
			/**
			 * @brief 复制一个unit, 深拷贝
			 *
			 *  将unit的内容复制到本对象。
			 * 类型需要匹配，只能从group复制到group，array复制到array
			 * @return  int 0 成功，其他是错误号
			 * @author zhang_rui
			**/
			virtual int copy_unit(const ConfigUnit & unit, int except=0);
			virtual  ConfigUnit & operator= (ConfigUnit & unit){
				return ConfigUnit::operator=(unit);
			}
		protected:
			//str_t key;
			//str_t value;
	};

}









#endif  //__CONFIGERROR_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
