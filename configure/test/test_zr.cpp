/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: test_zr.cpp,v 1.4 2009/12/21 05:39:44 scmpf Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file test_conf.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/24 18:47:10
 * @version $Revision: 1.4 $ 
 * @brief 
 *  
 **/

#include "EnvGet.h"
#include "ul_conf.h"
#include "Configure.h"
#include "stdio.h"
#include <string>

using namespace comcfg;
ENV_CHAR_DECL(MAC, 0);
ENV_UCHAR_DECL(MAC, 0);
ENV_INT16_DECL(MAC, 0);
ENV_UINT16_DECL(MAC, 0);
ENV_INT32_DECL(MAC, 0);
ENV_UINT32_DECL(MAC, 0);
ENV_INT64_DECL(MAC, 0);
ENV_UINT64_DECL(MAC, 0);
ENV_FLOAT_DECL(MAC, 0);
ENV_DOUBLE_DECL(MAC, 0);
ENV_BSLSTR_DECL(MAC, 0);

int main(){
	comcfg::Configure conf;
	// test Configure::getErrKeyPath()
	conf.load("./", "new.conf");
	comcfg::str_t str;
	if(conf["COMLOG"]["FILE"]["TYPE"].get_bsl_string(&str) == 0){
		printf("read = [%s]\n", str.c_str());
	}else{
		printf("No such key: %s\n", conf.getErrKeyPath());
	}
	if(conf["COMLOG"]["FILEx"]["TYPE"][2].get_bsl_string(&str) == 0){
		printf("read = [%s]\n", str.c_str());
	}else{
		printf("No such key: %s\n", conf.getErrKeyPath());
	}
	try{
		str = conf["COMLOG"]["FILE"]["TYPE"].to_bsl_string();
		printf("to bsl string: %s\n", str.c_str());
	}
	catch(bsl::Exception e){
		printf("%s\n", e.what());
		printf("No such key: %s\n", conf.getErrKeyPath());
	}
	try{
		conf["COMLOG"]["FILE"]["abc"].to_char();
	}
	catch(bsl::Exception e){
		printf("%s\n", e.what());
		printf("No such key: %s\n", conf.getErrKeyPath());
	}
	int err;
	int tmp;
	try {
	tmp = conf["COMLOG"]["FILE"]["SIZE"].to_int32();
	printf("read = [%d]\n", tmp);}
	catch (bsl::Exception e) {
		printf("%s\n", e.what());
	}

	//test ConfigUnit::deepGet()
	if(conf.deepGet("COMLOG.FILE.TYPE").get_bsl_string(&str) == 0){
		printf("deepGet(\"COMLOG.FILE.TYPE\") = [%s]\n", str.c_str());
	}else{
		printf("No such key: %s\n", conf.getErrKeyPath());
	}
	if(conf.deepGet("COMLOG.DEVICE[2]").get_bsl_string(&str) == 0){
		printf("deepGet(\"COMLOG.DEVICE[2]\") = [%s]\n", str.c_str());
	}else{
		printf("No such key: %s\n", conf.getErrKeyPath());
	}

	printf("dump:%s\n%s\n\n", conf.dump(NULL));

	comcfg::Configure nc;
	bsl::ResourcePool rp;
	nc.loadIVar(conf.to_IVar(&rp,&err));
	printf("new dump:%s\n%s\n\n", nc.dump(NULL));

	//test enviroment macro
	printf("ENV_CHAR:  MAC= %c\n", g_char_MAC);
	printf("ENV_UCHAR:  MAC= %c\n", g_uchar_MAC);
	printf("ENV_INT16:  MAC= %d\n", g_int16_MAC);
	printf("ENV_UINT16:  MAC= %d\n", g_uint16_MAC);
	printf("ENV_INT32:  MAC= %d\n", g_int32_MAC);
	printf("ENV_UINT32:  MAC= %d\n", g_uint32_MAC);
	printf("ENV_INT64:  MAC= %lld\n", g_int64_MAC);
	printf("ENV_UINT64:  MAC= %lld\n", g_uint64_MAC);
	printf("ENV_FLOAT:  MAC= %f\n", g_float_MAC);
	printf("ENV_DOUBLE:  MAC= %f\n", g_double_MAC);
	printf("ENV_BSLSTR:  MAC= %s\n", g_bslstr_MAC.c_str());

	bsl::string a;
	printf("%s\n", a.c_str());
	if (a == "") printf("NULL STR\n");


	comcfg::Configure conf1;
	conf1.load("./","zr.conf");
	const char * tstr;
	try {
		tstr = conf1["server"][1]["ip"].to_cstr();
	}
	catch (bsl::Exception e){
		printf("%s\n", e.what());
	}
	printf("to server[1].ip: %s\n", tstr);
	printf("to server[1].ip: %s\n", conf1["server"][1]["ip"].to_bsl_string().c_str());
	printf("to person.name: %s\n", conf1["person"]["name"].to_bsl_string().c_str());
	printf("to person.age: %s\n", conf1["person"]["age"].to_bsl_string().c_str());
	printf("to int person.age: %d\n", conf1["person"]["age"].to_int32());
	


	return 0;
}

















/* vim: set ts=4 sw=4 sts=4 tw=100 */
