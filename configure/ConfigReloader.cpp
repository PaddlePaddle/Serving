/***************************************************************************
 * 
 * Copyright (c) 2010 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file ConfigReloader.cpp
 * @author zhang_rui(com@baidu.com)
 * @date 2010-1-21
 * @brief 
 *  
 **/


#include "ConfigReloader.h"
#include <time.h>
#include <bsl/exception/bsl_exception.h>

static const int hashmap_bitems = 256;

namespace comcfg {

#define COMCFG_SAFEFREE(obj) do {if (NULL != (obj)) { delete (obj); (obj)=NULL; }} while(0)

ConfigReloader :: ConfigReloader() : _finit(0), _last_modify(0)  {
	_config[0] = 0;
	_config[1] = 0;
	_config_curid = 0;
	_load_param.filename = "";
	_load_param.rangename = "";
	_load_param.path = "";
	_load_param.version = CONFIG_VERSION_1_0;

	_chrcbmap.create(hashmap_bitems);
	_strcbmap.create(hashmap_bitems);
	_intcbmap.create(hashmap_bitems);
	_uint64cbmap.create(hashmap_bitems);
	_floatcbmap.create(hashmap_bitems);
	_groupcbmap.create(hashmap_bitems);
}

/**
 * @brief 设定要监控的配置文件
 *
 * @param [in] path   : const char* 配置文件的路径
 * @param [in] conf   : const char* 配置文件名
 * @param [in] range   : const char* 约束文件，约束文件不进行更新检查
 * @param [in] version   : int 文件格式的版本（暂未使用）
 * @return  int  0表示成功，其余为失败
 **/
int ConfigReloader :: init(const char * path, const char * conf,
		const char * range, int version) {
	if(path == NULL || conf == NULL){
		LOG(WARNING) << "ConfigReloader.init : path or filename error...";
		return -1;
	}
	if (_finit) {
		LOG(WARNING) << "ConfigReloader.init : already init. Can not init twice.";
		return -1;
	}
	_load_param.filename = conf;
	if (NULL != range) {
		_load_param.rangename = range;
	} else {
		_load_param.rangename = "";
	}
	_load_param.path = path;
	_load_param.version = version;
	int ret=0;
	_config[_config_curid] = new comcfg::Configure;
	ret = _config[_config_curid]->load(path, conf,range,version);
	if (0 != ret) {
		COMCFG_SAFEFREE(_config[_config_curid]);
		return ret;
	}
	_last_modify = _config[_config_curid]->lastConfigModify();
	if (0 == _last_modify) {
        LOG(WARNING) << "ConfigReloader.init : get last modify time error...";
		COMCFG_SAFEFREE(_config[_config_curid]);
		return -1;
	}
	if (0 == ret) {
		_finit = 1;
	}
	return ret;
}

int ConfigReloader :: _add_key_monitor(config_monmap_t * map, const char * key,
		key_change_cb_t proc, void * prm) {
	if(key == NULL || NULL == proc){
		LOG(WARNING) << "ConfigReloader._add_key_monitor : NULL key or callback";
		return -1;
	}
	bsl::string kstr = key;
	config_monitor_t * pmon = new config_monitor_t;
	pmon->callback = proc;
	pmon->param = prm;
	config_monmap_pr_t ptr;
	ptr = map->find(kstr);
	if(NULL != ptr) {
		if (NULL != ptr->second) {
			delete ptr->second;
		}
		map->erase(ptr->first);
		LOG(WARNING) << "ConfigReloader._add_key_monitor : "
				"same key exist, update callback("<< proc << "), param(" << prm << ")";
	}
	(*map).set(kstr, pmon, 1);
	return 0;
}

int ConfigReloader :: reload() {
	int nextid = (_config_curid + 1) % 2;
	COMCFG_SAFEFREE(_config[nextid]);
	_config[nextid] = new comcfg :: Configure;
	int ret = _config[nextid]->load(_load_param.path.c_str(), _load_param.filename.c_str(),
			_load_param.rangename.c_str(), _load_param.version);
	if (0 != ret) {
		COMCFG_SAFEFREE(_config[nextid]);
		return -1;
	}
	_config_curid = nextid;
	return ret;
}

int ConfigReloader :: check() {
	int oldid = (_config_curid + 1) % 2;
	try {
		int ret = 0;
		bsl::hashmap <bsl::string, config_monitor_t *> :: iterator  itr;
		//先检查合法
		for(itr = _chrcbmap.begin(); itr != _chrcbmap.end(); ++itr){
			_config[_config_curid]->deepGet(itr->first).to_char();
		}
		for (itr = _strcbmap.begin(); itr != _strcbmap.end(); ++itr) {
			_config[_config_curid]->deepGet(itr->first).to_bsl_string();
		}
		for (itr = _intcbmap.begin(); itr != _intcbmap.end(); ++itr) {
			_config[_config_curid]->deepGet(itr->first).to_int64();
		}
		for (itr = _uint64cbmap.begin(); itr != _uint64cbmap.end(); ++itr) {
			_config[_config_curid]->deepGet(itr->first).to_uint64();
		}
		for (itr = _floatcbmap.begin(); itr != _floatcbmap.end(); ++itr) {
			_config[_config_curid]->deepGet(itr->first).to_double();
		}
		for (itr = _groupcbmap.begin(); itr != _groupcbmap.end(); ++itr) {
			int ty = _config[_config_curid]->deepGet(itr->first).selfType();
			if (CONFIG_GROUP_TYPE != ty && CONFIG_ARRAY_TYPE != ty) {
				LOG(WARNING) << "ConfigReloader.add_group_monitor :"
						" (" << itr->first.c_str() << ") is not group or array";
				return -1;
			}
		}


		//再检查数据是否改变
		for(itr = _chrcbmap.begin(); itr != _chrcbmap.end(); ++itr){
			if (NULL != itr->second) {
				config_monitor_t * pmon = (config_monitor_t *) (itr->second);
				if (_config[_config_curid]->deepGet(itr->first).to_char() !=
						_config[oldid]->deepGet(itr->first).to_char()) {
					pmon->callback(_config[oldid]->deepGet(itr->first),
							_config[_config_curid]->deepGet(itr->first), pmon->param);
					++ret;
				}
			}
		}
		for (itr = _strcbmap.begin(); itr != _strcbmap.end(); ++itr) {
			if (NULL != itr->second) {
				config_monitor_t * pmon = (config_monitor_t *) (itr->second);
				if (_config[_config_curid]->deepGet(itr->first).to_bsl_string()
						!= _config[oldid]->deepGet(itr->first).to_bsl_string()) {
					pmon->callback(_config[oldid]->deepGet(itr->first),
							_config[_config_curid]->deepGet(itr->first),
							pmon->param);
					++ret;
				}
			}
		}
		for (itr = _intcbmap.begin(); itr != _intcbmap.end(); ++itr) {
			if (NULL != itr->second) {
				config_monitor_t * pmon = (config_monitor_t *) (itr->second);
				if (_config[_config_curid]->deepGet(itr->first).to_int64()
						!= _config[oldid]->deepGet(itr->first).to_int64()) {
					pmon->callback(_config[oldid]->deepGet(itr->first),
							_config[_config_curid]->deepGet(itr->first),
							pmon->param);
					++ret;
				}
			}
		}
		for (itr = _uint64cbmap.begin(); itr != _uint64cbmap.end(); ++itr) {
			if (NULL != itr->second) {
				config_monitor_t * pmon = (config_monitor_t *) (itr->second);
				if (_config[_config_curid]->deepGet(itr->first).to_uint64()
						!= _config[oldid]->deepGet(itr->first).to_uint64()) {
					pmon->callback(_config[oldid]->deepGet(itr->first),
							_config[_config_curid]->deepGet(itr->first),
							pmon->param);
					++ret;
				}
			}
		}
		for (itr = _floatcbmap.begin(); itr != _floatcbmap.end(); ++itr) {
			if (NULL != itr->second) {
				config_monitor_t * pmon = (config_monitor_t *) (itr->second);
				if (_config[_config_curid]->deepGet(itr->first).to_double()
						!= _config[oldid]->deepGet(itr->first).to_double()) {
					pmon->callback(_config[oldid]->deepGet(itr->first),
							_config[_config_curid]->deepGet(itr->first),
							pmon->param);
					++ret;
				}
			}
		}
		for (itr = _groupcbmap.begin(); itr != _groupcbmap.end(); ++itr) {
			if (NULL != itr->second) {
				config_monitor_t * pmon = (config_monitor_t *) (itr->second);
				if (0 != _config[_config_curid]->deepGet(itr->first)
						.equals(_config[oldid]->deepGet(itr->first)) ) {
					pmon->callback(_config[oldid]->deepGet(itr->first),
							_config[_config_curid]->deepGet(itr->first),
							pmon->param);
					++ret;
				}
			}
		}
		for(int i = 0; i < (int)_filecblist.size(); ++i){
			config_monitor_t * pmon =  _filecblist[i];
			if (NULL != pmon && NULL!= pmon->callback) {
				pmon->callback(*(_config[oldid]), *(_config[_config_curid]), pmon->param);
				++ret;
			}
		}
		return ret;
	} catch (bsl::Exception &e) {
		LOG(WARNING) << "ConfigReloader.check : error(" << e.what() << ")";
		return -1;
	} catch (...) {
		LOG(WARNING) << "ConfigReloader.check : unknown error";
		return -1;
	}
	LOG(WARNING) << "ConfigReloader.check : unknown error, the codes lost road.";
	return -1;
}

/**
 * @brief 检查配置文件是否发生变化。
 *
 * @return  int  0表示配置没有不安化，1表示配置发生了变化并进行了通知，-1表示更新后的配置内容错误或约束错误。
 * -2表示其他错误
 **/
int ConfigReloader :: monitor() {
	if (NULL == _config[_config_curid]) {
		LOG(WARNING) << "ConfigReloader.monitor : NULL config(" << _config_curid << "), init failed?";
		return -2;
	}
	time_t modtime = _config[_config_curid]->lastConfigModify();
	if (modtime > _last_modify)  {
		if (0 != reload()) {
			LOG(WARNING) << "ConfigReloader.monitor : reload error("<< _load_param.filename.c_str() << ")";
			return -1;
		}
		int ret =  check();
		if (-1 == ret) {
			LOG(WARNING) << "ConfigReloader.monitor : check error(" << _load_param.filename.c_str() << ")";
			int oldid = _config_curid;
			_config_curid = (_config_curid + 1) % 2;

			COMCFG_SAFEFREE(_config[oldid]);
			return -2;
		}
		_last_modify = modtime;
		return ret;
	} else {
		return 0;
	}
	LOG(WARNING) << "ConfigReloader.monitor : unknown error(" << _load_param.filename.c_str() << ")";
	return -2;
}
/**
 * @brief 增加要监控的整数key
 *
 * @param [in] key   : const char* 配置项的全路径
 * @param [in] proc   : key_change_cb_t 处理回调函数
 * @return  int  0表示成功，-1为失败
 **/
int ConfigReloader :: add_key_int(const char * key, key_change_cb_t proc, void * prm) {
	try {
		if (NULL == key) {
			LOG(WARNING) << "ConfigReloader.add_key_int : NULL key.";
			return -1;
		}
		if (NULL == _config[_config_curid]) {
			LOG(WARNING) << "ConfigReloader.add_key_int : NULL config. init failed?";
			return -1;
		}
		_config[_config_curid]->deepGet(key).to_int64();
	} catch (bsl::Exception &e) {
		LOG(WARNING) << "ConfigReloader.add_key_int : error(" << e.what() << ")";
		return -1;
	} catch (...) {
		LOG(WARNING) << "ConfigReloader.add_key_int : unknown error";
		return -1;
	}
	return _add_key_monitor(&_intcbmap, key, proc, prm);
}
/**
 * @brief 增加要监控的uint64 key
 *
 * @param [in] key   : const char* 配置项的全路径
 * @param [in] proc   : key_change_cb_t 处理回调函数
 * @return  int  0表示成功，-1为失败
 **/
int ConfigReloader :: add_key_uint64(const char * key, key_change_cb_t proc, void * prm) {
	try {
		if (NULL == key) {
			LOG(WARNING) << "ConfigReloader.add_key_uint64 : NULL key.";
			return -1;
		}
		if (NULL == _config[_config_curid]) {
			LOG(WARNING) << "ConfigReloader.add_key_uint64 : NULL config. init failed?";
			return -1;
		}
		_config[_config_curid]->deepGet(key).to_uint64();
	} catch (bsl::Exception &e) {
		LOG(WARNING) << "ConfigReloader.add_key_uint64 : error(" << e.what() << ")";
		return -1;
	} catch (...) {
		LOG(WARNING) << "ConfigReloader.add_key_uint64 : unknown error";
		return -1;
	}
	return _add_key_monitor(&_uint64cbmap, key, proc, prm);
}
/**
 * @brief 增加要监控的浮点类型key
 *
 * @param [in] key   : const char* 配置项的全路径
 * @param [in] proc   : key_change_cb_t 处理回调函数
 * @return  int  0表示成功，-1为失败
 **/
int ConfigReloader :: add_key_float(const char * key, key_change_cb_t proc, void * prm) {
	try {
		if (NULL == key) {
			LOG(WARNING) << "ConfigReloader.add_key_float : NULL key.";
			return -1;
		}
		if (NULL == _config[_config_curid]) {
			LOG(WARNING) << "ConfigReloader.add_key_float : NULL config. init failed?";
			return -1;
		}
		_config[_config_curid]->deepGet(key).to_double();
	} catch (bsl::Exception &e) {
		LOG(WARNING) << "ConfigReloader.add_key_float : error(" << e.what() << ")";
		return -1;
	} catch (...) {
		LOG(WARNING) << "ConfigReloader.add_key_float : unknown error";
		return -1;
	}
	return _add_key_monitor(&_floatcbmap, key, proc, prm);
}
/**
 * @brief 增加要监控的字符key
 *
 * @param [in] key   : const char* 配置项的全路径
 * @param [in] proc   : key_change_cb_t 处理回调函数
 * @return  int  0表示成功，-1为失败
 **/
int ConfigReloader :: add_key_char(const char * key, key_change_cb_t proc, void * prm) {
	try {
		if (NULL == key) {
			LOG(WARNING) << "ConfigReloader.add_key_char : NULL key.";
			return -1;
		}
		if (NULL == _config[_config_curid]) {
			LOG(WARNING) << "ConfigReloader.add_key_char : NULL config. init failed?";
			return -1;
		}
		_config[_config_curid]->deepGet(key).to_char();
	} catch (bsl::Exception &e) {
		LOG(WARNING) << "ConfigReloader.add_key_char : error(" << e.what() << ")";
		return -1;
	} catch (...) {
		LOG(WARNING) << "ConfigReloader.add_key_char : unknown error";
		return -1;
	}
	return _add_key_monitor(&_chrcbmap, key, proc, prm);
}
/**
 * @brief 增加要监控的字符串key
 *
 * @param [in] key   : const char* 配置项的全路径
 * @param [in] proc   : key_change_cb_t 处理回调函数
 * @return  int  0表示成功，-1为失败
 **/
int ConfigReloader :: add_key_string(const char * key, key_change_cb_t proc, void * prm) {
	try {
		if (NULL == key) {
			LOG(WARNING) << "ConfigReloader.add_key_string : NULL key.";
			return -1;
		}
		if (NULL == _config[_config_curid]) {
			LOG(WARNING) << "ConfigReloader.add_key_string : NULL config. init failed?";
			return -1;
		}
		_config[_config_curid]->deepGet(key).to_bsl_string();
	} catch (bsl::Exception &e) {
		LOG(WARNING) << "ConfigReloader.add_key_string : error(" << e.what() << ")";
		return -1;
	} catch (...) {
		LOG(WARNING) << "ConfigReloader.add_key_string : unknown error";
		return -1;
	}
	return _add_key_monitor(&_strcbmap, key, proc, prm);
}
/**
 * @brief 增加监控[group]的回调
 *
 * @param [in] group   : const char* 配置项的全路径
 * @param [in] proc   : key_change_cb_t 处理回调函数
 * @return  int  0表示成功，-1为失败
 **/
int ConfigReloader :: add_group_monitor(const char * group, key_change_cb_t proc, void * prm) {
	try {
		if (NULL == group) {
			LOG(WARNING) << "ConfigReloader.add_group_monitor : NULL key.";
			return -1;
		}
		if (NULL == _config[_config_curid]) {
			LOG(WARNING) << "ConfigReloader.add_group_monitor : NULL config. init failed?";
			return -1;
		}
		int ty =_config[_config_curid]->deepGet(group).selfType();
		if (CONFIG_GROUP_TYPE != ty && CONFIG_ARRAY_TYPE != ty) {
			LOG(WARNING) << "ConfigReloader.add_group_monitor :"
					" (" << group << ") is not group or array";
			return -1;
		}
	} catch (bsl::Exception &e) {
		LOG(WARNING) << "ConfigReloader.add_group_monitor : error(" << e.what() << ")";
		return -1;
	} catch (...) {
		LOG(WARNING) << "ConfigReloader.add_group_monitor : unknown error";
		return -1;
	}
	return _add_key_monitor(&_groupcbmap, group, proc, prm);
}
/**
 * @brief 增加监控整个文件的回调
 *
 * @param [in] proc   : key_change_cb_t 处理回调函数
 * @return  int  0表示成功，-1为失败
 **/
int ConfigReloader :: add_file_monitor(key_change_cb_t proc, void * prm) {
	if (NULL == _config[_config_curid]) {
        LOG(WARNING) << "ConfigReloader.add_file_monitor : NULL config. init failed?";
		return -1;
	}
	if(NULL == proc) {
		LOG(WARNING) << "ConfigReloader.add_file_monitor : NULL callback";
		return -1;
	}

	for(int i = 0; i < (int)_filecblist.size(); ++i){
		config_monitor_t * pmon =  _filecblist[i];
		if (NULL != pmon && proc == pmon->callback) {
			pmon->param = prm;
			LOG(WARNING) << "ConfigReloader.add_file_monitor : "
					"same callback exist, updated param to " << prm;
			return -1;
		}
	}

	config_monitor_t * pmon = new config_monitor_t;
	pmon->callback = proc;
	pmon->param = prm;
	_filecblist.push_back(pmon);
	return 0;
}

Configure * ConfigReloader :: get_config() {
	return _config[_config_curid];
}


void ConfigReloader :: _free_map(bsl::hashmap <bsl::string, config_monitor_t *> *map) {
	bsl::hashmap<bsl::string, config_monitor_t *>::iterator itr;	
	for(itr = map->begin(); itr != map->end(); ++itr) { 
		if (NULL != itr->second) {
			delete itr->second;
		}
	}
	if(0 != map->destroy())
		LOG(WARNING) << "ConfigReloader._free_map : "
				"map->destory() failed ";
}

void ConfigReloader :: _free_list(std::vector <config_monitor_t *> *list) {
	for(int i = 0; i < (int)list->size(); ++i){
		if (NULL != (*list)[i]) {
			delete (*list)[i];
		}
	}
}
ConfigReloader :: ~ConfigReloader() {
	_free_map(&_chrcbmap);
	_free_map(&_strcbmap);
	_free_map(&_intcbmap);
	_free_map(&_uint64cbmap);
	_free_map(&_floatcbmap);
	_free_map(&_groupcbmap);
	_free_list(&_filecblist);
	COMCFG_SAFEFREE(_config[0]);
	COMCFG_SAFEFREE(_config[1]);
}

}//namespace comcfg
