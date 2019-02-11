/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: stlsort.cpp,v 1.1 2008/09/03 06:47:49 xiaowei Exp $ 
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


#include <stdlib.h>
#include <list>
#include "yperfbench.h"

int main(int argc, char ** argv )
{

	pb::init_opt(argc, argv);
	int len = pb::getopt<int>("len");
	std::list<int> s;
	for(int i=0; i < len; i++)
		s.push_front(rand());
	pb::timer t;
	s.sort();
	t.check();
}









/* vim: set ts=4 sw=4 sts=4 tw=100 */
