/***************************************************************************
 * 
 * Copyright (c) 2010 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file test_filereload.cpp
 * @author zhang_rui(com@baidu.com)
 * @date 2010-2-1
 * @brief 
 *  
 **/

#include "ul_conf.h"
#include "Configure.h"
#include <FileReloader.h>
#include <pthread.h>
#include "stdio.h"
#include <string>

using namespace comcfg;

static int g_run = 0;

static void * change_threadfun(void *param)
{
	FileReloader * prd = (FileReloader *)param;
	while (g_run) {
		printf("g_run:%d, monitor: %d\n",g_run, prd->monitor());
		sleep(10);
	}
	return NULL;
}

int file_change(const char * filename, void * ) {
	printf("file[%s]: changed\n", filename);
	if (g_run) {
		if (++g_run >3) {
			g_run = 0;
		}
	}
	return 0;
}

int main(){
	FileReloader rd;
	rd.add_file_monitor("./reload.conf", file_change, NULL);
	int add = rd.add_file_monitor("./reload.conf", file_change, NULL);
	printf("add:%d\n", add);
	rd.add_file_monitor("./reload.range", file_change, NULL);
	g_run = 1;
	pthread_t tid;
	pthread_create(&tid, NULL, change_threadfun, &rd);
	pthread_join(tid, NULL);
	return 0;
}


