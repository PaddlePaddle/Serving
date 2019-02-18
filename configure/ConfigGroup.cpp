/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: ConfigGroup.cpp,v 1.15 2010/04/13 09:59:41 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file ConfigGroup.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/21 18:10:28
 * @version $Revision: 1.15 $ 
 * @brief 
 *  
 **/

#include "ConfigGroup.h"
#include "ConfigArray.h"
#include "Configure.h"
#include "utils/cc_utils.h"
#include "bsl/var/Dict.h"

namespace comcfg{
	ConfigUnit * ConfigGroup :: relativeSection(ConfigUnit *globalSec, str_t str, int *depth){
		LOG(INFO) << "Config : [relativeSection]Current section : [" << getName() << "] depth[" << *depth << "]";
		if(str == ""){
			return this;
		}
		str_t child;
		size_t global_len = strlen(CONFIG_GLOBAL);
		if(str.substr(0, global_len) == str_t(CONFIG_GLOBAL) && 
				(str.size() == global_len || str[global_len] == '.')){
			ConfigGroup * sec = this;
			while(sec->getFather()){
				sec = sec->getFather();
			}
			(*depth) = 0;
			str = str.size() > global_len ? str.substr(global_len+1) : "";
			LOG(INFO) << "Config : switch to global [" << sec->getName() << "] <-- This must be GLOBAL";
			return sec->relativeSection(globalSec, str, depth);
		}
		for(int i = 0; i < (int)str.size(); ++i){
			if(str[i] == '.'){
				child = str.substr(0, i);
				str = str.substr(i+1);
				ptr_t ptr = fieldMap.find(child);
				if(NULL == ptr){
					LOG(WARNING) << "Configure : No previous section named [" << child.c_str() << "]";
					throw ConfigException();
				}
				++(*depth);
				return (ptr->second)->relativeSection(globalSec, str, depth);
			}
		}
		ConfigGroup *gSec = (ConfigGroup *)globalSec;
		ptr_t ptr = fieldMap.find(str);
		if(NULL == ptr){
			ConfigGroup * tmp = new ConfigGroup(str.c_str(), this);

			if((str[0] != '@') && !( (GROUP_DUP_LEVEL0 == gSec->_dupLevel) || (GROUP_DUP_LEVEL1==gSec->_dupLevel && 1<*depth))){
				struct dup_position dpos = {gSec->_at_file, gSec->_at_line,};
				dup_vector dvec;
				dvec.push_back(dpos);
				gSec->_dupMap.set(tmp, dvec, 1);
			}

			++(*depth);
			push(str, tmp);
			return tmp;
		}
		else{
			++(*depth);
			if((ptr->second)->selfType() != CONFIG_GROUP_TYPE){
				LOG(WARNING) << "Config : Duplicate key or group name [" << str.c_str() << "]";
				throw ConfigException();
			}
			else if(!( (GROUP_DUP_LEVEL0 == gSec->_dupLevel) || (GROUP_DUP_LEVEL1==gSec->_dupLevel && 1<*depth))){
				dup_ptr dtr = gSec->_dupMap.find(ptr->second);
				if(NULL == dtr){
					LOG(WARNING) << "Config : dup-map failed to find [" << str.c_str() << "]";
					throw ConfigException();
				}
				struct dup_position dpos = {gSec->_at_file, gSec->_at_line,};
				(dtr->second).push_back(dpos);
			}
			
			return ptr->second;
		}
	}
	void ConfigGroup :: create(const char * __name, ConfigGroup * __father){
		const char *p = __name;
		if(*p == '@'){
			++p;
		}
		while(*p != 0){
			if((!isalpha(*p)) && (!isdigit(*p)) && *p != '_'){
				LOG(WARNING) << "Configure : Unsupport  _name [" << __name << "]";
				throw ConfigException();
				//throw ConfigException("Unsupport section _name");
			}
			++p;
		}
		LOG(INFO) << "Config: this=" << this << ", Set _name = " << __name << ", _father = " << __father << "[" << (__father==NULL?"NULL":__father->getName()) <<"]"; 
		_name = __name;
		_father = __father;
		fieldMap.create(256);
		if(_father){
			_level = _father->getLevel() + 1;
		}
		else{
			_level = 0;
		}
	}

	int ConfigGroup :: push(const str_t& key, ConfigUnit* unit){
		LOG(INFO) << "Configure : Push key [" << key.c_str() << "] to [" << getName() << "]";
		if(key[0] != '@'){
			if(NULL == fieldMap.find(key)){
				fieldMap.set(key, unit, 1);
				fieldList.push_back(key);
			}
			else{
				LOG(WARNING) << "Configure : [" << getName() << "] Duplicate key [" << key.c_str() << "]";
				throw ConfigException();
				return -1;
			}
		}
		else{
			ptr_t ptr = fieldMap.find(key.substr(1));
			if(NULL != ptr){
				if(ptr->second->selfType() == CONFIG_ARRAY_TYPE){
					((ConfigArray*)(ptr->second))->push(key, unit);
				}
				else{
					LOG(WARNING) << "Configure : ambiguity key [" << key.c_str() << "]";
					throw ConfigException();
					return -1;
				}
			}
			else{
				ConfigArray * array = new ConfigArray(key.c_str() + 1, this);
				if(this->push(key.substr(1), array) == 0){
					//fieldList.push_back(key.substr(1));
					LOG(INFO) << "Config : we have new array [" << key.c_str() + 1 << "]";
					array->push(key, unit);
				}
			}
		}
		return 0;
	}

	const ConfigUnit & ConfigGroup :: operator[] (const char * str) const{
		return (*this)[str_t(str)];
	}
	const ConfigUnit & ConfigGroup :: operator[] (const str_t & str) const{
		ptr_t ptr = fieldMap.find(str);
		if(NULL != ptr){
			return *(ptr->second);
		}
		LOG(INFO) << "Configure: visiting key not found: ConfigGroup[str_t] : " << _name.c_str() << "[" << str.c_str() << "]";
		str_t tmp = this->_name;
		if (tmp == CONFIG_GLOBAL){
			tmp = str;
		} else {
			tmp.append(".").append(str);
		}
		setErrorKeyPath(tmp);
		return *get_err_unit();
	}
	const ConfigUnit & ConfigGroup :: operator[] (int idx) const{
		LOG(WARNING) << "Configure: visit ConfigGroup[int] : " << _name.c_str() << "[" << idx << "]";
		str_t tmp=this->_name;
		char tmpid[64];
		snprintf(tmpid,64, "%d",idx);
		tmp.append("[").append(tmpid).append("]");;
		setErrorKeyPath(tmp);
		return *get_err_unit();
	}
	ConfigUnit & ConfigGroup :: operator[] (const char * str) {
		return (*this)[str_t(str)];
	}
	ConfigUnit & ConfigGroup :: operator[] (const str_t & str) {
		ptr_t ptr = fieldMap.find(str);
		if(NULL != ptr){
			return *(ptr->second);
		}
		LOG(INFO) << "Configure: visiting key not found: ConfigGroup[str_t] : " << _name.c_str() << "[" << str.c_str() << "]";
		str_t tmp = this->_name;
		if (tmp == CONFIG_GLOBAL){
			tmp = str;
		} else {
			tmp.append(".").append(str);
		}
		setErrorKeyPath(tmp);
		return *get_err_unit();
	}
	ConfigUnit & ConfigGroup :: operator[] (int idx) {
		LOG(WARNING) << "Configure: visit ConfigGroup[int] : "<< _name.c_str() << "[" << idx << "]";
		str_t tmp=this->_name;
		char tmpid[64];
		snprintf(tmpid,64, "%d",idx);
		tmp.append("[").append(tmpid).append("]");;
		setErrorKeyPath(tmp);
		return *get_err_unit();
	}
	ConfigGroup :: ~ConfigGroup(){
		clear();
		if(0 != fieldMap.destroy())
			LOG(WARNING) << "Configure::~CofigGroup() : fieldMap.destory() failed ";
	}
	void ConfigGroup :: print(int indent) const{//for debug
		str_t sec;
		get_full_path(sec);
		pindent(indent);
		printf("Section-------[%s]\n", sec.c_str());
		for(int i = 0; i < (int)fieldList.size(); ++i){
			if(fieldMap.find(fieldList[i])-> second -> selfType() == CONFIG_UNIT_TYPE){
				fieldMap.find(fieldList[i]) -> second -> print(indent + 1);
			}
		}
		for(int i = 0; i < (int)fieldList.size(); ++i){
			if(fieldMap.find(fieldList[i]) -> second -> selfType() == CONFIG_ARRAY_TYPE){
				fieldMap.find(fieldList[i]) -> second -> print(indent + 1);
			}
		}
		for(int i = 0; i < (int)fieldList.size(); ++i){
			if(fieldMap.find(fieldList[i]) -> second -> selfType() == CONFIG_GROUP_TYPE){
				fieldMap.find(fieldList[i]) -> second -> print(indent + 1);
			}
		}
	}

	bsl::var::IVar& ConfigGroup :: to_IVar(bsl::ResourcePool * vpool, ErrCode* errCode)const{
		if(vpool == NULL){
			LOG(WARNING) << "Configure: visit ConfigGroup.to_IVar() failed : NULLBUFFER";
			setErrorKeyPath(this->_name);
			popError(errCode, NULLBUFFER);
			return bsl::var::Null::null;
		}
		bsl::var::Dict & dict = vpool->create<bsl::var::Dict>();
		for(int i = 0; i < (int)fieldList.size(); ++i){
			ConfigUnit * p = fieldMap.find(fieldList[i]) -> second;
			bsl::var::IVar & son = p->to_IVar(vpool, errCode);
			dict.set( fieldList[i], son );
		}
		return dict;
	}

	char ConfigGroup :: to_char(ErrCode * errCode)const{
		setErrorKeyPath(this->_name);
		popError(errCode, GROUPTOUNIT);
		return char();
	}

	unsigned char ConfigGroup :: to_uchar(ErrCode * errCode)const{
		setErrorKeyPath(this->_name);
		popError(errCode, GROUPTOUNIT);
		return (unsigned char)(0);
	}

	int16_t ConfigGroup :: to_int16(ErrCode * errCode)const{
		setErrorKeyPath(this->_name);
		popError(errCode, GROUPTOUNIT);
		return int16_t();
	}

	u_int16_t ConfigGroup :: to_uint16(ErrCode * errCode)const{
		setErrorKeyPath(this->_name);
		popError(errCode, GROUPTOUNIT);
		return u_int16_t();
	}

	int ConfigGroup :: to_int32(ErrCode * errCode)const{
		setErrorKeyPath(this->_name);
		popError(errCode, GROUPTOUNIT);
		return int();
	}

	u_int32_t ConfigGroup :: to_uint32(ErrCode * errCode)const{
		setErrorKeyPath(this->_name);
		popError(errCode, GROUPTOUNIT);
		return u_int32_t();
	}

	long long ConfigGroup :: to_int64(ErrCode * errCode)const{
		setErrorKeyPath(this->_name);
		popError(errCode, GROUPTOUNIT);
		return (long long)(0);
	}

	unsigned long long ConfigGroup :: to_uint64(ErrCode * errCode)const{
		setErrorKeyPath(this->_name);
		popError(errCode, GROUPTOUNIT);
		return (unsigned long long)(0);
	}

	float ConfigGroup :: to_float(ErrCode * errCode)const{
		setErrorKeyPath(this->_name);
		popError(errCode, GROUPTOUNIT);
		return float();
	}

	double ConfigGroup :: to_double(ErrCode * errCode)const{
		setErrorKeyPath(this->_name);
		popError(errCode, GROUPTOUNIT);
		return double();
	}
#if 1
	bsl_string ConfigGroup :: to_bsl_string(ErrCode * errCode)const{
		setErrorKeyPath(this->_name);
		popError(errCode, GROUPTOUNIT);
		return bsl_string();
	}
#endif
	str_t ConfigGroup :: to_raw_string(ErrCode * errCode)const{
		setErrorKeyPath(this->_name);
		popError(errCode, GROUPTOUNIT);
		return str_t();
	}
	const char * ConfigGroup :: to_cstr(ErrCode * errCode)const{
		setErrorKeyPath(this->_name);
		popError(errCode, GROUPTOUNIT);
		return NULL;
	}

	//----------------to_xxx with default value
	char ConfigGroup :: to_char(ErrCode * errCode, const char & def)const {
		if(errCode){
			*errCode  = GROUPTOUNIT;
		}
		setErrorKeyPath(this->_name);
		return def;
	}
	unsigned char ConfigGroup :: to_uchar(ErrCode * errCode, const unsigned char & def)const{
		setErrorKeyPath(this->_name);
		if(errCode){
			*errCode  = GROUPTOUNIT;
		}
		return def;
	}
	int16_t ConfigGroup :: to_int16(ErrCode * errCode, const int16_t & def)const{
		setErrorKeyPath(this->_name);
		if(errCode){
			*errCode  = GROUPTOUNIT;
		}
		return def;
	}
	u_int16_t ConfigGroup :: to_uint16(ErrCode * errCode, const u_int16_t & def)const{
		setErrorKeyPath(this->_name);
		if(errCode){
			*errCode  = GROUPTOUNIT;
		}
		return def;
	}
	int ConfigGroup :: to_int32(ErrCode * errCode, const int & def)const{
		setErrorKeyPath(this->_name);
		if(errCode){
			*errCode  = GROUPTOUNIT;
		}
		return def;
	}
	u_int32_t ConfigGroup :: to_uint32(ErrCode * errCode, const u_int32_t & def)const{
		setErrorKeyPath(this->_name);
		if(errCode){
			*errCode  = GROUPTOUNIT;
		}
		return def;
	}
	long long ConfigGroup :: to_int64(ErrCode * errCode, const long long & def)const{
		setErrorKeyPath(this->_name);
		if(errCode){
			*errCode  = GROUPTOUNIT;
		}
		return def;
	}
	unsigned long long ConfigGroup :: to_uint64(ErrCode * errCode, const unsigned long long & def)const{
		setErrorKeyPath(this->_name);
		if(errCode){
			*errCode  = GROUPTOUNIT;
		}
		return def;
	}
	float ConfigGroup :: to_float(ErrCode * errCode, const float & def)const{
		setErrorKeyPath(this->_name);
		if(errCode){
			*errCode  = GROUPTOUNIT;
		}
		return def;
	}
	double ConfigGroup :: to_double(ErrCode * errCode, const double & def)const{
		setErrorKeyPath(this->_name);
		if(errCode){
			*errCode  = GROUPTOUNIT;
		}
		return def;
	}
	bsl_string ConfigGroup :: to_bsl_string(ErrCode * errCode, const bsl_string & def)const{
		setErrorKeyPath(this->_name);
		if(errCode){
			*errCode  = GROUPTOUNIT;
		}
		return def;
	}
	str_t ConfigGroup :: to_raw_string(ErrCode * errCode, const str_t & def)const{
		setErrorKeyPath(this->_name);
		if(errCode){
			*errCode  = GROUPTOUNIT;
		}
		return def;
	}
	//获取C风格的字符串（常量）
	const char * ConfigGroup :: to_cstr(ErrCode * errCode, const char * def)const{
		setErrorKeyPath(this->_name);
		if(errCode){
			*errCode  = GROUPTOUNIT;
		}
		return def;
	}

	ErrCode ConfigGroup :: get_char(char * /*valueBuf*/)const{
		setErrorKeyPath(this->_name);
		return GROUPTOUNIT;
	}

	ErrCode ConfigGroup :: get_uchar(unsigned char * /*valueBuf*/)const{
		setErrorKeyPath(this->_name);
		return GROUPTOUNIT;
	}

	ErrCode ConfigGroup :: get_int16(int16_t * /*valueBuf*/)const{
		setErrorKeyPath(this->_name);
		return GROUPTOUNIT;
	}

	ErrCode ConfigGroup :: get_uint16(u_int16_t * /*valueBuf*/)const{
		setErrorKeyPath(this->_name);
		return GROUPTOUNIT;
	}

	ErrCode ConfigGroup :: get_int32(int * /*valueBuf*/)const{
		setErrorKeyPath(this->_name);
		return GROUPTOUNIT;
	}

	ErrCode ConfigGroup :: get_uint32(u_int32_t * /*valueBuf*/)const{
		setErrorKeyPath(this->_name);
		return GROUPTOUNIT;
	}

	ErrCode ConfigGroup :: get_int64(long long * /*valueBuf*/)const{
		setErrorKeyPath(this->_name);
		return GROUPTOUNIT;
	}

	ErrCode ConfigGroup :: get_uint64(unsigned long long * /*valueBuf*/)const{
		setErrorKeyPath(this->_name);
		return GROUPTOUNIT;
	}

	ErrCode ConfigGroup :: get_float(float * /*valueBuf*/)const{
		setErrorKeyPath(this->_name);
		return GROUPTOUNIT;
	}

	ErrCode ConfigGroup :: get_double(double * /*valueBuf*/)const{
		setErrorKeyPath(this->_name);
		return GROUPTOUNIT;
	}
#if 1
	ErrCode ConfigGroup :: get_bsl_string(bsl_string * /*valueBuf*/)const{
		setErrorKeyPath(this->_name);
		return GROUPTOUNIT;
	}
#endif
	ErrCode ConfigGroup :: get_raw_string(str_t * /*valueBuf*/)const{
		setErrorKeyPath(this->_name);
		return GROUPTOUNIT;
	}
	ErrCode ConfigGroup :: get_cstr(char * /*valueBuf*/, size_t /*len*/)const{
		setErrorKeyPath(this->_name);
		return GROUPTOUNIT;
	}

	//--------------get_xx with default-----------

	ErrCode ConfigGroup :: get_char(char * valueBuf, const char & def)const{
		setErrorKeyPath(this->_name);
		if (valueBuf) {
			*valueBuf = def;
		}
		return GROUPTOUNIT;
	}

	ErrCode ConfigGroup :: get_uchar(unsigned char * valueBuf, const unsigned char & def)const{
		setErrorKeyPath(this->_name);
		if (valueBuf) {
			*valueBuf = def;
		}
		return GROUPTOUNIT;
	}

	ErrCode ConfigGroup :: get_int16(int16_t * valueBuf, const int16_t & def)const{
		setErrorKeyPath(this->_name);
		if (valueBuf) {
			*valueBuf = def;
		}
		return GROUPTOUNIT;
	}

	ErrCode ConfigGroup :: get_uint16(u_int16_t * valueBuf, const u_int16_t & def)const{
		setErrorKeyPath(this->_name);
		if (valueBuf) {
			*valueBuf = def;
		}
		return GROUPTOUNIT;
	}

	ErrCode ConfigGroup :: get_int32(int * valueBuf, const int & def)const{
		setErrorKeyPath(this->_name);
		if (valueBuf) {
			*valueBuf = def;
		}
		return GROUPTOUNIT;
	}

	ErrCode ConfigGroup :: get_uint32(u_int32_t * valueBuf, const u_int32_t & def)const{
		setErrorKeyPath(this->_name);
		if (valueBuf) {
			*valueBuf = def;
		}
		return GROUPTOUNIT;
	}

	ErrCode ConfigGroup :: get_int64(long long * valueBuf, const long long & def)const{
		setErrorKeyPath(this->_name);
		if (valueBuf) {
			*valueBuf = def;
		}
		return GROUPTOUNIT;
	}

	ErrCode ConfigGroup :: get_uint64(unsigned long long * valueBuf, const unsigned long long & def)const{
		setErrorKeyPath(this->_name);
		if (valueBuf) {
			*valueBuf = def;
		}
		return GROUPTOUNIT;
	}

	ErrCode ConfigGroup :: get_float(float * valueBuf, const float & def)const{
		setErrorKeyPath(this->_name);
		if (valueBuf) {
			*valueBuf = def;
		}
		return GROUPTOUNIT;
	}

	ErrCode ConfigGroup :: get_double(double * valueBuf, const double & def)const{
		setErrorKeyPath(this->_name);
		if (valueBuf) {
			*valueBuf = def;
		}
		return GROUPTOUNIT;
	}

	ErrCode ConfigGroup :: get_bsl_string(bsl_string * valueBuf, const bsl_string & def)const{
		setErrorKeyPath(this->_name);
		if (valueBuf) {
			*valueBuf = def;
		}
		return GROUPTOUNIT;
	}

	ErrCode ConfigGroup :: get_raw_string(str_t * valueBuf, const str_t & def)const{
		setErrorKeyPath(this->_name);
		if (valueBuf) {
			*valueBuf = def;
		}
		return GROUPTOUNIT;
	}

	ErrCode ConfigGroup :: get_cstr(char * valueBuf, size_t len, const char * def)const{
		setErrorKeyPath(this->_name);
		if (valueBuf) {
			snprintf(valueBuf, len, "%s", def);
		}
		return GROUPTOUNIT;
	}

	const ConfigUnit & ConfigGroup :: get_sub_unit(int index) const {
		int susize = (int) fieldMap.size();
		if ((0 > index) || (index >= susize)) {
			return *get_err_unit();
		}
		ConfigUnit * p = fieldMap.find(fieldList[index]) -> second;
		if (NULL == p) {
			return *get_err_unit();
		}
		return *p;
	}

	int ConfigGroup :: equals(const ConfigUnit & conf) const {
		try {
			const_itr_t itr;
			for  (itr = fieldMap.begin();itr != fieldMap.end(); ++itr) {
				if (0 != itr->second->equals(conf[itr->first])) {
					return -1;
				}
			}
			return 0;
		} catch (bsl::Exception) {
			return -1;
		}
		return -1;
	}

	int ConfigGroup :: add_unit(const bsl_string & key, const bsl_string& value, 
			const int objtype, int except, ConfigUnit ** ref) {
		if (CONFIG_ERROR_TYPE == (*this)[key].selfType()) {
			ConfigUnit * tmp = create_unit(key, value, objtype, this);
			if (NULL == tmp){
				if (except) {
					popError(NULL, CONFIG_ERROR);
				}
				return CONFIG_ERROR;
			}
			this->push(key,tmp);
			if (NULL != ref) {
				*ref = tmp;
			}
		} else {
			if (except) {
				if (0 != ul_seterrbuf("%s", "")) {
					LOG(WARNING) << "Configure: failed to write error buffer";
				}
				popError(NULL, DUPLICATED_KEY);
			}
			return DUPLICATED_KEY;
		}
		return 0;
	}
	int ConfigGroup :: del_unit(const bsl_string & key, int except) {
		if (CONFIG_ERROR_TYPE != (*this)[key].selfType()) {
			std::vector <str_t > :: iterator iter;
			for ( iter = fieldList.begin(); iter != fieldList.end(); )
			{
				if ( *iter == key )
				{
					iter = fieldList.erase( iter );
					break;
				}
				iter++;
			}
			ConfigUnit * delu = NULL;
			ptr_t ptr = fieldMap.find(key);
			if(NULL != ptr){
				delu = ptr->second;
				fieldMap.erase(ptr->first);
			}
			if (delu) {
				delete delu;
			}
		} else {
			if (except) {
				if (0 != ul_seterrbuf("%s", "")) {
					LOG(WARNING) << "Configure: failed to write error buffer";
				}
				popError(NULL, NOSUCHKEY);
			}
			return NOSUCHKEY;
		}
		return 0;
	}
	int ConfigGroup :: _append_unit(const ConfigUnit & unit,int check, int except) {
		int ret = 0;
		int i;
		int size;
		if (this->selfType()!= unit.selfType()) {
			ret = ERROR;
			goto cpend;
		}
		size = unit.size();
		
		if (check) {
			for (i=0; i< size; ++i) {
				const comcfg::ConfigUnit & su = unit.get_sub_unit(i);
				if (CONFIG_ERROR_TYPE == su.selfType()) {
					ret = ERROR;
					goto cpend;
				}
				if (CONFIG_ERROR_TYPE != (*this)[su.get_key_name()].selfType()) {
					bsl_string ep = this->_name;
					ep.append(su.get_key_name());
					setErrorKeyPath(ep);
					ret = DUPLICATED_KEY;
					goto cpend;
				}
			}
		}
		
		for (i=0; i< size; ++i) {
			const comcfg::ConfigUnit & subunit = unit.get_sub_unit(i);
			switch (subunit.selfType()) {
			case comcfg :: CONFIG_ERROR_TYPE:
				ret = ERROR;
				break;
			case comcfg :: CONFIG_ARRAY_TYPE:
			case comcfg :: CONFIG_GROUP_TYPE:
				ConfigUnit * adu;
				ret = add_unit(subunit.get_key_name(), "", subunit.selfType(), 0, &adu);
				if (0 == ret) {
					ret = adu->_append_unit(subunit,0, except);
				}
				if (0 != ret) {
					goto cpend;
				}
				break;
			case comcfg :: CONFIG_UNIT_TYPE:
				ret = add_unit(subunit.get_key_name(), subunit.to_raw_string(), subunit.selfType());
				if (0 != ret) {
					goto cpend;
				}
				break;
			default:
				ret = ERROR;
				break;
			}
			if (0 != ret) {
				break;
			}
		}

		cpend:
		if (except && (0 != ret)) {
			if (0 != ul_seterrbuf("%s", "")) {
				LOG(WARNING) << "ConfigUnit: failed to write error buffer";
			}
			popError(NULL, ERROR);
		}
		return ret;
	}

	void ConfigGroup :: clear() {
		itr_t itr = fieldMap.begin();
		while(itr != fieldMap.end()){
			delete itr->second;
			++itr;
		}
		fieldMap.clear();
		fieldList.clear();
	}

	int ConfigGroup :: copy_unit(const ConfigUnit & unit, int except){
		int ret =0;
		if (this->selfType()!= unit.selfType()) {
			ret = ERROR;
			goto cpend;
		}
		clear();
		return _append_unit(unit, 1, except);
	cpend:
		if (except && (0 != ret)) {
			if (0 != ul_seterrbuf("%s", "")) {
				LOG(WARNING) << "ConfigUnit: failed to write error buffer";
			}
			popError(NULL, ERROR);
		}
		return ret;
	}
	int ConfigGroup::printGroup(){
		if(_dupLevel == GROUP_DUP_LEVEL0){
			return 0;
		}
		size_t threshold = 2;
		if(_dupLevel == GROUP_DUP_LEVEL3){
			threshold = 1;
		}
		int dupNum = 0;
		for(dup_itr itr=_dupMap.begin(); itr!=_dupMap.end(); itr++){
			if(itr->second.size() >= threshold){
				++ dupNum ;
			}
		}
		LOG(INFO) << "ConfigGroup: There are " << dupNum << " (duplicate) group names in total:";
		for(dup_itr itr=_dupMap.begin(); itr!=_dupMap.end(); itr++){
			const char *groupName = ((ConfigGroup*)itr->first)->getName();
			const int groupDepth = ((ConfigGroup *)itr->first)->getLevel();
			dup_vector &dvec = itr->second;
			if(dvec.size() >= threshold){
				for(size_t i=0; i<dvec.size(); i++){
					LOG(INFO)<< "\t[Group: " << groupName << "][Depth: " << groupDepth << "][File: " << dvec[i].file.c_str() << "][Line: " << dvec[i].line << "]";
				}
			}
		}
		if(_dupLevel==GROUP_DUP_LEVEL3 || dupNum==0){
			return 0;
		}
		return DUPLICATED_GROUP;
	}
}










/* vim: set ts=4 sw=4 sts=4 tw=100 */
