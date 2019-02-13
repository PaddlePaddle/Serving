/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: ConstraintFunction.cpp,v 1.7 2010/01/08 03:24:38 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file ConstraintFunction.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/28 23:17:51
 * @version $Revision: 1.7 $ 
 * @brief 
 *  
 **/

#include "ConstraintFunction.h"
#include "cc_default.h"

static const int hashmap_bitems = 256;

namespace comcfg{
    class ConsUnitControl {
        private:
            static int _needfree;
        public:
            ConsUnitControl() {
                ConfigUnit :: get_err_unit();
            }
        ~ConsUnitControl() {
                if (_needfree) {
                    _needfree =0;
                    ConstraintLibrary* cu = ConstraintLibrary :: getInstance();
                    delete cu;
                }
            }
    };
    int ConsUnitControl::_needfree = 1;

    static ConsUnitControl cons_unit_control;



	ConstraintLibrary * ConstraintLibrary :: lib = NULL;

	//=======ConstraintFunction Methods
	int ConstraintFunction :: check(const confIDL::cons_func_t&, const confIDL::var_t &, ConfigGroup*, ConfigUnit*){
		LOG(WARNING) << "Unknown function : calling virtual method";
		return 0;
	};

	void ConstraintFunction :: setLevel(int lv){
		_level = lv;
	}
	int ConstraintFunction :: getLevel(){
		return _level;
	}

	//=======ConstraintLibrary Methods
	ConstraintLibrary :: ConstraintLibrary(){
		cfMap.create(hashmap_bitems);
		registConstraint(str_t("comment"), CFdefault :: cons_do_nothing, CF_VALUE);
		registConstraint(str_t("array"), CFdefault :: cons_array, CF_STRUCT);
		registConstraint(str_t("default"), CFdefault :: cons_default, CF_KEY);
		registConstraint(str_t("ip"), CFdefault :: cons_ip, CF_VALUE);
		registConstraint(str_t("length"), CFdefault :: cons_length, CF_VALUE);
		registConstraint(str_t("regexp"), CFdefault :: cons_regexp, CF_VALUE);
		registConstraint(str_t("enum"), CFdefault :: cons_enum, CF_VALUE);
		registConstraint(str_t("range"), CFdefault :: cons_range, CF_VALUE);
	};
	ConstraintLibrary :: ~ConstraintLibrary(){
		CFMAP_T::iterator itr;	
		for(itr = cfMap.begin(); itr != cfMap.end(); ++itr) { 
			if (NULL != itr->second) {
				delete itr->second;
			}
		}
		cfMap.destroy();
	}
	ConstraintLibrary* ConstraintLibrary :: getInstance(){
		if(lib == NULL){
			lib = new ConstraintLibrary();
		}
		return lib;
	}
	void ConstraintLibrary :: destroyInstance(){
		if(lib != NULL){
			delete lib;
			lib = NULL;
		}
	}
	int ConstraintLibrary :: getLevel(const str_t& function){
		CFMAP_T::_Pair * ptr = cfMap.find(function);
		if(NULL != ptr && NULL != ptr->second){
			return ptr->second->getLevel();
		}
		return CF_DEFAULT;
	}
	int ConstraintLibrary :: getLevel_static(const str_t& function){
		return getInstance() -> getLevel(function);
	}
	int ConstraintLibrary :: check(const confIDL::cons_func_t& func, 
			const confIDL::var_t& param, ConfigGroup* father, ConfigUnit * self){
		CFMAP_T::_Pair * ptr = cfMap.find(func.func.data);
		if(NULL != ptr && NULL != ptr->second){
			return ptr->second->check(func, param, father, self);
		}
		LOG(WARNING) << "Unknown function : [File:" << func.func.file.c_str() << ", Line:" << func.func.lineno << "]=[" << func.func.data.c_str() << "]"; 
		return -1;
	}

	int ConstraintLibrary :: checkGlobalConstraint(ConfigUnit *conf)
	{
		GCFLIST_T :: iterator itr = _gcfList.begin();
		for(; itr != _gcfList.end(); ++itr){
			if(itr->type() == typeid(GlobalConsFun<int>)){
				GlobalConsFun<int> *gcf = gcf_any_cast<GlobalConsFun<int> >(&*itr);
				LOG(INFO) <<  "Now checking with global constraint function: " << gcf->_name.c_str() << "<int>()";
				if(0 != gcf->_gcf(gcf->_arg, conf)){
					return -1;
				}
			}
			else if(itr->type() == typeid(GlobalConsFun<str_t>)){
				GlobalConsFun<str_t> *gcf = gcf_any_cast<GlobalConsFun<str_t> >(&*itr);
				LOG(INFO) << "Now checking with global constraint function: " << gcf->_name.c_str() << "<bsl::string>()";
				if(0 != gcf->_gcf(gcf->_arg, conf)){
					return -1;
				}
			}
			else if(itr->type() == typeid(GlobalConsFun<double>)){
				GlobalConsFun<double> *gcf = gcf_any_cast<GlobalConsFun<double> >(&*itr);
				LOG(INFO) <<  "Now checking with global constraint function: " << gcf->_name.c_str() << "<double>()";
				if(0 != gcf->_gcf(gcf->_arg, conf)){
					return -1;
				}
			}
			else if(itr->type() == typeid(GlobalConsFun<long long>)){
				GlobalConsFun<long long> *gcf = gcf_any_cast<GlobalConsFun<long long> >(&*itr);
				LOG(INFO) << "Now checking with global constraint function: " << gcf->_name.c_str() << "<long long>()";
				if(0 != gcf->_gcf(gcf->_arg, conf)){
					return -1;
				}
			}
			else{
				LOG(WARNING) << "An unrecognizable global constraint function is set.";
				return -1;
			}
		}
		return 0;
	}

	int ConstraintLibrary :: registConstraint(const str_t& fname, ConstraintFunction* cons, bool overwrite)
	{
		CFMAP_T::_Pair * ptr = cfMap.find(fname);
		if(false == overwrite && NULL != ptr){
			return -1;
		}
		if(NULL != ptr && NULL != ptr->second) {
			delete ptr->second;
		}
		cfMap.set(fname,cons,1);
		return 0;
	}

	int ConstraintLibrary :: registConstraint(const str_t& fname, CF_T cf, int cfLevel, bool overwrite){
		WrapFunction * wf = new WrapFunction();
		wf->setCF(cf);
		wf->setLevel(cfLevel);
		return registConstraint(fname, wf, overwrite);
	}

}



















/* vim: set ts=4 sw=4 sts=4 tw=100 */
