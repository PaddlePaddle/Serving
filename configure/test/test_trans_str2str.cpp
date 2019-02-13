/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: test_trans_str2str.cpp,v 1.3 2009/01/20 14:27:12 yingxiang Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file test_trans_str2str.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/24 18:08:20
 * @version $Revision: 1.3 $ 
 * @brief 
 *  
 **/
#include "utils/cc_trans.h"
#include <assert.h>
#include <stdio.h>

int main(){
	FILE * fp = fopen("str2str", "r");
	assert(fp);
	char buf[1024];
	while(fgets(buf, 1024, fp) != NULL){
		buf[strlen(buf) - 1] = 0;
		comcfg::str_t str;
		if(comcfg::Trans::str2str(comcfg::str_t(buf), &str) == 0){
			puts("----- Read -----");
			puts(buf);
			puts("----- trans to -----");
			puts(str.c_str());
			puts("------ Finish -----");
		}
		else {
			puts("Error");
		}
	}
	return 0;
}










/* vim: set ts=4 sw=4 sts=4 tw=100 */
