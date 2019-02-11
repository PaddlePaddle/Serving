#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <deque>
#include <vector>
#include <algorithm>

#include <bsl/containers/deque/bsl_deque.h>
#include <bsl/containers/string/bsl_string.h>
#include <cxxtest/TestSuite.h>

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
    char *u;
    MyClass() {
        u = (char*)malloc(256);
        memset(u, 0, 128);
    }
    ~MyClass() {
        free(u);
    }
    MyClass(const MyClass &__x) {
        memcpy(name, __x.name, sizeof(name));
        memcpy(value, __x.value, sizeof(value));
        u = (char*)malloc(256);
        memcpy(u, __x.u, 256);
    }
};

class bsl_test_deque : public CxxTest::TestSuite
{
public:
    void test_operator() 
    {
        {
            bsl::deque<int> t1;
            for (int i = 0; i < 100; i ++) {
                t1.push_back(i);
            }
            TS_ASSERT( 100 == t1.size() );
            bsl::deque<int> t2;
            t2 = t1;
            TS_ASSERT( 100 == t2.size() );
            int i = 0;
            for (bsl::deque<int>::iterator Iter = t2.begin(); Iter != t2.end(); ++ Iter) {
                TS_ASSERT( *Iter == i ++ );
            }
            i = 0;
            t1 = t1;
            TS_ASSERT( 100 == t1.size() );
            for (bsl::deque<int>::iterator Iter = t1.begin(); Iter != t1.end(); ++ Iter) {
                TS_ASSERT( *Iter == i ++ );
            }
            bsl::deque<int> t3;
            t3.push_back(1);
            TS_ASSERT( 1 == *t3.begin() );
            bsl::deque<int> t5;
            bsl::deque<int> t4;
            t4 = t3;
            t3.pop_back();
            t3 = t5;
            TS_ASSERT( 0 == t3.size() );
            t1 = t3;
            TS_ASSERT( 0 == t1.size() );
            TS_ASSERT( 0 == t3.size() );
        }
        {
            bsl::deque<bsl::string> l1;
            bsl::deque<bsl::string> l2;
            for (int i = 0; i < 1000; i ++) {
                l1.push_front( "hello world" );
            }
            TS_ASSERT( 1000 == l1.size() );
            for (int i = 0; i < 100; i ++) {
                l2 = l1;
            }
            TS_ASSERT( 1000 == l2.size() );
        }
        {
            bsl::deque<int> l1;
            for (int i = 0; i < 100; i ++) {
                l1.push_back(i);
            }
            TS_ASSERT( 100 == l1.size() );
            bsl::deque<int> l2; 
            for (int i = 10; i > 0; i --) {
                l2.push_back(i);
            }
            l2 = l1;
            TS_ASSERT( 100 == l2.size() );
            TS_ASSERT( 100 == l1.size() );
            int i = 0;
            for (bsl::deque<int>::iterator iter = l2.begin(); iter != l2.end(); ++ iter, i ++) {
                TS_ASSERT( *iter == i );
            }
        }
    }
    void test_create()
    {
        bsl::deque<int> l1;
        int i;
        for (i = 0; i < 10000; i ++) {
            l1.create();
        }
        for (i = 0; i < 10000; i ++) {
            l1.create();
        }
        bsl::deque<int> l2;
        for (i = 0; i < 10000; i ++) {
            l2.create();
        }
        for (i = 0; i < 10; i ++) {
            l1.push_back(i);
        }
        TS_ASSERT( 10 == l1.size() );
        for (i = 0; i < 10; i ++) {
            TS_ASSERT( i == l1[i] );
        }
    }

    void test_nocreate_0()
    {
        bsl::deque<int> t0;
        int x;
        TS_ASSERT_EQUALS(t0.resize(100), 0);
        TS_ASSERT_EQUALS(t0.resize(100,300), 0);
        TS_ASSERT_EQUALS(t0.assign(56,23), 0);
        TS_ASSERT_EQUALS(t0.push_back(30), 0);
        TS_ASSERT_EQUALS(t0.pop_back(), 0);
        TS_ASSERT_EQUALS(t0.push_front(30), 0);
        TS_ASSERT_EQUALS(t0.pop_front(), 0);
        TS_ASSERT_EQUALS(t0.clear(), 0);
        TS_ASSERT_EQUALS(t0.set(23, 34), -1);
        TS_ASSERT_EQUALS(t0.get(21, &x), -1);
    }

    void test_create_0()
    {
        bsl::deque<int> t0;
        TS_ASSERT(t0.destroy() == 0);
    }

    void test_create_1()
    {
        bsl::deque<int> t1;
        t1.create();
        t1.create();
        t1.create();
        TS_ASSERT(t1.destroy() == 0);
    }

    void test_create_2()
    {
        bsl::deque<int, 4096> t0;
        TS_ASSERT_EQUALS(t0.create(),0);
        TS_ASSERT_EQUALS(t0.create(),0);
        TS_ASSERT_EQUALS(t0.create(),0);
        TS_ASSERT_EQUALS(t0.destroy(),0);
        TS_ASSERT_EQUALS(t0.create(),0);
    }

    void test_set_get_0()
    {
        bsl::deque<int> t0;
        TS_ASSERT_EQUALS(t0.create(), 0);
        TS_ASSERT_EQUALS(t0.resize(300, 20),0);
        int x;
        TS_ASSERT_EQUALS(t0.get(5, &x), 0);
        TS_ASSERT_EQUALS(x, 20);
        TS_ASSERT_EQUALS(t0.set(5, 45), 0);
        TS_ASSERT_EQUALS(t0.get(5, &x), 0);
        TS_ASSERT_EQUALS(x, 45);
    }
    void test_resize_0()
    {
        bsl::deque<int> t0;
        TS_ASSERT_EQUALS(t0.create(), 0);
        t0.resize(1000, 98);
        TS_ASSERT_EQUALS(t0.size(), 1000);
        for (int i = 0; i < (int)t0.size(); ++i) {
            TS_ASSERT_EQUALS(t0[i], 98);
        }
    }

    void test_resize_1()
    {
        bsl::deque<MyClass> t0;
        TS_ASSERT_EQUALS(t0.create(), 0);
        t0.resize(1000);
        TS_ASSERT_EQUALS(t0.size(), 1000);
    }

	void test_push_front_0()
	{
		bsl::deque<int> t0;
		TS_ASSERT(t0.create() == 0);
        int n = 5000;
		for(int i=0; i < n; i++) {
            int ret = t0.push_front(i);
            TS_ASSERT_EQUALS(ret, 0);
            TS_ASSERT(t0.front() == i);
		}

        int v = n - 1;
        for (bsl::deque<int>::iterator iter = t0.begin();
                iter != t0.end();
                ++iter) {
           TS_ASSERT(*iter == v);
            --v;
        }
        v = n - 1;

        for (bsl::deque<int>::const_iterator iter = t0.begin();
                iter != t0.end();
                ++iter) {
            TS_ASSERT(*iter == v);
            --v;
        }
        v = 0;

        for (bsl::deque<int>::reverse_iterator iter = t0.rbegin();
                iter != t0.rend();
                ++iter) {
            TS_ASSERT(*iter == v);
            ++v;
        }
       

        /*
        v = n - 1;

        for (bsl::deque<int>::const_reverse_iterator iter = t0.rbegin();
                iter != t0.rend();
                ++iter) {
            TS_ASSERT(*iter == v);
            v++;
        }
        */


		TS_ASSERT_EQUALS(t0.size() , n);
		TS_ASSERT(t0.destroy() == 0);
	}

    void test_push_front_1()
    {
        bsl::deque<std::string> t1;
        std::deque<std::string> dt1;
        TS_ASSERT(t1.create() == 0);
        int n = 5000;
        for (int i = 0; i < n; ++i) {
            std::string str(rand_string());
            int ret = t1.push_front(str);
            TS_ASSERT_EQUALS(ret, 0);
            TS_ASSERT(t1.front() == str);
            dt1.push_front(str);
        }

        std::deque<std::string>::iterator dt_iter = dt1.begin();
        for (bsl::deque<std::string>::iterator iter = t1.begin();
                iter != t1.end();
                ++iter, ++dt_iter) {
            TS_ASSERT(*iter == *dt_iter);
        }

        std::deque<std::string>::const_iterator dt_iter1 = dt1.begin();
        for (bsl::deque<std::string>::const_iterator iter = t1.begin();
                iter != t1.end();
                ++iter, ++dt_iter1) {
            TS_ASSERT(*iter == *dt_iter1);
        }


        TS_ASSERT_EQUALS(t1.size() , n);
        TS_ASSERT(t1.destroy() == 0);
    }

 
    void test_push_front_2()
    {
        bsl::deque<MyClass> t1;
        std::deque<MyClass> dt1;
        TS_ASSERT(t1.create() == 0);
        int n = 5000;
        for (int i = 0; i < n; ++i) {
            MyClass tmp;
            snprintf(tmp.name, sizeof(tmp.name), "%s", rand_string().c_str());
            snprintf(tmp.value, sizeof(tmp.value), "%s", rand_string().c_str());
            int ret = t1.push_front(tmp);
            TS_ASSERT_EQUALS(ret, 0);
            TS_ASSERT(t1.front() == tmp);
            dt1.push_front(tmp);
        }

        std::deque<MyClass>::iterator dt_iter = dt1.begin();
        for (bsl::deque<MyClass>::iterator iter = t1.begin();
                iter != t1.end();
                ++iter, ++dt_iter) {
            TS_ASSERT(*iter == *dt_iter);
        }

        std::deque<MyClass>::const_iterator dt_iter1 = dt1.begin();
        for (bsl::deque<MyClass>::const_iterator iter = t1.begin();
                iter != t1.end();
                ++iter, ++dt_iter1) {
            TS_ASSERT(*iter == *dt_iter1);
        }


        TS_ASSERT_EQUALS(t1.size() , n);
        TS_ASSERT(t1.destroy() == 0);
    }

       
	void test_push_back_0()
	{
		bsl::deque<int> t0;
		TS_ASSERT(t0.create() == 0);
        int n = 5000;
		for(int i=0; i < n; i++) {
            int ret = t0.push_back(i);
            TS_ASSERT_EQUALS(ret, 0);
            TS_ASSERT(t0.back() == i);
		}
        int v = 0;
        for (bsl::deque<int>::iterator iter = t0.begin();
                iter != t0.end();
                ++iter) {
           TS_ASSERT(*iter == v);
            ++v;
        }
        v = 0;

        for (bsl::deque<int>::const_iterator iter = t0.begin();
                iter != t0.end();
                ++iter) {
            TS_ASSERT(*iter == v);
            ++v;
        }
        v = n - 1;

        for (bsl::deque<int>::reverse_iterator iter = t0.rbegin();
                iter != t0.rend();
                ++iter) {
            TS_ASSERT(*iter == v);
            --v;
        }
       
		TS_ASSERT_EQUALS(t0.size() , n);
		TS_ASSERT(!t0.destroy());
	}

    void test_push_back_1()
    {
        bsl::deque<std::string> t1;
        std::deque<std::string> dt1;
        TS_ASSERT(t1.create() == 0);
        int n = 5000;
        for (int i = 0; i < n; ++i) {
            std::string str(rand_string());
            int ret = t1.push_back(str);
            TS_ASSERT_EQUALS(ret, 0);
            TS_ASSERT(t1.back() == str);
            dt1.push_back(str);
        }
        std::deque<std::string>::iterator dt_iter = dt1.begin();
        for (bsl::deque<std::string>::iterator iter = t1.begin();
                iter != t1.end();
                ++iter, ++dt_iter) {
            TS_ASSERT(*iter == *dt_iter);
        }

        std::deque<std::string>::const_iterator dt_iter1 = dt1.begin();
        for (bsl::deque<std::string>::const_iterator iter = t1.begin();
                iter != t1.end();
                ++iter, ++dt_iter1) {
            TS_ASSERT(*iter == *dt_iter1);
        }

        TS_ASSERT_EQUALS(t1.size() , n);
        TS_ASSERT(t1.destroy() == 0);
    }

    void test_push_back_2()
    {
        bsl::deque<MyClass> t1;
        std::deque<MyClass> dt1;
        TS_ASSERT(t1.create() == 0);
        int n = 5000;
        for (int i = 0; i < n; ++i) {
            MyClass tmp;
            snprintf(tmp.name, sizeof(tmp.name), "%s", rand_string().c_str());
            snprintf(tmp.value, sizeof(tmp.value), "%s", rand_string().c_str());
            int ret = t1.push_front(tmp);
            TS_ASSERT_EQUALS(ret, 0);
            TS_ASSERT(t1.front() == tmp);
            dt1.push_front(tmp);
        }

        std::deque<MyClass>::iterator dt_iter = dt1.begin();
        for (bsl::deque<MyClass>::iterator iter = t1.begin();
                iter != t1.end();
                ++iter, ++dt_iter) {
            TS_ASSERT(*iter == *dt_iter);
        }

        std::deque<MyClass>::const_iterator dt_iter1 = dt1.begin();
        for (bsl::deque<MyClass>::const_iterator iter = t1.begin();
                iter != t1.end();
                ++iter, ++dt_iter1) {
            TS_ASSERT(*iter == *dt_iter1);
        }


        TS_ASSERT_EQUALS(t1.size() , n);
        TS_ASSERT(t1.destroy() == 0);
    }


    void test_push_pop_0()
    {
        bsl::deque<int> t1;
        std::deque<int> dt1;
        int n = 5000;
        //int mpush = 0, mpop = 0;
        TS_ASSERT(t1.create() == 0);
        for (int i = 0; i < n; ++i) {
            int x = rand();
            switch (rand()%4) {
                case 0: TS_ASSERT(t1.push_back(x) == 0);
                        dt1.push_back(x);
                        break;
                case 1: 
                        if (t1.size() >0) {
                            TS_ASSERT(t1.pop_front() == 0);
                            dt1.pop_front();
                            break;
                        }
                case 2: TS_ASSERT(t1.push_front(x) == 0);
                        dt1.push_front(x);
                        break;

                case 3: 
                        if (t1.size() > 0) {
                            TS_ASSERT(t1.pop_front() == 0);
                            dt1.pop_front();
                        }
                        break;
                default:
                        break;
            }
            
            TS_ASSERT_EQUALS(t1.size(), dt1.size());
            if (t1.size() != 0) {
                TS_ASSERT_EQUALS(t1.back(), dt1.back());
                TS_ASSERT_EQUALS(t1.front(), dt1.front());
            }
        }
        TS_ASSERT(t1.destroy() == 0);
    }

    void test_rand_0()
    {
        bsl::deque<int> t1;
        std::deque<int> dt1;
        TS_ASSERT(t1.create() == 0);
        int n = 5000;
        for (int i = 0; i < n; ++i) {
            int x = rand();
            t1.push_back(x);
            dt1.push_back(x);
        }
        for (int i = 0; i < n; ++i) {
            int x = rand()%n;
            TS_ASSERT_EQUALS(t1[x], dt1[x]);
        }
        TS_ASSERT(t1.destroy() == 0);
    }

    void test_rand_1()
    {
        bsl::deque<std::string> t1;
        std::deque<std::string> dt1;
        TS_ASSERT(t1.create() == 0);
        int n = 5000;
        for (int i = 0; i < n; ++i) {
            std::string str(rand_string());
            t1.push_back(str);
            dt1.push_back(str);
        }
        for (int i = 0; i < n; ++i) {
            int x = rand()%n;
            TS_ASSERT_EQUALS(t1[x], dt1[x]);
        }
        TS_ASSERT(t1.destroy() == 0);
    }


    void test_rand_2()
    {
        bsl::deque<MyClass> t1;
        std::deque<MyClass> dt1;
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
            TS_ASSERT_EQUALS(t1[x], dt1[x]);
        }
        TS_ASSERT(t1.destroy() == 0);
    }

    void test_clear_0()
    {
        bsl::deque<int> t1;
        std::deque<int> dt1;
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
        TS_ASSERT(t1.size() == dt1.size());

        for (int i = 0; i < n; ++i) {
            int x = rand()%n;
            t1.push_back(x);
            dt1.push_back(x);
        }
        TS_ASSERT(t1.size() == dt1.size());
        TS_ASSERT(t1.destroy() == 0);
    }

    void test_ceate_0()
    {
        bsl::deque<int> t1;
        TS_ASSERT(t1.create() == 0);
        int n = 5000;
        for (int i = 0; i < n; ++i) {
            t1.push_back(i);
        }
        TS_ASSERT(t1.create() == 0);
        TS_ASSERT(t1.size() == 0);
    }

    void test_swap_0()
    {
        bsl::deque<int> t1;
        bsl::deque<int> t2;

        TS_ASSERT(t1.create() == 0);
        TS_ASSERT(t2.create() == 0);
        for (int i = 0; i < 100; ++i) {
            TS_ASSERT(t1.push_back(i) == 0);
            TS_ASSERT(t2.push_front(i) == 0);
        }
        bsl::swap(t1, t2);
        for (int i = 0; i < 100; ++i) {
            TS_ASSERT(t2[i]==i);
            TS_ASSERT(t1[i]==(100-i-1)); 
        }

        for (int i = 0; i < 100; ++i) {
            TS_ASSERT(t1.push_back(i*2) == 0);
        }

        for (int i = 0; i < 100; ++i) {
            TS_ASSERT(t1[i+100] == i*2);
        }

        for (int i = 0; i < 200; ++i) {
            TS_ASSERT(t1.pop_back() == 0);
        }

    }

    void test_assign_0()
    {
        bsl::deque<MyClass> t1;
        MyClass tmp;
        TS_ASSERT_EQUALS(t1.create(), 0);
        snprintf(tmp.name, sizeof(tmp.name), "%s", rand_string().c_str()); 
        snprintf(tmp.value, sizeof(tmp.value), "%s", rand_string().c_str());
        TS_ASSERT_EQUALS(t1.assign(5000, tmp), 0);
        TS_ASSERT_EQUALS(t1.size(),5000);
        for(int i = 0; i < 5000; ++i) {
            TS_ASSERT(t1[i] == tmp);
        }
        for (int i = 0; i < 5000; ++i) {
            t1.pop_back();
        }
        TS_ASSERT_EQUALS(t1.size(), 0);
    }

    void test_assign_1()
    {
        bsl::deque<MyClass> t1;
        std::vector<MyClass> t2;
        for (int i = 0; i < 5000; ++i) {
            MyClass tmp;
            snprintf(tmp.name, sizeof(tmp.name), "%s", rand_string().c_str()); 
            snprintf(tmp.value, sizeof(tmp.value), "%s", rand_string().c_str());       
            t2.push_back(tmp);
        }
        t1.create();
        t1.assign(t2.begin(), t2.end());
        TS_ASSERT_EQUALS(t1.size(), t2.size());
        for (int i = 0; i < 5000; ++i) {
            TS_ASSERT(t1[i] == t2[i]);
        }
    }

    void test_assign_2()
    {
        bsl::deque<int> t1;
        TS_ASSERT_EQUALS(t1.create(), 0);
        TS_ASSERT_EQUALS(t1.assign(5000, 30), 0);
        for (int i = 0; i < 5000; ++i) {
            TS_ASSERT_EQUALS(t1[i], 30);
        }
    }

    void test_iter_0()
    {
        bsl::deque<int> t1;
        t1.create();
        for (int i = 0; i < 5000; ++i) {
            t1.push_back(i);
        }
        bsl::deque<int>::iterator iter;
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


        bsl::deque<int, 2048>::iterator iter2;
        bsl::deque<int, 2048> t2;
        t2.create();
        TS_ASSERT_EQUALS(t2.push_back(30), 0);
        TS_ASSERT_EQUALS(t2.push_back(900), 0);
        iter2 = t2.begin();

        bsl::deque<int, 2048>::iterator iter3;
        iter3 = iter2+1; 
        TS_ASSERT(!(iter2 == iter3));
        TS_ASSERT(iter2 != iter3);
        TS_ASSERT(iter2 < iter3);
        TS_ASSERT(iter2 <= iter3);
        TS_ASSERT(iter3 >= iter2);
        TS_ASSERT(iter3 > iter2);


        bsl::deque<int, 2048>::const_iterator iter4;
        iter4 = t2.begin();
        TS_ASSERT(iter4 < iter3);
        TS_ASSERT(iter4 != iter3);
        TS_ASSERT(iter4 == iter2);
        TS_ASSERT(iter4 >= iter2);
        TS_ASSERT(iter2 <= iter4);
        TS_ASSERT(iter4 < iter3);
        TS_ASSERT(iter3 > iter4);

        TS_ASSERT(iter3-iter4 == 1);
        TS_ASSERT(iter4+1 == iter3);
        
    }

    void test_stl_alg_0()
    {
        //swap²»Ö§³Ö
        bsl::deque<int> t1;
        t1.create();
        int n = 5000;
        for (int i = 0; i < n; ++i) {
            t1.push_front(i);
        }
        //sort
        std::sort(t1.begin(), t1.end());
        for (int i = 0; i < n; ++i) {
            TS_ASSERT(t1[i] == i);
        }


        std::vector<int> m(n);;
        //copy
        std::copy(t1.begin(), t1.end(), m.begin());
        for (int i = 0; i < n; ++i) {
            TS_ASSERT(t1[i] == i);
            TS_ASSERT(t1[i] == m[i]);
        }

        bsl::deque<int> t2;
        t2.create();
        t2.assign(t1.begin(), t1.end());

        std::copy(t1.begin(), t1.end(), t2.begin());
        for (int i = 0; i < n; ++i) {
            TS_ASSERT(t1[i] == i);
            TS_ASSERT(t1[i] == t2[i]);
        }

        //find
       TS_ASSERT(*(std::find(t1.begin(), t1.end(), 30)) == 30);

       //count
       TS_ASSERT(std::count(t1.begin(), t1.end(), 20) == 1);
    }

    void test_serialization_0()
    {   
        bsl::deque<int> t1;

        TS_ASSERT(t1.create() == 0);
        for (int i = 0; i< 5000; ++i) {   
            TS_ASSERT(t1.push_back(i) == 0);
        }   
        bsl::filestream fd; 
        fd.open("data", "w");
        bsl::binarchive ar(fd);
        TS_ASSERT(ar.write(t1) == 0); 
        fd.close();

        t1.clear();
        TS_ASSERT(t1.size() == 0); 

        fd.open("data", "r");
        TS_ASSERT(ar.read(& t1) == 0); 
        fd.close();
        TS_ASSERT(t1.size() == 5000);
        for (int i = 0; i < 5000; ++i) {
            TS_ASSERT(i == t1[i]);
        }
    }   



};
