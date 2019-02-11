/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_thread_hash.cpp,v 1.2 2008/11/12 04:13:35 xiaowei Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file bsl_thread_hash.cpp
 * @author xiaowei(com@baidu.com)
 * @date 2008/08/26 11:45:39
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/

#include <stdio.h>
#include <iostream>
#include <xutils.h>
#include <yperfbench.h>

#include <alloc/allocator.h>
#include <ans/hash_map.h>
#include <containers/hash/bsl_hashmap.h>
#include <containers/hash/bsl_phashmap.h>
#include <string>

template <size_t size>
struct type_t
{
	char data[size];
};

struct point_t
{
	size_t hash_bucket;
	size_t hash_datanum;
	int thread_num;
	int value_size;
	long query_loop;
	int insert_speed;
	std::string hash_type;
public:
	point_t() {
		hash_bucket = 0;
		hash_datanum = 0;
		thread_num = 0;
		value_size = 0;
		query_loop = 0;
		insert_speed = 0;
		hash_type = "";
	}
};

int **g_randsize = 0;
int g_size = 0;
pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
int g_pos = 0;
int g_num = 0;

void randint(int num, int data)
{
	g_randsize = (int **) malloc (sizeof(int *) * num);
	for (int i=0; i<num; ++i) {
		g_randsize[i] = (int *)malloc(sizeof(int) * data);
		for (int j=0; j<data; ++j) {
			g_randsize[i][j] = rand();
		}
	}
	g_pos = 0;
	g_size = data;
	g_num = num;
}

int * getrand() {
	pthread_mutex_lock(&g_lock);
	if (g_pos >= g_num) g_pos = 0;
	int *val = g_randsize[g_pos];
	++ g_pos;
	pthread_mutex_unlock(&g_lock);
	return val;
}

template <class hash_t>
struct test_query_t
{
	hash_t & _1;
	Slotime _2;
	point_t & _3;
public:
	test_query_t(hash_t & h, point_t &p) :
		_1(h), _2(p.thread_num), _3(p) {}
};

template <class hash_t, typename value_type>
void * test_insert_r(void *param)
{
	test_query_t<hash_t> *fd = (test_query_t<hash_t> *)param;
	timeval s, e;
	gettimeofday(&s, NULL);
	for (int i=0; i<fd->_3.hash_datanum; ++i) {
		fd->_1.set(rand(), value_type());
		if ((i%fd->_3.insert_speed) == fd->_3.insert_speed - 1) {
			gettimeofday(&e, NULL);
			long used = XTIMEDIFF(s, e);
			if (used < 1000) {
				timespec t = {
					(1000-used) / 1000,
					(1000-used) * 1000000
				};
				timespec e;
				nanosleep(&t, &e);
			}
			gettimeofday(&s, NULL);
		}
	}

	return NULL;
}


template <class hash_t, typename type_t>
void * test_query_r(void *arg)
{
	test_query_t<hash_t> *fd = (test_query_t<hash_t> *)arg;
	type_t v;
	int *val = getrand();
	long loop = fd->_3.query_loop / (long)fd->_3.thread_num;
	fd->_2.start();
	for (long i=0; i<loop; ++i) {
		fd->_1.get(val[i%g_size], &v);
	}
	fd->_2.stop();
	return NULL;
}

unsigned int ans_hash_fcnt(const long & v) { return v; }

template <typename hash_t, typename stype_t>
void test(hash_t &hash, point_t &p)
{
	for (int i=0; i<(int)((float)p.hash_datanum/(1.5f)); ++i) {
		hash.set(rand(), stype_t());
	}
	test_query_t<hash_t> qval(hash, p);
	test_query_t<hash_t> uval(hash, p);
	xthread_t ptr[] = {
		{test_query_r<hash_t, stype_t>, &qval, p.thread_num},
		{test_insert_r<hash_t, stype_t>, &uval, 1},
	};
	run_thread(ptr, 2);
	double speed = (double)(p.query_loop) / ((float)(qval._2.cost()) + 1e-10);
	char flag[1024];
	snprintf(flag, sizeof(flag), "speed(%s)", p.hash_type.c_str());
	//std::cout<<flag<<std::endl;
	pb::put_result(flag, speed);
}

template <typename T, size_t size>
void testcase(point_t &p)
{
	//ans::hashmap
	{
		std::cout<<"ans_hashmap "<<size<<std::endl;
		typedef ans::hash_map<T, type_t<size> > anshash1_t;
		anshash1_t hash;
		hash.create(p.hash_bucket, ans_hash_fcnt);
		p.hash_type = "ans_hashmap";
		test<anshash1_t, type_t<size> >(hash, p);
		std::cout<<std::endl;
		hash.clear();
	}
	//bsl::phashmap
	{
		std::cout<<"bsl_phashmap"<<size<<std::endl;
		typedef bsl::phashmap<T, type_t<size> > bslhash1_t;
		bslhash1_t hash;
		hash.create(p.hash_bucket);
		p.hash_type = "bsl_phashmap";
		test<bslhash1_t, type_t<size> >(hash, p);
		std::cout<<std::endl;
	}
}

int main(int argc, char **argv)
{
	pb::init_opt(argc, argv);
	randint(20, 1<<20);
	point_t p;
	p.hash_bucket = pb::getopt<int>("hash_bucket");
	p.hash_datanum = pb::getopt<int>("hash_datanum");
	p.thread_num = pb::getopt<int>("thread_num");
	p.value_size = pb::getopt<int>("value_size");
	p.query_loop = pb::getopt<long>("query_loop");
	p.query_loop *= 1000000;
	p.insert_speed = pb::getopt<int>("insert_speed");

	if (p.value_size <= 4) {
		testcase<long, 4> (p);
	} else if (p.value_size <= 8) {
		testcase <long, 8> (p);
	} else if (p.value_size < 32) {
		testcase <long, 16> (p);
	} else {
		testcase <long, 32> (p);
	}
	return 0;
}


/* vim: set ts=4 sw=4 sts=4 tw=100 */
