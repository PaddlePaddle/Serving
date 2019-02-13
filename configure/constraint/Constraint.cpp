/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: Constraint.cpp,v 1.14 2009/11/10 04:50:30 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file Constraint.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/28 21:17:07
 * @version $Revision: 1.14 $ 
 * @brief 
 *  
 **/
#include <set>
#include "Constraint.h"
#include "ConstraintFunction.h"
#include "utils/cc_utils.h"
#include "idl_conf_if.h"

#define ARRAY_FUNC "array"
#define CONSTRAINT_FUNC "constraint"
namespace comcfg{
	static const str_t baseType[] = {
		CC_CHAR,
		CC_UCHAR,
		CC_INT16,
		CC_UINT16,
		CC_INT32,
		CC_UINT32,
		CC_INT64,
		CC_UINT64,
		CC_FLOAT,
		CC_DOUBLE,
		CC_STRING
	};
	static std::set <str_t> baseSet(baseType, baseType + (sizeof(baseType)/sizeof(baseType[0])));

	//also used by other *.cpp 
	//like : cfgext.cpp
	bool isBaseType(const str_t & s){
		return baseSet.find(s) != baseSet.end();
	}

	//约束函数需要有序执行，这个是比较函数
	class CF_cmp_t{
		public:
			int operator()(const confIDL::cons_func_t * a, const confIDL::cons_func_t * b)const{
				return ConstraintLibrary :: getLevel_static(a->func.data) < 
					ConstraintLibrary :: getLevel_static(b->func.data);
			}
	};

	str_t showIDLMsg(const confIDL::meta_t& meta){
		str_t s;
		s.appendf("[File:%s Line:%d]=[%s]", meta.file.c_str(), meta.lineno, meta.data.c_str());
		return s;
	}

	int Constraint :: printError(){
		if(idl == NULL){
			return -1;
		}
		LOG(WARNING) << " =========== We have error in Constraint file, see below ============";
		confIDL:: err_list_t :: iterator itr;
		for(itr = idl->err_list.begin(); itr != idl->err_list.end(); ++itr){
			LOG(WARNING) << "Constraint Error : " << showIDLMsg((*itr)).c_str();
		}
		return 0;
	}

	int Constraint :: run(confIDL::idl_t * __idl, Configure * __conf){
		idl = __idl;
		conf = __conf;

		if(idl == NULL || conf == NULL){
			return -1;
		}
		if(idl->retval != 0){
			printError();
			return -1;
		}
		
		confIDL:: var_list_t :: iterator itr;
		//=== init ===
		//排序，保证约束函数有序执行
		for(itr = idl->rc_var_list.begin(); itr != idl->rc_var_list.end(); ++itr){
			(*itr)->cf_list.sort(CF_cmp_t());
		}

		//check all variables
		int result = 0;
		for(itr = idl->var_list.begin(); itr != idl->var_list.end(); ++itr){
			try{
				if( runVar(*itr, conf) != 0 ){
					throw ConfigException();
				}
			}
			catch(...){
				result = -1;
			}
		}
		return result;
	}

	int Constraint :: runVar(confIDL::var_t* var, ConfigGroup * father, ConfigUnit * self){
		int ret = 0;

		//constraint()约束，重新约束
		confIDL :: cf_map_t :: iterator cfitr = var->cf_map.find(str_t(CONSTRAINT_FUNC));
		if(cfitr != var->cf_map.end() && cfitr->second->arg_vec.size() > 0 ){
			LOG(INFO) << "Re-constraint " << showIDLMsg(var->name).c_str() << " to " << cfitr->second->arg_vec[0]->data.c_str();
			var->type.data = cfitr->second->arg_vec[0]->data;
		}
		confIDL:: cf_list_t :: const_iterator itr = var->cf_list.begin(); 
		str_t plog;
		for(itr = var->cf_list.begin(); itr != var->cf_list.end(); ++itr){
			plog.append((*itr)->func.data).append("();");
		}
		LOG(INFO) << "Config : var[" << showIDLMsg(var->name).c_str() << "], func[" << plog.c_str() << "]";
		for(itr=var->cf_list.begin(); itr != var->cf_list.end(); ++itr){
			if( (*itr)->func.data == str_t(ARRAY_FUNC)){
				try{
					LOG(INFO) << "Pre-process array constraint : " << showIDLMsg(var->name).c_str();
					ret = ConstraintLibrary :: getInstance() -> check(*(*itr), *var, father, self);
				}catch(ConfigException){
					//...
					LOG(WARNING) << "Pre-process Array error...";
					return -1;
				}
				if(ret){
					return ret;
				}
				break;
			}
		}
#if 1
		const ConfigUnit & myref = (self == NULL) ? (*father)[var->name.data] : *self;
		//array
		if(myref.selfType() == CONFIG_ARRAY_TYPE){
			const ConfigUnit & arr = myref;
			LOG(INFO) << "Constraint : check array elements[" << arr.size() << "]";
			confIDL::var_t xvar = *var;
			if(xvar.cf_map.find(str_t(ARRAY_FUNC)) == xvar.cf_map.end()){
				LOG(WARNING) << "Constraint : " << arr[0].info().c_str() << " should not be array! " << showIDLMsg(xvar.name).c_str();
				return -1;
			}
			xvar.cf_map.erase(str_t(ARRAY_FUNC));
			//xvar.cf_list.erase(xvar.cf_list.begin());//第一个一定是array
			for(confIDL::cf_list_t::iterator arritr = xvar.cf_list.begin(); arritr != xvar.cf_list.end(); ++arritr){
				if((*arritr)->func.data == str_t(ARRAY_FUNC)){
					xvar.cf_list.erase(arritr);
					break;
				}
			}
			for(int i = 0; i < (int)arr.size(); ++i){
				ConfigUnit * unit = const_cast<ConfigUnit*>(&arr[i]);
				try{
					if( runVar(&xvar, NULL, unit) != 0 ){
						ret = -1;
					}
				}catch(...){
					LOG(WARNING) << "Constaint : invalid value " << unit->info().c_str() << " : " << showIDLMsg(xvar.name).c_str();
					ret = -1;
				}
			}
		}
#endif
		else {
			if( isBaseType(var->type.data) ){
				ret = runKey(var, father, self);
			}
			else{
				ret = runGroup(var, father, self);
			}
		}
		return ret;
	}

	int Constraint :: runGroup(confIDL::var_t* var, ConfigGroup * father, ConfigUnit * self){
		LOG(INFO) << "Constaint : working as group : " << showIDLMsg(var->name).c_str() << " [type:" << var->type.data.c_str() << "]";
#if 0
		if(father != NULL){
			printf("----father----\n");
			father->print();
		}
		if(self != NULL){
			printf("----self----\n");
			self->print();
		}
#endif
		confIDL:: group_map_t :: iterator mpitr = idl->group_map.find(var->type.data);
		int ret = 0;
		if( mpitr == idl->group_map.end() ){ // No such type
			LOG(WARNING) << "Constraint : error constraint type : " << showIDLMsg(var->type).c_str();
			throw ConfigException();
		}
		else{
			//group
			ConfigUnit * gunit;
			if(father == NULL){ // is an element of array
				LOG(INFO) << var->name.data.c_str() << " is one of group array";
				gunit = self;
			}
			else{
				gunit = const_cast <ConfigUnit *> (&((*father)[var->name.data]));
			}

			if(gunit == NULL || gunit->selfType() == CONFIG_ERROR_TYPE){
				LOG(INFO) <<  "No such section in config : " << showIDLMsg(var->name).c_str() << ". try to create."; 
				gunit = new ConfigGroup(var->name.data.c_str(), father);
				father->push(var->name.data, gunit);
			}
			if(gunit->selfType() != CONFIG_GROUP_TYPE){
				LOG(WARNING) << var->name.data.c_str() << " should be a Group type.";
				return -1;
			}
			confIDL:: var_list_t :: iterator gitr;
			for(gitr = mpitr->second->var_list.begin(); gitr != mpitr->second->var_list.end(); ++gitr){
				try{//run constraints for every fields of the group
					if( runVar(*gitr, (ConfigGroup *)gunit, NULL) != 0){
						throw ConfigException();
					}
				}
				catch(...){
					ret = -1;
				}
			}
		}
		return ret;
	}

	int Constraint :: runKey(confIDL::var_t* var, ConfigGroup * father, ConfigUnit * self){
		//run一开始有对约束函数排序，然后逐一执行
		LOG(INFO) << "Constaint : working as simple type : " << showIDLMsg(var->name).c_str();
#if 0
		if(father != NULL){
			printf("----father----\n");
			father->print();
		}
		if(self != NULL){
			printf("----self----\n");
			self->print();
		}
#endif
		int ret = 0;
		confIDL:: cf_list_t :: const_iterator itr;
		for(itr = var->cf_list.begin(); itr != var->cf_list.end(); ++itr){
			if(var->cf_map.find((*itr)->func.data) == var->cf_map.end()){
				continue;
			}
			try{
				LOG(INFO) << "Constaint : " << (*itr)->func.data.c_str();
				ret = ConstraintLibrary :: getInstance() -> check(*(*itr), *var, father, self);
				if(ret){
					return ret;
				}

				ConfigUnit & gunit = const_cast<ConfigUnit &>((self == NULL) ? (*father)[var->name.data] : *self);
				if(gunit.selfType() == CONFIG_UNIT_TYPE){
					gunit.setConstrainted(true);
				}
			}catch(ConfigException){
				const ConfigUnit *unit = self;
				if(unit == NULL){
					unit = &(*father)[var->name.data];
				}
				LOG(WARNING) << "Constaint : invalid value " << unit->info().c_str() << " : " << showIDLMsg(var->name).c_str();
				ret = -1;
			}
			catch(...){
				LOG(WARNING) << "Constaint : Unknown exception";
				ret = -1;
			}

		}
		if(ret){
			return ret;
		}
		const ConfigUnit *unit = self;
		if(unit == NULL){
			unit = &(*father)[var->name.data];
		}
		ret = runType(var->type.data, *unit);
		if(ret){
			LOG(WARNING) << "Constaint : Value is not a valid [" << var->type.data.c_str() << "]type " << unit->info().c_str() << " : " << showIDLMsg(var->name).c_str();
		}
		return ret;
	}


	int Constraint :: runType(const str_t& type, const ConfigUnit & unit){
		try{
			if(strcmp(type.c_str(), CC_CHAR) == 0){
				unit.to_char();
				return 0;
			}

			if(strcmp(type.c_str(), CC_UCHAR) == 0){
				unit.to_uchar();
				return 0;
			}

			if(strcmp(type.c_str(), CC_INT16) == 0){
				unit.to_int16();
				return 0;
			}

			if(strcmp(type.c_str(), CC_UINT16) == 0){
				unit.to_uint16();
				return 0;
			}

			if(strcmp(type.c_str(), CC_INT32) == 0){
				unit.to_int32();
				return 0;
			}

			if(strcmp(type.c_str(), CC_UINT32) == 0){
				unit.to_uint32();
				return 0;
			}

			if(strcmp(type.c_str(), CC_INT64) == 0){
				unit.to_int64();
				return 0;
			}

			if(strcmp(type.c_str(), CC_UINT64) == 0){
				unit.to_uint64();
				return 0;
			}

			if(strcmp(type.c_str(), CC_FLOAT) == 0){
				unit.to_float();
				return 0;
			}

			if(strcmp(type.c_str(), CC_DOUBLE) == 0){
				unit.to_double();
				return 0;
			}

			if(strcmp(type.c_str(), CC_STRING) == 0){
				unit.to_bsl_string();
				return 0;
			}
		}
		catch(...){
			return -1;
		}
		return -1;
	}
}
















/* vim: set ts=4 sw=4 sts=4 tw=100 */
