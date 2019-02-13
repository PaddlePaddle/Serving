/***************************************************************************
 * 
 * Copyright (c) 2009 Baidu.com, Inc. All Rights Reserved
 * $Id: cfgext.cpp,v 1.4 2009/11/10 04:50:30 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file cfgext.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2009/03/10 21:48:10
 * @version $Revision: 1.4 $ 
 * @brief 
 *  
 **/

#include "cfgext.h"
#include "idl_conf_if.h"
#include "Configure.h"

#define ARRAY_FUNC "array"
#define COMMENT_FUNC "comment"
#define DEFAULT_FUNC "default"

namespace comcfg{

	// is basic type
	// in constraint/Constraint.cpp
	extern bool isBaseType(const str_t & s);

	void printBaseType(confIDL::idl_t * idl, confIDL::var_t * var, str_t * conf, str_t * section, bool isArray);
	void printGroup(confIDL::idl_t * idl, confIDL::var_t * var, str_t * conf, str_t * section, bool isArray);
	void printVar(confIDL::idl_t * idl, confIDL::var_t * var, str_t * conf, str_t * section);

	void printBaseType(confIDL::idl_t * /*idl*/, confIDL::var_t * var, str_t * conf, str_t * /*section*/, bool isArray){
		//comment
		confIDL::cf_map_t::iterator cfitr = var->cf_map.find(str_t(COMMENT_FUNC));
		if(cfitr != var->cf_map.end() && cfitr->second->arg_list.size() > 0){
			conf->appendf("#%s\n", cfitr->second->arg_vec[0]->data.c_str());
		}
		//key
		conf->appendf("%s%s : ", isArray?"@":"", var->name.data.c_str());

		//default value
		cfitr = var->cf_map.find(str_t(DEFAULT_FUNC));
		if(cfitr != var->cf_map.end() && cfitr->second->arg_list.size() > 0){
			conf->appendf("%s", cfitr->second->arg_vec[0]->data.c_str());
		}
		//end of line
		conf->append("\n");
	}

	void printGroup(confIDL::idl_t * idl, confIDL::var_t * var, str_t * conf, str_t * section, bool isArray){
		confIDL::group_map_t::iterator gitr = idl->group_map.find(var->type.data);
		if(gitr == idl->group_map.end()){
			throw ConfigException() << BSL_EARG << "Unknown type : " << var->type.data.c_str() << " [ " 
				<< var->type.file.c_str() << " : " << var->type.lineno << " }";
		}
		if(*section == str_t(CONFIG_GLOBAL)){
			*section = "";
			section->appendf("%s%s", isArray?"@":"", var->name.data.c_str());
		}
		else{
			if(strstr(section->c_str(), "@") != NULL){
				//前面的已经有数组了，只能使用相对位置
				str_t tmp;
				for(int i = 0; i < (int)section->size(); ++i){
					if((*section)[i] == '.'){
						tmp.append(".");
					}
				}
				*section = tmp;
			}
			section->appendf(".%s%s", isArray?"@":"", var->name.data.c_str());
		}
		conf->appendf("\n[%s]\n", section->c_str());
		confIDL::var_list_t::iterator vitr;
		str_t bak_section = *section;
		for(vitr = gitr->second->var_list.begin(); vitr != gitr->second->var_list.end(); ++vitr){
			if(isBaseType((*vitr)->type.data)){
				printVar(idl, *vitr, conf, section);
			}
		}
		for(vitr = gitr->second->var_list.begin(); vitr != gitr->second->var_list.end(); ++vitr){
			if(! isBaseType((*vitr)->type.data)){
				if(*section != bak_section){
					*section = bak_section;
					//conf->appendf("\n[%s]\n", section->c_str());
				}
				printVar(idl, *vitr, conf, section);
			}
		}
	}	



	void printVar(confIDL::idl_t * idl, confIDL::var_t * var, str_t * conf, str_t * section){
		bool isArray = false;
		int arrSize = 1;

		confIDL::cf_map_t::iterator cfitr = var->cf_map.find(str_t(ARRAY_FUNC));
		if(cfitr != var->cf_map.end()){
			isArray = true;
			unsigned long long s;
			if(cfitr->second->arg_list.size() > 0){
				if(Trans::str2uint64(cfitr->second->arg_vec[0]->data, &s) == 0){
					arrSize = (int)s;
				}
			}
		}

		if(arrSize <= 0){
			arrSize = 1;
		}

		str_t bak_section = *section;
		//*section = bak_section;
		if(isBaseType(var->type.data)){
			printBaseType(idl, var, conf, section, isArray);
		}
		else{
			printGroup(idl, var, conf, section, isArray);
		}
		//增加注释的元素
		if (1 < arrSize)
		{
			//把数组元素的配置字符串写到cmt中
			str_t cmt = "";
			str_t cmted = "";
			*section = bak_section;
			if(isBaseType(var->type.data)){
				printBaseType(idl, var, &cmt, section, isArray);
			}
			else{
				cmted.append("\n");
				printGroup(idl, var, &cmt, section, isArray);
			}
			cmt.append("\n");
			//对cmt中每行字符串前面增加#，改写到cmted中
			int prelinehead = 0;
			for (int i=0; i<(int) cmt.size(); ++i)
			{
				if ('\n'==cmt[i])
				{
					if (i>prelinehead){
						cmted.append("#").append(cmt.substr(prelinehead, i - prelinehead + 1));
					}
					prelinehead = i+1;
				}
			}
			//将注释的配置字符串cmted加到conf中
			conf->append(cmted);
		}
	}


	str_t autoConfigGen(const char * range_file){
		confIDL::idl_t * idl;
		idl = confIDL::alloc_idl();

		load_idl(range_file, idl);
		if(idl->retval != 0){
			confIDL:: err_list_t :: iterator itr;
			ConfigException e;
			e << range_file << ": Read File Error\n";
			for(itr = idl->err_list.begin(); itr != idl->err_list.end(); ++itr){
				e << itr->file.c_str() << ":" << itr->lineno << " " << itr->data.c_str() << "\n";
			}
			free_idl(idl);
			throw e;
		}

		confIDL :: var_list_t :: iterator vitr;
		str_t conf;
		str_t section = CONFIG_GLOBAL;
		for(vitr = idl->var_list.begin(); vitr != idl->var_list.end(); ++vitr){
			if(isBaseType((*vitr)->type.data) && section != CONFIG_GLOBAL){
				conf.append("\n[" CONFIG_GLOBAL "]\n");
			}
			section = CONFIG_GLOBAL;
			printVar(idl, *vitr, &conf, &section);
		}
		free_idl(idl);
		return conf;
	}












}//namespace comcfg

/* vim: set ts=4 sw=4 sts=4 tw=100 */
