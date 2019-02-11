/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: xutils.h,v 1.1 2008/09/03 06:47:49 xiaowei Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file xutils.h
 * @author xiaowei(com@baidu.com)
 * @date 2008/06/12 23:45:24
 * @version $Revision: 1.1 $ 
 * @brief 
 *  
 **/


#ifndef  __XUTILS_H_
#define  __XUTILS_H_
#include <pthread.h>
#include <sys/time.h>

#define XTIMEDIFF(s, e) ((e.tv_sec-s.tv_sec)*1000+(e.tv_usec-s.tv_usec)/1000)


typedef void * thread_fun_t(void *);
struct xthread_t
{
	thread_fun_t * fun;
	void * param;
	int pnum;
};

int run_thread(void *(*fun)(void *), void *, int);
int run_thread(xthread_t *ptr, int size);

class Slotime
{
public:
	pthread_mutex_t lock;
	pthread_cond_t cond;
	timeval _s, _e;
	int pnum;
	int inp;
public:
	Slotime(int tpnum);
	~Slotime();
	int cost();
	int start();
	int stop();
};

void xrecord(const char *fn, const char *fmt, ...);








#endif  //__XUTILS_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
