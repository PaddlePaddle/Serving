#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <vector>
#include <algorithm>

#include <bsl/containers/deque/bsl_rwseque.h>
#include <cxxtest/TestSuite.h>

#include <bsl/containers/string/bsl_string.h>
#include <bsl/archive/bsl_serialization.h>
#include <bsl/archive/bsl_filestream.h>
#include <bsl/archive/bsl_binarchive.h>

std::string rand_string()
{
    std::string str;
    srand(time(NULL));
    int n = rand()%100;
    for (int i = 0; i < n; ++i) {
        str += 'a' + rand()%26;
    }
    return str;
}

struct MyClass {
    public:
    char name[800];
    char value[800];
    bool operator == (const MyClass & __x) const {
        return ((strcmp(name, __x.name) == 0)&&
                (strcmp(value, __x.value) == 0));
    }
    const MyClass & operator = (const MyClass &__x) {
        memcpy(name, __x.name, sizeof(name));
        memcpy(value, __x.value, sizeof(value));
        memcpy(u, __x.u, 256);
        return *this;
    }
    char u[256];
    MyClass() {
        memset(u, 0, 256);
    }
    ~MyClass() {
    }
    MyClass(const MyClass &__x) {
        memcpy(name, __x.name, sizeof(name));
        memcpy(value, __x.value, sizeof(value));
        memcpy(u, __x.u, 256);
    }
};

class bsl_test_rwseque : public CxxTest::TestSuite
{
public:
    void test_operator() 
    {
        {
            bsl::rwseque<bsl::string> l1;
            TS_ASSERT(0 == l1.create());
            for (int i = 0; i < 1000; i ++) {
                l1.push_back( "hello world" );
            }
            TS_ASSERT( 1000 == l1.size() );
        }
        {
            bsl::rwseque<int> l1;
            TS_ASSERT(0 == l1.create());
            for (int i = 0; i < 100; i ++) {
                l1.push_back(i);
            }
            TS_ASSERT( 100 == l1.size() );
            int i = 0;
            for (bsl::rwseque<int>::iterator iter = l1.begin(); iter != l1.end(); ++ iter, i ++) {
                TS_ASSERT( *iter == i );
            }
        }
    }
    void test_create()
    {
        bsl::rwseque<int> l1;
        int i;
        for (i = 0; i < 10000; i ++) {
            l1.create();
        }
        for (i = 0; i < 10000; i ++) {
            l1.create();
        }
        bsl::rwseque<int> l2;
        for (i = 0; i < 10000; i ++) {
            l2.create();
        }
        for (i = 0; i < 10; i ++) {
            l1.push_back(i);
        }
        TS_ASSERT( 10 == l1.size() );
    }
    void test_create_0()
    {
        bsl::rwseque<int> t0;
        TS_ASSERT(t0.destroy() == 0);
    }

    void test_create_1()
    {
        bsl::rwseque<int> t1;
        t1.create();
        t1.create();
        t1.create();
        TS_ASSERT(t1.destroy() == 0);
    }

	void test_push_back_0()
	{
		bsl::rwseque<int> t0;
		TS_ASSERT(t0.create() == 0);
        int n = 5000;
		for(int i=0; i < n; ++i) {
            int ret = t0.push_back(i);
            TS_ASSERT_EQUALS(ret, 0);
		}
        for (int i=0; i < n; ++i) {
            int s;
            TS_ASSERT(t0.get(i, &s) == 0);
            TS_ASSERT(s==i);
        }
       
		TS_ASSERT_EQUALS(t0.size() , n);
		TS_ASSERT(!t0.destroy());
	}

    void test_push_back_1()
    {
        bsl::rwseque<std::string> t1;
        std::vector<std::string> dt1;
        TS_ASSERT(t1.create() == 0);
        int n = 5000;
        for (int i = 0; i < n; ++i) {
            std::string str(rand_string());
            int ret = t1.push_back(str);
            TS_ASSERT_EQUALS(ret, 0);
            dt1.push_back(str);
        }
        for (int i = 0; i < n; ++i) {
            std::string str;
            TS_ASSERT(t1.get(i, &str)==0);
            TS_ASSERT(str == dt1[i]);
        }

        TS_ASSERT_EQUALS(t1.size() , n);
        TS_ASSERT(t1.destroy() == 0);
    }

    void test_push_back_2()
    {
        bsl::rwseque<MyClass> t1;
        std::vector<MyClass> dt1;
        TS_ASSERT(t1.create() == 0);
        int n = 5000;
        for (int i = 0; i < n; ++i) {
            MyClass tmp;
            snprintf(tmp.name, sizeof(tmp.name), "%s", rand_string().c_str());
            snprintf(tmp.value, sizeof(tmp.value), "%s", rand_string().c_str());
            int ret = t1.push_back(tmp);
            TS_ASSERT_EQUALS(ret, 0);
            dt1.push_back(tmp);
        }
        for (int i = 0; i < n; ++i) {
            MyClass tmp;
            TS_ASSERT(t1.get(i, &tmp)==0);
            TS_ASSERT(tmp == dt1[i]);
        }


        TS_ASSERT_EQUALS(t1.size() , n);
        TS_ASSERT(t1.destroy() == 0);
    }


    void test_push_pop_back_0()
    {
        bsl::rwseque<int> t1;
        std::vector<int> dt1;
        int n = 1000;
        //int mpush = 0, mpop_back = 0;
        TS_ASSERT(t1.create() == 0);
        for (int i = 0; i < n; ++i) {
            int x = rand();
            switch (rand()%2) {
                case 0: TS_ASSERT(t1.push_back(x) == 0);
                        dt1.push_back(x);
                        break;

                case 1: 
                        if (t1.size() > 0) {
                            TS_ASSERT(t1.pop_back() == 0);
                            dt1.pop_back();
                        }
                        break;
                default:
                        break;
            }
            
            TS_ASSERT_EQUALS(t1.size(), dt1.size());
            if (t1.size() != 0) {
                for (int j = 0; j < (int)t1.size(); ++j) {
                    int s;
                    TS_ASSERT(t1.get((unsigned int)j, &s) == 0);
                    TS_ASSERT(s == dt1[j]);
                }
            }
        }
        TS_ASSERT(t1.destroy() == 0);
    }

    void test_get_0()
    {
        bsl::rwseque<int> t1;
        std::vector<int> dt1;
        TS_ASSERT(t1.create() == 0);
        int n = 5000;
        for (int i = 0; i < n; ++i) {
            int x = rand();
            t1.push_back(x);
            dt1.push_back(x);
        }
        for (int i = 0; i < n; ++i) {
            int x = rand()%n;
            int s;
            TS_ASSERT(t1.get(x, &s) == 0);
            TS_ASSERT_EQUALS(s, dt1[x]);
        }
        TS_ASSERT(t1.destroy() == 0);
    }

    void test_get_1()
    {
        bsl::rwseque<std::string> t1;
        std::vector<std::string> dt1;
        TS_ASSERT(t1.create() == 0);
        int n = 5000;
        for (int i = 0; i < n; ++i) {
            std::string str(rand_string());
            t1.push_back(str);
            dt1.push_back(str);
        }
        for (int i = 0; i < n; ++i) {
            int x = rand()%n;
            std::string str;
            TS_ASSERT(t1.get(x, &str)==0);
            TS_ASSERT(str == dt1[x]);
        }
        TS_ASSERT(t1.destroy() == 0);
    }


    void test_rand_2()
    {
        bsl::rwseque<MyClass> t1;
        std::vector<MyClass> dt1;
        TS_ASSERT(t1.create() == 0);
        int n = 5000;
        for (int i = 0; i < n; ++i) {
            MyClass s;
            snprintf(s.name, sizeof(s.name), "%s", rand_string().c_str());
            snprintf(s.value, sizeof(s.value), "%s", rand_string().c_str());
            t1.push_back(s);
            dt1.push_back(s);
        }
        for (int i = 0; i < n; ++i) {
            int x = rand()%n;
            MyClass tmp;
            TS_ASSERT_EQUALS(t1.get(x, &tmp), 0);
            TS_ASSERT_EQUALS(tmp, dt1[x]);
        }
        TS_ASSERT(t1.destroy() == 0);
    }

    void test_clear_0()
    {
        bsl::rwseque<int> t1;
        std::vector<int> dt1;
        TS_ASSERT(t1.create() == 0);
        int n = 5000;
        for (int i = 0; i < n; ++i) {
            int x = rand();
            t1.push_back(x);
            dt1.push_back(x);
        }
        t1.clear();
        t1.clear();
        t1.clear();
        dt1.clear();
        TS_ASSERT_EQUALS(t1.size(), dt1.size());

        for (int i = 0; i < n; ++i) {
            int x = rand()%n;
            t1.push_back(x);
            dt1.push_back(x);
        }
        TS_ASSERT_EQUALS(t1.size(), dt1.size());
        TS_ASSERT_EQUALS(t1.destroy(), 0);
    }

    void test_ceate_0()
    {
        bsl::rwseque<int> t1;
        TS_ASSERT(t1.create() == 0);
        int n = 5000;
        for (int i = 0; i < n; ++i) {
            t1.push_back(i);
        }
        TS_ASSERT(t1.create() == 0);
        TS_ASSERT(t1.size() == 0);
    }

    void test_assign_0()
    {
        bsl::rwseque<MyClass> t1;
        t1.create();
        MyClass tmp;
        snprintf(tmp.name, sizeof(tmp.name), "%s", rand_string().c_str()); 
        snprintf(tmp.value, sizeof(tmp.value), "%s", rand_string().c_str());
        t1.assign(5000, tmp);
        TS_ASSERT_EQUALS(t1.size(), 5000);
        for(int i = 0; i < 5000; ++i) {
            MyClass stmp;
            TS_ASSERT_EQUALS(t1.get(i, &stmp), 0);
            TS_ASSERT_EQUALS(tmp , stmp);
        }
        for (int i = 0; i < 5000; ++i) {
            t1.pop_back();
        }
        TS_ASSERT(t1.size() == 0);
    }

    void test_assign_1()
    {
        bsl::rwseque<MyClass> t1;
        t1.create();
        std::vector<MyClass> t2;
        for (int i = 0; i < 5000; ++i) {
            MyClass tmp;
            snprintf(tmp.name, sizeof(tmp.name), "%s", rand_string().c_str()); 
            snprintf(tmp.value, sizeof(tmp.value), "%s", rand_string().c_str());       
            t2.push_back(tmp);
        }
        t1.assign(t2.begin(), t2.end());
        TS_ASSERT(t1.size() == t2.size());
        for (int i = 0; i < 50; ++i) {
            MyClass tmp;
            TS_ASSERT(t1.get(i, &tmp) == 0);
            TS_ASSERT(tmp == t2[i]);
        }
    }

    void test_iter_0()
    {
        bsl::rwseque<int> t1;
        t1.create();
        for (int i = 0; i < 5000; ++i) {
            t1.push_back(i);
        }
        bsl::rwseque<int>::iterator iter;
        iter = t1.begin();
        for (int i = 1; i < 5000; ++i) {
            iter++;
            TS_ASSERT(*iter == i);
        }
        for (int i = 4999; i > 0; --i) {
            TS_ASSERT(*iter == i);
            iter--;
        }
        for (int i = 1; i < 5000; ++i) {
            ++iter;
            TS_ASSERT(*iter == i);
        }
        for (int i = 4999; i > 0; --i) {
            TS_ASSERT(*iter == i);
            --iter;
        }

        iter += 100;
        TS_ASSERT(*iter == 100);

        TS_ASSERT(*(iter+100) == 200);

        TS_ASSERT(*(iter-20) == 80);

        iter -= 80;
        TS_ASSERT(*iter == 20);

        TS_ASSERT(iter[100] == 120);


        bsl::rwseque<int, 2048>::iterator iter2;
        bsl::rwseque<int, 2048> t2;
        t2.create();
        TS_ASSERT_EQUALS(t2.push_back(30), 0);
        TS_ASSERT_EQUALS(t2.push_back(900), 0);
        iter2 = t2.begin();

        bsl::rwseque<int, 2048>::iterator iter3;
        iter3 = iter2+1; 
        TS_ASSERT(!(iter2 == iter3));
        TS_ASSERT(iter2 != iter3);
    }
};
