/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: cc_default.cpp,v 1.13 2009/11/10 04:50:30 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file cc_default.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/29 01:43:14
 * @version $Revision: 1.13 $ 
 * @brief 
 *  
 **/

#include "cc_default.h"
#include "utils/cc_utils.h"
#include "Configure.h"
#include "ConfigUnit.h"
#include "ConfigGroup.h"
#include "ConfigArray.h"
#include "spreg.h"
namespace comcfg{

	extern str_t showIDLMsg(const confIDL::meta_t& meta);

	int CFdefault :: cons_do_nothing(const confIDL::cons_func_t& /*func*/, const confIDL::var_t & /*var*/, ConfigGroup * /*father*/, ConfigUnit * /*self*/){
		return 0;
	}
	int CFdefault :: cons_array(const confIDL::cons_func_t& func, const confIDL::var_t & var, ConfigGroup * father, ConfigUnit * self){
		LOG(INFO) << "In constraint function array(), var is " << showIDLMsg(var.name).c_str();
		if(func.arg_list.size() > 1){
			LOG(WARNING) << "Constraint : Too many args for array(), in " << showIDLMsg(var.name).c_str();
			return -1;
		}
		const ConfigUnit * unit = self;
		if(unit == NULL){
			unit = & (*father)[var.name.data];
		}
		if(unit->selfType() == CONFIG_ERROR_TYPE){
			if(father == NULL){
				LOG(WARNING) << "Constraint : Unknown error in array(). " << showIDLMsg(var.name).c_str();
				return -1;
			}
			father->push(var.name.data, new ConfigArray(var.name.data.c_str(), father));
			LOG(INFO) << "Constraint : create array, in " << showIDLMsg(var.name).c_str();
		}
		else if(unit->selfType() != CONFIG_ARRAY_TYPE){
			LOG(WARNING) << "Constraint : variable exists but is not array. " << showIDLMsg(var.name).c_str();
			return -1;
		}
		else{
			if(func.arg_list.size() == 0){
				return 0;
			}
			unsigned long long asize;
			if(Trans::str2uint64(func.arg_vec[0]->data, &asize) != 0 ){
				LOG(WARNING) << "Constraint : array size is invalid. Ignored! " << showIDLMsg(var.name).c_str();
				return -1;
			}
			else{
				if(asize == 0){
					return 0;
				}
				if(asize < unit->size()){
					LOG(WARNING) << "Constraint : array has too many elements! " << showIDLMsg(var.name).c_str() << " [MaxSize=" << asize << "][We have " << unit->size() << " elements]";
					return -1;
				}
			}
		}
		return 0;
	}

	int CFdefault :: cons_default(const confIDL::cons_func_t& func, const confIDL::var_t & var, ConfigGroup * father, ConfigUnit * self){
		LOG(INFO) << "In constraint function default(), var is " << showIDLMsg(var.name).c_str();
		if(func.arg_list.size() != 1){
			LOG(WARNING) << "Constraint : wrong args for default(), in " << showIDLMsg(var.name).c_str();
			return -1;
		}
		const ConfigUnit * unit = self;
		if(unit == NULL){
			unit = & (*father)[var.name.data];
		}
		if(unit->selfType() == CONFIG_ERROR_TYPE){
			if(father == NULL){
				LOG(WARNING) << "Constraint : Unknown error in default(). " << showIDLMsg(var.name).c_str();
				return -1;
			}
			father->push(var.name.data, new ConfigUnit(var.name.data.c_str(), func.arg_vec[0]->data, NULL, father));
			LOG(INFO) << "Constraint : create default key, use " << showIDLMsg(var.name).c_str();
			return 0;
		}
		return 0;
	}

	int CFdefault :: cons_length(const confIDL::cons_func_t& func, const confIDL::var_t & var, ConfigGroup * father, ConfigUnit * self){
		LOG(INFO) << "In constraint function length(), var is " << showIDLMsg(var.name).c_str();
		if(func.arg_list.size() != 1){
			LOG(WARNING) << "Constraint : wrong args for length(), in " << showIDLMsg(var.name).c_str();
			return -1;
		}
		unsigned long long asize;
		if(Trans::str2uint64(func.arg_vec[0]->data, &asize) != 0 ){
			LOG(WARNING) << "Constraint : wrong args for length(). " << showIDLMsg(var.name).c_str();
			return -1;
		}
		const ConfigUnit * unit = self;
		if(unit == NULL){
			unit = & (*father)[var.name.data];
		}
		if(unit->selfType() == CONFIG_UNIT_TYPE){
			str_t ref = unit->to_bsl_string();
			if( ref.size() > asize){
				LOG(WARNING) << "Constraint : length() : value is too long. " << unit->info().c_str() << " : " << showIDLMsg(var.name).c_str();
				return -1;
			}
		}
		return 0;
	}

	int CFdefault :: cons_enum(const confIDL::cons_func_t& func, const confIDL::var_t & var, ConfigGroup * father, ConfigUnit * self){
		LOG(INFO) << "In constraint function enum(), var is " << showIDLMsg(var.name).c_str();
		const ConfigUnit * unit = self;
		if(unit == NULL){
			unit = & (*father)[var.name.data];
		}
		if(unit->selfType() == CONFIG_UNIT_TYPE){
			str_t str = unit->to_bsl_string();
			str_t buf;
			for( int i = 0; i < (int)func.arg_vec.size(); ++i){
				//printf("ENUM: %s\n", func.arg_vec[i]->data.c_str());
				if( Trans::str2str(func.arg_vec[i]->data, &buf) != 0 ){
					buf = func.arg_vec[i]->data;
				}
				if(str == buf){
					return 0;
				}
			}
		}
		LOG(WARNING) << "Constraint : key not in enum. " << unit->info().c_str() << " : " << showIDLMsg(var.name).c_str();
		return -1;
	}

	int CFdefault :: cons_regexp(const confIDL::cons_func_t& func, const confIDL::var_t & var, ConfigGroup * father, ConfigUnit * self){
		LOG(INFO) << "In constraint function regexp(), var is " << showIDLMsg(var.name).c_str();
		if(func.arg_list.size() != 1){
			LOG(WARNING) << "Constraint : wrong args for regexp(), in " << showIDLMsg(var.name).c_str();
			return -1;
		}
		char const * err;
		str_t reg;
		spreg_t *re;
		int ret = Trans::str2str(func.arg_vec[0]->data, &reg);
		re = spreg_init(reg.c_str(), &err);
		if(ret != 0 || err != NULL){
			LOG(WARNING) << "Constraint : wrong args for regexp(). " << showIDLMsg(var.name).c_str();
			return -1; // init failed, do not need to destroy spreg_t
		}

		const ConfigUnit * unit = self;
		if(unit == NULL){
			unit = & (*father)[var.name.data];
		}
		if(unit->selfType() == CONFIG_UNIT_TYPE){
			str_t str = unit->to_bsl_string();
			if( spreg_match(re, str.c_str(), str.size()) > 0){
				ret = 0;
				goto end;
			}
			else{
				LOG(WARNING) << "Constraint : key not match regexp(). " << unit->info().c_str() << " : " << showIDLMsg(var.name).c_str();
				ret = -1;
				goto end;
			}
		}
		ret = 0;
end:
		spreg_destroy(re);
		return ret;
	}

	int CFdefault :: cons_range(const confIDL::cons_func_t& func, const confIDL::var_t & var, ConfigGroup * father, ConfigUnit * self){
		LOG(INFO) << "In constraint function range(), var is " << showIDLMsg(var.name).c_str() << ", type=" << var.type.data.c_str();
		if(func.arg_list.size() != 2){
			LOG(WARNING) << "Constraint : wrong args for range(), in " << showIDLMsg(var.name).c_str();
			return -1;
		}
		const ConfigUnit * unit = self;
		if(unit == NULL){
			unit = & (*father)[var.name.data];
		}
		if(unit->selfType() == CONFIG_UNIT_TYPE){
			if(var.type.data == CC_FLOAT || var.type.data == CC_DOUBLE){
				double val = unit->to_double();
				double buf1, buf2;
				if( Trans::str2double(func.arg_vec[0]->data, &buf1) || 
						Trans::str2double(func.arg_vec[1]->data, &buf2)){
					LOG(WARNING) << "Constraint : wrong args in range(), in " << showIDLMsg(var.name).c_str();
					return -1;
				}
				if(val < buf1 || val > buf2){
					LOG(WARNING) << "Constraint : Key not in range(), in " << unit->info().c_str() << " : " << showIDLMsg(var.name).c_str();
					return -1;
				}
			}
			else if(var.type.data == CC_UINT64){
				unsigned long long val = unit->to_uint64();
				unsigned long long buf1, buf2;
				if( Trans::str2uint64(func.arg_vec[0]->data, &buf1) || 
						Trans::str2uint64(func.arg_vec[1]->data, &buf2)){
					LOG(WARNING) << "Constraint : wrong args in range(), in " << showIDLMsg(var.name).c_str();
					return -1;
				}
				if(val < buf1 || val > buf2){
					LOG(WARNING) << "Constraint : Key not in range(), in " << unit->info().c_str() << " : " << showIDLMsg(var.name).c_str();
					return -1;
				}
			}
			else if(var.type.data == CC_STRING){
				LOG(WARNING) << "Constraint : wrong usage of range(), in " << showIDLMsg(var.name).c_str();
				return -1;
			}
			else{// use int64 is enough
				long long val = unit->to_int64();
				long long buf1, buf2;
				if( Trans::str2int64(func.arg_vec[0]->data, &buf1) || 
						Trans::str2int64(func.arg_vec[1]->data, &buf2)){
					LOG(WARNING) << "Constraint : wrong args in range(), in " << showIDLMsg(var.name).c_str();
					return -1;
				}
				if(val < buf1 || val > buf2){
					LOG(WARNING) << "Constraint : Key not in range(), in " << unit->info().c_str() << " : " << showIDLMsg(var.name).c_str();
					return -1;
				}
			}
		}
		return 0;
	}



	//ip    :   ***.***.***.***
	//status: 0  1 2 3 4 5 6 7 
	static bool check_ip(const str_t& str, int pos, int status, int val){
		if(status > 7){
			return false;
		}
		if(pos == (int)str.size()){
			return status == 7;
		}
		if(str[pos] == '.'){
			if(status % 2 == 0){
				LOG(WARNING) << "Unexpected '.' in ip [" << str.c_str() << "]";
				return false;
			}
			return check_ip(str, pos+1, status+1, 0);
		}
		else if (isdigit(str[pos])){
			if(status % 2 == 0){
				return check_ip(str, pos+1, status+1, str[pos] - '0');
			}
			else{
				val = val * 10 + str[pos] - '0';
				if(val > 255) {
					LOG(WARNING) << "Unexpected digits in ip [" << str.c_str() << "]";
					return false;
				}
				return check_ip(str, pos+1, status, val);
			}
		}
		else{
			LOG(WARNING) << "Unexpected char '" <<str[pos] << "' in ip [" << str.c_str() << "]";
			return false;
		}
	}

	int CFdefault :: cons_ip(const confIDL::cons_func_t& func, const confIDL::var_t & var, ConfigGroup * father, ConfigUnit * self){
		LOG(INFO) << "In constraint function ip(), var is " << showIDLMsg(var.name).c_str();
		if(func.arg_list.size() != 0){
			LOG(WARNING) << "Constraint : wrong args for ip(), in " << showIDLMsg(var.name).c_str();
			return -1;
		}
		const ConfigUnit * unit = self;
		if(unit == NULL){
			unit = & (*father)[var.name.data];
		}
		if(unit->selfType() == CONFIG_UNIT_TYPE){
			str_t str = unit->to_bsl_string();
			if(check_ip(str, 0, 0, 0)){
				return 0;
			}
			LOG(WARNING) << "Constraint : Key is not ip(), in " << unit->info().c_str() << " : " << showIDLMsg(var.name).c_str();
			return -1;
		}
		return 0;
	}

	int CFdefault :: gcons_regexp(const std::vector<str_t> &arg_vec, ConfigUnit *conf)
	{
		int cb_check_regexp(ConfigUnit *, void *);
		if(arg_vec.size() != 1){
			LOG(WARNING) << 
					"Constraint: wrong arg number " << arg_vec.size() << " for global constraint gcons_regexp(), 1 arg is required.";
			return -1;
		}
		str_t reg = arg_vec[0];
		char const *err;
		spreg_t *re = spreg_init(reg.c_str(), &err);
		if(err != NULL){
			LOG(WARNING) << "Constraint: wrong args for gcons_regexp(): " << reg.c_str();
			return -1;
		}
		int ret = conf->traverse_unit(cb_check_regexp, &re);
		spreg_destroy(re);
		return ret;
	}

	int CFdefault :: gcons_ip(const std::vector<str_t> &arg_vec, ConfigUnit *conf)
	{
		int cb_check_ip(ConfigUnit *, void *);
		if(arg_vec.size() != 0){
			LOG(WARNING) << 
					"Constraint: wrong arg number " << arg_vec.size() << " for global constraint gcons_ip(), no arg is required.";
			return -1;
		}
		return conf->traverse_unit(cb_check_ip, NULL);
	}
	
	int CFdefault ::  gcons_length(const std::vector<int> &arg_vec, ConfigUnit *conf)
	{
		std::vector<long long> long_vec(arg_vec.begin(), arg_vec.end());
		return gcons_length(long_vec, conf);
	}

	int CFdefault ::  gcons_length(const std::vector<long long> &arg_vec, ConfigUnit *conf)
	{
		int cb_check_length(ConfigUnit *, void *);
		if(arg_vec.size() != 1){
			LOG(WARNING) << 
					"Constraint: wrong arg number " << arg_vec.size() << " for global constraint gcons_length(), 1 arg is required.";
			return -1;
		}
		long long len_limit = arg_vec[0];
		return conf->traverse_unit(cb_check_length, &len_limit); 
	}

	int cb_check_regexp(ConfigUnit *unit, void *reg)
	{
		unit->setConstrainted(true);
		if(unit->selfType() != CONFIG_UNIT_TYPE){
			return 0;
		}
		str_t str = unit->to_bsl_string();
		if(spreg_match(*(spreg_t **)reg, str.c_str(), str.size()) > 0){
			return 0;
		}
		int errcode = 0;
		LOG(WARNING) << 
				"Constraint: gcons_regexp() [key : value]:[" << unit->get_key_name().c_str() << " : " << unit->to_cstr(&errcode, "ERROR") << "]: value does not match regexp.";
		return -1;
	}

	int cb_check_ip(ConfigUnit *unit, void *)
	{
		unit->setConstrainted(true);
		if(unit->selfType() != CONFIG_UNIT_TYPE){
			return 0;
		}
		str_t str = unit->to_bsl_string();
		if(check_ip(str, 0, 0, 0)){
			return 0;
		}
		int errcode = 0;
		LOG(WARNING) << 
				"Constraint: gcons_ip() [key : value]:[" << unit->get_key_name().c_str() << " : " << unit->to_cstr(&errcode, "ERROR") << "]: value is not an IP string.";
		return -1;
	}

	int cb_check_default(ConfigUnit *unit, void *def_value)
	{
		unit->setConstrainted(true);
		if(unit->selfType() != CONFIG_UNIT_TYPE || !unit->to_bsl_string().empty()){
			return 0;
		}
		if(unit->set_value(*(str_t *)def_value) != 0){
			int errcode = 0;
			LOG(WARNING) << 
					"Constraint: gcons_default() [key : value]:[" << unit->get_key_name().c_str() << " : " << unit->to_cstr(&errcode, "ERROR") << "]: value is illegal."; 
			return -1;
		}
		return 0;
	}

	int cb_check_length(ConfigUnit *unit, void *limit)
	{
		unit->setConstrainted(true);
		if(unit->selfType() == CONFIG_UNIT_TYPE 
				&& (long long)(unit->to_bsl_string().size()) > *(long long*)(limit)){
			int errcode = 0;
			LOG(WARNING) << 
					"Constraint: gcons_length() [key : value]:[" << unit->get_key_name().c_str() << " : " << unit->to_cstr(&errcode, "ERROR") << "]: value length exceeds limit.";
			return -1;
		}
		return 0;
	}
	
	int cb_check_enum(ConfigUnit *unit, void *enum_vec)
	{
		unit->setConstrainted(true);
		if(unit->selfType() != CONFIG_UNIT_TYPE){
			return 0;
		}
		std::vector<str_t> &vec = *(std::vector<str_t> *)enum_vec;
		for(size_t i=0; i<vec.size(); ++i){
			if(unit->to_bsl_string() == vec[i]){
				return 0;
			}
		}
		int errcode = 0;
		LOG(WARNING) << 
				"Constraint: gcons_enum() [key : value]:[" << unit->get_key_name().c_str() << " : " <<unit->to_cstr(&errcode, "ERROR") << "]: value not in enum.";
		return -1;
	}
}















/* vim: set ts=4 sw=4 sts=4 tw=100 */
