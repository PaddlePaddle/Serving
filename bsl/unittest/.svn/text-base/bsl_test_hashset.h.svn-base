/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_test_hashset.h,v 1.5 2008/12/15 09:57:00 xiaowei Exp $ 
 *
 * hashset phashset的测试代码
 * test_hashset	测试hashset phashset的基本功能
 * test_hashset_iterator	测试hashset的迭代器功能
 * test_hashset_serialization	测试容器的序列化功能
 * 
 * 注意:phashset的线程安全性问题，在另外的测试文件里面单独测试
 **************************************************************************/
 
 
 
/**
 * @file settest.cpp
 * @author xiaowei(com@baidu.com)
 * @date 2008/08/14 11:10:06
 * @version $Revision: 1.5 $ 
 * @brief 
 *  
 **/

#ifndef __BSL_TEST_HASHSET_H
#define __BSL_TEST_HASHSET_H

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <bsl/alloc/allocator.h>
#include <bsl/containers/hash/bsl_hashset.h>
#include <bsl/containers/hash/bsl_phashset.h>
#include <bsl/containers/hash/bsl_rwhashset.h>
#include <bsl/archive/bsl_filestream.h>
#include <bsl/archive/bsl_serialization.h>
#include <bsl/archive/bsl_binarchive.h>
#include <vector>
#include <set>

#define __XASSERT(flag, fmt, arg...)  \
{\
	bool ___bsl_flag = flag; \
	if (!(___bsl_flag)) { \
		fprintf(stdout, "\n[error][%s:%d][%s]"fmt"\n", __FILE__, __LINE__, #flag, ##arg); \
		return false; \
	}\
}

#define __XASSERT2(flag) __XASSERT(flag, "")

char s[][32]={"yingxiang", "yufan", "wangjiping", "xiaowei", "yujianjia", "maran", "baonenghui",
	"gonglei", "wangyue", "changxinglong", "chenxiaoming", "guoxingrong", "kuangyuheng"};
const size_t N = sizeof(s) / sizeof(s[0]);

std::string randstr()
{
	char buf[130] = {0};
	int len = rand()%64 + 1;
	for (int i=0; i<len; ++i) {
		buf[i] = rand()%26 + 'a';
	}
	return buf;
};

/**
 *key 的hash仿函数
 */
struct hash_key
{
	size_t operator () (const std::string &__key) const {
		size_t key = 0;
		for (size_t i=0; i<__key.size(); ++i) {
			key = (key<<8)+__key[i];
		}
		return key;
	}
};


/*
 * hashset测试
 */

template <typename THMp>
bool test_hashset()
{
	int ret = 0;
	{
		int loop = 1<<10;
		for (int i=0; i<loop; ++i) {
			THMp test_0;
		}
		for (int i=0; i<loop; ++i) {
			THMp test_0;
			test_0.create(10000);
		}
	}

	THMp test;
	//疯狂create
	for (int i=0; i<100; ++i) {
		__XASSERT2 (test.create(100) == 0);
	}
	//疯狂destroy
	for (int i=0; i<100; ++i) {
		__XASSERT2 (test.destroy() == 0);
	}

	__XASSERT2 (test.create(102400) == 0);

	//测试插入查询，是否正常
	int insertsiz = 1<<10;
	std::vector<std::string> vec;
	for (int i=0; i<insertsiz; ++i) {
		std::string str = randstr();
		ret = test.set(str);
		__XASSERT (ret != -1, "set %s error", str.c_str());
		if (ret == bsl::HASH_INSERT_SEC) {
			vec.push_back(str);
		}
	}

	int relinsertsiz = (int)vec.size();

	//是否能重复加入，在不覆盖下是否正常
	for (int i=0; i<relinsertsiz; ++i) {
		ret = test.set(vec[i]);
		__XASSERT( ret == bsl::HASH_EXIST,
				"upece %s ret=%d", vec[i].c_str(), ret);
	}

	for (int i=0; i<relinsertsiz; ++i) {
		ret = test.get(vec[i]);
		__XASSERT (ret == bsl::HASH_EXIST, "get %s err", vec[i].c_str());
	}

	//查询是否能成功删除
	for (int i=0; i<relinsertsiz; ++i) {
		ret = test.erase(vec[i]);
		__XASSERT (ret == bsl::HASH_EXIST, "erase %s no exist, ret=%d", vec[i].c_str(), ret);
		ret = test.get(vec[i]);
		__XASSERT (ret == bsl::HASH_NOEXIST, "no erase %s", vec[i].c_str());
	}

	//再次插入
	for (int i=0; i<relinsertsiz; ++i) {
		ret = test.set(vec[i]);
		__XASSERT(ret == bsl::HASH_INSERT_SEC, "insert %s error, ret=%d", vec[i].c_str(), ret);
	}

	//查询是否正常
	for (int i=0; i<relinsertsiz; ++i) {
		ret = test.get(vec[i]);
		__XASSERT( ret == bsl::HASH_EXIST, "hash %s no exist ret=%d",
				vec[i].c_str(), ret);
	}

	return test.destroy() == 0;
}

template <typename THMp>
bool test_hashset_iterator()
{
	int ret = 0;
	THMp test;
	std::set<std::string> vec;
	__XASSERT2 (test.create(1<<15) == 0);

	int insertsize = 1<<10;
	for (int i=0; i<insertsize; ++i) {
		std::string key = randstr();
		ret = test.set(key);
		__XASSERT2 (ret != -1);
		vec.insert(key);
	}

	__XASSERT(test.size() == vec.size(), "size unequl %lu != %lu",
			(unsigned long)test.size(), (unsigned long)vec.size());

	for (typename THMp::iterator iter = test.begin(); iter != test.end(); ++iter) {
		__XASSERT2 (vec.find(*iter) != vec.end());
	}
	for (typename THMp::iterator iter = test.begin(); iter != test.end(); iter++) {
		__XASSERT2 (vec.find(*iter) != vec.end());
	}
	for (typename THMp::const_iterator iter = test.begin(); iter != test.end(); ++iter) {
		__XASSERT2 (vec.find(*iter) != vec.end());
	}
	for (typename THMp::const_iterator iter = test.begin(); iter != test.end(); iter++) {
		__XASSERT2 (vec.find(*iter) != vec.end());
	}

	const THMp & testp = test;
	for (typename THMp::const_iterator iter = testp.begin(); iter != testp.end(); ++iter){
		__XASSERT2 (vec.find(*iter) != vec.end());
	}
	for (typename THMp::const_iterator iter = testp.begin(); iter != testp.end(); iter++){
		__XASSERT2 (vec.find(*iter) != vec.end());
	}
	testp.get(randstr());

	//不调用destroy，自己调用析构函数析构
	//需要valgrind检查是否内存泄露
	return true;
}

template <class THMp>
bool test_hashset_serialization()
{
	THMp test;
	int ret = 0;
	std::set<std::string> vec;
	__XASSERT2 (test.create(1<<15) == 0);
	int loop = 1<<12;
	for (int i=0; i<loop; ++i) {
		std::string key = randstr();
		__XASSERT2 (test.set(key) != -1);
		vec.insert(key);
	}

	const char * htdat = "settest.data";
	//写硬盘
	{
		bsl::filestream fs;
		__XASSERT2 (fs.open(htdat, "w") == 0);
		bsl::binarchive ar(fs);
		__XASSERT2 (ar.write(test) == 0);
		fs.close();
	}
	//读硬盘
	{
		bsl::filestream fs;
		__XASSERT2 (fs.open(htdat, "r") == 0);
		bsl::binarchive ar(fs);
		__XASSERT2 (ar.read(&test) == 0);
		fs.close();

		//check
		__XASSERT (vec.size() == test.size(), "%lu != %lu",
				(unsigned long)vec.size(), (unsigned long)test.size());
		for (std::set<std::string>::iterator iter = vec.begin();
				iter != vec.end(); ++iter) {
			ret = test.get(*iter);
			__XASSERT2 (ret == bsl::HASH_EXIST);
		}
	}
	//test.clear();
	//读硬盘到新结构体
	{
		THMp test_0;
		__XASSERT2 (test_0.create(100) == 0);
		bsl::filestream fs;
		__XASSERT2 (fs.open(htdat, "r") == 0);
		bsl::binarchive ar(fs);
		__XASSERT2 (ar.read(&test_0) == 0);
		fs.close();

		//check
		__XASSERT (vec.size() == test_0.size(), "%lu != %lu",
				(unsigned long)vec.size(), (unsigned long)test_0.size());
		for (std::set<std::string>::iterator iter = vec.begin();
				iter != vec.end(); ++iter) {
			ret = test_0.get(*iter);
			__XASSERT2 (ret == bsl::HASH_EXIST);
		}
	}
	//读硬盘到新结构体, 新结构体不初始化
	{
		THMp test_0;
		bsl::filestream fs;
		__XASSERT2 (fs.open(htdat, "r") == 0);
		bsl::binarchive ar(fs);
		__XASSERT2 (ar.read(&test_0) == 0);
		fs.close();

		//check
		__XASSERT (vec.size() == test_0.size(), "%lu != %lu",
				(unsigned long)vec.size(), (unsigned long)test_0.size());
		for (std::set<std::string>::iterator iter = vec.begin();
				iter != vec.end(); ++iter) {
			std::string val = "";
			ret = test_0.get(*iter);
			__XASSERT2 (ret == bsl::HASH_EXIST);
		}
	}
	return true;
}

//assign测试
template <class THMp>
bool test_hashset_assign()
{
	THMp test;
	std::set<std::string> vec;
	__XASSERT2(test.create(1<<15) == 0);
	int loop = 1<<15;
	for (int i=0; i<loop; ++i) {
		vec.insert(randstr());
	}
	__XASSERT2(test.assign(vec.begin(), vec.end()) == 0);
	for (std::set<std::string>::iterator iter = vec.begin();
			iter != vec.end(); ++iter) {
		int ret = test.get(*iter);
		__XASSERT2(ret == bsl::HASH_EXIST);
	}
	return true;
}

template <typename THMp>
bool test_hashset_clear()
{
	//没有create的情况下clear
	{
		THMp test0;
		__XASSERT2(test0.size() == 0);
		for (int i=0; i<10000; ++i) {
			__XASSERT2(test0.clear() == 0);
			__XASSERT2(test0.size() == 0);
		}
	}
	//create的情况下clear
	{
		THMp test0;
		__XASSERT2(test0.size() == 0);
		__XASSERT2(test0.create(100000) == 0);
		__XASSERT2(test0.size() == 0);
		__XASSERT2(test0.clear() == 0);
		__XASSERT2(test0.size() == 0);
	}
	//有数据的情况下clear
	{
		THMp test0;
		typedef std::set<std::string> map_t;
		map_t vmap;
		for (int i=0; i<10000; ++i) {
			vmap.insert(randstr());
		}
		__XASSERT2(test0.create(vmap.size() * 2) == 0);
		for (int i=0; i<10; ++i) {
			__XASSERT2(test0.assign(vmap.begin(), vmap.end()) == 0);
		}

		for (int i=0; i<10; ++i) {
			__XASSERT2(test0.clear() == 0);
			__XASSERT2(test0.size() == 0);
		}

		for (int i=0; i<100; ++i) {
			__XASSERT2(test0.destroy() == 0);
		}
	}
	return true;
}

class bsl_test_hashmap : public CxxTest::TestSuite
{
public:
	typedef std::string key;
	typedef bsl::hashset<std::string, hash_key> THMp1;
	
	void test_operator() {
		{
			THMp1 ht;
			std::set<key> st;
			ht.assign(st.begin(),st.end());
			THMp1 ht2;
			ht2 = ht;
			THMp1 ht3(ht);
		}
		{
			std::set<key> st;
			for (size_t i = 0; i < N; i ++) {
				st.insert( key(s[i] ) );
			}
			THMp1 ht;
			ht.assign(st.begin(),st.end());
			THMp1 ht2;
			ht2 = ht;
			THMp1 ht3(ht);
			for (size_t i = 0; i < N; i ++) {
				int ret = ht.get(key(s[i]));
				TS_ASSERT( ret == bsl::HASH_EXIST );
				ret = ht2.get(key(s[i]));
				TS_ASSERT( ret == bsl::HASH_EXIST );
				ret = ht3.get(key(s[i]));
				TS_ASSERT( ret == bsl::HASH_EXIST );
			}
		}
	}
	void test_create() {
		THMp1 ht;
		for (int i = 10; i < 100; i ++) {
			ht.create(i);
		}
		THMp1 ht2;
		for (int i = 0; i < 100; i ++) {
			ht2 = ht;
		}
	}
	void test_test_hashset_(void) {
		__BSL_DEBUG("open debug mode");
		TSM_ASSERT ("", test_hashset<THMp1>());
	}
	void test_hashset_iterator_() {
		TSM_ASSERT ("", test_hashset_iterator<THMp1>());
	}
	void test_hashset_serialization_() {
		TSM_ASSERT ("", test_hashset_serialization<THMp1>());
	}
	void test_hashset_assign_() {
		TSM_ASSERT ("", test_hashset_assign<THMp1>());
	}
	void test_hashset_clear_() {
		TSM_ASSERT ("", test_hashset_clear<THMp1>());
	}

	typedef bsl::hashset<std::string, hash_key, bsl::equal<std::string>,
				bsl::bsl_alloc<std::string> > THMp2;
	void test_hashset2_() {
		TSM_ASSERT ("", test_hashset<THMp2>());
	}
	void test_hashset2_iterator_() {
		TSM_ASSERT ("", test_hashset_iterator<THMp2>());
	}
	void test_hashset2_serialization_() {
		TSM_ASSERT ("", test_hashset_serialization<THMp2>());
	}
	void test_hashset2_assign_() {
		TSM_ASSERT ("", test_hashset_assign<THMp2>());
	}
	void test_hashset2_clear_() {
		TSM_ASSERT ("", test_hashset_clear<THMp2>());
	}

	typedef bsl::phashset<std::string, hash_key> THMp3;
	void test_phashset_() {
		TSM_ASSERT ("", test_hashset<THMp3>());
	}
	void test_phashset_iterator_() {
		TSM_ASSERT ("", test_hashset_iterator<THMp3>());
	}
	void test_phashset_serialization_() {
		TSM_ASSERT ("", test_hashset_serialization<THMp3>());
	}
	void test_phashset_assign_() {
		TSM_ASSERT ("", test_hashset_assign<THMp3>());
	}
	void test_phashset_clear_() {
		TSM_ASSERT ("", test_hashset_clear<THMp3>());
	}

	typedef bsl::phashset<std::string, hash_key, bsl::equal<std::string>,
				bsl::bsl_alloc<std::string> > THMp4;
	void test_phashset2_() {
		TSM_ASSERT ("", test_hashset<THMp4>());
	}
	void test_phashset2_iterator_() {
		TSM_ASSERT ("", test_hashset_iterator<THMp4>());
	}
	void test_phashset2_serialization() {
		TSM_ASSERT ("", test_hashset_serialization<THMp4>());
	}
	void test_phashset2_assign_() {
		TSM_ASSERT ("", test_hashset_assign<THMp4>());
	}
	void test_phashset2_clear_() {
		TSM_ASSERT ("", test_hashset_clear<THMp4>());
	}

	typedef bsl::bsl_rwhashset<std::string, hash_key, bsl::equal<std::string>,
				bsl::bsl_alloc<std::string> > THMp5;
	void test_rwhashset_() {
		TSM_ASSERT ("", test_hashset<THMp5>());
	}
	void test_rwhashset_clear_() {
		TSM_ASSERT ("", test_hashset_clear<THMp5>());
	}

	typedef bsl::phashset<std::string, hash_key, bsl::equal<std::string>,
				bsl::bsl_cpsalloc<bsl::bsl_alloc<std::string> > > THMp6;
	void test_phashset3_() {
		TSM_ASSERT ("", test_hashset<THMp6>());
	}
	void test_phashset3_iterator_() {
		TSM_ASSERT ("", test_hashset_iterator<THMp6>());
	}
	void test_phashset3_serialization() {
		TSM_ASSERT ("", test_hashset_serialization<THMp6>());
	}
	void test_phashset3_assign_() {
		TSM_ASSERT ("", test_hashset_assign<THMp6>());
	}
	void test_phashset3_clear_() {
		TSM_ASSERT ("", test_hashset_clear<THMp6>());
	}

};

#endif
/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
