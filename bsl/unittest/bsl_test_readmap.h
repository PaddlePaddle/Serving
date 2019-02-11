/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_test_readmap.h,v 1.5 2008/12/15 09:57:00 xiaowei Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file bsl_test_fun.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2008/08/14 17:44:12
 * @version $Revision: 1.5 $ 
 * @brief 
 *  
 **/


 #ifndef  __BSL_TEST_FUN_
 #define  __BSL_TEST_FUN_
 #include <cxxtest/TestSuite.h>
#include <string>
#include <bsl/containers/hash/bsl_readmap.h>
#include <bsl/exception/bsl_exception.h>
#include <bsl/archive/bsl_binarchive.h>
#include <bsl/archive/bsl_filestream.h>
#include <bsl/archive/bsl_serialization.h>
#include "test_alloc.h.ini"
#include <stdlib.h>
#include <time.h>
#include <map>

char s[][32]={"yingxiang", "yufan", "wangjiping", "xiaowei", "yujianjia", "maran", "baonenghui",
	"gonglei", "wangyue", "changxinglong", "chenxiaoming", "guoxingrong", "kuangyuheng"	
};
char t[][128]={"AAAA", "qsbbs", "yufa", "yingxian", "f*ck", "hello world!", "baidu"};

const int N = sizeof(s) / sizeof(s[0]);
const int M = sizeof(t) / sizeof(t[0]);

class bsl_test_fun : public CxxTest::TestSuite
{
	public:
	typedef std::string key;
	typedef int value;
	std::map <key, value> mp;
	public:
		class hash_func{
		public:
			static const int BASE = 33;
			size_t operator()(const std::string& s) const {
				size_t i, t = 0;
				for(i = 0; i < s.size(); i++){
					t *= BASE;
					t += s[i];
				}
				return t;
			}
		};
		void test_operator() {
			{
				bsl::readmap<key,value> rmp;
				init();
				rmp.assign(mp.begin(),mp.end());
				bsl::readmap<key,value> rmp2;
				rmp2 = rmp;
				TS_ASSERT(rmp2.size() == rmp.size());
				TS_ASSERT(rmp.size() == mp.size());
			}
			{
				std::map<key,value> mp;
				bsl::readmap<key,value> rmp;
				rmp.assign(mp.begin(),mp.end());
			}
			{
				bsl::readmap<key,value> rmp;
				init();
				rmp.assign(mp.begin(),mp.end());
				bsl::readmap<key,value> rmp2(rmp);
				TS_ASSERT(rmp2.size() == rmp.size());
				TS_ASSERT(rmp.size() == mp.size());
			}
			{
				bsl::readmap<key,value,hash_func> rmp;
				init();
				rmp.assign(mp.begin(),mp.end());
				int i,v,v2;
				bsl::readmap<key,value,hash_func> rmp2(rmp);
				for (i = 0; i < N; i ++) {
					rmp.get(key(s[i]),&v);
					rmp2.get(key(s[i]),&v2);
					TS_ASSERT(v == v2);
				}
			}
			{
				try {
					bsl::readmap<key,value> rmp(0);
					bsl::readmap<key,value> rmp2(rmp);
				} catch (bsl::Exception& e) {
					printf("\n==\n%s\n==\n",e.all());
				}
			}

		}
		void test_create() {
			bsl::readmap<key,value> rmp;
			for (int i = 0; i < 100; i ++) {
				rmp.create();
			}
			bsl::readmap<key,value> rmp2;
			for (int i = 0; i < 10; i ++) {
				rmp = rmp2;
			}
		}

		void test_normal_1(void) 
		{   
			int val = 0;
			TSM_ASSERT(val, val == 0);
		}

		void test_rm1(void){
			bsl::readmap<std::string, int> rmp;
			int ret = rmp.create();
			TSM_ASSERT(ret, ret ==0);
		}

		void test_rm2(void){
			bsl::readmap<std::string, int, hash_func> rmp;
			int ret = rmp.create();
			TSM_ASSERT(ret, ret == 0);
		}

		void test_rm3(void){
			bsl::readmap<key, value, hash_func> rmp;
			int ret = rmp.create();
			TSM_ASSERT(ret, ret == 0);
			init();
			ret = rmp.assign(mp.begin(), mp.end());
			TSM_ASSERT(ret, ret == 0);
			TSM_ASSERT(ret, rmp.size() == mp.size());
		}

		void test_rm4(void){
			bsl::readmap<key, value, hash_func> rmp;
			int ret = rmp.create();
			TSM_ASSERT(ret, ret == 0);
			init();
			ret = rmp.assign(mp.begin(), mp.end());
			int i;
			for(i = 0; i < 100; i++){
				key k = rnd_item();
				value v;
				ret = rmp.get(k, &v);

				if(mp.find(k) != mp.end()){
					TSM_ASSERT(ret, ret == bsl::HASH_EXIST);
					TSM_ASSERT(v, v == mp[k]);
				}
				else{
					TSM_ASSERT(ret, ret = bsl::HASH_NOEXIST);
				}
			}
		}   

		void test_rm5(void){
			bsl::readmap<key, value, hash_func> rmp;
			int ret = rmp.create();
			TSM_ASSERT(ret, ret == 0);
			init();
			ret = rmp.assign(mp.begin(), mp.end());
			int i, v;
			for(i = 0; i < N; i++){
				ret = rmp.get(key(s[i]), &v);
				TSM_ASSERT(v, v == i);
				TSM_ASSERT(ret, ret = bsl::HASH_EXIST);
			}
		}

		void test_rm6(void){
			bsl::readmap<key, value, hash_func> rmp;
			int ret = rmp.create();
			TSM_ASSERT(ret, ret == 0);
			init();
			ret = rmp.assign(mp.begin(), mp.end());
			int i, v = -1;
			for(i = 0; i < M; i++){
				ret = rmp.get(key(t[i]), &v);
				TSM_ASSERT(v, v == -1);
				TSM_ASSERT(ret, ret = bsl::HASH_NOEXIST);
			}
		}

		void test_rm7(void){
			bsl::readmap<key, key, hash_func> rmp;
			int ret = rmp.create(12345);
			TSM_ASSERT(ret, ret == 0);
			std::pair<key, key> p[N];
			int i;
			key k;
			for(i = 0; i < N; i++) {
				p[i].first = key(s[i]);
				p[i].second = p[i].first;
			}

			ret = rmp.assign(p, p+N);
			TSM_ASSERT(ret, ret == 0);
			for(i = 0; i < N; i++){
				ret = rmp.get(key(s[i]), &k);
				TSM_ASSERT(k, k == key(s[i]));
			}
#if 1
			TS_ASSERT(rmp.size() == (size_t)N);

			bsl::readmap<key, key, hash_func> rmp2;
			TS_ASSERT(rmp2.create() == 0);
			TS_ASSERT(rmp2.assign(rmp.begin(), rmp.end()) == 0);
			TS_ASSERT(rmp2.size() == (size_t)N);
			rmp.clear();
			TS_ASSERT(rmp.size() == 0);
			TS_ASSERT(rmp.bucket_size() == 12345);
			rmp.destroy();
			TS_ASSERT(rmp.bucket_size() == 0);
#endif
			//bsl::destruct(p, p+N);

		}
#if 1
		void test_rm8(void){
			bsl::readmap<key, key, hash_func> rmp;
			int ret = rmp.create();
			TSM_ASSERT(ret, ret == 0);
			std::pair<key, key> p[N];
			int i;
			key k;
			for(i = 0; i < N; i++) {
				p[i].first = key(s[i]);
				p[i].second = p[i].first;
			}   
			ret = rmp.assign(p, p+N);
			TSM_ASSERT(ret, ret == 0);
			for(i = 0; i < N; i++){
				ret = rmp.get(key(s[i]), &k);
				TSM_ASSERT(k, k == key(s[i]));
			}

			bsl::filestream fs;
			ret = fs.open("hash.dat", "w");
			TSM_ASSERT(ret, ret == 0);
			bsl::binarchive ar(fs);
			ret = ar.write(rmp);
			TSM_ASSERT(ret, ret == 0);
			fs.close();
		}

		void test_rm9(){
			test_rm8();
			bsl::readmap<key, key, hash_func> rmp;
			int ret = rmp.create();
			TSM_ASSERT(ret, ret == 0);

			bsl::filestream fs;
			ret = fs.open("hash.dat", "r");
			TSM_ASSERT(ret, ret == 0);

			bsl::binarchive ar(fs);
			ret = ar.read(&rmp);
			TSM_ASSERT(ret, ret == 0);
			fs.close()
				;
			int i;
			key k;
			for(i = 0; i < N; i++){
				ret = rmp.get(key(s[i]), &k);
				TSM_ASSERT(k, k == key(s[i]));
			}
			ret = get_const(&rmp);
			TSM_ASSERT(ret, ret == 0);
		}

		void test_rm10(){
			test_rm8();
			bsl::readmap<key, key, hash_func > rmp;
			int ret = rmp.create();
			FILE * fp = fopen("failtest", "w");
			fclose(fp);
			TSM_ASSERT(ret, ret == 0);

			bsl::filestream fs; 
			ret = fs.open("failtest", "r");
			TSM_ASSERT(ret, ret == 0); 

			bsl::binarchive ar(fs);
			ret = ar.read(&rmp);
			TSM_ASSERT(ret, ret != 0); 
			fs.close();
		} 

#endif
	private:
		key rnd_item(){
			srand(time(NULL));
			return (key)(s[rand() % N]);
		}
		void init(){
			int i;
			mp.clear();
			for(i = 0; i < N; i++){
				mp[(key)s[i]] = i;
			}
		}
		int get_const(const bsl::readmap<key,key,hash_func> *bmp){
			for(int i = 0; i < N; i++){
				key k;
				int ret = bmp->get(key(s[i]), &k);
				if(k != key(s[i]) || ret != bsl::HASH_EXIST) return -1;
			}
			return 0;
		}
};


#endif
















/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
