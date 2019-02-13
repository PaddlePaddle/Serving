/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: cc_default.h,v 1.6 2009/03/23 03:14:52 yingxiang Exp $ 
 * 
 **************************************************************************/



/**
 * @file cc_default.h
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/29 01:04:16
 * @version $Revision: 1.6 $ 
 * @brief 
 *  
 **/


#ifndef  __CC_DEFAULT_H_
#define  __CC_DEFAULT_H_

#include "Constraint.h"
#include "../compiler/src/idl_conf_if.h"
#include "ConstraintFunction.h"

namespace comcfg{
	class CFdefault{
		public:
			static int cons_array(const confIDL::cons_func_t&, const confIDL::var_t &, ConfigGroup *, ConfigUnit *);
			static int cons_default(const confIDL::cons_func_t&, const confIDL::var_t &, ConfigGroup *, ConfigUnit *);
			static int cons_length(const confIDL::cons_func_t&, const confIDL::var_t &, ConfigGroup *, ConfigUnit *);
			static int cons_enum(const confIDL::cons_func_t&, const confIDL::var_t &, ConfigGroup *, ConfigUnit *);
			static int cons_regexp(const confIDL::cons_func_t&, const confIDL::var_t &, ConfigGroup *, ConfigUnit *);
			static int cons_range(const confIDL::cons_func_t&, const confIDL::var_t &, ConfigGroup *, ConfigUnit *);
			static int cons_ip(const confIDL::cons_func_t&, const confIDL::var_t &, ConfigGroup *, ConfigUnit *);
			static int cons_do_nothing(const confIDL::cons_func_t&, const confIDL::var_t &, ConfigGroup *, ConfigUnit *);

			static int gcons_regexp(const std::vector<str_t> &arg_vec, ConfigUnit *conf);
			static int gcons_ip(const std::vector<str_t> &arg_vec, ConfigUnit *conf);	
			static int gcons_length(const std::vector<int> &arg_vec, ConfigUnit *conf);
			static int gcons_length(const std::vector<long long> &arg_vec, ConfigUnit *conf);
			
			template<class T>
			static int gcons_default(const std::vector<T> &arg_vec, ConfigUnit *conf)
			{
				int cb_check_default(ConfigUnit *, void *);
				if(arg_vec.size() != 1){
					LOG(WARNING) << "Constraint: wrong arg number " << arg_vec.size() << " for global constraint gcons_default(), 1 arg is required.";
					return -1;
				}
				str_t def_value;
				if(typeid(T) == typeid(int)){
					def_value.setf("%d", *reinterpret_cast<const int *>(&arg_vec[0]));
				}
				else if(typeid(T) == typeid(str_t)){
					def_value = *reinterpret_cast<const str_t *>(&arg_vec[0]);
				}
				else if(typeid(T) == typeid(long long)){
					def_value.setf("%lld", *reinterpret_cast<const long long *>(&arg_vec[0]));
				}
				else if(typeid(T) == typeid(double)){
					def_value.setf("%lf", *reinterpret_cast<const double *>(&arg_vec[0]));
				}
				return conf->traverse_unit(cb_check_default, &def_value);
			}

			template<class T>
			static int gcons_enum(const std::vector<T> &arg_vec, ConfigUnit *conf)
			{
				int cb_check_enum(ConfigUnit *, void *);
				std::vector<str_t> enum_vec;
				str_t enum_str;
				if(typeid(T) == typeid(int)){
					for(size_t i=0; i<arg_vec.size(); ++i){
						enum_str.setf("%d", *reinterpret_cast<const int *>(&arg_vec[i]));
						enum_vec.push_back(enum_str);
					}
				}
				else if(typeid(T) == typeid(str_t)){
					for(size_t i=0; i<arg_vec.size(); ++i){
						enum_vec.push_back(*reinterpret_cast<const str_t *>(&arg_vec[i]));
					}
				}
				else if(typeid(T) == typeid(long long)){
					for(size_t i=0; i<arg_vec.size(); ++i){
						enum_str.setf("%lld", *reinterpret_cast<const long long *>(&arg_vec[i]));
						enum_vec.push_back(enum_str);
					}
				}
				else if(typeid(T) == typeid(double)){
					for(size_t i=0; i<arg_vec.size(); ++i){
						enum_str.setf("%lf", *reinterpret_cast<const double *>(&arg_vec[i]));
						enum_vec.push_back(enum_str);
					}
				}
				if(arg_vec.size() == 0){
					LOG(WARNING) << "Constraint: wrong arg number 0 for global constraint gcons_enum(), at least 1 arg is required.";
					return -1;
				}
				return conf->traverse_unit(cb_check_enum, &enum_vec);
			}

			template<class T>
			static int gcons_range(const std::vector<T> &arg_vec, ConfigUnit *conf)
			{
				if(typeid(T) == typeid(str_t)){
					LOG(WARNING) << "Constraint: gcons_range() does not support str_t type constraint";
					return -1;
				}
				if(arg_vec.size() != 2){
					LOG(WARNING) << "Constraint: wrong arg number " << arg_vec.size() << " for global constraint gcons_range(), 2 args are required.";
					return -1;
				}
				void *arg = &const_cast<std::vector<T> &>(arg_vec);
				return conf->traverse_unit(cb_check_range<T>, arg);
			}
			
			template<class T>
			static int cb_check_range(ConfigUnit *unit, void *range_vec)
			{
				unit->setConstrainted(true);
				if(unit->selfType() != CONFIG_UNIT_TYPE){
					return 0;
				}	
				std::vector<T> &rvec = *static_cast<std::vector<T> *>(range_vec);
				if(typeid(T) != typeid(double)){
					long long val = unit->to_int64();
					if(val >= rvec[0] && val <= rvec[1]){
						return 0;
					}
				}
				else{
					double val = unit->to_double();
					if(val >= rvec[0] && val <= rvec[1]){
						return 0;
					}
				}
				int errcode = 0;
				LOG(WARNING) << "Constraint: gcons_range() [key : value]:[" << unit->get_key_name().c_str() << " : " << unit->to_cstr(&errcode, "ERROR") << "]: value not in range.";
				return -1;
			}
	};
}
















#endif  //__CC_DEFAULT_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
