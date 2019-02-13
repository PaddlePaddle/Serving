/***************************************************************************
 * 
 * Copyright (c) 2009 Baidu.com, Inc. All Rights Reserved
 * $Id: test_get_default.cpp,v 1.3 2009/11/10 04:50:30 scmpf Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file test_get_default.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2009/03/12 01:30:32
 * @version $Revision: 1.3 $ 
 * @brief 
 *  
 **/

#include "Configure.h"


#define ASSERT(x) do{\
	if(!(x))printf("Assert failed %s:%d\n", __FILE__, __LINE__);\
	else printf("Assertion Successful. %s:%d\n", __FILE__, __LINE__);\
}while(0)

int main(){
	using namespace comcfg;
	system("touch Nothing.conf");
	system("echo A : B > Nothing.conf");
	comcfg::Configure conf;
	int ret = conf.load(".", "Nothing.conf");
	ASSERT(ret == 0);

	char v0;
	ASSERT( comcfg::NOSUCHKEY == conf["xxx"].get_char(&v0, '0') );
	ASSERT( v0 == '0' );
	unsigned char v1;
	ASSERT( comcfg::NOSUCHKEY == conf["xxx"].get_uchar(&v1, '1') );
	ASSERT( v1 == '1' );
	int16_t v2;
	ASSERT( comcfg::NOSUCHKEY == conf["xxx"].get_int16(&v2, 2) );
	ASSERT( v2 == 2 );
	u_int16_t v3;
	ASSERT( comcfg::NOSUCHKEY == conf["xxx"].get_uint16(&v3, 3) );
	ASSERT( v3 == 3 );
	int v4;
	ASSERT( comcfg::NOSUCHKEY == conf["xxx"].get_int32(&v4, 4) );
	ASSERT( v4 == 4 );
	u_int32_t v5;
	ASSERT( comcfg::NOSUCHKEY == conf["xxx"].get_uint32(&v5, 5) );
	ASSERT( v5 == 5 );
	long long v6;
	ASSERT( comcfg::NOSUCHKEY == conf["xxx"].get_int64(&v6, 6) );
	ASSERT( v6 == 6 );
	unsigned long long v7;
	ASSERT( comcfg::NOSUCHKEY == conf["xxx"].get_uint64(&v7, 7) );
	ASSERT( v7 == 7 );
	float v8;
	ASSERT( comcfg::NOSUCHKEY == conf["xxx"].get_float(&v8, 8) );
	ASSERT( v8 == 8 );
	double v9;
	ASSERT( comcfg::NOSUCHKEY == conf["xxx"].get_double(&v9, 9) );
	ASSERT( v9 == 9 );
	bsl_string v10;
	ASSERT( comcfg::NOSUCHKEY == conf["xxx"].get_bsl_string(&v10, "10") );
	ASSERT( v10 == str_t("10") );
	str_t v11;
	ASSERT( comcfg::NOSUCHKEY == conf["xxx"].get_raw_string(&v11, "11") );
	ASSERT( v11 == str_t("11") );
	char s[12];
	ASSERT( comcfg::NOSUCHKEY == conf["xxx"].get_cstr(s,12, "12") );
	ASSERT(strcmp(s, "12") == 0);

	return 0;
}
	
















/* vim: set ts=4 sw=4 sts=4 tw=100 */
