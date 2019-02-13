/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: ConfigUnit.cpp,v 1.16 2010/04/13 09:59:41 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file ConfigUnit.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/21 20:26:57
 * @version $Revision: 1.16 $ 
 * @brief 
 *  
 **/

#include "ConfigUnit.h"
#include "ConfigError.h"
#include "ConfigArray.h"

#include "utils/cc_utils.h"
#include "reader/Reader.h"
#include "bsl/var/String.h"
namespace comcfg{

	ConfigUnit * ConfigUnit :: get_err_unit() {
		static ConfigUnit* g_err_unit = NULL;
		if (NULL == g_err_unit) {
			g_err_unit = new ConfigError();
		}
		return g_err_unit;
	}

	class ErrorUnitControl {
	private:
		static int _needfree;
	public:
		ErrorUnitControl() {
			ConfigUnit :: get_err_unit();
		}
		~ErrorUnitControl() {
			if (_needfree) {
				_needfree =0;
				ConfigUnit *eu = ConfigUnit :: get_err_unit();
				delete eu;
			}
		}
	};
 
	int ErrorUnitControl::_needfree = 1;

	static ErrorUnitControl err_unit_control;

	//Reader * g_cur_reader = NULL;
	//str_t ConfigUnit :: g_unknown = "Unknown";

	const ConfigUnit & ConfigUnit :: operator[] (const char * str) const {
		if (selfType() == CONFIG_UNIT_TYPE) {
			LOG(INFO) << "Configure: visit ConfigUnit[char *] : " << _key.c_str() << "[" << str << "]";
			str_t tmp=this->_key;
			tmp.append(".").append(str);
			setErrorKeyPath(tmp);
		}
		return *get_err_unit();
	}
	const ConfigUnit & ConfigUnit :: operator[] (const str_t & str) const{
		if (selfType() == CONFIG_UNIT_TYPE) {
			LOG(INFO) << "Configure: visit ConfigUnit[str_t] : " << _key.c_str() << "[" << str.c_str() << "]";
			str_t tmp=this->_key;
			tmp.append(".").append(str);
			setErrorKeyPath(tmp);
		}
		return *get_err_unit();
	}
	const ConfigUnit & ConfigUnit :: operator[] (int idx) const{
		if (selfType() == CONFIG_UNIT_TYPE) {
			LOG(INFO) << "Configure: visit ConfigUnit[int] : " << _key.c_str() << "[" << idx << "]";
			str_t tmp=this->_key;
			char tmpid[64];
			snprintf(tmpid,64, "%d",idx);
			tmp.append("[").append(tmpid).append("]");;
			setErrorKeyPath(tmp);
		}
		return *get_err_unit();
	}
	ConfigUnit & ConfigUnit :: operator[] (const char * str) {
		if (selfType() == CONFIG_UNIT_TYPE) {
			LOG(INFO) << "Configure: visit ConfigUnit[char *] : " << _key.c_str() << "[" << str << "]";
			str_t tmp=this->_key;
			tmp.append(".").append(str);
			setErrorKeyPath(tmp);
		}
		return *get_err_unit();
	}
	ConfigUnit & ConfigUnit :: operator[] (const str_t & str) {
		if (selfType() == CONFIG_UNIT_TYPE) {
			LOG(INFO) << "Configure: visit ConfigUnit[str_t] : " << _key.c_str() << "[" << str.c_str() << "]";
			str_t tmp=this->_key;
			tmp.append(".").append(str);
			setErrorKeyPath(tmp);
		}
		return *get_err_unit();
	}
	ConfigUnit & ConfigUnit :: operator[] (int idx) {
		if (selfType() == CONFIG_UNIT_TYPE) {
			LOG(INFO) << "Configure: visit ConfigUnit[int] : " << _key.c_str() << "[" << idx << "]";
			str_t tmp=this->_key;
			char tmpid[64];
			snprintf(tmpid,64, "%d",idx);
			tmp.append("[").append(tmpid).append("]");;
			setErrorKeyPath(tmp);
		}
		return *get_err_unit();
	}

	ConfigUnit & ConfigUnit :: operator= (ConfigUnit & unit) {
		copy_unit(unit, 1);
		return *this;
	}
	ConfigUnit :: ConfigUnit():  _key(""), _at_file ("NULL"), _at_line(-1), _father(NULL), _bConstrainted(false)  {
	}
	ConfigUnit :: ConfigUnit(const str_t& __key, const str_t& __value, const Reader * __cur_reader, ConfigGroup * father)
				: _at_file("NULL"), _at_line(-1){
		for(int i = 0; i < (int)__key.size(); ++i){
			if(i == 0 && __key[i] == '@'){
				continue;
			}
			if((!isalpha(__key[i])) && (!isdigit(__key[i])) && __key[i] != '_'){
				LOG(WARNING) << "Unsupport _key format [" << __key.c_str() << "]";
				throw ConfigException();
			}
		}
		_key = __key;
		_value = __value;
		_cstr = _value;
		_cstr_err = 0;
		_father = father;
		_bConstrainted = false;
		if(__cur_reader != NULL){
			//如果这个reader被释放了，_at_file就会指向非法内存片
			setFilePosition(__cur_reader->_cur_file.c_str(),  __cur_reader->_cur_line);
		}
		if(_value.size() > 0 && _value[0] == '"'){
			_cstr_err = Trans :: str2str(_value, &_cstr);
			if(_cstr_err){
				LOG(WARNING) << "Config : Error format : _key[" << _key.c_str() << "] _value[" << _value.c_str() << "]";
			}
		}
	}

	const char * ConfigUnit :: seeError(const ErrCode & err)const{
		switch(err){
			case ERROR :
				return "Something error";
			case OK :
				return "OK";
			case FORMATERROR :
				return "Format error";
			case OUTOFRANGE :
				return "Out of range";
			case NULLVALUE :
				return "Value is null";
			case NULLBUFFER :
				return "Given buffer is null";
			case NOSUCHKEY :
				return "No such _key";
			case CONSTRAINT_ERROR :
				return "Constraint error";
			case CONFIG_ERROR :
				return "config error";
			case GROUPTOUNIT :
				return "use group as key";
			case DUPLICATED_KEY :
				return "duplicated key";
			case UNKNOWN :
			default:
				return "Unknown error";
		}
		return "Unknown error";
	}
	void ConfigUnit :: popError(ErrCode * err, ErrCode code)const{
		if(err){
			*err  = code;
		}
		else{
			ConfigThrower::throwException(code);
		}
	}

	void ConfigUnit :: setErrorKeyPath(str_t str)const{
		str_t errkeypath = str; //this->_key;
		//errkeypath.append(".").append(str);
		const ConfigGroup * top = this->_father;
		for (;NULL !=  top; top = top->_father){
			if (top->_name == CONFIG_GLOBAL){
				break;
			} else {
				errkeypath = str_t(top->_name).append(".").append(errkeypath);
			}
		}
		if (0 != ul_seterrbuf("%s", errkeypath.c_str())) {
			LOG(WARNING) << "Configure: failed to write Error key path to error buffer";
		}
	}

	ConfigUnit :: ~ConfigUnit(){} 
	char ConfigUnit :: to_char(ErrCode * errCode)const{
		char tmp;
		popError(errCode, get_char(&tmp));
		return tmp;
	}

	unsigned char ConfigUnit :: to_uchar(ErrCode * errCode)const{
		unsigned char tmp;
		popError(errCode, get_uchar(&tmp));
		return tmp;
	}

	int16_t ConfigUnit :: to_int16(ErrCode * errCode)const{
		int16_t tmp;
		popError(errCode, get_int16(&tmp));
		return tmp;
	}

	u_int16_t ConfigUnit :: to_uint16(ErrCode * errCode)const{
		u_int16_t tmp;
		popError(errCode, get_uint16(&tmp));
		return tmp;
	}

	int ConfigUnit :: to_int32(ErrCode * errCode)const{
		int tmp;
		popError(errCode, this->get_int32(&tmp));
		return tmp;
	}

	u_int32_t ConfigUnit :: to_uint32(ErrCode * errCode)const{
		u_int32_t tmp;
		popError(errCode, get_uint32(&tmp));
		return tmp;
	}

	long long ConfigUnit :: to_int64(ErrCode * errCode)const{
		long long tmp;
		popError(errCode, get_int64(&tmp));
		return tmp;
	}

	unsigned long long ConfigUnit :: to_uint64(ErrCode * errCode)const{
		unsigned long long tmp;
		popError(errCode, get_uint64(&tmp));
		return tmp;
	}

	float ConfigUnit :: to_float(ErrCode * errCode)const{
		float tmp;
		popError(errCode, get_float(&tmp));
		return tmp;
	}

	double ConfigUnit :: to_double(ErrCode * errCode)const{
		double tmp;
		popError(errCode, get_double(&tmp));
		return tmp;
	}
#if 1
	bsl_string ConfigUnit :: to_bsl_string(ErrCode * errCode)const{
		bsl_string tmp;
		popError(errCode, get_bsl_string(&tmp));
		return tmp;
	}
#endif
#if 0
	std_string ConfigUnit :: to_std_string(ErrCode * errCode)const{
		std_string tmp;
		popError(errCode, get_std_string(&tmp));
		return tmp;
	}
#endif
	str_t ConfigUnit :: to_raw_string(ErrCode * errCode)const{
		str_t tmp;
		popError(errCode, get_raw_string(&tmp));
		return tmp;
	}
#if 1
	const char * ConfigUnit :: to_cstr(ErrCode * errCode)const{
		if (_cstr_err) {
			LOG(INFO) << "Configure: visit ConfigUnit.to_cstr() failed";
			setErrorKeyPath(this->_key);
		}
		popError(errCode, _cstr_err);
		return _cstr.c_str();
	}
#endif

	//----------------to_xxx with default value
	char ConfigUnit :: to_char(ErrCode * errCode, const char & def)const {
		char ret;
		*errCode = get_char(&ret, def);
		return ret;
	}
	unsigned char ConfigUnit :: to_uchar(ErrCode * errCode, const unsigned char & def)const{
		unsigned char ret;
		*errCode = get_uchar(&ret, def);
		return ret;
	}
	int16_t ConfigUnit :: to_int16(ErrCode * errCode, const int16_t & def)const{
		int16_t ret;
		*errCode = get_int16(&ret, def);
		return ret;
	}
	u_int16_t ConfigUnit :: to_uint16(ErrCode * errCode, const u_int16_t & def)const{
		u_int16_t ret;
		*errCode = get_uint16(&ret, def);
		return ret;
	}
	int ConfigUnit :: to_int32(ErrCode * errCode, const int & def)const{
		int ret;
		*errCode = get_int32(&ret, def);
		return ret;
	}
	u_int32_t ConfigUnit :: to_uint32(ErrCode * errCode, const u_int32_t & def)const{
		u_int32_t ret;
		*errCode = get_uint32(&ret, def);
		return ret;
	}
	long long ConfigUnit :: to_int64(ErrCode * errCode, const long long & def)const{
		long long ret;
		*errCode = get_int64(&ret, def);
		return ret;
	}
	unsigned long long ConfigUnit :: to_uint64(ErrCode * errCode, const unsigned long long & def)const{
		unsigned long long ret;
		*errCode = get_uint64(&ret, def);
		return ret;
	}
	float ConfigUnit :: to_float(ErrCode * errCode, const float & def)const{
		float ret;
		*errCode = get_float(&ret, def);
		return ret;
	}
	double ConfigUnit :: to_double(ErrCode * errCode, const double & def)const{
		double ret;
		*errCode = get_double(&ret, def);
		return ret;
	}
	bsl_string ConfigUnit :: to_bsl_string(ErrCode * errCode, const bsl_string & def)const{
		bsl_string ret;
		*errCode = get_bsl_string(&ret, def);
		return ret;
	}
	str_t ConfigUnit :: to_raw_string(ErrCode * errCode, const str_t & def)const{
		str_t ret;
		*errCode = get_raw_string(&ret, def);
		return ret;
	}
	//获取C风格的字符串（常量）
	const char * ConfigUnit :: to_cstr(ErrCode * errCode, const char * def)const{
		str_t str;
		*errCode = get_bsl_string(&str);
		if (0 != *errCode){
			return def;
		}
		return _cstr.c_str();
	}


	ErrCode ConfigUnit :: get_char(char * valueBuf)const{
		LOG(INFO) << "Config : Get char from _key [" << _key.c_str() << "]";
		if(valueBuf == NULL){
			LOG(INFO) << "Configure: visit ConfigUnit.get_char() failed : NULLBUFFER";
			setErrorKeyPath(this->_key);
			return NULLBUFFER;
		}
		str_t buf;
		int ret = Trans :: str2str(_value, &buf);
		if(ret){
			LOG(INFO) << "Configure: visit ConfigUnit.get_char() failed";
			setErrorKeyPath(this->_key);
			return ret;
		}
		if(buf.size()){
			*valueBuf = char(buf[0]);
		}
		else{
			LOG(INFO) << "Configure: visit ConfigUnit.get_char() failed: NULLVALUE";
			setErrorKeyPath(this->_key);
			return NULLVALUE;
		}
		return 0;
	}   
	ErrCode ConfigUnit :: get_uchar(unsigned char * valueBuf)const{
		LOG(INFO) << "Config : Get uchar from _key [" << _key.c_str() << "]";
		if(valueBuf == NULL){
			LOG(INFO) << "Configure: visit ConfigUnit.get_uchar() failed : NULLBUFFER";
			setErrorKeyPath(this->_key);
			return NULLBUFFER;
		}
		str_t buf;
		int ret = Trans :: str2str(_value, &buf);
		if(ret){
			LOG(INFO) << "Configure: visit ConfigUnit.get_uchar() failed";
			setErrorKeyPath(this->_key);
			return ret;
		}
		if(buf.size()){
			*valueBuf = (unsigned char)(buf[0]);
		}
		else{
			LOG(INFO) << "Configure: visit ConfigUnit.get_uchar() failed: NULLVALUE";
			setErrorKeyPath(this->_key);
			return NULLVALUE;
		}
		return 0;
	}   

	ErrCode ConfigUnit :: get_int16(int16_t * valueBuf)const{
		LOG(INFO) << "Config : Get int16 from _key [" << _key.c_str() << "]";
		if(valueBuf == NULL){
			LOG(INFO) << "Configure: visit ConfigUnit.get_int16() failed : NULLBUFFER";
			setErrorKeyPath(this->_key);
			return NULLBUFFER;
		}
		long long val;
		int ret = Trans :: str2int64(_value, &val);
		if(ret) {
			LOG(INFO) << "Configure: visit ConfigUnit.get_int16() failed";
			setErrorKeyPath(this->_key);
			return ret;
		}
		if( val > (long long)0x7fff || val < -(long long)0x8000 ) {
			LOG(WARNING) << "Config : Get [" << _key.c_str() << "] : out of range.";
			setErrorKeyPath(this->_key);
			return OUTOFRANGE;
		}
		*valueBuf = (int16_t)val;
		return 0;
	}   

	ErrCode ConfigUnit :: get_uint16(u_int16_t * valueBuf)const{
		LOG(INFO) << "Config : Get uint16 from _key [" << _key.c_str() << "]";
		if(valueBuf == NULL){
			LOG(INFO) << "Configure: visit ConfigUnit.get_uint16() failed : NULLBUFFER";
			setErrorKeyPath(this->_key);
			return NULLBUFFER;
		}
		unsigned long long val;
		int ret = Trans :: str2uint64(_value, &val);
		if(ret) {
			LOG(INFO) << "Configure: visit ConfigUnit.get_uint16() failed";
			setErrorKeyPath(this->_key);
			return ret;
		}
		if( val > (unsigned long long)0xffff ) {
			LOG(WARNING) << "Config : Get [" << _key.c_str() << "] : out of range.";
			setErrorKeyPath(this->_key);
			return OUTOFRANGE;
		}
		*valueBuf = (u_int16_t)val;
		return 0;
	}   

	ErrCode ConfigUnit :: get_int32(int * valueBuf)const{
		LOG(INFO) <<  "Config : Get int32 from _key [" << _key.c_str() << "]";
		if(valueBuf == NULL){
			LOG(INFO) << "Configure: visit ConfigUnit.get_int32() failed : NULLBUFFER";
			setErrorKeyPath(this->_key);
			return NULLBUFFER;
		}
		long long val;
		int ret = Trans :: str2int64(_value, &val);
		if(ret) {
			LOG(INFO) <<  "Configure: visit ConfigUnit.get_int32() failed";
			setErrorKeyPath(this->_key);
			return ret;
		}
		if( val > (long long)0x7fffffff || val < -(long long)0x80000000 ) {
			LOG(WARNING) << "Config : Get [" << _key.c_str() << "] : out of range.";
			setErrorKeyPath(this->_key);
			return OUTOFRANGE;
		}
		*valueBuf = (int)val;
		return 0;
	}   

	ErrCode ConfigUnit :: get_uint32(u_int32_t * valueBuf)const{
		LOG(INFO) << "Config : Get uint32 from _key [" << _key.c_str() << "]";
		if(valueBuf == NULL){
			LOG(INFO) << "Configure: visit ConfigUnit.get_uint32() failed : NULLBUFFER";
			setErrorKeyPath(this->_key);
			return NULLBUFFER;
		}
		unsigned long long val;
		int ret = Trans :: str2uint64(_value, &val);
		if(ret) {
			LOG(INFO) << "Configure: visit ConfigUnit.get_uint32() failed";
			setErrorKeyPath(this->_key);
			return ret;
		}
		if( val > (unsigned long long)0xffffffff ) {
			LOG(WARNING) << "Config : Get [" << _key.c_str() << "] : out of range.";
			setErrorKeyPath(this->_key);
			return OUTOFRANGE;
		}
		*valueBuf = (u_int32_t)val;
		return 0;
	}   

	ErrCode ConfigUnit :: get_int64(long long * valueBuf)const{
		LOG(INFO) << "Config : Get int64 from _key [" << _key.c_str() << "]";
		if(valueBuf == NULL){
			LOG(INFO) << "Configure: visit ConfigUnit.get_int64() failed : NULLBUFFER";
			setErrorKeyPath(this->_key);
			return NULLBUFFER;
		}
		long long val;
		int ret = Trans :: str2int64(_value, &val);
		if(ret) {
			LOG(INFO) << "Configure: visit ConfigUnit.get_int64() failed";
			setErrorKeyPath(this->_key);
			return ret;
		}
		*valueBuf = val;
		return 0;
	}   

	ErrCode ConfigUnit :: get_uint64(unsigned long long * valueBuf)const{
		LOG(INFO) << "Config : Get uint64 from _key [" << _key.c_str() << "]";
		if(valueBuf == NULL){
			LOG(INFO) << "Configure: visit ConfigUnit.get_uint64() failed : NULLBUFFER";
			setErrorKeyPath(this->_key);
			return NULLBUFFER;
		}
		unsigned long long val;
		int ret = Trans :: str2uint64(_value, &val);
		if(ret) {
			LOG(INFO) << "Configure: visit ConfigUnit.get_uint64() failed";
			setErrorKeyPath(this->_key);
			return ret;
		}
		*valueBuf = val;
		return 0;
	}   


	ErrCode ConfigUnit :: get_float(float * valueBuf)const{  
		LOG(INFO) << "Config : Get float from _key [" << _key.c_str() << "]";
		if(valueBuf == NULL){
			LOG(INFO) << "Configure: visit ConfigUnit.get_float() failed : NULLBUFFER";
			setErrorKeyPath(this->_key);
			return NULLBUFFER;
		}
		double buf = 0.0;
		int ret = Trans :: str2double(_value, &buf);
		if(ret) {
			LOG(INFO) << "Configure: visit ConfigUnit.get_float() failed";
			setErrorKeyPath(this->_key);
		}
		*valueBuf = float(buf);
		return ret;
	}   
	ErrCode ConfigUnit :: get_double(double * valueBuf)const{
		LOG(INFO) << "Config : Get double from _key [" << _key.c_str() << "]";
		if(valueBuf == NULL){
			LOG(INFO) << "Configure: visit ConfigUnit.get_double() failed : NULLBUFFER";
			setErrorKeyPath(this->_key);
			return NULLBUFFER;
		}
		int ret = Trans :: str2double(_value, valueBuf);
		if(ret) {
			LOG(INFO) << "Configure: visit ConfigUnit.get_double() failed";
			setErrorKeyPath(this->_key);
		}
		return ret;
	}   
#if 0
	ErrCode ConfigUnit :: get_std_string(std_string * valueBuf) const{
		LOG(INFO) << "Config : Get std::string from _key [%s]", _key.c_str());
		if(valueBuf == NULL){
			return NULLBUFFER;
		}
		return Trans :: str2str(_value, valueBuf);
	}   
#endif
	ErrCode ConfigUnit :: get_bsl_string(bsl_string * valueBuf) const{
		LOG(INFO) << "Config : Get std::string from _key [" << _key.c_str() << "]";
		if(valueBuf == NULL){
			LOG(INFO) << "Configure: visit ConfigUnit.get_bsl_string() failed : NULLBUFFER";
			setErrorKeyPath(this->_key);
			return NULLBUFFER;
		}
		int ret = Trans :: str2str(_value, valueBuf);
		if(ret) {
			LOG(INFO) <<"Configure: visit ConfigUnit.get_bsl_string() failed";
			setErrorKeyPath(this->_key);
		}
		return ret;
	}   
	ErrCode ConfigUnit :: get_raw_string(str_t * valueBuf) const{
		LOG(INFO) << "Config : Get raw string from _key [" << _key.c_str() << "]";
		if(valueBuf == NULL){
			LOG(INFO) << "Configure: visit ConfigUnit.get_raw_string() failed : NULLBUFFER";
			setErrorKeyPath(this->_key);
			return NULLBUFFER;
		}
		*valueBuf = _value;
		return 0;
	}   
	ErrCode ConfigUnit :: get_cstr(char * valueBuf, size_t len) const{
		LOG(INFO) << "Config : Get c style string from _key [" << _key.c_str() << "]";
		if(valueBuf == NULL){
			LOG(INFO) << "Configure: visit ConfigUnit.get_cstr() failed : NULLBUFFER";
			setErrorKeyPath(this->_key);
			return NULLBUFFER;
		}
		str_t str;
		int ret = get_bsl_string(&str);
		if(ret == 0){
			snprintf(valueBuf, len, "%s", str.c_str());
		} else {
			LOG(INFO) << "Configure: visit ConfigUnit.get_cstr() failed";
			setErrorKeyPath(this->_key);
		}
		return ret;
	}
	//--------------get_xx with default-----------

	ErrCode ConfigUnit :: get_char(char * valueBuf, const char & def)const{
		ErrCode ret = get_char(valueBuf);
		if(ret != 0){
			*valueBuf = def;
		}
		return ret;
	}

	ErrCode ConfigUnit :: get_uchar(unsigned char * valueBuf, const unsigned char & def)const{
		ErrCode ret = get_uchar(valueBuf);
		if(ret != 0){
			*valueBuf = def;
		}
		return ret;
	}

	ErrCode ConfigUnit :: get_int16(int16_t * valueBuf, const int16_t & def)const{
		ErrCode ret = get_int16(valueBuf);
		if(ret != 0){
			*valueBuf = def;
		}
		return ret;
	}

	ErrCode ConfigUnit :: get_uint16(u_int16_t * valueBuf, const u_int16_t & def)const{
		ErrCode ret = get_uint16(valueBuf);
		if(ret != 0){
			*valueBuf = def;
		}
		return ret;
	}

	ErrCode ConfigUnit :: get_int32(int * valueBuf, const int & def)const{
		ErrCode ret = get_int32(valueBuf);
		if(ret != 0){
			*valueBuf = def;
		}
		return ret;
	}

	ErrCode ConfigUnit :: get_uint32(u_int32_t * valueBuf, const u_int32_t & def)const{
		ErrCode ret = get_uint32(valueBuf);
		if(ret != 0){
			*valueBuf = def;
		}
		return ret;
	}

	ErrCode ConfigUnit :: get_int64(long long * valueBuf, const long long & def)const{
		ErrCode ret = get_int64(valueBuf);
		if(ret != 0){
			*valueBuf = def;
		}
		return ret;
	}

	ErrCode ConfigUnit :: get_uint64(unsigned long long * valueBuf, const unsigned long long & def)const{
		ErrCode ret = get_uint64(valueBuf);
		if(ret != 0){
			*valueBuf = def;
		}
		return ret;
	}

	ErrCode ConfigUnit :: get_float(float * valueBuf, const float & def)const{
		ErrCode ret = get_float(valueBuf);
		if(ret != 0){
			*valueBuf = def;
		}
		return ret;
	}

	ErrCode ConfigUnit :: get_double(double * valueBuf, const double & def)const{
		ErrCode ret = get_double(valueBuf);
		if(ret != 0){
			*valueBuf = def;
		}
		return ret;
	}

	ErrCode ConfigUnit :: get_bsl_string(bsl_string * valueBuf, const bsl_string & def)const{
		ErrCode ret = get_bsl_string(valueBuf);
		if(ret != 0){
			*valueBuf = def;
		}
		return ret;
	}

	ErrCode ConfigUnit :: get_raw_string(str_t * valueBuf, const str_t & def)const{
		ErrCode ret = get_raw_string(valueBuf);
		if(ret != 0){
			*valueBuf = def;
		}
		return ret;
	}

	ErrCode ConfigUnit :: get_cstr(char * valueBuf, size_t len, const char * def)const{
		ErrCode ret = get_cstr(valueBuf, len);
		if(ret != 0){
			snprintf(valueBuf, len, "%s", def);
		}
		return ret;
	}

	//-------------------to_IVar------------------
	bsl::var::IVar& ConfigUnit :: to_IVar(bsl::ResourcePool * vpool, ErrCode* errCode)const{
		if(vpool == NULL){
			LOG(INFO) << "Configure: visit ConfigUnit.to_IVar() failed : NULLBUFFER";
			setErrorKeyPath(this->_key);
			popError(errCode, NULLBUFFER);
			return bsl::var::Null::null;
		}
		if (_cstr_err) {
			setErrorKeyPath(this->_key);
		}
		popError(errCode, _cstr_err);
		bsl::var::IVar & var = vpool->create<bsl::var::String, str_t>(_cstr);
		return var;
	}

	const ConfigUnit & ConfigUnit :: deepGet(const str_t path) const{
		for (int i=0; i<(int)path.size(); ++i){
			if ('.' == path[i]){
				str_t first = path.substr(0,i);
				str_t sub = path.substr(i+1);
				return deepGetSegment(first).deepGet(sub);
			}
		}
		return deepGetSegment(path);
	}

	const ConfigUnit & ConfigUnit :: deepGetSegment(const str_t segment) const {
		int idleft=0;
		int idright=-1;
		for (int i=0; i<(int)segment.size(); ++i){
			if ('['==segment[i]){
				idleft = i+1;
			} else if (']' == segment[i]) {
				idright = i-1;
			}
		}
		if (idright >= idleft) {
			str_t arrid = segment.substr(idleft, idright-idleft+1);
			if (Trans::isInteger(arrid)) {
				return (*this)[segment.substr(0,idleft-1)][atoi(arrid.c_str())];
			} else {
				return (*this)[segment.substr(0,idleft-1)][arrid];
			}
		} else {
			return (*this)[segment];
		}
	}

	const char * ConfigUnit :: getErrKeyPath() const{
		return ul_geterrbuf();
	}

	const ConfigUnit & ConfigUnit :: get_sub_unit(int /*index*/) const {
		return *get_err_unit();
	}

	void ConfigUnit :: get_full_path(bsl::string &path, const int get_global)const {
		path = get_key_name();
		ConfigGroup * tmp = getFather();
		if(get_global){
			while(tmp != NULL){
				const char * group_name = tmp->get_key_name().c_str();
				str_t app = path;
				path.setf("%s.%s", group_name, app.c_str());
				tmp = tmp->getFather();
			}
		}
		else{
			while(tmp != NULL){
				const char * group_name = tmp->get_key_name().c_str();
				if(strcmp(group_name, CONFIG_GLOBAL)){	//ignore CONFIG_GLOBAL
					str_t app = path;
					path.setf("%s.%s", group_name, app.c_str());
				}
				tmp = tmp->getFather();
			}
		}
	}

	int ConfigUnit :: equals(const ConfigUnit & conf) const {
		try {
			if (conf.to_bsl_string() == to_bsl_string()) {
				return 0;
			}
			return -1;
		} catch (bsl::Exception) {
			return -1;
		}
		return -1;
	}

	int ConfigUnit :: add_unit(const bsl_string & /*key*/, const bsl_string& /*value*/,
			const int /*objtype*/, int except, ConfigUnit ** /*ref*/) {
		if (except) {
			if (0 != ul_seterrbuf("%s", "")) {
				LOG(WARNING) << "ConfigUnit: failed to write error buffer";
			}
			popError(NULL, ERROR);
		}
		return ERROR;
	}

	int ConfigUnit :: _append_unit(const ConfigUnit & /*unit*/,int /*check*/, int except) {
		if (except) {
			if (0 != ul_seterrbuf("%s", "")) {
				LOG(WARNING) << "ConfigUnit: failed to write error buffer";
			}
			popError(NULL, ERROR);
		}
		return ERROR;
	}

	int ConfigUnit :: append_unit(const ConfigUnit & unit, int except) {
		return _append_unit(unit, 1, except);
	}

	int ConfigUnit :: copy_unit(const ConfigUnit & unit, int except) {
		int ret = 0;
		if (this->selfType()!= unit.selfType()) {
			ret = ERROR;
			goto cpend;
		}
		/*
		if (NULL != this->_father && CONFIG_ERROR_TYPE == (*_father)[unit._key].selfType()) {
			ret = init_unit(unit._key, unit._value);
		} else {
			ret = DUPLICATED_KEY;
		}
		*/
		ret = init_unit(unit._key, unit._value);
		return ret;
	cpend:
		if (except && (0 != ret)) {
			if (0 != ul_seterrbuf("%s", "")) {
				LOG(WARNING) << "ConfigUnit: failed to write error buffer";
			}
			popError(NULL, ERROR);
		}
		return ret;
	}
	int ConfigUnit :: del_unit(const bsl_string & /*key*/, int except) {
		if (except) {
			if (0 != ul_seterrbuf("%s", "")) {
				LOG(WARNING) << "ConfigUnit: failed to write error buffer";
			}
			popError(NULL, NOSUCHKEY);
		}
		return NOSUCHKEY;
	}

	int ConfigUnit :: set_value(const bsl_string & value, int /*except*/){
		_value = value;
		_cstr = _value;
		if(_value.size() > 0 && _value[0] == '"'){
			_cstr_err = Trans :: str2str(_value, &_cstr);
			if(_cstr_err){
				LOG(WARNING) << "Config : Error format : _key[" << _key.c_str() << "] _value[" << _value.c_str() << "]";
			}
		}
		return 0;
	}
	int ConfigUnit :: traverse_unit(cb_tr cb_fn, void *cb_arg)
	{
		for(size_t i=0; i<this->size(); ++i){
			ConfigUnit & subunit = const_cast<ConfigUnit &>(this->get_sub_unit(i));
			int ret = cb_fn(&subunit, cb_arg);
			if(ret != 0){
				return ret;
			}
			switch (subunit.selfType()) {
				case CONFIG_UNIT_TYPE:
				case CONFIG_ERROR_TYPE:
					break;
				case CONFIG_ARRAY_TYPE:
				case CONFIG_GROUP_TYPE:
					ret = subunit.traverse_unit(cb_fn, cb_arg);
					if(ret != 0){
						return ret;
					}
					break;
			}
		}
		return 0;
	}
	int ConfigUnit :: traverse_unit_order(cb_tr cb_fn, void *cb_arg)
	{
		return traverse_sub_type(cb_fn, cb_arg, CONFIG_ERROR_TYPE)
				|| traverse_sub_type(cb_fn, cb_arg, CONFIG_UNIT_TYPE)
				|| traverse_sub_type(cb_fn, cb_arg, CONFIG_ARRAY_TYPE)
				|| traverse_sub_type(cb_fn, cb_arg, CONFIG_GROUP_TYPE);
	}
	int ConfigUnit :: traverse_sub_type(cb_tr cb_fn, void *cb_arg, const int type)
	{
		int ret = 0;
		for(size_t i=0; i<this->size() && !ret; ++i){
			ConfigUnit & subunit = const_cast<ConfigUnit &>(this->get_sub_unit(i));
			if(subunit.selfType() != type){
				continue;
			}
			int ret = cb_fn(&subunit, cb_arg);
			if(type == CONFIG_GROUP_TYPE || type == CONFIG_ARRAY_TYPE){
				ret = ret || subunit.traverse_unit_order(cb_fn, cb_arg);
			}
		}
		return ret;
	}
	ConfigUnit * ConfigUnit :: create_unit(const bsl_string & key, const bsl_string& value, 
			int objtype, ConfigGroup * father) {
		ConfigUnit * tmp=NULL;
		try {
			switch (objtype) {
			case CONFIG_UNIT_TYPE :
				tmp = new ConfigUnit(key, value, NULL, father);
				return tmp;
				break;
			case CONFIG_GROUP_TYPE :
				tmp = new ConfigGroup(key.c_str(), father);
				return tmp;
				break;
			case CONFIG_ARRAY_TYPE :
				tmp = new ConfigArray(key.c_str(), father);
				return tmp;
				break;
			default :
				break;
			}
		} catch (...) {
			if (tmp) {
				delete tmp;
			}
			return NULL;
		}
		return NULL;
	}

	void ConfigUnit :: clear() {
		return;
	}

	int ConfigUnit :: init_unit(const str_t& /*__key*/, const str_t& __value) {
		/*
		for(int i = 0; i < (int)__key.size(); ++i){
			if(i == 0 && __key[i] == '@'){
				continue;
			}
			if((!isalpha(__key[i])) && (!isdigit(__key[i])) && __key[i] != '_'){
				ul_writelog(Log::warning(), "Unsupport _key format [%s]", __key.c_str());
				return FORMATERROR;
			}
		}
		_key = __key;
		*/
		_value = __value;
		_cstr = _value;
		_cstr_err = 0;
		if(_value.size() > 0 && _value[0] == '"'){
			_cstr_err = Trans :: str2str(_value, &_cstr);
			if(_cstr_err){
				LOG(WARNING) << "Config : Error format : _key[" << _key.c_str() << "] _value[" << _value.c_str() << "]";
			}
		}
		return 0;
	}

	
};
















/* vim: set ts=4 sw=4 sts=4 tw=100 */
