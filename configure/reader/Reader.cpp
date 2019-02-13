/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: Reader.cpp,v 1.8 2009/03/23 03:14:52 yingxiang Exp $ 
 * 
 **************************************************************************/



/**
 * @file reader/Reader.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/21 18:49:01
 * @version $Revision: 1.8 $ 
 * @brief 
 *  
 **/

#include <ctype.h>
#include "utils/cc_utils.h"
#include "reader/Reader.h"

namespace comcfg{
	Reader :: Reader(){
		_isRebuild = false;
	}

	str_t get_middle(const str_t& str){
		int i, j;
		if(str.size() == 0){
			return str;
		}
		for(i = 0; i < (int)str.size(); ++i){
			if(! isspace(str[i])){
				break;
			}
		}
		for(j = (int)str.size() - 1; j >= 0; --j){
			if(! isspace(str[j])){
				break;
			}
		}
		return str.substr(i, j+1-i);
	}

	int Reader::read(const char * file, Configure * cfg){
		if( _fd.load(file) != 0 ){
			return -1;
		}
		_cur_file = file;
		return build(cfg);
	}
	int Reader :: build(Configure *cfg){
		int ret = 0;
		for(size_t i = 0; i < _fd.lineNum(); ++i){
			_cur_line = i+1;
			cfg->setFileLine(_cur_line);
			size_t length;
			if(i != _fd.lineNum() - 1){
				length = (size_t)_fd.getLine(i+1) - (size_t)_fd.getLine(i);
			}
			else{
				length = (size_t)_fd.size() - (size_t)_fd.getLine(i);
			}
			if(push(cfg, _fd.getLine(i), length) != 0){
				LOG(WARNING) << "Error [File:" << _cur_file.c_str() << ", Line:" << i + 1 << "] " <<  _fd.getLine(i);
				ret = -1;
			}
		}
		_cur_line = -1;
		return ret;
	}

	int Reader::push(Configure * cfg, const char * buf, size_t length){
		const char *p = buf;
		while(*p != 0 && isspace(*p)){
			++p;
		}
		try{
			if(*p == '#' || *p == 0){ //is comment or blank line
				return 0;
			}
			else if (*p == '['){ //must be section
				str_t str;
				if( sectionParser(p, length, &str) != 0 ){
					return -1;
				}
				LOG(INFO) << "Configure Reader: Change section to : [" << str.c_str() << "]";
				cfg->changeSection(str);
			}
			else{ // is config key-value pair
				pair_t pair;
				if( keyParser(p, length, &pair) != 0 ){
					return -1;
				}
				if(strcmp(pair.key.c_str(), CONFIG_INCLUDE) == 0){
					//$include : filename
					//Sub Configure file
					LOG(INFO) << "Configure Reader: read $include : " << pair.value.c_str();
					if(_isRebuild){
						LOG(INFO) << "Configure : I am rebuilding, $include is ignored.";
					}else{
						str_t realfile;
						Trans::str2str(pair.value, &realfile);
						cfg->pushSubReader(realfile.c_str());
					}
				}
				else{
					LOG(INFO) << "Configure Reader: read key[" << pair.key.c_str() << "] value[" << pair.value.c_str() <<"]";
					cfg->pushPair(pair.key, pair.value);
				}
			}
		}
		catch(ConfigException){
			return -1;
		}
		catch(...){
			LOG(WARNING) << "Configure Reader: Unknown exception.";
			return -1;
		}
		return 0;
	}

	int Reader::sectionParser(const char * buf, size_t /*length*/, str_t * str){
		const char *p = buf;
		const char *end = buf;
		while(*end != 0 && *end != ']'){
			++end;
		}
		if(0 == *end || p+1 >= end){
			return -1;
		}
		if(getWord(p+1, end, str) != 0){
			return -1;
		}
		++end;
		//检查末尾没有多余字符
		while(*end){
			if(*end == '#'){
				return 0;
			}
			if(! isspace(*end)){
				return -1;
			}
			++end;
		}
		return 0;
	}


	int Reader::keyParser(const char * buf, size_t /*length*/, pair_t * pair){
		const char *p = buf;
		while(*p != 0 && *p != ':'){
			++p;
		}
		if(0 == *p || p == buf){ //到达末尾或在开头
			return -1;
		}
		if( getWord(buf, p, &(pair->key)) != 0 ){
			return -1;
		}

		pair->value = p+1;
		//ul_writelog(Log::debug(), "Configure Reader : key=[%s] whole value=[%s]", pair->key.c_str(), pair->value.c_str());
		pair->value = get_middle(pair->value);
		//value : 引号转义？
		return 0;
	}


	int Reader::getWord(const char * from, const char * to, str_t * str){
		//左闭右开区间[from, to)
		int status = 0; //空格0->字符串1->空格2
		const char *f1 = from;
		const char *f2 = to;
		for(const char *i = from; i < to; ++i){
			if(isspace(*i)){
				if(status == 1){
					f2 = i;
					status = 2;
				}
			}
			else{
				if(status == 0){
					status = 1;
					f1 = i;
				}
				else if(status == 2){
					return -1;
				}
			}
		}
		if(f1 >= f2){
			return -1;
		}
		//如果先clear str，再str->append(f1, f2-f1)的话：
		//这里是有风险的,如果传入的是 getWord(str.begin(), str.end(), &str) 会挂
		//但是bsl::string不支持assign接口，只能先clear再append
		//如果创建一个新对象，再copy回来，成本又太高，bsl::string不是copy-on-write
		//还是copy一次吧：
		str_t tmp;
		tmp.append(f1, f2-f1);
		*str = tmp;
		return 0;
	}

	void Reader::getCommentUpwards(str_t &comment, int line){
		const char * conf_line = _fd.getLine(line);
		while(isCommentLine(conf_line)){
			str_t comment_line = conf_line;
			comment_line.append("\n");
			comment_line.append(comment);
			comment = comment_line;
			--line;
			conf_line = _fd.getLine(line);
		}
	}

	int Reader::isCommentLine(const char *line){
		if(line == NULL){
			return 0;
		}
		while(*line != 0 && isspace(*line)){
			++ line;
		}
		if(*line == '#' || *line == 0){
			return 1;
		}
		return 0;
	}

	char * Reader :: getRebuildBuffer(size_t size){
		return _fd.getRebuildBuffer(size);
	}
	int Reader :: rebuild(Configure * cfg){
		_isRebuild = true;
		if( _fd.rebuild() != 0 ){
			return -1;
		}
		_cur_file = "./"REBUILD_CONF;
		return build(cfg);
	}
	char * Reader :: dump(size_t * size){
		return _fd.dump(size);
	}
}











/* vim: set ts=4 sw=4 sts=4 tw=100 */
