/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: test_get_value.cpp,v 1.5 2009/11/10 04:50:30 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file test_get_value.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/29 16:45:20
 * @version $Revision: 1.5 $ 
 * @brief 
 *  
 **/

#include "ConfigUnit.h"
#include "Configure.h"
#include "utils/cc_struct.h"
#include <iostream>
#include "ul_log.h"

using namespace comcfg;

void runtest(const comcfg::ConfigUnit &);

int main(){
	ul_logstat_t ulg;
	ulg.events = 2;
	ul_openlog("./log", "test_get_value", &ulg, 2048);
	comcfg::Configure conf;
	conf.load(".", "test_value.conf", NULL);
	printf( "\n\nRead test_int : ");
	runtest(conf["test_int"]);
	printf( "\n\nRead test_float : ");
	runtest(conf["test_float"]);
	printf( "\n\nRead test_str : ");
	runtest(conf["test_str"]);
	printf( "\n\nRead test_err : ");
	runtest(conf["test_err"]);
	return 0;
}

void runtest(const comcfg::ConfigUnit & conf){

	//=========  Test char=============
	try{
		char a0 = conf.to_char();
		std::cout<<"to char successful, value = "<<a0<<std::endl;
	}catch(...){
		printf( "to char failed!\n");
	}

	//=========  Test uchar=============
	try{
		unsigned char a1 = conf.to_uchar();
		std::cout<<"to uchar successful, value = "<<a1<<std::endl;
	}catch(...){
		printf( "to uchar failed!\n");
	}

	//=========  Test int16=============
	try{
		int16_t a2 = conf.to_int16();
		std::cout<<"to int16 successful, value = "<<a2<<std::endl;
	}catch(...){
		printf( "to int16 failed!\n");
	}

	//=========  Test uint16=============
	try{
		u_int16_t a3 = conf.to_uint16();
		std::cout<<"to uint16 successful, value = "<<a3<<std::endl;
	}catch(...){
		printf( "to uint16 failed!\n");
	}

	//=========  Test int32=============
	try{
		int a4 = conf.to_int32();
		std::cout<<"to int32 successful, value = "<<a4<<std::endl;
	}catch(...){
		printf( "to int32 failed!\n");
	}

	//=========  Test uint32=============
	try{
		u_int32_t a5 = conf.to_uint32();
		std::cout<<"to uint32 successful, value = "<<a5<<std::endl;
	}catch(...){
		printf( "to uint32 failed!\n");
	}

	//=========  Test int64=============
	try{
		long long a6 = conf.to_int64();
		std::cout<<"to int64 successful, value = "<<a6<<std::endl;
	}catch(...){
		printf( "to int64 failed!\n");
	}

	//=========  Test uint64=============
	try{
		unsigned long long a7 = conf.to_uint64();
		std::cout<<"to uint64 successful, value = "<<a7<<std::endl;
	}catch(...){
		printf( "to uint64 failed!\n");
	}

	//=========  Test float=============
	try{
		float a8 = conf.to_float();
		std::cout<<"to float successful, value = "<<a8<<std::endl;
	}catch(...){
		printf( "to float failed!\n");
	}

	//=========  Test double=============
	try{
		double a9 = conf.to_double();
		std::cout<<"to double successful, value = "<<a9<<std::endl;
	}catch(...){
		printf( "to double failed!\n");
	}

	//=========  Test bsl_string=============
	try{
		bsl_string a10 = conf.to_bsl_string();
		std::cout<<"to bsl_string successful, value = "<<a10<<std::endl;
	}catch(...){
		printf( "to bsl_string failed!\n");
	}




	//=========  Test char=============
	char val0;
	int ret0 = conf.get_char(&val0);
	if(ret0 == 0){
		std::cout<<"Get char successful, value = "<<val0<<std::endl;
	}else{
		printf( "Get char failed!\n");
	}

	//=========  Test uchar=============
	unsigned char val1;
	int ret1 = conf.get_uchar(&val1);
	if(ret1 == 0){
		std::cout<<"Get uchar successful, value = "<<val1<<std::endl;
	}else{
		printf( "Get uchar failed!\n");
	}

	//=========  Test int16=============
	int16_t val2;
	int ret2 = conf.get_int16(&val2);
	if(ret2 == 0){
		std::cout<<"Get int16 successful, value = "<<val2<<std::endl;
	}else{
		printf( "Get int16 failed!\n");
	}

	//=========  Test uint16=============
	u_int16_t val3;
	int ret3 = conf.get_uint16(&val3);
	if(ret3 == 0){
		std::cout<<"Get uint16 successful, value = "<<val3<<std::endl;
	}else{
		printf( "Get uint16 failed!\n");
	}

	//=========  Test int32=============
	int val4;
	int ret4 = conf.get_int32(&val4);
	if(ret4 == 0){
		std::cout<<"Get int32 successful, value = "<<val4<<std::endl;
	}else{
		printf( "Get int32 failed!\n");
	}

	//=========  Test uint32=============
	u_int32_t val5;
	int ret5 = conf.get_uint32(&val5);
	if(ret5 == 0){
		std::cout<<"Get uint32 successful, value = "<<val5<<std::endl;
	}else{
		printf( "Get uint32 failed!\n");
	}

	//=========  Test int64=============
	long long val6;
	int ret6 = conf.get_int64(&val6);
	if(ret6 == 0){
		std::cout<<"Get int64 successful, value = "<<val6<<std::endl;
	}else{
		printf( "Get int64 failed!\n");
	}

	//=========  Test uint64=============
	unsigned long long val7;
	int ret7 = conf.get_uint64(&val7);
	if(ret7 == 0){
		std::cout<<"Get uint64 successful, value = "<<val7<<std::endl;
	}else{
		printf( "Get uint64 failed!\n");
	}

	//=========  Test float=============
	float val8;
	int ret8 = conf.get_float(&val8);
	if(ret8 == 0){
		std::cout<<"Get float successful, value = "<<val8<<std::endl;
	}else{
		printf( "Get float failed!\n");
	}

	//=========  Test double=============
	double val9;
	int ret9 = conf.get_double(&val9);
	if(ret9 == 0){
		std::cout<<"Get double successful, value = "<<val9<<std::endl;
	}else{
		printf( "Get double failed!\n");
	}

	//=========  Test bsl_string=============
	bsl_string val10;
	int ret10 = conf.get_bsl_string(&val10);
	if(ret10 == 0){
		std::cout<<"Get bsl_string successful, value = "<<val10<<std::endl;
	}else{
		printf( "Get bsl_string failed!\n");
	}

}





/* vim: set ts=4 sw=4 sts=4 tw=100 */
