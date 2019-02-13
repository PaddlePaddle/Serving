/***************************************************************************
 * 
 * Copyright (c) 2009 Baidu.com, Inc. All Rights Reserved
 * $Id: test_cfg.cpp,v 1.2 2009/03/23 03:14:53 yingxiang Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file test_cfg.cpp
 * @author xiaowei(com@baidu.com)
 * @date 2009/03/09 15:53:27
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/
#include <cfgflag.h>
int main(int argc, char **argv)
{
	comcfg::Flag flag;
	flag.init(argc, argv);
	flag.loadconfig();
	return 0;
}



















/* vim: set ts=4 sw=4 sts=4 tw=100 */
