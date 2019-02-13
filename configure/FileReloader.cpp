/***************************************************************************
 * 
 * Copyright (c) 2010 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file FileReloader.cpp
 * @author zhang_rui(com@baidu.com)
 * @date 2010-2-1
 * @brief 
 *  
 **/

#include "FileReloader.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

namespace comcfg {

static time_t get_file_time(const char * filename) {

	if (NULL == filename) {
		LOG(WARNING) << "FileReloader.add_file_monitor : NULL filename";
		return -1;
	}
	time_t t = time_t(0);
	struct stat st;
	if(stat(filename, &st) == 0){
		if(t < st.st_mtime){
			t = st.st_mtime;
		}
	} else {
		LOG(WARNING) << "FileReloader add_file_monitor : Can not stat file " << filename;
		return -1;
	}
	return t;
}

int FileReloader :: monitor() {
	int ret = 0;
	bsl::hashmap <bsl::string, config_filemonitor_t *> :: iterator  itr;
	for(itr = _filemap.begin(); itr != _filemap.end(); ++itr){
		config_filemonitor_t * fm = itr->second;
		time_t tnow = get_file_time(itr->first.c_str());
		if (tnow<0) {
			return -1;
		}
		if (tnow > fm->_last_modify) {
			fm->_last_modify = tnow;
			fm->callback(itr->first.c_str(), fm->param);
			++ret;
		}
	}
	return ret;
}

int FileReloader :: add_file_monitor(const char * filename, file_change_cb_t proc, void * prm) {
	if (NULL == filename) {
		LOG(WARNING) << "FileReloader.add_file_monitor : NULL filename";
		return -1;
	}
	if (NULL == proc) {
		LOG(WARNING) << "FileReloader.add_file_monitor : NULL callback";
		return -1;
	}
	if (!_filemap.is_created())
	{
		LOG(WARNING) << "Initial FileReloader : create filemap";
		_filemap.create(256);
	}
	time_t t = get_file_time(filename);
	if (0 > t) {
		return -1;
	}

	config_filemonitor_t * pmon = new config_filemonitor_t;
	pmon->callback = proc;
	pmon->param = prm;
	pmon->_last_modify = t;

	bsl::string kstr = filename;
	bsl::hashmap <bsl::string, config_filemonitor_t *> :: _Pair *itr;
	itr = _filemap.find(kstr);
	if(NULL != itr) {
		if (NULL != itr->second){
			delete itr->second;
		}
		LOG(WARNING) << "FileReloader.add_file_monitor : same file("<< filename << ") exist, update callback(" << proc << "), param(" << prm << ")";
	}
	_filemap.set(kstr,pmon,1);
	return 0;
}

FileReloader :: ~FileReloader() {
	bsl::hashmap<bsl::string, config_filemonitor_t *>::iterator itr;	
	for(itr = _filemap.begin(); itr != _filemap.end(); ++itr) { 
		if (NULL != itr->second) {
			delete itr->second;
		}
	}
	if(0 != _filemap.destroy()){
		LOG(WARNING) << "FileReloader.~FileReloader : _filemap.destory() failed ";
	}
}

}//namespace comcfg
