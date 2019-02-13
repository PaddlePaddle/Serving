/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: Constraint.h,v 1.7 2009/02/27 10:14:00 yingxiang Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file Constraint.h
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/28 20:35:35
 * @version $Revision: 1.7 $ 
 * @brief 
 *  
 **/


#ifndef  __CONSTRAINT_H_
#define  __CONSTRAINT_H_

#include "Configure.h"
#include "ConstraintFunction.h"

namespace comcfg{
	class Constraint{
		public:
			/**
			 * @brief 检查约束信息
			 *
			 * @param [in] __idl   : confIDL::idl_t* 约束信息
			 * @param [in/out] __conf   : Configure* 配置信息
			 * @return  int 
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2008/12/29 01:40:40
			**/
			int run(confIDL::idl_t * __idl, Configure * __conf);
		protected:
			int printError();
			int runGroup(confIDL::var_t* var, ConfigGroup * father, ConfigUnit * self = NULL);
			int runKey(confIDL::var_t* var, ConfigGroup * father, ConfigUnit * self = NULL);
			int runVar(confIDL::var_t* var, ConfigGroup * father, ConfigUnit * self = NULL);
			int runType(const str_t& type, const ConfigUnit & unit);
			Configure * conf;
			confIDL::idl_t * idl;
	};
}
















#endif  //__CONSTRAINT_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
