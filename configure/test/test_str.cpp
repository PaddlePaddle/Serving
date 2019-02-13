/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: test_str.cpp,v 1.5 2009/02/13 15:25:31 yingxiang Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file test_str.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/30 00:57:26
 * @version $Revision: 1.5 $ 
 * @brief i
 *  
 **/

#include "Configure.h"
#include <iostream>

int main(){
	comcfg :: Configure conf;
	char buf[128];
	conf.load(".", "str.conf");

	std::cout<<"str1 : to raw : "<<conf["str1"].to_raw_string()<<std::endl;
	std::cout<<"str1 : to std : "<<conf["str1"].to_bsl_string()<<std::endl;
	conf["str1"].get_cstr(buf, 128);
	std::cout<<"str1 : to cstr: "<<buf<<std::endl;


	std::cout<<"str2 : to raw : "<<conf["str2"].to_raw_string()<<std::endl;
	std::cout<<"str2 : to std : "<<conf["str2"].to_bsl_string()<<std::endl;
	conf["str2"].get_cstr(buf, 16);
	std::cout<<"str2 : to cstr: "<<buf<<std::endl;

	return 0;
}

















/* vim: set ts=4 sw=4 sts=4 tw=100 */
