/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $测试phashmap在多线程环境下的安全性问题$ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file bsl_test_phashmap.h
 * @author xiaowei(com@baidu.com)
 * @date 2008/08/18 12:37:07
 * @version $Revision: 1.4 $ 
 * @brief 
 *  
 **/


#ifndef  __BSL_TEST_PHASHMAP_H_SHMAP_H_
#define  __BSL_TEST_PHASHMAP_H_

#include <header.hpp>

static const int DATANUM = 100000;

/////////////
/**
 * 测试多线程的读写安全情况
 * 这个case测试两个hash表，
 * 一个是phash，一个是rwhash
 */
template <class HASH>
void * __test_phashmap_fun_query(void *param)
{
	auto_timer t("query time at one thread");
	HASH * test = (HASH *)param;
	__XASSERT2(test!=NULL);
	int qloop = DATANUM / 20;
	for (int i=0; i<qloop; ++i) {
		//if (i % (1<<15) == 0) fprintf(stderr, "query success[%d]\n", i);
		std::string str = randstr();
		std::string val;
		int ret = test->get(str, &val);
		if (ret == bsl::HASH_EXIST) {
			__XASSERT2_R(str == val);
		}
	}

	return NULL;
}
template <class HASH>
void * __test_phashmap_fun_update(void *param)
{
	//auto_timer t("update time at one thread");
	HASH *test = (HASH *)param;
	__XASSERT2(test!=NULL);
	int uloop = DATANUM;
	for (int i=0; i<uloop; ++i) {
		//if (i % (1<<15) == 0) fprintf(stderr, "update success[%d]\n", i);
		std::string s1 = randstr();
		__XASSERT2_R(test->set(s1, s1) != -1);
		test->erase(randstr());
	}
	return NULL;
}


template <class HASH>
bool test_phashmap()
{
	return test_phashmap_temp<HASH>(20, __test_phashmap_fun_query<HASH>, 
			1, __test_phashmap_fun_update<HASH>);
}

//测试在写情况下dump数据问题
template <class HASH>
void * __test_cp_fun1(void *param)
{
	HASH *test = (HASH *)param;
	__XASSERT2(test != NULL);
        int result = 0;
	int loop = 1000;
	for (int i=0; i<loop; ++i) {
		int res = test->make_checkpoint();
                if(res==0){
                    result++;
                }
                test->set(int2str(i),"100");
                //test->end_checkpoint();
                //test->get(int2str(i+1000));
	}
        printf("\n\nstart_checkpoint result = %d\n",result);
	return NULL;
}
template <class HASH>
void * __test_cp_fun2(void *param)
{
	HASH *test = (HASH *)param;
	__XASSERT2(test != NULL);
	int loop = 1000;
        int result = 0;
	for (int i=0; i<loop; ++i) {
		int res = test->end_checkpoint();
                if(res==0){
                    result++;
                }
                test->get(int2str(i));
                //test->make_checkpoint();
                test->erase(int2str(i));
	}
        printf("\n\nend_checkpoint result = %d\n",result);
	return NULL;
}

template <class HASH>
bool test_phashmap_cp1()
{
	return test_phashmap_temp<HASH>(10, __test_cp_fun1<HASH>,
			10, __test_cp_fun2<HASH>);
}



//测试在写情况下dump数据问题
template <class HASH>
void * __test_cp_fun3(void *param)
{
	HASH *test = (HASH *)param;
	__XASSERT2_R(test != NULL);
	int num = DATANUM;
	for (int i=0; i<num; ++i) {
		__XASSERT2_R(test->set(int2str(i), int2str(i)) == bsl::HASH_INSERT_SEC);
	}
	{
		auto_timer t("make checkpoint");
		__XASSERT2_R(test->make_checkpoint() == 0);
	}
	for (int i=0; i<num; ++i) {
		auto_timer t("1 set", i);
		__XASSERT2_R(test->set(int2str(i), int2str(num/2 - i), 1) == bsl::HASH_OVERWRITE);
	}
#if 0
	//check
	for (int i=0; i<num; ++i) {
		std::string val;
		__XASSERT2_R(test->get(int2str(i), &val) == bsl::HASH_EXIST);
		__XASSERT2_R(val == int2str(num/2 - i));
	}
	__BSL_ERROR("start to erase");
#endif
	for (int i=0; i<num; ++i) {
		auto_timer t("2 erase", i);
		__XASSERT_R(test->erase(int2str(i)) == bsl::HASH_EXIST, "erase %d %d", i, 
				test->dumpstatus());
	}
	for (int i=0; i<num; ++i) {
		auto_timer t("3 set", i);
		__XASSERT2_R(test->set(int2str(i), int2str(num - i)) == bsl::HASH_INSERT_SEC);
	}
	return NULL;
}
template <class HASH>
void * __test_cp_fun4(void *param)
{
	HASH *test = (HASH *)param;
	__XASSERT2_R(test != NULL);
	while (test->dumpstatus() == bsl::PHASH_NORMAL) {
		struct timespec tv;
		tv.tv_sec = 0;
		tv.tv_nsec = 10 * 1000;
	
		nanosleep(&tv, NULL);
	}
	bsl::filestream fd;
	__XASSERT2_R(fd.open("phash.dat.1", "w") == 0);
	bsl::binarchive ar(fd);
	__XASSERT2_R(ar.write(*test) == 0);
	fd.close();

	__XASSERT2_R(fd.open("phash.dat.1", "r") == 0);
	bsl::binarchive ar2(fd);
	HASH test2;
	__XASSERT2_R(ar.read(&test2) == 0);
	fd.close();

	int num = DATANUM;
	__BSL_DEBUG("---------start to check size %d", (int)test2.size());
	for (int i=0; i<num; ++i) {
		std::string val;
		__XASSERT2_R(test2.get(int2str(i), &val) == bsl::HASH_EXIST);
		__XASSERT2_R(val == int2str(i));
	}

	return NULL;
}

template <class HASH>
void * __test_cp_fun5(void *param)
{
	HASH *test = (HASH *)param;
	__XASSERT2_R(test != NULL);
	int loop = DATANUM / 20;
	for (int i=0; i<loop; ++i) {
		test->get(int2str(i));
	}
	return NULL;
}

template <class HASH>
bool test_phashmap_cp2()
{
	return test_phashmap_temp<HASH>(1, __test_cp_fun3<HASH>,
			1, __test_cp_fun4<HASH>, 20, __test_cp_fun5<HASH>);
}


template<typename _T, int _I>
void inc_node(_T* v, void* /*args = NULL*/){
	*v += _I;
}

template<typename _T, int _I>
class IncNode{
public:
	void operator()(_T* v, void* /*args = NULL*/){
		*v += _I;
	}
};

template<typename HASH>
void* update_func(void* args){
	HASH *ht = (HASH *)args;
	for(int i = 0; i < 1000; ++i){
		for(int j = 0; j < 100; ++j){
			TS_ASSERT(ht->get(i, NULL, inc_node<typename HASH::value_type, 1>) == bsl::HASH_EXIST);
		}
		for(int j = 0; j < 100; ++j){
			TS_ASSERT(ht->get(i, NULL, IncNode<typename HASH::value_type, 1>()) == bsl::HASH_EXIST);
		}
	}
	return NULL;
}



class bsl_test_phashmap : public CxxTest::TestSuite
{
public:
	typedef bsl::phashmap<std::string, std::string, hash_key> type1_t;
	void test_phashmap_(void) {
		TSM_ASSERT ("", test_phashmap<type1_t>());
	}
	void test_phashmap_cp1_() {
		TSM_ASSERT ("", test_phashmap_cp1<type1_t>());
	}
	void test_phashmap_cp2_() {
		TSM_ASSERT ("", test_phashmap_cp2<type1_t>());
	}

	typedef bsl::phashmap<std::string, std::string, hash_key, bsl::equal<std::string>,
			bsl::bsl_alloc<std::string> > type2_t;
	void test_phashmap_2(void) {
		TSM_ASSERT ("", test_phashmap<type2_t>());
	}
	void test_phashmap_cp1_2() {
		TSM_ASSERT ("", test_phashmap_cp1<type2_t>());
	}
	void test_phashmap_cp2_2() {
		TSM_ASSERT ("", test_phashmap_cp2<type2_t>());
	}

	///
	typedef bsl::phashmap<std::string, std::string, hash_key, bsl::equal<std::string>,
			bsl::bsl_cpsalloc<bsl::bsl_alloc<std::string> > > type3_t;
	void test_phashmap_3(void) {
		TSM_ASSERT ("", test_phashmap<type3_t>());
	}
	void test_phashmap_cp1_3() {
		TSM_ASSERT ("", test_phashmap_cp1<type3_t>());
	}
	void test_phashmap_cp2_3() {
		TSM_ASSERT ("", test_phashmap_cp2<type3_t>());
	}
	
	void test_get_callback(){	
		typedef int _Key;
		typedef int _Value;
		typedef bsl::phashmap<_Key, _Value> hash_type;

		hash_type ht;
		int hash_num = 1333;
		int n = 1000;
		const int INC = 1000;
		
		TS_ASSERT(ht.create(hash_num) == 0);
		for(int i = 0; i < n; ++i){
			ht.set(i, 0);
		}
		TS_ASSERT((int)ht.size() == n);

		for(int i = 0; i < n; ++i){
			_Value old_val;
			TS_ASSERT(ht.get(i, &old_val, inc_node<hash_type::value_type, INC>) == bsl::HASH_EXIST);
			TS_ASSERT(old_val == 0);
			TS_ASSERT(ht.get(i, &old_val) == bsl::HASH_EXIST);
			TS_ASSERT(old_val == INC);
			TS_ASSERT((int)ht.size() == n);
		}
	
		ht.clear();
		TS_ASSERT(ht.create(hash_num) == 0);
		for(int i = 0; i < n; ++i){
			ht.set(i, 0);
		}
		TS_ASSERT((int)ht.size() == n);

		for(int i = 0; i < n; ++i){
			_Value old_val;
			TS_ASSERT(ht.get(i, &old_val, IncNode<hash_type::value_type, INC>()) == bsl::HASH_EXIST);
			TS_ASSERT(old_val == 0);
			TS_ASSERT(ht.get(i, &old_val) == bsl::HASH_EXIST);
			TS_ASSERT(old_val == INC);
			TS_ASSERT((int)ht.size() == n);
		}
		return;
	}
	
	void test_get_callback_multithread(){
		typedef int _Key;
		typedef int _Value;
		typedef bsl::phashmap<_Key, _Value> hash_type;

		int rep = 20;
		while(rep--){
			int hash_num = 1333;
			hash_type ht;
			TS_ASSERT(ht.create(hash_num) == 0);
			int n = 1000;
			for(int i = 0; i < n; ++i){
				ht.set(i, 0);
			}
			//check
			TS_ASSERT((int)ht.size() == n);
			for(int i = 0; i < n; ++i){
				hash_type::value_type val;
				TS_ASSERT(ht.get(i, &val) == bsl::HASH_EXIST);
				TS_ASSERT(val == 0);
			}
			int pn = 50;
			std::vector<pthread_t> pids(pn);
			for(int i = 0; i < pn; ++i){
				TS_ASSERT(pthread_create(&pids[i], NULL, update_func<hash_type>, &ht) == 0);
			}
			for(int i = 0; i < pn; ++i){
				pthread_join(pids[i], NULL);
			}
			//check
			TS_ASSERT((int)ht.size() == n);
			for(int i = 0; i < n; ++i){
				hash_type::value_type val;
				TS_ASSERT(ht.get(i, &val) == bsl::HASH_EXIST);
				TS_ASSERT(val == pn * 100 * 2);
			}
		}
		return;
	}
};

#endif  //__BSL_TEST_PHASHMAP_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
