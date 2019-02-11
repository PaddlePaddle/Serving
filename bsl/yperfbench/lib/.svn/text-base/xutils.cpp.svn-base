/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: xutils.cpp,v 1.1 2008/09/03 06:47:49 xiaowei Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file xutils.cpp
 * @author xiaowei(com@baidu.com)
 * @date 2008/06/12 23:45:39
 * @version $Revision: 1.1 $ 
 * @brief 
 *  
 **/

#include <xutils.h>
#include <stdio.h>
#include <stdarg.h>

int run_thread(void *(*fun)(void *), void *p, int pnum)
{
	pthread_t *pid = new pthread_t[pnum];
	for (int i=0; i<pnum; ++i) {
		pthread_create(pid+i, NULL, fun, p);
	}
	for (int i=0; i<pnum; ++i) {
		pthread_join(pid[i], NULL);
	}
	delete [] pid;
	return 0;
}

int run_thread(xthread_t *ptr, int size)
{
	pthread_t **pidvec = new pthread_t *[size];
	for (int i=0; i<size; ++i) {
		pidvec[i] = new pthread_t[ptr[i].pnum];
		for (int j=0; j<ptr[i].pnum; ++j) {
			pthread_create(pidvec[i]+j, NULL, ptr[i].fun, ptr[i].param);
		}
	}
	for (int i=0; i<size; ++i) {
		for (int j=0; j<ptr[i].pnum; ++j) {
			pthread_join(pidvec[i][j], NULL);
		}
		delete [] pidvec[i];
	}
	delete [] pidvec;
	return 0;
}


void xrecord(const char *fn, const char *format, ...)
{
	FILE *fp = fopen(fn, "a");
	if (fp == NULL) return;	
	
	char buffer[1024];
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	fprintf(fp, "%s", buffer);
	fclose(fp);
}

Slotime::Slotime(int tpnum) {
	this->pnum = tpnum;
	_s.tv_sec = 0;
	_e.tv_sec = 0;
	_s.tv_usec = 0;
	_e.tv_usec = 0;
	this->inp = 0;
	pthread_mutex_init(&lock, NULL);
	pthread_cond_init(&cond, NULL);
}
Slotime::~Slotime() {
	pthread_mutex_destroy(&lock);
	pthread_cond_destroy(&cond);
}
int Slotime::cost() {
	return XTIMEDIFF(_s, _e);
}

int Slotime::start() {
	pthread_mutex_lock(&lock);
	++inp;
	if (inp == pnum) {
		for (int i=1; i<inp; ++i) {
			pthread_cond_signal(&cond);
		}
		gettimeofday(&_s, NULL);
	} else {
		pthread_cond_wait(&cond, &lock);
		gettimeofday(&_s, NULL);
	}
	pthread_mutex_unlock(&lock);
	
	return 0;
}

int Slotime::stop() {
	pthread_mutex_lock(&lock);
	gettimeofday(&_e, NULL);
	pthread_mutex_unlock(&lock);
	return cost();
}

/* vim: set ts=4 sw=4 sts=4 tw=100 */
