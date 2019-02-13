/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: test_reloader.cpp,v 1.2 2010/02/09 01:58:01 scmpf Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file test_conf.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/24 18:47:10
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/

#include "ul_conf.h"
#include "Configure.h"
#include <ConfigReloader.h>
#include <pthread.h>
#include "stdio.h"
#include <string>

using namespace comcfg;

static int g_run = 0;

static void * change_threadfun(void *param)
{
	ConfigReloader * prd = (ConfigReloader *)param;
	while (g_run) {
		printf("g_run:%d, monitor: %d\n",g_run, prd->monitor());
		sleep(10);
	}
	return NULL;
}

int int_key_change(const ConfigUnit &keyold, const ConfigUnit &keynew, void * ) {
	printf("old:%d, new:%d\n", keyold.to_int32(), keynew.to_int32());
	if (g_run) {
		if (++g_run >3) {
			g_run = 0;
		}
	}
	return 0;
}

int int_string_change(const ConfigUnit &keyold, const ConfigUnit &keynew, void * ) {
	printf("old:%s, new:%s\n", keyold.to_bsl_string().c_str(), keynew.to_bsl_string().c_str());
	if (g_run) {
		if (++g_run >3) {
			g_run = 0;
		}
	}
	return 0;
}

int uint64_key_change(const ConfigUnit &keyold, const ConfigUnit &keynew, void * ) {
	printf("[%s]old:%llu, new:%llu\n",keynew.get_key_name().c_str(), keyold.to_uint64(), keynew.to_uint64());
	if (g_run) {
		if (++g_run >3) {
			g_run = 0;
		}
	}
	return 0;
}

int char_key_change(const ConfigUnit &keyold, const ConfigUnit &keynew, void * ) {
	printf("[%s]old:%c, new:%c\n",keynew.get_key_name().c_str(), keyold.to_char(), keynew.to_char());
	if (g_run) {
		if (++g_run >3) {
			g_run = 0;
		}
	}
	return 0;
}

int float_key_change(const ConfigUnit &keyold, const ConfigUnit &keynew, void * ) {
	printf("[%s]old:%f, new:%f\n",keynew.get_key_name().c_str(), keyold.to_double(), keynew.to_double());
	if (g_run) {
		if (++g_run >3) {
			g_run = 0;
		}
	}
	return 0;
}

int group_change(const ConfigUnit &, const ConfigUnit &keynew, void * ) {
//	printf("[%s]\n",keynew.print());
	keynew.print();
	if (g_run) {
		if (++g_run >3) {
			g_run = 0;
		}
	}
	return 0;
}

int file_change(const ConfigUnit &, const ConfigUnit &keynew, void * ) {
	printf("file[ivar]:\n");
	keynew["ivar"].print();
	if (g_run) {
		if (++g_run >3) {
			g_run = 0;
		}
	}
	return 0;
}

int main(){
	ConfigReloader rd;
	if (0 != rd.init("./", "reload.conf","reload.range")) {
		printf("init failed\n");
		return 1;
	}
	rd.add_key_char("ivar.b", char_key_change, NULL);
	rd.add_key_char("ivar.b", int_key_change, NULL);
	rd.add_key_char("ivar.b", char_key_change, NULL);
	printf("add char:%c\n", rd.get_config()->deepGet("ivar.a").to_char());
	rd.add_key_int("ivar.a", char_key_change, NULL);
	rd.add_key_int("ivar.a", int_key_change, NULL);
	printf("add int:%d\n", rd.get_config()->deepGet("ivar.a").to_int32());
	rd.add_key_uint64("ivar.c", uint64_key_change, NULL);
	printf("add int:%llu\n", rd.get_config()->deepGet("ivar.c").to_uint64());
	rd.add_key_float("ivar.d", float_key_change, NULL);
	printf("add char:%c\n", rd.get_config()->deepGet("ivar.a").to_char());
	rd.add_key_string("ubclient.proto[4].method[5].name", int_string_change, NULL);
	printf("add string:%s\n", rd.get_config()->deepGet("ubclient.proto[4].method[5].name").to_bsl_string().c_str());

	rd.add_file_monitor(file_change, NULL);
	rd.add_file_monitor(file_change, NULL);
	printf("add file\n");

	rd.add_group_monitor("ivar.g", group_change, NULL);
	rd.add_group_monitor("ivar", group_change, NULL);
	printf("\ngroup:\n");rd.get_config()->deepGet("ivar").print();
	g_run = 1;
	pthread_t tid;
	pthread_create(&tid, NULL, change_threadfun, &rd);
	pthread_join(tid, NULL);
	return 0;
}

















/* vim: set ts=4 sw=4 sts=4 tw=100 */
