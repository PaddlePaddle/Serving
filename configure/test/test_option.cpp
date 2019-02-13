/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: test_option.cpp,v 1.2 2009/01/13 13:35:13 yingxiang Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file test_option.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/29 23:54:04
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/

#include "CmdOption.h"

int showVersion(comcfg::CmdOption * cmd){
	printf("Version : %g\n", (*cmd)["v"].to_float());
	return 0;
}

int main(){
	char *s[] = {"-a", "-h", "-v1.0", "--port=80", "roba", "cnhawk", "-ccode", "wtommy", "-f"};

	comcfg:: CmdOption cmd;
	cmd.init(7, s, "a:hv::c::");
	cmd.registHook("v", showVersion);
	cmd.runHook();
	return 0;
}


















/* vim: set ts=4 sw=4 sts=4 tw=100 */
