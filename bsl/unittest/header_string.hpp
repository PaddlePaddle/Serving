#ifndef _HEADER_HPP
#define _HEADER_HPP


#include <cxxtest/TestSuite.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <bsl/containers/hash/bsl_hashmap.h>
#include <bsl/containers/hash/bsl_phashmap.h>
#include <bsl/containers/string/bsl_string.h>
#include <bsl/archive/bsl_filestream.h>
#include <bsl/archive/bsl_serialization.h>
#include <bsl/archive/bsl_binarchive.h>
#include <bsl/alloc/allocator.h>
#include <vector>
#include <map>
#include <sys/time.h>
#include <signal.h>
#include <ul_sign.h>

//////utils
#define __XASSERT(flag, fmt, arg...)  \
{\
	bool ___bsl_flag = flag; \
	if (!(___bsl_flag)) { \
		fprintf(stdout, "\n[error][%s:%d][%s]"fmt"\n", __FILE__, __LINE__, #flag, ##arg); \
		return false; \
	}\
}

#define __XASSERT2(flag) __XASSERT(flag, "")


static std::vector<pthread_t> g_pid;
static bool g_pthread_flag = true;
#define __XASSERT_R(flag, fmt, arg...) \
{\
	bool ___bsl_flag = flag; \
	if (!(___bsl_flag)) { \
		fprintf(stdout, "\n[error][%s:%d][%s]"fmt"\n", __FILE__, __LINE__, #flag, ##arg); \
		g_pthread_flag = false; \
		for (int i=0; i<(int)g_pid.size(); ++i) { \
			if (g_pid[i] != pthread_self()) { \
				int ret = pthread_cancel(g_pid[i]); \
				fprintf(stdout, "\n[error]kill pthread_t[%ld][%d]ret[%d][%m]\n", (long)g_pid[i], i, ret); \
			} \
		} \
		return NULL; \
	}\
}

#define __XASSERT2_R(flag) __XASSERT_R(flag, "")

class auto_timer
{
	char buf[64];
	timeval s, e;
	public:
	auto_timer(const char *str = NULL) {
		buf[0] = 0;
		if (str) snprintf(buf, sizeof(buf), "%s", str);
		gettimeofday(&s, NULL);
	}
	auto_timer(const char *str, int val) {
		snprintf(buf, sizeof(buf), "%s %d ", str, val);
		gettimeofday(&s, NULL);
	}
	~auto_timer() {
		gettimeofday(&e, NULL);
		long t = (long)((e.tv_sec - s.tv_sec) * 1000000 + (e.tv_usec - s.tv_usec));
		if (t/1000 > 0) {
			__BSL_DEBUG("%s %ld", buf, t);
		}
	}
};

class dis_timer
{
	timeval s, e;
public:
	void start() {
		gettimeofday(&s, NULL);
	}
	void end() {
		gettimeofday(&e, NULL);
	}
	int costms () {
		return (int)((e.tv_sec-s.tv_sec)*1000 + (e.tv_usec-s.tv_usec)/1000);
	}
};



bsl::string randstr(int wan=32)
{
	char buf[130] = {0};
	int len = rand()%wan + 1;
	for (int i=0; i<len; ++i) {
		buf[i] = rand()%26 + 'a';
	}
	return buf;
};

bsl::string int2str(int val)
{
	char buf[20];
	snprintf(buf, sizeof(buf), "%d", val);
	return buf;
}

/**
 *key µÄhash·Âº¯Êý
 */
struct hash_key
{
	size_t operator () (const bsl::string &__key) const {
#if 1
		size_t key = 0;
		for (size_t i=0; i<__key.size(); ++i) {
			key = (key<<8)+__key[i];
		}
#else
		unsigned long key;
		unsigned int *k = (unsigned int *) & key;
		creat_sign_f64((char *)__key.c_str(), __key.size(), k, k+1);
#endif
		return key;
	}
};

template <class HASH>
bool test_phashmap_temp(int pnum, void *(*pfun)(void *), int upnum, void *(*upfun)(void *),
		int lnum = 0, void *(*lfun)(void *) = 0) 
{
	HASH * test = new (std::nothrow) HASH;
	g_pthread_flag = true;
	__XASSERT2(test != NULL);
	__XASSERT2(test->create(1<<20) == 0);
	g_pid.clear();
	pthread_t pid;

	{
		//auto_timer t("phashmap costime");
		
		for (int i=0; i<pnum; ++i) {
			__XASSERT(pthread_create(&pid, NULL, pfun, test) == 0,
					"create %d thread error[%m]", i);
			g_pid.push_back(pid);
		}
		for (int i=0; i<upnum; ++i) {
			__XASSERT(pthread_create(&pid, NULL, upfun, test) == 0,
					"create %d thread for up erro[%m]", i);
			g_pid.push_back(pid);
		}
		for (int i=0; i<lnum; ++i) {
			__XASSERT(pthread_create(&pid, NULL, lfun, test) == 0, 
					"create %d thread for l error[%m]", i);
			g_pid.push_back(pid);
		}

		for (int i=0; i<(int)g_pid.size(); ++i) {
			pthread_join(g_pid[i], NULL);
			//fprintf(stdout, "---------------join %d %ld success\n", i, (long)g_pid[i]);
		}
	}

	delete test;
	return g_pthread_flag;
}

#endif
