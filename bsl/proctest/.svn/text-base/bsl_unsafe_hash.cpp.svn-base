/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_unsafe_hash.cpp,v 1.2 2008/11/12 04:00:31 xiaowei Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file bsl_unsf_hash.h
 * @author xiaowei(com@baidu.com)
 * @date 2008/08/22 18:53:11
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
#include <containers/hash/bsl_readmap.h>
#include <string>
#include <vector>
#include "stl_hashmap.h"

/*
 * 单线程下的性能测试比较
 */

struct point_t
{
	size_t hash_bucket;
	size_t hash_datanum;
	int thread_num;
	int value_size;
	long query_loop;
	std::string hash_type;
public:
	point_t() {
		hash_bucket = 0;
		hash_datanum = 0;
		thread_num = 0;
		value_size = 0;
		query_loop = 0;
		hash_type = "";
	}
};

template <size_t size>
struct type_t
{
	char data[size];
};

template <class hash_t, typename value_type>
void test_insert(hash_t &hash, int datanum)
{
	Slotime st(1);
	st.start();
	for (int i=0; i<datanum; ++i) {
		hash.set(i, value_type());
	}
	st.stop();
	//std::cout<<st.cost();
}


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
template <class hash_t, class type_t>
void test_query(hash_t &hash, point_t &p)
{
	test_query_t<hash_t> val(hash, p);
	run_thread(test_query_r<hash_t, type_t>, &val, p.thread_num);
	//std::cout<<" "<<val._2.cost() / val._2.pnum<<" ";
	double speed = (double)(p.query_loop) / ((float)(val._2.cost()) + 1e-10);
	char flag[1024];
	snprintf(flag, sizeof(flag), "speed(%s)", p.hash_type.c_str());
	//std::cout<<flag<<std::endl;
	pb::put_result(flag, speed);
};

unsigned int ans_hash_fcnt(const long & v) { return v; }

template <typename hash_t, typename stype_t>
void test(hash_t &hash, point_t &p)
{
	test_insert<hash_t, stype_t >(hash, p.hash_datanum);
	test_query<hash_t, stype_t >(hash, p);
}

template <typename T, size_t size>
void testcase(point_t &p)
{
	//stl::hashmap
	{
		g_pos = 0;
		std::cout<<"stl_hashmap "<<size<<std::endl;
		typedef xhashmap<T, type_t<size> > xhash_t;
		xhash_t hash;
		p.hash_type = "stl_hashmap";
		test<xhash_t, type_t<size> >(hash, p);
		std::cout<<std::endl;
	}
	//ans::hashmap
	{
		g_pos = 0;
		std::cout<<"ans_hashmap "<<size<<std::endl;
		typedef ans::hash_map<T, type_t<size> > anshash1_t;
		anshash1_t hash;
		hash.create(p.hash_bucket, ans_hash_fcnt);
		p.hash_type = "ans_hashmap";
		test<anshash1_t, type_t<size> >(hash, p);
		std::cout<<std::endl;
		hash.clear();
	}
	//bsl::readmap
	{
		g_pos = 0;
		std::cout<<"bsl_readmap"<<size<<std::endl;
		typedef bsl::readmap<T, type_t<size> > rm_t;
		rm_t hash;
		hash.create(p.hash_bucket);
		p.hash_type = "bsl_readmap";
		std::cout<<std::endl;
		std::vector<std::pair<T, type_t<size> > >vec;
		for (int i=0; i<(int)p.hash_datanum; ++i) {
			vec.push_back(std::make_pair(i, type_t<size>()));
		}
		hash.assign(vec.begin(), vec.end());
		test_query<rm_t, type_t<size> >(hash, p);
		hash.clear();
	}
	//bsl::hashmap
	{
		g_pos = 0;
		std::cout<<"bsl_hashmap sample_alloc "<<size<<std::endl;
		typedef bsl::hashmap<T, type_t<size> > bslhash1_t;
		bslhash1_t hash;
		hash.create(p.hash_bucket);
		p.hash_type = "bsl_hashmap/sample_alloc";
		test<bslhash1_t, type_t<size> >(hash, p);
		std::cout<<std::endl;
	}
	{
		g_pos = 0;
		std::cout<<"bsl_hashmap alloc "<<size<<std::endl;
		typedef bsl::hashmap<T, type_t<size>, bsl::xhash<T>,
				bsl::equal<T>, bsl::bsl_alloc<T> > bslhash2_t;
		bslhash2_t hash;
		hash.create(p.hash_bucket);
		p.hash_type = "bsl_hashmap/alloc";
		test<bslhash2_t, type_t<size> >(hash, p);
		std::cout<<std::endl;
	}
	{
		g_pos = 0;
		std::cout<<"bsl_hashmap cpsalloc "<<size<<std::endl;
		typedef bsl::hashmap<T, type_t<size>, bsl::xhash<T>,
				bsl::equal<T>, bsl::bsl_cpsalloc<bsl::bsl_alloc<T> > > bslhash3_t;
		bslhash3_t hash;
		hash.create(p.hash_bucket);
		p.hash_type = "bsl_hashmap/cpsalloc";
		test<bslhash3_t, type_t<size> >(hash, p);
		std::cout<<std::endl;
	}
	//bsl::phashmap
	{
		g_pos = 0;
		std::cout<<"bsl_phashmap "<<size<<std::endl;
		typedef bsl::phashmap<T, type_t<size> > bslphash1_t;
		bslphash1_t hash;
		hash.create(p.hash_bucket);
		p.hash_type = "bsl_phashmap/sample_alloc";
		test<bslphash1_t, type_t<size> >(hash, p);
		std::cout<<std::endl;
	}
	{
		g_pos = 0;
		std::cout<<"bsl_phashmap bsl_alloc "<<size<<std::endl;
		typedef bsl::phashmap<T, type_t<size>, bsl::xhash<T>,
				bsl::equal<T>, bsl::bsl_alloc<T> > bslphash2_t;
		bslphash2_t hash;
		hash.create(p.hash_bucket);
		p.hash_type = "bsl_phashmap/alloc";
		test<bslphash2_t, type_t<size> >(hash, p);
		std::cout<<std::endl;
	}
	{
		g_pos = 0;
		std::cout<<"bsl_phashmap cpsalloc "<<size<<std::endl;
		typedef bsl::phashmap<T, type_t<size>, bsl::xhash<T>,
				bsl::equal<T>, bsl::bsl_cpsalloc<bsl::bsl_alloc<T> > > bslphash3_t;
		bslphash3_t hash;
		hash.create(p.hash_bucket);
		p.hash_type = "bsl_phashmap/cpsalloc";
		test<bslphash3_t, type_t<size> >(hash, p);
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

#if 0
	pb::put_result("hash_bucket", (int)p.hash_bucket);
	pb::put_result("hash_datanum", (int)p.hash_datanum);
	pb::put_result("thread_num", (int)p.thread_num);
	pb::put_result("value_size", (int)p.value_size);
	pb::put_result("query_loop", (int)p.query_loop);
#endif
	//testcase<long, 8>(p);
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

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
