/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: test_rawdata.cpp,v 1.2 2009/01/13 13:35:13 yingxiang Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file test_rawdata.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/20 23:06:45
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/

#include "reader/RawData.h"
#include <stdio.h>

int main(){
	comcfg::RawData rd;
	rd.load("raw.conf");
	for(int i = 0; (size_t)i < rd.lineNum(); ++i){
		printf("%s\n", rd.getLine(i));
	}
	return 0;
}










/* vim: set ts=4 sw=4 sts=4 tw=100 */
