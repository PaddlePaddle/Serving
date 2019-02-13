/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: ConfigError.cpp,v 1.9 2010/04/13 09:59:41 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file ConfigError.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/24 14:22:03
 * @version $Revision: 1.9 $ 
 * @brief 
 *  
 **/
#include "ConfigError.h"

namespace comcfg{

	char ConfigError :: to_char(ErrCode * errCode)const{
		popError(errCode, NOSUCHKEY);
		return char();
	}

	unsigned char ConfigError :: to_uchar(ErrCode * errCode)const{
		popError(errCode, NOSUCHKEY);
		return (unsigned char)(0);
	}

	int16_t ConfigError :: to_int16(ErrCode * errCode)const{
		popError(errCode, NOSUCHKEY);
		return int16_t();
	}

	u_int16_t ConfigError :: to_uint16(ErrCode * errCode)const{
		popError(errCode, NOSUCHKEY);
		return u_int16_t();
	}

	int ConfigError :: to_int32(ErrCode * errCode)const{
		popError(errCode, NOSUCHKEY);
		return int();
	}

	u_int32_t ConfigError :: to_uint32(ErrCode * errCode)const{
		popError(errCode, NOSUCHKEY);
		return u_int32_t();
	}

	long long ConfigError :: to_int64(ErrCode * errCode)const{
		popError(errCode, NOSUCHKEY);
		return (long long)(0);
	}

	unsigned long long ConfigError :: to_uint64(ErrCode * errCode)const{
		popError(errCode, NOSUCHKEY);
		return (unsigned long long)(0);
	}

	float ConfigError :: to_float(ErrCode * errCode)const{
		popError(errCode, NOSUCHKEY);
		return float();
	}

	double ConfigError :: to_double(ErrCode * errCode)const{
		popError(errCode, NOSUCHKEY);
		return double();
	}
#if 1
	bsl_string ConfigError :: to_bsl_string(ErrCode * errCode)const{
		popError(errCode, NOSUCHKEY);
		return bsl_string();
	}
#endif
#if 0
	std_string ConfigError :: to_std_string(ErrCode * errCode)const{
		popError(errCode, NOSUCHKEY);
		return std_string();
	}
#endif
	str_t ConfigError :: to_raw_string(ErrCode * errCode)const{
		popError(errCode, NOSUCHKEY);
		return str_t();
	}
	const char * ConfigError :: to_cstr(ErrCode * errCode)const{
		popError(errCode, NOSUCHKEY);
		return NULL;
	}

	
	//----------------to_xxx with default value
	char ConfigError :: to_char(ErrCode * errCode, const char & def)const {
		if(errCode){
			*errCode  = NOSUCHKEY;
		}
		return def;
	}
	unsigned char ConfigError :: to_uchar(ErrCode * errCode, const unsigned char & def)const{
		if(errCode){
			*errCode  = NOSUCHKEY;
		}
		return def;
	}
	int16_t ConfigError :: to_int16(ErrCode * errCode, const int16_t & def)const{
		if(errCode){
			*errCode  = NOSUCHKEY;
		}
		return def;
	}
	u_int16_t ConfigError :: to_uint16(ErrCode * errCode, const u_int16_t & def)const{
		if(errCode){
			*errCode  = NOSUCHKEY;
		}
		return def;
	}
	int ConfigError :: to_int32(ErrCode * errCode, const int & def)const{
		if(errCode){
			*errCode  = NOSUCHKEY;
		}
		return def;
	}
	u_int32_t ConfigError :: to_uint32(ErrCode * errCode, const u_int32_t & def)const{
		if(errCode){
			*errCode  = NOSUCHKEY;
		}
		return def;
	}
	long long ConfigError :: to_int64(ErrCode * errCode, const long long & def)const{
		if(errCode){
			*errCode  = NOSUCHKEY;
		}
		return def;
	}
	unsigned long long ConfigError :: to_uint64(ErrCode * errCode, const unsigned long long & def)const{
		if(errCode){
			*errCode  = NOSUCHKEY;
		}
		return def;
	}
	float ConfigError :: to_float(ErrCode * errCode, const float & def)const{
		if(errCode){
			*errCode  = NOSUCHKEY;
		}
		return def;
	}
	double ConfigError :: to_double(ErrCode * errCode, const double & def)const{
		if(errCode){
			*errCode  = NOSUCHKEY;
		}
		return def;
	}
	bsl_string ConfigError :: to_bsl_string(ErrCode * errCode, const bsl_string & def)const{
		if(errCode){
			*errCode  = NOSUCHKEY;
		}
		return def;
	}
	str_t ConfigError :: to_raw_string(ErrCode * errCode, const str_t & def)const{
		if(errCode){
			*errCode  = NOSUCHKEY;
		}
		return def;
	}
	//获取C风格的字符串（常量）
	const char * ConfigError :: to_cstr(ErrCode * errCode, const char * def)const{
		if(errCode){
			*errCode  = NOSUCHKEY;
		}
		return def;
	}

	ErrCode ConfigError :: get_char(char * /*valueBuf*/)const{
		return NOSUCHKEY;
	}

	ErrCode ConfigError :: get_uchar(unsigned char * /*valueBuf*/)const{
		return NOSUCHKEY;
	}

	ErrCode ConfigError :: get_int16(int16_t * /*valueBuf*/)const{
		return NOSUCHKEY;
	}

	ErrCode ConfigError :: get_uint16(u_int16_t * /*valueBuf*/)const{
		return NOSUCHKEY;
	}

	ErrCode ConfigError :: get_int32(int * /*valueBuf*/)const{
		return NOSUCHKEY;
	}

	ErrCode ConfigError :: get_uint32(u_int32_t * /*valueBuf*/)const{
		return NOSUCHKEY;
	}

	ErrCode ConfigError :: get_int64(long long * /*valueBuf*/)const{
		return NOSUCHKEY;
	}

	ErrCode ConfigError :: get_uint64(unsigned long long * /*valueBuf*/)const{
		return NOSUCHKEY;
	}

	ErrCode ConfigError :: get_float(float * /*valueBuf*/)const{
		return NOSUCHKEY;
	}

	ErrCode ConfigError :: get_double(double * /*valueBuf*/)const{
		return NOSUCHKEY;
	}
#if 1
	ErrCode ConfigError :: get_bsl_string(bsl_string * /*valueBuf*/)const{
		return NOSUCHKEY;
	}
#endif
#if 0
	ErrCode ConfigError :: get_std_string(std_string * /*valueBuf*/)const{
		return NOSUCHKEY;
	}
#endif
	ErrCode ConfigError :: get_raw_string(str_t * /*valueBuf*/)const{
		return NOSUCHKEY;
	}
	ErrCode ConfigError :: get_cstr(char * /*valueBuf*/, size_t /*len*/)const{
		return NOSUCHKEY;
	}

	//--------------get_xx with default-----------

	ErrCode ConfigError :: get_char(char * valueBuf, const char & def)const{
		if (valueBuf) {
			*valueBuf = def;
		}
		return NOSUCHKEY;
	}

	ErrCode ConfigError :: get_uchar(unsigned char * valueBuf, const unsigned char & def)const{
		if (valueBuf) {
			*valueBuf = def;
		}
		return NOSUCHKEY;
	}

	ErrCode ConfigError :: get_int16(int16_t * valueBuf, const int16_t & def)const{
		if (valueBuf) {
			*valueBuf = def;
		}
		return NOSUCHKEY;
	}

	ErrCode ConfigError :: get_uint16(u_int16_t * valueBuf, const u_int16_t & def)const{
		if (valueBuf) {
			*valueBuf = def;
		}
		return NOSUCHKEY;
	}

	ErrCode ConfigError :: get_int32(int * valueBuf, const int & def)const{
		if (valueBuf) {
			*valueBuf = def;
		}
		return NOSUCHKEY;
	}

	ErrCode ConfigError :: get_uint32(u_int32_t * valueBuf, const u_int32_t & def)const{
		if (valueBuf) {
			*valueBuf = def;
		}
		return NOSUCHKEY;
	}

	ErrCode ConfigError :: get_int64(long long * valueBuf, const long long & def)const{
		if (valueBuf) {
			*valueBuf = def;
		}
		return NOSUCHKEY;
	}

	ErrCode ConfigError :: get_uint64(unsigned long long * valueBuf, const unsigned long long & def)const{
		if (valueBuf) {
			*valueBuf = def;
		}
		return NOSUCHKEY;
	}

	ErrCode ConfigError :: get_float(float * valueBuf, const float & def)const{
		if (valueBuf) {
			*valueBuf = def;
		}
		return NOSUCHKEY;
	}

	ErrCode ConfigError :: get_double(double * valueBuf, const double & def)const{
		if (valueBuf) {
			*valueBuf = def;
		}
		return NOSUCHKEY;
	}

	ErrCode ConfigError :: get_bsl_string(bsl_string * valueBuf, const bsl_string & def)const{
		if (valueBuf) {
			*valueBuf = def;
		}
		return NOSUCHKEY;
	}

	ErrCode ConfigError :: get_raw_string(str_t * valueBuf, const str_t & def)const{
		if (valueBuf) {
			*valueBuf = def;
		}
		return NOSUCHKEY;
	}

	ErrCode ConfigError :: get_cstr(char * valueBuf, size_t len, const char * def)const{
		if (valueBuf) {
			snprintf(valueBuf, len, "%s", def);
		}
		return NOSUCHKEY;
	}

	bsl::var::IVar& ConfigError :: to_IVar(bsl::ResourcePool * /*vpool*/, ErrCode* errCode)const{
		popError(errCode, NOSUCHKEY);
		return bsl::var::Null::null;
	}

	int ConfigError :: equals(const ConfigUnit & conf) const {
		if (CONFIG_ERROR_TYPE == conf.selfType()) {
			return 0;
		} else {
			return -1;
		}
		return -1;
	}

	int ConfigError :: set_value(const bsl_string & /*value*/, int except){
		if (except) {
			if (0 != ul_seterrbuf("%s", "")) {
				LOG(WARNING) << "ConfigUnit: failed to write error buffer";
			}
			popError(NULL, NOSUCHKEY);
		}
		return NOSUCHKEY;
	}
	int ConfigError :: copy_unit(const ConfigUnit & /*unit*/, int except) {
		if (except) {
			popError(NULL, NOSUCHKEY);
		}
		return ERROR;
	}
}




 









/* vim: set ts=4 sw=4 sts=4 tw=100 */
