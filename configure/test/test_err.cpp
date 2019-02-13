/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: test_err.cpp,v 1.3 2009/02/13 15:25:31 yingxiang Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file test_err.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/30 01:14:34
 * @version $Revision: 1.3 $ 
 * @brief 
 *  
 **/

#include "Configure.h"
#include <iostream>

int main(){
	comcfg :: Configure conf;
	conf.load(".", "str.conf");
	int err;
	conf["xxx"].to_int32(&err);
	std::cout<<conf.seeError(err)<<std::endl;
	conf["str1"].to_int32(&err);
	std::cout<<conf.seeError(err)<<std::endl;
	return 0;
}
















/* vim: set ts=4 sw=4 sts=4 tw=100 */
