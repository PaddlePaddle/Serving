/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: test_t1.cpp,v 1.4 2009/11/10 04:50:30 scmpf Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file test_t1.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/28 23:39:58
 * @version $Revision: 1.4 $ 
 * @brief 
 *  
 **/

#include "Configure.h"
#include <bsl/ResourcePool.h>
#include "bsl/var/implement.h"
int main(){
	comcfg::Configure conf, conf2;
	conf.load(".", "r.conf", "r.cons");
	printf("=======conf1=========\n");
	conf.print(0);
	bsl::var::Dict dict;
	bsl::var::Int32 i32 = 1234;
	dict["key"] = i32;
	bsl :: ResourcePool rp;
	conf2.loadIVar(dict);//conf.to_IVar(&rp)
	printf("=======conf2=========\n");
	conf2.print(0);
	return 0;
}

















/* vim: set ts=4 sw=4 sts=4 tw=100 */
