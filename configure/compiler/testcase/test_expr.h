/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: test_expr.h,v 1.1 2008/12/30 03:39:55 yingxiang Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file normal.h
 * @author xiaowei(com@baidu.com)
 * @date 2008/11/21 14:11:44
 * @version $Revision: 1.1 $ 
 * @brief 
 *  
 **/


#ifndef  __NORMAL_H_
#define  __NORMAL_H_

#include <iostream>
#include "../src/expr.h"
#include <cxxtest/TestSuite.h>

#define DEQ(x, y) ( ((x)-(y)) < 1e5 || ((x)-(y)) > -1e5 )

class test_normal : public CxxTest::TestSuite
{
public:
	void test_test1()
	{
		int val = 0;
		TSM_ASSERT("", xarthop::calc("1+1", &val) == 0);
		TSM_ASSERT("", val == 1+1);

		TSM_ASSERT("", xarthop::calc("	1+	1    ", &val) == 0);
		TSM_ASSERT("", val == 1+1);

		TSM_ASSERT("", xarthop::calc("	1 + 3 - 2 ", &val) == 0);
		TSM_ASSERT("", val == 1+3-2);
	}

	void test_test2() 
	{
		int val = 0;
		TSM_ASSERT("", xarthop::calc("1+2/3-2*4", &val) == 0);
		//std::cout<<"-------"<<val<<"--------"<<std::endl;
		TSM_ASSERT("", val == 1+2/3-2*4);

		TSM_ASSERT("", xarthop::calc("1+3234234/2348*34+2", &val) == 0);
		TSM_ASSERT("", val == 1+3234234/2348*34+2);
	}

	void test_test3()
	{
		int val = 0;
		TSM_ASSERT("", xarthop::calc("123/(23+2)*12+3*2/(1+2)", &val) == 0);
		//std::cout<<"---------------"<<123/(23+2)*12+3*2/(1+2)<<"	=	"<<val<<std::endl;
		TSM_ASSERT("", val == 123/(23+2)*12+3*2/(1+2));

		TSM_ASSERT("", xarthop::calc("23/(234/23+3242*(3423+2)/234+1)*23+2342+23*32/23", &val) == 0);
		TSM_ASSERT("", val == 23/(234/23+3242*(3423+2)/234+1)*23+2342+23*32/23);
	}

	void test_test4()
	{
		int val = 0;
		TSM_ASSERT("", xarthop::calc("324/(1<<2)+234>>3", &val) == 0);
		//std::cout<<"---------------"<<(324/(1<<2)+234>>3)<<"	=	"<<val<<std::endl;
		TSM_ASSERT("", val == 324/(1<<2)+234>>3);

		TSM_ASSERT("", xarthop::calc("1<<2>>3<<4>>3<<3", &val) == 0);
		//std::cout<<"-----------------"<<(1<<2>>4<<4>>3<<3)<<"------"<<val<<std::endl;
		TSM_ASSERT("", val == 1<<2>>4<<4>>3<<3);

		TSM_ASSERT("", xarthop::calc("1*23+3*(23+23/2)+1/325", &val) == 0);
		TSM_ASSERT("", val == 1*23+3*(23+23/2)+1/325);
	}


	void test_fail()
	{
		double val = 0;
		TSM_ASSERT("", xarthop::calc("324/(1<<2)+234>>3", &val) != 0);
		TSM_ASSERT("", xarthop::calc("1<<2>>3<<4>>3<<3", &val) != 0);
		TSM_ASSERT("", xarthop::calc("", &val) != 0);
		TSM_ASSERT("", xarthop::calc("adsfsdf", &val) != 0);
		TSM_ASSERT("", xarthop::calc("123124..123", &val) != 0);
		TSM_ASSERT("", xarthop::calc("123a+234", &val) != 0);
		TSM_ASSERT("", xarthop::calc("1231+(123)+2", &val) == 0);
	}

	void test_double2()
	{
		double val = 0;
		TSM_ASSERT("", xarthop::calc("32.23 / 1.23 + 28 * (23.2+32) + 3-2 /21", &val) == 0);
		TSM_ASSERT("", DEQ(val,  32.23 / 1.23 + 28 * (23.2+32) + 3-2 /21));

		TSM_ASSERT("", xarthop::calc("32.2/1.32+(23.1-23.4)*6+13.2", &val) == 0);
		TSM_ASSERT("", DEQ(val, 32.2/1.32+(23.1-23.4)*6+13.2));
	}
};



#endif  //__NORMAL_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
