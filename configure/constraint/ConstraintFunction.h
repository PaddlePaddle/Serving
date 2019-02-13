/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: ConstraintFunction.h,v 1.9 2010/01/08 03:24:38 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file ConstraintFunction.h
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/10 18:15:58
 * @version $Revision: 1.9 $ 
 * @brief 
 *  
 **/


#ifndef  __CONSTRAINTFUNCTION_H_
#define  __CONSTRAINTFUNCTION_H_

#include <string>
#include <list>
#include <map>
#include "Configure.h"
#include "ConfigGroup.h"
#include "ConfigUnit.h"
#include "GlobalConsFun.h"

namespace confIDL{
	struct cons_func_t;
	struct var_t;
	struct idl_t;
}

namespace comcfg{
	//typedef std::string str_t;
	typedef std::list <str_t> param_list_t;

	//Constraint function pointer
	typedef int (* CF_T)(const confIDL::cons_func_t&, const confIDL::var_t &, ConfigGroup *, ConfigUnit*);

	enum CFLevel{
		CF_STRUCT = 0,	//最高级别的约束，必须最早执行，如array，约束某个字段的类型
		CF_KEY,			//字段约束，如default，约束某个字段的基本情况
		CF_TYPE,		//类型约束，int16, int32, int64...,也是一种值的约束(CF_VALUE)
		CF_VALUE,		//普通级别的约束，如ip(), length()等，约束某个字段值的内容
		CF_DEFAULT,
	};

	class ConstraintFunction{
		int _level;
		public:
			virtual int check(const confIDL::cons_func_t&, const confIDL::var_t &, ConfigGroup*, ConfigUnit*);
			void setLevel(int lv);
			int getLevel();
            virtual ~ConstraintFunction();
	};
	typedef bsl::hashmap<str_t, ConstraintFunction*> CFMAP_T;

	//Constraint Library is Singleton
	class ConstraintLibrary{
		ConstraintLibrary();
		~ConstraintLibrary();

		static ConstraintLibrary * lib;
		public:
			/**
			 * @brief 获取Library的实例
			 * @note 该实例在用户程序的整个生命周期都存在
			 *			【用户不用关心本接口】
			 * @return  ConstraintLibrary* 
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2008/12/28 23:26:14
			**/
			static ConstraintLibrary* getInstance();
			/**
			 * @brief 删除Library的实例，回收内存
			 * @note 在不再需要该实例，或者需要重新生成实例时，先删除原有实例
			 * @return   
			 * @retval   
			 * @see 
			 * @author linjieqiong
			 * @date 2013/01/22 16:26:14
			**/
			static void destroyInstance();
			/**
			 * @brief 获取某个函数的级别，对不存在的函数返回default
			 *			【用户不用关心本接口】
			 *
			 * @param [in] function   : const str_t&
			 * @return  int 
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2008/12/28 23:26:28
			**/
			int getLevel(const str_t& function);
			static int getLevel_static(const str_t& function);
			/**
			 * @brief 根据函数检查一个内容 
			 * 			内容以父节点的形式传入（因为被检查的字段不一定存在，如default就是在父节点下新建子结点）
			 * 			【用户不用关心本接口】
			 *
			 * @param [in] function   : const str_t& 约束函数名
			 * @param [in]    	: const confIDL::var_t& 约束信息
			 * @param [in] 		: ConfigGroup * 传入的父节点
			 * @return  int 
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2008/12/28 23:26:42
			**/
			int check(const confIDL::cons_func_t&, const confIDL::var_t &, ConfigGroup*, ConfigUnit*);

			/**
			 * @brief 使用用户注册的全局约束函数检查所有key-value配置项，
			 *				默认将该key-value标记为被约束的，即如果进行了全局约束检查，则所有项都是被约束的
			 *				用户不用关心本接口
			 *
			 * @param [in] conf : ConfigUnit * 读入配置文件后建立的configure结构的根
			 * @return  int : 0, 所有key-value均满足约束，其他，不满足约束或其他错误发生 
			 * @retval   
			 * @see 
			 * @author linjieqiong
			 * @date 2013/01/15 23:29:50
			**/
			int checkGlobalConstraint(ConfigUnit *conf);
			
			/**
			 * @brief 注册一个约束函数 【用于用户的扩展】
			 *
			 * @param [in] fname   : const str_t& 约束函数的名字
			 * @param [in] cons   : const ConstraintFunction& 约束函数实体，相当于仿函数
			 * 			【注意】请new出来再传进去，然后ConstraintLibrary会delete之
			 * @param [in] overwrite   : bool 是否覆盖同名的函数
			 * @return  int 
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2008/12/28 23:29:50
			**/
			int registConstraint(const str_t& fname, ConstraintFunction* cons, bool overwrite = false);

			int registConstraint(const str_t& fname, CF_T cf, bool overwrite = false){
				return registConstraint(fname, cf, CF_VALUE, overwrite);
			}
		
			/**
			 * @brief 注册一个全局约束函数 【用于用户的扩展】
			 *
			 * @param [in] fname   : const str_t& 约束函数的名字
			 * @param [in] gcf_t   : int (*gcf_t)(const std::vector<T> &, ConfigUnit *) 约束函数的函数指针
			 * @param [in] arg_vec : const std::vector<T> & 约束函数的参数
			 * @return  int 
			 * @retval   
			 * @see 
			 * @author linjieqiong
			 * @date 2013/01/15 10:00:00
			**/
			template<class T>
			int registGlobalConstraint(const str_t& fname, 
					int (*gcf_t)(const std::vector<T> &, ConfigUnit *),  
					const std::vector<T> &arg_vec)
			{
				GlobalConsFun<T> gFun(fname, gcf_t, arg_vec);
				return registGlobalConstraint(gFun);
			}
			
			/**
			 * @brief 注册一个全局约束函数 【用于用户的扩展】
			 * 			带可变参数的函数的调用一定要慎重，尽量确保传入参数类型正确
			 *
			 * @param [in] fname   : const str_t& 约束函数的名字
			 * @param [in] gcf_t   : int (*gcf_t)(const std::vector<T> &, ConfigUnit *) 约束函数的函数指针
			 * @param [in] argnum  : size_t 约束函数的参数个数
			 * @param [in] ...	   : 可变参数，表示每一个约束参数
			 * @return  int 
			 * @retval   
			 * @see 
			 * @author linjieqiong
			 * @date 2013/01/15 10:00:00
			**/
			template<class T>
			int registGlobalConstraint(const str_t& fname,
					int (*gcf_t)(const std::vector<T> &, ConfigUnit *),  
					size_t argnum, ...) 
			{
				std::vector<T> arg_vec;
				va_list argp;
				va_start(argp, argnum);
				while(argnum > 0)
				{
					T arg = va_arg(argp, T);
					arg_vec.push_back(arg);
					-- argnum;
				}
				va_end(argp);

				return registGlobalConstraint(fname, gcf_t, arg_vec);
			}
			
			
			/**
			 * @brief 注册一个全局约束函数 【用于用户的扩展】
			 *
			 * @param [in] GlobalConsFun: 该全局约束函数的描述信息
			 * @return  int 
			 * @retval   
			 * @see 
			 * @author linjieqiong
			 * @date 2013/01/15 10:00:00
			**/
			template<class T>
			int registGlobalConstraint(const GlobalConsFun<T>& gFun)
			{
				_gcfList.push_back(gFun);
				return 0;
			}

		private:
			friend class Constraint;
			friend class ConsUnitControl;
			int registConstraint(const str_t& fname, CF_T cf, int cfLevel, bool overwrite = false);
		protected:
			GCFLIST_T _gcfList;
			CFMAP_T cfMap;
			class WrapFunction : public ConstraintFunction{
				public:
					virtual int check(const confIDL::cons_func_t& cons, 
							const confIDL::var_t& param, ConfigGroup* father, ConfigUnit* self){
						return _cf(cons, param, father, self);
					}
					void setCF(CF_T cf){
						_cf = cf;
					}
					WrapFunction(){}
                    virtual ~WrapFunction();
				protected:
					CF_T _cf;
			};
	};
}




#endif  //__CONSTRAINTFUNCTION_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
