#include <iostream>
#include "../src/expr.h"
#include <cxxtest/TestSuite.h>
#define DEQ(x, y) ( ((x)-(y)) < 1e5 || ((x)-(y)) > -1e5 )
class test_expr_suit : public CxxTest::TestSuite
{
public:
	void test_case0_0() {
		int val = 0;
		TSM_ASSERT ("", xarthop::calc("1+1", &val) == 0);
		TSM_ASSERT ("", val == (int)(1+1));
	}
	void test_case1_0() {
		int val = 0;
		TSM_ASSERT ("", xarthop::calc("123124+32", &val) == 0);
		TSM_ASSERT ("", val == (int)(123124+32));
	}
	void test_case2_0() {
		int val = 0;
		TSM_ASSERT ("", xarthop::calc("1*234/32+23*(1+23)", &val) == 0);
		TSM_ASSERT ("", val == (int)(1*234/32+23*(1+23)));
	}
	void test_case3_0() {
		int val = 0;
		TSM_ASSERT ("", xarthop::calc("1+2/3-2*4", &val) == 0);
		TSM_ASSERT ("", val == (int)(1+2/3-2*4));
	}
	void test_case4_0() {
		int val = 0;
		TSM_ASSERT ("", xarthop::calc("1+3234234/2348*34+2", &val) == 0);
		TSM_ASSERT ("", val == (int)(1+3234234/2348*34+2));
	}
	void test_case5_0() {
		int val = 0;
		TSM_ASSERT ("", xarthop::calc("23/(234/23+3242*(3423+2)/234+1)*23+2342+23*32/23", &val) == 0);
		TSM_ASSERT ("", val == (int)(23/(234/23+3242*(3423+2)/234+1)*23+2342+23*32/23));
	}
	void test_case6_0() {
		int val = 0;
		TSM_ASSERT ("", xarthop::calc("23.2/234.2+234.2", &val) == 0);
		TSM_ASSERT ("", val == (int)(23.2/234.2+234.2));
	}
	void test_case7_0() {
		int val = 0;
		TSM_ASSERT ("", xarthop::calc("1+23*(1+32/2-23)+232-111<<2+32+(1>>23)<<1+3<<2*(1<<2)", &val) == 0);
		TSM_ASSERT ("", val == (int)(1+23*(1+32/2-23)+232-111<<2+32+(1>>23)<<1+3<<2*(1<<2)));
	}
	void test_case0_1() {
		long val = 0;
		TSM_ASSERT ("", xarthop::calc("1+1", &val) == 0);
		TSM_ASSERT ("", val == (long)(1+1));
	}
	void test_case1_1() {
		long val = 0;
		TSM_ASSERT ("", xarthop::calc("123124+32", &val) == 0);
		TSM_ASSERT ("", val == (long)(123124+32));
	}
	void test_case2_1() {
		long val = 0;
		TSM_ASSERT ("", xarthop::calc("1*234/32+23*(1+23)", &val) == 0);
		TSM_ASSERT ("", val == (long)(1*234/32+23*(1+23)));
	}
	void test_case3_1() {
		long val = 0;
		TSM_ASSERT ("", xarthop::calc("1+2/3-2*4", &val) == 0);
		TSM_ASSERT ("", val == (long)(1+2/3-2*4));
	}
	void test_case4_1() {
		long val = 0;
		TSM_ASSERT ("", xarthop::calc("1+3234234/2348*34+2", &val) == 0);
		TSM_ASSERT ("", val == (long)(1+3234234/2348*34+2));
	}
	void test_case5_1() {
		long val = 0;
		TSM_ASSERT ("", xarthop::calc("23/(234/23+3242*(3423+2)/234+1)*23+2342+23*32/23", &val) == 0);
		TSM_ASSERT ("", val == (long)(23/(234/23+3242*(3423+2)/234+1)*23+2342+23*32/23));
	}
	void test_case6_1() {
		long val = 0;
		TSM_ASSERT ("", xarthop::calc("23.2/234.2+234.2", &val) == 0);
		TSM_ASSERT ("", val == (long)(23.2/234.2+234.2));
	}
	void test_case7_1() {
		long val = 0;
		TSM_ASSERT ("", xarthop::calc("1+23*(1+32/2-23)+232-111<<2+32+(1>>23)<<1+3<<2*(1<<2)", &val) == 0);
		TSM_ASSERT ("", val == (long)(1+23*(1+32/2-23)+232-111<<2+32+(1>>23)<<1+3<<2*(1<<2)));
	}
	void test_case0_2() {
		unsigned int val = 0;
		TSM_ASSERT ("", xarthop::calc("1+1", &val) == 0);
		TSM_ASSERT ("", val == (unsigned int)(1+1));
	}
	void test_case1_2() {
		unsigned int val = 0;
		TSM_ASSERT ("", xarthop::calc("123124+32", &val) == 0);
		TSM_ASSERT ("", val == (unsigned int)(123124+32));
	}
	void test_case2_2() {
		unsigned int val = 0;
		TSM_ASSERT ("", xarthop::calc("1*234/32+23*(1+23)", &val) == 0);
		TSM_ASSERT ("", val == (unsigned int)(1*234/32+23*(1+23)));
	}
	void test_case3_2() {
		unsigned int val = 0;
		TSM_ASSERT ("", xarthop::calc("1+2/3-2*4", &val) == 0);
		TSM_ASSERT ("", val == (unsigned int)(1+2/3-2*4));
	}
	void test_case4_2() {
		unsigned int val = 0;
		TSM_ASSERT ("", xarthop::calc("1+3234234/2348*34+2", &val) == 0);
		TSM_ASSERT ("", val == (unsigned int)(1+3234234/2348*34+2));
	}
	void test_case5_2() {
		unsigned int val = 0;
		TSM_ASSERT ("", xarthop::calc("23/(234/23+3242*(3423+2)/234+1)*23+2342+23*32/23", &val) == 0);
		TSM_ASSERT ("", val == (unsigned int)(23/(234/23+3242*(3423+2)/234+1)*23+2342+23*32/23));
	}
	void test_case6_2() {
		unsigned int val = 0;
		TSM_ASSERT ("", xarthop::calc("23.2/234.2+234.2", &val) == 0);
		TSM_ASSERT ("", val == (unsigned int)(23.2/234.2+234.2));
	}
	void test_case7_2() {
		unsigned int val = 0;
		TSM_ASSERT ("", xarthop::calc("1+23*(1+32/2-23)+232-111<<2+32+(1>>23)<<1+3<<2*(1<<2)", &val) == 0);
		TSM_ASSERT ("", val == (unsigned int)(1+23*(1+32/2-23)+232-111<<2+32+(1>>23)<<1+3<<2*(1<<2)));
	}
	void test_case0_3() {
		unsigned long val = 0;
		TSM_ASSERT ("", xarthop::calc("1+1", &val) == 0);
		TSM_ASSERT ("", val == (unsigned long)(1+1));
	}
	void test_case1_3() {
		unsigned long val = 0;
		TSM_ASSERT ("", xarthop::calc("123124+32", &val) == 0);
		TSM_ASSERT ("", val == (unsigned long)(123124+32));
	}
	void test_case2_3() {
		unsigned long val = 0;
		TSM_ASSERT ("", xarthop::calc("1*234/32+23*(1+23)", &val) == 0);
		TSM_ASSERT ("", val == (unsigned long)(1*234/32+23*(1+23)));
	}
	void test_case3_3() {
		unsigned long val = 0;
		TSM_ASSERT ("", xarthop::calc("1+2/3-2*4", &val) == 0);
		TSM_ASSERT ("", val == (unsigned long)(1+2/3-2*4));
	}
	void test_case4_3() {
		unsigned long val = 0;
		TSM_ASSERT ("", xarthop::calc("1+3234234/2348*34+2", &val) == 0);
		TSM_ASSERT ("", val == (unsigned long)(1+3234234/2348*34+2));
	}
	void test_case5_3() {
		unsigned long val = 0;
		TSM_ASSERT ("", xarthop::calc("23/(234/23+3242*(3423+2)/234+1)*23+2342+23*32/23", &val) == 0);
		TSM_ASSERT ("", val == (unsigned long)(23/(234/23+3242*(3423+2)/234+1)*23+2342+23*32/23));
	}
	void test_case6_3() {
		unsigned long val = 0;
		TSM_ASSERT ("", xarthop::calc("23.2/234.2+234.2", &val) == 0);
		TSM_ASSERT ("", val == (unsigned long)(23.2/234.2+234.2));
	}
	void test_case7_3() {
		unsigned long val = 0;
		TSM_ASSERT ("", xarthop::calc("1+23*(1+32/2-23)+232-111<<2+32+(1>>23)<<1+3<<2*(1<<2)", &val) == 0);
		TSM_ASSERT ("", val == (unsigned long)(1+23*(1+32/2-23)+232-111<<2+32+(1>>23)<<1+3<<2*(1<<2)));
	}
	void test_case0_4() {
		long long val = 0;
		TSM_ASSERT ("", xarthop::calc("1+1", &val) == 0);
		TSM_ASSERT ("", val == (long long)(1+1));
	}
	void test_case1_4() {
		long long val = 0;
		TSM_ASSERT ("", xarthop::calc("123124+32", &val) == 0);
		TSM_ASSERT ("", val == (long long)(123124+32));
	}
	void test_case2_4() {
		long long val = 0;
		TSM_ASSERT ("", xarthop::calc("1*234/32+23*(1+23)", &val) == 0);
		TSM_ASSERT ("", val == (long long)(1*234/32+23*(1+23)));
	}
	void test_case3_4() {
		long long val = 0;
		TSM_ASSERT ("", xarthop::calc("1+2/3-2*4", &val) == 0);
		TSM_ASSERT ("", val == (long long)(1+2/3-2*4));
	}
	void test_case4_4() {
		long long val = 0;
		TSM_ASSERT ("", xarthop::calc("1+3234234/2348*34+2", &val) == 0);
		TSM_ASSERT ("", val == (long long)(1+3234234/2348*34+2));
	}
	void test_case5_4() {
		long long val = 0;
		TSM_ASSERT ("", xarthop::calc("23/(234/23+3242*(3423+2)/234+1)*23+2342+23*32/23", &val) == 0);
		TSM_ASSERT ("", val == (long long)(23/(234/23+3242*(3423+2)/234+1)*23+2342+23*32/23));
	}
	void test_case6_4() {
		long long val = 0;
		TSM_ASSERT ("", xarthop::calc("23.2/234.2+234.2", &val) == 0);
		TSM_ASSERT ("", val == (long long)(23.2/234.2+234.2));
	}
	void test_case7_4() {
		long long val = 0;
		TSM_ASSERT ("", xarthop::calc("1+23*(1+32/2-23)+232-111<<2+32+(1>>23)<<1+3<<2*(1<<2)", &val) == 0);
		TSM_ASSERT ("", val == (long long)(1+23*(1+32/2-23)+232-111<<2+32+(1>>23)<<1+3<<2*(1<<2)));
	}
	void test_case0_5() {
		unsigned long long val = 0;
		TSM_ASSERT ("", xarthop::calc("1+1", &val) == 0);
		TSM_ASSERT ("", val == (unsigned long long)(1+1));
	}
	void test_case1_5() {
		unsigned long long val = 0;
		TSM_ASSERT ("", xarthop::calc("123124+32", &val) == 0);
		TSM_ASSERT ("", val == (unsigned long long)(123124+32));
	}
	void test_case2_5() {
		unsigned long long val = 0;
		TSM_ASSERT ("", xarthop::calc("1*234/32+23*(1+23)", &val) == 0);
		TSM_ASSERT ("", val == (unsigned long long)(1*234/32+23*(1+23)));
	}
	void test_case3_5() {
		unsigned long long val = 0;
		TSM_ASSERT ("", xarthop::calc("1+2/3-2*4", &val) == 0);
		TSM_ASSERT ("", val == (unsigned long long)(1+2/3-2*4));
	}
	void test_case4_5() {
		unsigned long long val = 0;
		TSM_ASSERT ("", xarthop::calc("1+3234234/2348*34+2", &val) == 0);
		TSM_ASSERT ("", val == (unsigned long long)(1+3234234/2348*34+2));
	}
	void test_case5_5() {
		unsigned long long val = 0;
		TSM_ASSERT ("", xarthop::calc("23/(234/23+3242*(3423+2)/234+1)*23+2342+23*32/23", &val) == 0);
		TSM_ASSERT ("", val == (unsigned long long)(23/(234/23+3242*(3423+2)/234+1)*23+2342+23*32/23));
	}
	void test_case6_5() {
		unsigned long long val = 0;
		TSM_ASSERT ("", xarthop::calc("23.2/234.2+234.2", &val) == 0);
		TSM_ASSERT ("", val == (unsigned long long)(23.2/234.2+234.2));
	}
	void test_case7_5() {
		unsigned long long val = 0;
		TSM_ASSERT ("", xarthop::calc("1+23*(1+32/2-23)+232-111<<2+32+(1>>23)<<1+3<<2*(1<<2)", &val) == 0);
		TSM_ASSERT ("", val == (unsigned long long)(1+23*(1+32/2-23)+232-111<<2+32+(1>>23)<<1+3<<2*(1<<2)));
	}
};
