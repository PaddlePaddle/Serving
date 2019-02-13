/***************************************************************************
 * 
 * Copyright (c) 2009 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file test_checkone.cpp
 * @author zhang_rui(com@baidu.com)
 * @date 2009-12-15
 * @brief 
 *  
 **/

#include "Configure.h"

int main(int argc, char* argv[]){
	if (argc < 3) {
		printf("usage : %s conf range\n",argv[0]);
		return -1;
	}
	comcfg::Configure conf;
	conf.load("./", argv[1]);
	int chk = conf.check_once(argv[2]);
	printf("checkone:%d\n", chk);

	return 0;
}

