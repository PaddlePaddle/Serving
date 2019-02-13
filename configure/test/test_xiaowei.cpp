/***************************************************************************
 * 
 * Copyright (c) 2009 Baidu.com, Inc. All Rights Reserved
 * $Id: test_xiaowei.cpp,v 1.4 2009/02/13 15:25:31 yingxiang Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file test_xiaowei.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2009/01/05 11:43:08
 * @version $Revision: 1.4 $ 
 * @brief i
 *  
 **/
#include "Configure.h"
#include <iostream>
int main(){
	//comcfg :: Log :: closeLowLevel();
	comcfg :: Configure conf;
	conf.load(".", "xiaowei.conf", NULL);
	conf.print();
	std::cout<<"conf size = "<<conf.size()<<std::endl;

	for(int i = 0; i < 3; i++){
		std::cout<<conf["server"][i]["ip"].to_bsl_string()<<std::endl;
		std::cout<<conf["server"][i]["port"].to_bsl_string()<<std::endl;
	}
	try{
		std::cout<<"error "<<conf["server"].to_int32()<<std::endl;
	}catch(comcfg :: NoSuchKeyException){
		std::cout<<"-------catch NoSuchKey---------"<<std::endl;
	}
	return 0;
}




















/* vim: set ts=4 sw=4 sts=4 tw=100 */
