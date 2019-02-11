/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_filestream.cpp,v 1.5 2008/12/15 09:56:59 xiaowei Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file bsl_filestream.cpp
 * @author xiaowei(com@baidu.com)
 * @date 2008/07/25 17:17:56
 * @version $Revision: 1.5 $ 
 * @brief 
 *  
 **/

#include <stdlib.h>
#include <string.h>
#include <bsl/archive/bsl_filestream.h>

#include <bsl/utils/bsl_debug.h>

namespace bsl
{
filestream::filestream() : _fp(NULL), _trans(0) 
{
	_wbuffersize = 0;
	_wbuffer = 0;
	_rbuffer = 0;
	_rbuffersize = 0;
}
filestream::~filestream()
{
	close();
}
int filestream::open(const char *fname, const char *mode)
{
	if (fname == NULL || mode == NULL) {
		__BSL_ERROR("invalid parama\n");
		return -1;
	}
	//²ÎÊý¼ì²é
	_trans = 0;
	_fp = ::fopen(fname, mode);
	if (_fp == NULL) {
		__BSL_ERROR("open %s file fail err[%m]", fname);
		return -1;
	}
	_trans = ::fseek(_fp, 0, SEEK_CUR);
	return 0;
}

int filestream::close()
{
	if (_fp != NULL) {
		::fclose(_fp);
	}
	_fp = NULL;
	if (_rbuffer) {
		free (_rbuffer);
	}
	if (_wbuffer) {
		free (_wbuffer);
	}
	_rbuffer = _wbuffer = 0;
	_rbuffersize = _wbuffersize = 0;
	return 0;
}

int filestream::setwbuffer(size_t size)
{
	if (_fp == NULL) {
		__BSL_ERROR("invalid file pointer");
		return -1;
	}
	if (_wbuffer) {
		free (_wbuffer);
	}
	_wbuffersize = 0;
	_wbuffer = 0;
	if (size == 0) {
		setvbuf(_fp, NULL, _IONBF, BUFSIZ);
		return 0;
	}

	_wbuffer = (char *)malloc(size);
	if (_wbuffer) {
		_wbuffersize = size;
		setbuffer(_fp, _wbuffer, size);
		return 0;
	}
	return -1;
}

int filestream::setrbuffer(size_t size)
{
	if (_fp == NULL) {
		__BSL_ERROR("invalid file pointer");
		return -1;
	}
	if (_rbuffer) {
		free (_rbuffer);
	}
	_rbuffer = 0;
	_rbuffersize = 0;
	if (size == 0) return 0;

	_rbuffer = (char *)malloc(size);
	if (_rbuffer) {
		_rbuffersize = size;
		return 0;
	}
	return -1;
}
int filestream::write(const void *dat, size_t size)
{
	if (_fp == NULL) {
		__BSL_ERROR("invalid file pointer");
		return -1;
	}
	if (size == 0) {
		return 0;
	}
	if (dat == NULL) {
		__BSL_ERROR("input NULL pointer");
		return -1;
	}
	size_t ret = ::fwrite(dat, 1, size, _fp);
	if (ret == (size_t)size) {
		_trans_off += size;
		return 0;
	}
	__BSL_ERROR("write data to filesystem wan[%ld] rel[%ld] error[%m]", (long)size, (long)ret);
	if (ret > 0) {
		::fseek(_fp, -(long)ret, SEEK_CUR);
		return -1;
	}
	return -1;
}

int filestream::read(void *dat, size_t size)
{
	if (_fp == NULL) {
		__BSL_ERROR("invalid file pointer");
		return -1;
	}
	if (size == 0) {
		return 0;
	}
	if (dat == NULL) {
		__BSL_ERROR("read %zu data to NULL pointer", size);
		return -1;
	}
	size_t ret = ::fread(dat, 1, size, _fp);
	if (ret == (size_t)size) {
		_trans_off += size;
		return 0;
	}
	__BSL_ERROR("read data from filesystem wan[%ld] rel[%ld] error[%m]", (long)size, (long)ret);
	if (ret > 0) {
		::fseek(_fp, -(long)ret, SEEK_CUR);
		return -1;
	}
	return -1;
}
int filestream::start_trans()
{
	_trans_off = 0;
	//_trans = ::fseek(_fp, 0, SEEK_CUR);
	return 0;
}
int filestream::comit_trans()
{
	_trans += _trans_off;
	//_trans = ::fseek(_fp, 0, SEEK_CUR);
	return 0;
}

int filestream::drop_trans(bool trunc)
{
	if (_fp == NULL) {
		__BSL_ERROR("invalid file pointer");
		return -1;
	}
	int ret = ::fseek(_fp, _trans, SEEK_SET);
	if (ret == int(-1)) return -1;
	if (trunc && ::ftruncate(::fileno(_fp), _trans) != 0) {
		return -1;
	}
	return 0;
}

int filestream::flush() 
{ 
	if (_fp == NULL) {
		__BSL_ERROR("invalid file pointer");
		return -1;
	}
	return ::fflush(_fp); 
}

};

/* vim: set ts=4 sw=4 sts=4 tw=100 */
