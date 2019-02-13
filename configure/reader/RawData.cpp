/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: RawData.cpp,v 1.11 2010/01/08 03:24:38 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file RawData.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/21 20:49:06
 * @version $Revision: 1.11 $ 
 * @brief 
 *  
 **/

#include "RawData.h"
#include <fcntl.h>

namespace comcfg{
	size_t RawData :: lineNum(){
		return _lnum;
	}

	size_t RawData :: size(){
		return _fsize;
	}

	const char * RawData :: getLine(int idx){
		if(idx < 0 || (size_t)idx >= lineNum()){
			return NULL;
		}
		return (const char *)(_file + _line[idx]);
	}

	RawData :: RawData(){
		_file = NULL;
		_file_original = NULL;
		_lnum = 0;
		_fsize = 0;
		_bufsize = 0;
	}
	RawData :: ~RawData(){
		if(_file){
			free(_file);
		}
		if(_file_original){
			free(_file_original);
		}
	}

	int RawData :: load(const char * filename){
		LOG(INFO) << "Config : RawData read file [" << filename << "]";
		struct stat st;
		if( stat(filename, &st) != 0 ){
			LOG(WARNING) << "I can't stat [" << filename << "]";
			return -1;
		}

		int fd = open(filename, O_RDONLY);
		if(-1 == fd){
			LOG(WARNING) << "I can't open [" << filename << "]";
			return -1;
		}
		if(_file){
			free(_file);
			_file = NULL;
		}
		_fsize = (size_t)st.st_size;
		_bufsize = ((_fsize >> 12) + 1) << 12;//以4K为单位上取整

		_file = (char *)malloc(_bufsize);
		if(NULL == _file){
			LOG(WARNING) << "malloc failed.";
			close(fd);
			return -1;
		}

		if( read(fd, _file, _fsize) != (ssize_t)_fsize ){
			LOG(WARNING) << "Read file " << filename << " error...";
			free(_file);
			_file = NULL;
			close(fd);
			return -1;
		}

		close(fd);
		return rebuild();
#if 0
		_line.clear();

		try{
			_file = (char *)malloc(_bufsize);
			if(NULL == _file){
				ul_writelog(Log::warning(), "malloc failed...I can't load [%s]", filename);
				throw ConfigException("Malloc failed");
			}
			size_t cur_pos = 0;

			while(fgets(_file + cur_pos, _bufsize - cur_pos, fd) != NULL){
				_line.push_back(cur_pos);
				cur_pos += strlen(_file + cur_pos);
				if(cur_pos <= 0 || _file[cur_pos - 1] != '\n'){
					ul_writelog(Log::warning(), "Unrecognized end of line. file:[%s] line:[%d]",
							filename, (int)_line.size());
				}
				_file[cur_pos - 1] = '\0';
			}
			_lnum = _line.size();
			_line.push_back(cur_pos);
			_line.push_back(cur_pos);//保护
			if(cur_pos != size()){
				ul_writelog(Log::warning(), "[%s] file size = %llu bytes, But I only read = %llu bytes",
						filename, (unsigned long long)size(), (unsigned long long)cur_pos);
			}

		}catch(...){
			if(_file){
				free(_file);
				_file = NULL;
			}
			_line.clear();
			_fsize = _bufsize = 0;
			_lnum = 0;
			return -1;
		}
		return 0;
#endif
	}

	char * RawData :: getRebuildBuffer(size_t __size){
		if(_file){
			free(_file);
			_file = NULL;
		}
		_line.clear();
		_lnum = 0;
		_fsize = __size;
		_bufsize = ((_fsize >> 12) + 1) << 12;//以4K为单位上取整
		_file = (char *)malloc(_bufsize);
		return _file;
	}

	int RawData :: rebuild(){
		if(_fsize == 0){
			LOG(WARNING) << "The rebuild buffer is empty.";
			return -1;
		}
		if(_file_original != NULL){
			free(_file_original);
			_file_original = NULL;
		}
		_file_original = (char *)malloc(_bufsize);
		if(_file_original != NULL){
			memcpy(_file_original, _file, _bufsize);
		}
		else{
			LOG(INFO) << "Failed to create a copy of file.";
			//so I can't dump it. But it's OK to go on.
		}
		_line.clear();
		_line.push_back(0);
		for(size_t i = 0; i < _fsize; ++i){
			if(_file[i] == '\n' || _file[i] == '\r' || _file[i] == '\0'){
				_file[i] = '\0';
				_line.push_back(i+1);
			}
		}
		_file[_fsize] = '\0';
		_lnum = _line.size();
		_line.push_back(_fsize);//多余的一行的位置，便于计算最后一行的长度
		return 0;
	}

	char * RawData :: dump(size_t * __size){
		*__size = _fsize;
		return _file_original;
	}

}


















/* vim: set ts=4 sw=4 sts=4 tw=100 */
