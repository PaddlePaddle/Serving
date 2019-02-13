/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: gen.c,v 1.1 2008/12/30 03:39:55 yingxiang Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file gen.cpp
 * @author xiaowei(com@baidu.com)
 * @date 2008/11/21 18:22:15
 * @version $Revision: 1.1 $ 
 * @brief 
 *  
 **/

#include <iostream>
#include <string>
#include <vector>

int main()
{
	std::cout<<"#include <iostream>"<<std::endl;
	std::cout<<"#include \"../src/expr.h\""<<std::endl;
	std::cout<<"#include <cxxtest/TestSuite.h>"<<std::endl;
	std::cout<<"#define DEQ(x, y) ( ((x)-(y)) < 1e5 || ((x)-(y)) > -1e5 )"<<std::endl;
	std::cout<<"class test_expr_suit : public CxxTest::TestSuite"<<std::endl;
	std::cout<<"{"<<std::endl;
	std::cout<<"public:"<<std::endl;


	std::string vecs[] = {
		"1+1",
		"123124+32",
		"1*234/32+23*(1+23)",
		"1+2/3-2*4",
		"1+3234234/2348*34+2",
		"23/(234/23+3242*(3423+2)/234+1)*23+2342+23*32/23",
		"23.2/234.2+234.2",
		"1+23*(1+32/2-23)+232-111<<2+32+(1>>2)<<1+3<<2*(1<<2)"
	};
	std::vector<std::string> vec(vecs, vecs+sizeof(vecs)/sizeof(vecs[0]));

	std::vector<std::string> type;
	type.push_back("int");
	type.push_back("long");
	//type.push_back("char");
	type.push_back("unsigned int");
	type.push_back("unsigned long");
	type.push_back("long long");
	type.push_back("unsigned long long");

	for (int tp=0; tp<(int)type.size(); ++tp) {
		for (int i=0; i<(int)vec.size(); ++i) {
			std::cout<<"\t"<<"void test" <<"_case"<<i<<"_"<<tp<<"() {"<<std::endl;
			std::cout<<"\t\t"<<type[tp]<<" val = 0;"<<std::endl;
			std::cout<<"\t\tTSM_ASSERT (\"\", xarthop::calc(\""
				<<vec[i]<<"\", &val) == 0);"<<std::endl;
			std::cout<<"\t\tTSM_ASSERT (\"\", val == ("<<type[tp]<<")("
				<<vec[i]<<"));"<<std::endl;
			std::cout<<"\t}"<<std::endl;
		}
	}
	std::cout<<"};"<<std::endl;

	return 0;
}

















/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
