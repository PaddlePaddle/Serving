/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: test_reader.cpp,v 1.3 2009/02/13 15:25:31 yingxiang Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file test_reader.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/21 20:14:22
 * @version $Revision: 1.3 $ 
 * @brief i
 *  
 **/

#include "Configure.h"
#include "stdio.h"

int main(){
	comcfg::Configure conf;
	conf.load(".", "new.conf");
	return 0;
}



















/* vim: set ts=4 sw=4 sts=4 tw=100 */
