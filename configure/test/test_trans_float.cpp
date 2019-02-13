/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: test_trans_float.cpp,v 1.3 2009/01/20 14:27:12 yingxiang Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file test_trans_float.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/24 16:17:34
 * @version $Revision: 1.3 $ 
 * @brief 
 *  
 **/
#include "utils/cc_utils.h"
#include <string>
#include <stdio.h>

void check(const char * s, bool is){
	bool tmp = comcfg::Trans::isFloat(comcfg::str_t(s));
	double k = 0.0;
	if(tmp)sscanf(s, "%lf", &k);
	printf("[%s] check [%s] %s a float[%g]\n", tmp==is?"OK":"Error", s, tmp?"IS":"ISNOT", k);
}

int main(){
	check("3.1415926", 1);
	check("3.1415926e100", 1);
	check("3.1415926E-10", 1);
	check("3.1415926e", 0);
	check(".1415926", 1);
	check("-.1415926", 1);
	check("3.E", 0);
	check("+-3.1", 0);
	check("e103", 0);
	check("3.1415926e1.1", 0);
	check("3.1415926e-10", 1);
	check("3.1415926E+0997", 1);
	check("-.141e5926", 1);
	check("3+.1415926", 0);
	return 0;
}




















/* vim: set ts=4 sw=4 sts=4 tw=100 */
