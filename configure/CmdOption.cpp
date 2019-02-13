/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: CmdOption.cpp,v 1.6 2009/11/10 04:50:30 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file CmdOption.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/29 21:50:42
 * @version $Revision: 1.6 $ 
 * @brief 
 *  
 **/

#include "CmdOption.h"
#include "ConfigUnit.h"
#include "utils/cc_utils.h"
#include <string.h>

namespace comcfg{
	CmdOption :: CmdOption(){
	}

	int CmdOption :: setOptString(const char * p){
		int ret = 0;
		const char *prev = NULL;
		memset(ch, 0, sizeof(ch));
		while(*p != '\0'){
			if(*p == ':'){
				if(prev == NULL || ch[(unsigned char)(*prev)] == CMD_OPT2){
					ret = -1;
					break;
				}
				else{
					++ ch[(unsigned char)(*prev)];
				}
			}
			else{
				if(ch[(unsigned char)(*p)]){
					ret = -1;
					break;
				}
				ch[(unsigned char)(*p)] = CMD_OPT0;
			}
			prev = p;
			++p;
		}
		return ret;
	}

	int CmdOption :: init(int argc, char * const * argv, const char * optstring){
		if(argv == NULL || optstring == NULL){
			LOG(WARNING) << "Null pointer in CmdOption::init.....";
			return -1;
		}
		if(setOptString(optstring) != 0){
			LOG(WARNING) << "optstring error.";
			return -1;
		}
		int ret = 0;
		//反序处理
		try{
			for(int i = argc - 1; i >= 0; --i){
				if(argv[i][0] == '-'){
					str_t key, value;
					//option
					if(argv[i][1] == '-'){
						//long option
						key = argv[i] + 2;
						size_t sz;// = key.find('=', 0);
						for(sz = 0; sz < key.size(); ++sz){
							if(key[sz] == '='){
								break;
							}
						}
						if(sz != key.size()){
							value = key.substr(sz+1);
							key = key.substr(0, sz);
							LOG(INFO) << "[CmdOption init - long opt] key=" << key.c_str() << " value=" << value.c_str();
						}
						if( push(key, new ConfigUnit(key, value, NULL, this)) ){
							ret = -1;
						}
					}

					else if(ch[(unsigned char)(argv[i][1])] == CMD_OPT0){
						//opt 0
						key = "";
						key.push_back(argv[i][1]);
						if( push(key, new ConfigUnit(key, value, NULL, this)) ){
							ret = -1;
						}
					}

					else if(ch[(unsigned char)(argv[i][1])] == CMD_OPT1){
						//opt 1
						key = "";
						key.push_back(argv[i][1]);
						value = argv[i] + 2;
						if(value.size() == 0){
							if(arg.size() == 0){
								LOG(WARNING) << "Option [-" << argv[i][1] << ":] without any argument.";
								ret = -1;
							}
							else{
								value = arg[arg.size() - 1];
								arg.pop_back();
							}
						}
						if( push(key, new ConfigUnit(key, value, NULL, this)) ){
							ret = -1;
						}
					}

					else if(ch[(unsigned char)(argv[i][1])] == CMD_OPT2){
						//opt 2
						key = ""; 
						key.push_back(argv[i][1]);
						value = argv[i] + 2;
						if(value.size() == 0){
							LOG(WARNING) << "Option [-" << argv[i][1] << "::] without any argument.";
							ret = -1;
						}
						if( push(key, new ConfigUnit(key, value, NULL, this)) ){
							ret = -1;
						}
					}
					else{
						LOG(WARNING) << "Option [-" << argv[i][1] << "] unknown.";
						ret = -1;
					}
				}
				else{
					arg.push_back(str_t(argv[i]));
				}
			}
		}
		catch(...){
			LOG(WARNING) << "Option error..";
			ret = -1;
		}
		return ret;
	}

	bool CmdOption :: hasOption(const char * s) const {
		return (*this)[s].selfType() != CONFIG_ERROR_TYPE;
	}
	size_t CmdOption :: argSize() const {
		return arg.size();
	}
	const char * CmdOption :: getArg(int n) const {
		if(n >= (int)arg.size()){
			return NULL;
		}
		//反序的
		return arg[(int)arg.size() - n - 1].c_str();
	}
	int CmdOption :: registHook(const char * option, hook_t hook){
		pair_t p;
		p.first = str_t(option);
		p.second = hook;
		hkmap.insert(p);
		return 0;
	}
	int CmdOption :: runHook(){
		itr_t itr;
		for(itr = fieldMap.begin(); itr != fieldMap.end(); ++itr){
			hk_pair_t p = hkmap.equal_range(itr->first);
			for(hkitr_t hkitr = p.first; hkitr != p.second; ++hkitr){
				(hkitr->second)(this);
			}
		}
		return 0;
	}
}

















/* vim: set ts=4 sw=4 sts=4 tw=100 */
