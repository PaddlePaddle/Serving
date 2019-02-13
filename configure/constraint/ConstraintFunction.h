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
		CF_STRUCT = 0,	//��߼����Լ������������ִ�У���array��Լ��ĳ���ֶε�����
		CF_KEY,			//�ֶ�Լ������default��Լ��ĳ���ֶεĻ������
		CF_TYPE,		//����Լ����int16, int32, int64...,Ҳ��һ��ֵ��Լ��(CF_VALUE)
		CF_VALUE,		//��ͨ�����Լ������ip(), length()�ȣ�Լ��ĳ���ֶ�ֵ������
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
			 * @brief ��ȡLibrary��ʵ��
			 * @note ��ʵ�����û�����������������ڶ�����
			 *			���û����ù��ı��ӿڡ�
			 * @return  ConstraintLibrary* 
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2008/12/28 23:26:14
			**/
			static ConstraintLibrary* getInstance();
			/**
			 * @brief ɾ��Library��ʵ���������ڴ�
			 * @note �ڲ�����Ҫ��ʵ����������Ҫ��������ʵ��ʱ����ɾ��ԭ��ʵ��
			 * @return   
			 * @retval   
			 * @see 
			 * @author linjieqiong
			 * @date 2013/01/22 16:26:14
			**/
			static void destroyInstance();
			/**
			 * @brief ��ȡĳ�������ļ��𣬶Բ����ڵĺ�������default
			 *			���û����ù��ı��ӿڡ�
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
			 * @brief ���ݺ������һ������ 
			 * 			�����Ը��ڵ����ʽ���루��Ϊ�������ֶβ�һ�����ڣ���default�����ڸ��ڵ����½��ӽ�㣩
			 * 			���û����ù��ı��ӿڡ�
			 *
			 * @param [in] function   : const str_t& Լ��������
			 * @param [in]    	: const confIDL::var_t& Լ����Ϣ
			 * @param [in] 		: ConfigGroup * ����ĸ��ڵ�
			 * @return  int 
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2008/12/28 23:26:42
			**/
			int check(const confIDL::cons_func_t&, const confIDL::var_t &, ConfigGroup*, ConfigUnit*);

			/**
			 * @brief ʹ���û�ע���ȫ��Լ�������������key-value�����
			 *				Ĭ�Ͻ���key-value���Ϊ��Լ���ģ������������ȫ��Լ����飬��������Ǳ�Լ����
			 *				�û����ù��ı��ӿ�
			 *
			 * @param [in] conf : ConfigUnit * ���������ļ�������configure�ṹ�ĸ�
			 * @return  int : 0, ����key-value������Լ����������������Լ�������������� 
			 * @retval   
			 * @see 
			 * @author linjieqiong
			 * @date 2013/01/15 23:29:50
			**/
			int checkGlobalConstraint(ConfigUnit *conf);
			
			/**
			 * @brief ע��һ��Լ������ �������û�����չ��
			 *
			 * @param [in] fname   : const str_t& Լ������������
			 * @param [in] cons   : const ConstraintFunction& Լ������ʵ�壬�൱�ڷº���
			 * 			��ע�⡿��new�����ٴ���ȥ��Ȼ��ConstraintLibrary��delete֮
			 * @param [in] overwrite   : bool �Ƿ񸲸�ͬ���ĺ���
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
			 * @brief ע��һ��ȫ��Լ������ �������û�����չ��
			 *
			 * @param [in] fname   : const str_t& Լ������������
			 * @param [in] gcf_t   : int (*gcf_t)(const std::vector<T> &, ConfigUnit *) Լ�������ĺ���ָ��
			 * @param [in] arg_vec : const std::vector<T> & Լ�������Ĳ���
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
			 * @brief ע��һ��ȫ��Լ������ �������û�����չ��
			 * 			���ɱ�����ĺ����ĵ���һ��Ҫ���أ�����ȷ���������������ȷ
			 *
			 * @param [in] fname   : const str_t& Լ������������
			 * @param [in] gcf_t   : int (*gcf_t)(const std::vector<T> &, ConfigUnit *) Լ�������ĺ���ָ��
			 * @param [in] argnum  : size_t Լ�������Ĳ�������
			 * @param [in] ...	   : �ɱ��������ʾÿһ��Լ������
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
			 * @brief ע��һ��ȫ��Լ������ �������û�����չ��
			 *
			 * @param [in] GlobalConsFun: ��ȫ��Լ��������������Ϣ
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
