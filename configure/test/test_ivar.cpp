/***************************************************************************
 * 
 * Copyright (c) 2009 Baidu.com, Inc. All Rights Reserved
 * $Id: test_ivar.cpp,v 1.3 2009/08/13 07:23:01 scmpf Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file test_ivar.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2009/03/11 22:59:17
 * @version $Revision: 1.3 $ 
 * @brief 
 *  
 **/

#include "Configure.h"

int main(){
	//comcfg::Log::openLowLevel();
	comcfg::Configure conf;
	conf.load(".", "t1.conf");
	conf.print();
	bsl::ResourcePool rp;
	bsl::var::IVar& ivar = conf.to_IVar(&rp);
	//printf("\nivar to_string === \n%s\n\n", ivar.to_string(5).c_str());

	comcfg::Configure cf2;
	printf("\nloadIvar ===> %d \n", cf2.loadIVar(ivar));
	printf("\n=== conf2.dump ===\n%s\n\n", cf2.dump(NULL));
	printf("\n=== conf2.print ===\n");
	cf2.print();

	return 0;
}

















/* vim: set ts=4 sw=4 sts=4 tw=100 */
