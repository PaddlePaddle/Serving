/***************************************************************************
 * 
 * Copyright (c) 2009 Baidu.com, Inc. All Rights Reserved
 * $Id: test_1010new.cpp,v 1.2 2009/03/23 03:14:53 yingxiang Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file test_1010new.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2009/03/12 00:54:03
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/

#include "Configure.h"

#define ASSERT(x) do{\
	if(!(x))printf("Assert failed %s:%d\n", __FILE__, __LINE__);\
	else printf("Assertion Successful. %s:%d\n", __FILE__, __LINE__);\
}while(0)


int main(){
	comcfg::Configure  conf;
	int ret = conf.load(".", "t1.conf", "t1.cons");
	ASSERT(ret == comcfg::CONSTRAINT_ERROR);
	system("touch t1.conf -d '19870121 12:35'");
	//system : date '+%s' -d'19870121 12:35'
	//--> 538202100
	time_t t = conf.lastConfigModify();
	ASSERT(t == time_t(538202100) );
	return 0;
}



















/* vim: set ts=4 sw=4 sts=4 tw=100 */
