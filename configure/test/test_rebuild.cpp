/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: test_rebuild.cpp,v 1.3 2009/02/13 15:25:31 yingxiang Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file test_rebuild.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/29 21:29:18
 * @version $Revision: 1.3 $ 
 * @brief i
 *  
 **/

#include "Configure.h"

int main(){
	comcfg :: Configure conf;
	conf.load(".", "rebuild.conf");
	printf("key: %d\n", conf["all"]["xxx"]["key"].to_int32());
	size_t sz;
	char * s = conf.dump(&sz);
	comcfg :: Configure cf2;
	char * buf = cf2.getRebuildBuffer(sz);
	memcpy(buf, s, sz);
	cf2.rebuild();
	printf("key: %d\n", cf2["all"]["xxx"]["key"].to_int32());
	return 0;
}
















/* vim: set ts=4 sw=4 sts=4 tw=100 */
