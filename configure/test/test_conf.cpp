/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: test_conf.cpp,v 1.5 2009/02/13 15:25:31 yingxiang Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file test_conf.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/24 18:47:10
 * @version $Revision: 1.5 $ 
 * @brief 
 *  
 **/

#include "Configure.h"
#include "stdio.h"
#include <string>

int main(){
	comcfg::Configure conf;
	//comcfg::Log::openLowLevel();
	conf.load("./", "new.conf");
	comcfg::str_t str;
	if(conf["COMLOG"]["FILE"]["TYPE"].get_bsl_string(&str) == 0){
		printf("read = [%s]\n", str.c_str());
	}else{
		printf("No such key\n");
	}
	if(conf["COMLOG"]["FILEx"]["TYPE"][2].get_bsl_string(&str) == 0){
		printf("read = [%s]\n", str.c_str());
	}else{
		printf("No such key\n");
	}
	try{
		conf["COMLOG"]["FILEx"]["TYPE"][2].to_bsl_string();
	}
	catch(bsl::Exception e){
		printf("%s\n", e.what());
	}
	int err;
	int tmp = conf["COMLOG"]["FILE"]["SIZE"].to_int32(&err);
	printf("read = [%d]\n", tmp);
	return 0;
}

















/* vim: set ts=4 sw=4 sts=4 tw=100 */
