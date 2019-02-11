/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: stllist.cpp,v 1.1 2008/09/03 06:47:49 xiaowei Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file bsltest.cpp
 * @author yufan(com@baidu.com)
 * @date 2008/07/31 18:53:53
 * @version $Revision: 1.1 $ 
 * @brief 
 *  
 **/



#include <list>
#include "yperfbench.h"

int main(int argc, char ** argv )
{

	pb::init_opt(argc, argv);
	int c = pb::getopt<int>("time");
	std::list<int> s;
	pb::timer t;
	for (int i=0; i< c; i++)
	{
		s.push_front(i);
	}
	t.check();

}









/* vim: set ts=4 sw=4 sts=4 tw=100 */
