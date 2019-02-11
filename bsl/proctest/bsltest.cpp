/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsltest.cpp,v 1.2 2008/11/12 04:13:35 xiaowei Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file bsltest.cpp
 * @author yufan(com@baidu.com)
 * @date 2008/07/31 18:53:53
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/



#include <containers/slist/bsl_slist.h>
#include <yperfbench.h>

int main(int argc, char ** argv )
{

	pb::init_opt(argc, argv);
	int c = pb::getopt<int>("time");
	bsl::slist<int> s;
	pb::timer t;
	for (int i=0; i< c; i++)
	{
		s.push_front(i);
	}
	t.check();

}









/* vim: set ts=4 sw=4 sts=4 tw=100 */
