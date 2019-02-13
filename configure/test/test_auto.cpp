/***************************************************************************
 * 
 * Copyright (c) 2009 Baidu.com, Inc. All Rights Reserved
 * $Id: test_auto.cpp,v 1.2 2009/03/23 03:14:53 yingxiang Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file test_auto.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2009/03/11 17:06:15
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/

#include "cfgext.h"
#include "Configure.h"
int main(int argc, char ** argv){
	if(argc == 1){
		printf("Usage : %s constraint_file\n\n", argv[0]);
		return 0;
	}
	try{
		//comcfg::Log::openLowLevel();
		bsl::string str = comcfg::autoConfigGen(argv[1]);
		printf("%s", str.c_str());
#if 1
		comcfg::Configure conf;
		char * s = conf.getRebuildBuffer(str.size());
		strcpy(s, str.c_str());
		int ret = conf.rebuild();
		printf("================= rebuild ret = %d ===================\n", ret);
		conf.print();
#endif
	}catch(comcfg::ConfigException e){
		fprintf(stderr, "Catch : %s\n", e.what());
	}catch(...){
		fprintf(stderr, "Catch : Unknown\n");
	}
		
	return 0;
}



















/* vim: set ts=4 sw=4 sts=4 tw=100 */
