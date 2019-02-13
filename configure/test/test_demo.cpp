/***************************************************************************
 * 
 * Copyright (c) 2009 Baidu.com, Inc. All Rights Reserved
 * $Id: test_demo.cpp,v 1.5 2009/02/13 15:25:31 yingxiang Exp $ 
 * 
 **************************************************************************/



/**
 * @file test_demo.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2009/01/06 16:50:46
 * @version $Revision: 1.5 $ 
 * @brief 
 *  
 **/

#include "Configure.h"
#include <iostream>

int main(){
	//comcfg :: Log :: openLowLevel();
	comcfg :: Configure conf;
	int ret = conf.load("", "test.conf");
	if(ret != 0){
		return 0;
	}

	try{
		std::cout<< "Module Name : " << conf["module"].to_bsl_string() << std::endl;
		size_t num = conf["server"].size();
		std::cout<< "We have " << num << " servers." << std::endl;
		for(size_t i = 0; i < num; ++i){
			printf("Server[%d] is : ip = %s, port = %d\n", 
					(int)i,
				conf["server"][i]["ip"].to_cstr(), 
				conf["server"][i]["port"].to_int32() );
		}
	}
	catch(comcfg :: ConfigException e){
		std::cerr << "Something error  "<< e.what() << std::endl; 
	}
	catch(...){
		std::cerr << "What's wrong?" << std::endl;
	}
	size_t sz;
	printf("\n-------dump-------\n%s\n--------print---------\n", conf.dump(&sz));
	conf.print();
	comcfg::Configure nxt;
	char * b = nxt.getRebuildBuffer(sz);
	memcpy(b, conf.dump(NULL), sz);
	nxt.rebuild();
	printf("----nxt.print----\n");
	nxt.print();
	return 0;
}


















/* vim: set ts=4 sw=4 sts=4 tw=100 */
