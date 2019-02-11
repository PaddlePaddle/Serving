/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: yperfbench.h,v 1.1 2008/09/03 06:47:49 xiaowei Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file yperfbench.h
 * @author yufan(com@baidu.com)
 * @date 2008/07/30 17:15:22
 * @version $Revision: 1.1 $ 
 * @brief 
 *  
 **/


#ifndef  __YPERFBENCH_H_
#define  __YPERFBENCH_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#define __YPERFPENCH_NAME_LENGTH 64
#define __YPERFPENCH_VALUE_LENGTH 64

namespace pb
{

struct output
{
	char name[__YPERFPENCH_NAME_LENGTH];
	char value[__YPERFPENCH_VALUE_LENGTH];
};

extern int argc;
extern char ** argv;


void init_opt(int  _argc, char ** _argv);
void print_result();

	
template <typename T>
T getopt(char * arg)
{
	for(int i=1; i<argc; i++)
	{
		char name[__YPERFPENCH_NAME_LENGTH];
		char value[__YPERFPENCH_VALUE_LENGTH];
		if ( sscanf(argv[i], "-%1s=%s", name, value) == 2)
		{
			if(!strcmp(name, arg))
				return (T)atof(value);
		}
		else if ( sscanf(argv[i], "--%[^=]=%s", name, value) == 2)
		{
			if(!strcmp(name, arg))
				return (T)atof(value);
		}
		else
		{
			fprintf(stderr, "Invalide Argument: %s", arg);
			exit(1);
		}
	
	}
	fprintf(stderr, "Invalide Argument: %s", arg);
	exit(1);
}

void put_result(char * name, char * value);
void put_result(char * name, int value);
void put_result(char * name, double value);
#include <unistd.h>
#include <sys/time.h>
class timer
{
	timeval start;
	char name[__YPERFPENCH_NAME_LENGTH];
public:
	timer();
	timer(char * name);
	void check();
};


struct threading
{
	pthread_t *threads;
	int num;
};

threading create_threads(void *(void*), int num);
void wait_threads(threading t);
void run_threads(void *fun(void*), int num);
}










#endif  //__YPERFBENCH_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
