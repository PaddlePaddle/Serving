#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <vector>
#include <algorithm>

#include <bsl/containers/deque/bsl_rwseque.h>
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

std::vector<pthread_t> g_tid;

template <typename T>
class MyParam{
    public:
    bsl::rwseque<T> r;
    std::vector<T> data;
    T default_value;
};

const int MAX_SIZE = 1000;

template <typename T>
void* bsl_rwseque_push(void *arg)
{
    MyParam<T> *s = static_cast<MyParam<T>*>(arg);
    for (int i = 0; i < 1000; ++i) {
        for (int i = 0; i < MAX_SIZE; ++i) {
            s->r.push_back(s->data[i]); 
        }
        for (int i = 0;i < MAX_SIZE; ++i) {
            s->r.pop_back();
        }
    }
    return NULL;

}

template <typename T>
void* bsl_rwseque_set(void *arg)
{
    MyParam<T> *s = static_cast<MyParam<T>*>(arg);
    for (int i = 0; i < 1000; ++i) {
        for (int i = 0; i < MAX_SIZE; ++i) {
            int x = rand()%MAX_SIZE;
            s->r.set(x, s->data[x]);
        }
    }
    return NULL;

}

template <typename T>
void* bsl_rwseque_clear(void *arg)
{
    MyParam<T> *s = static_cast<MyParam<T>*>(arg);
    int a = 0, b = 0;
    for (int i = 0; i < 1000; ++i) {
//        size_t size = s->r.size();
        TS_ASSERT(s->r.clear() == 0);
        if (s->r.size()) {
            a++;
        } else {
            b++;
        }
    }
    return NULL;

}



template <typename T>
void* bsl_rwseque_get(void *arg)
{
    MyParam<T> *s = static_cast<MyParam<T>*>(arg);
    for (int i = 0; i < MAX_SIZE; ++i) {
        T tmp;
        int p = s->r.size()%MAX_SIZE;
        int ret = s->r.get(p, &tmp);
        if (ret == 0) {

            if (tmp == s->default_value) {
                continue;
            }
            if (s->data[p] != tmp) {
                TS_ASSERT(s->data[p] == tmp);
                for (int z = 0; z < (int)g_tid.size(); ++z) {
                    if (g_tid[z] != pthread_self()) {
                        pthread_cancel(g_tid[z]);
                    }
                }
                return NULL;
            } 

        }
    }
    return 0;
}

#define CREATE_PTHREAD(g_tid,func,num,var) \
    do { for (int i = 0; i < num; ++i) { \
        pthread_t tid; \
        pthread_create(&tid, NULL, func, &var);\
        g_tid.push_back(tid);} \
    }while(0);

#define PTHREAD_JOIN(g_tid) \
    do { \
        for (int i = 0; i < (int)g_tid.size(); ++i) {\
            pthread_join(g_tid[i], NULL); \
        }\
        g_tid.clear(); \
    }while(0);


class bsl_test_rwseque : public CxxTest::TestSuite
{
public:

    /**
     * 一个线程push, 另外的线程随机get 
     * 20个线程
    **/
    void test_push_pop_0()
    {

        MyParam<int> s;
        s.r.create();
        for (int i = 0; i < MAX_SIZE; ++i) s.data.push_back(i); 
        CREATE_PTHREAD(g_tid, bsl_rwseque_push<int>, 1, s);
        CREATE_PTHREAD(g_tid, bsl_rwseque_get<int>, 20, s);
        
        PTHREAD_JOIN(g_tid);
    }

    /**
     * 一个线程push, 另外的线程随机get 
     * 20个线程
    **/
    void test_push_pop_1()
    {
        MyParam<std::string> s;
        s.r.create();
        for (int i = 0; i < MAX_SIZE; ++i) s.data.push_back(rand_string()); 
        CREATE_PTHREAD(g_tid, bsl_rwseque_push<std::string>, 1, s);
        CREATE_PTHREAD(g_tid, bsl_rwseque_get<std::string>, 20, s);
        
        PTHREAD_JOIN(g_tid);
    }


    /**
     * 20个线程
    **/
    void test_push_pop_2()
    {

        MyParam<int> s;
        s.r.create();
        for (int i = 0; i < MAX_SIZE; ++i) s.data.push_back(3000); 
        CREATE_PTHREAD(g_tid, bsl_rwseque_push<int>, 10, s);
        CREATE_PTHREAD(g_tid, bsl_rwseque_get<int>, 20, s);
        
        PTHREAD_JOIN(g_tid);
    }

    /**
     * 20个线程
    **/
    void test_push_pop_3()
    {
        MyParam<std::string> s;
        s.r.create();
        for (int i = 0; i < MAX_SIZE; ++i) s.data.push_back("kdksoafuido"); 
        CREATE_PTHREAD(g_tid, bsl_rwseque_push<std::string>, 10, s);
        CREATE_PTHREAD(g_tid, bsl_rwseque_get<std::string>, 20, s);
        
        PTHREAD_JOIN(g_tid);
    }



    /**
     * 一个线程set,《喔鱿叱蘥et
    **/
    void test_set_get_0()
    {
        MyParam<int> s;
        s.r.create();
        for (int i = 0; i < MAX_SIZE; ++i) s.data.push_back(i);
        s.r.assign(MAX_SIZE, -1);
        s.default_value = -1;
        CREATE_PTHREAD(g_tid, bsl_rwseque_set<int>, 1, s);

        CREATE_PTHREAD(g_tid, bsl_rwseque_get<int>, 20, s);

        PTHREAD_JOIN(g_tid);
    }


    /**
     * 一个线程set,《喔鱿叱蘥et
    **/
    void test_set_get_1()
    {
        MyParam<std::string> s;
        s.r.create();
        for (int i = 0; i < MAX_SIZE; ++i) s.data.push_back(rand_string());
        s.r.assign(MAX_SIZE, "");
        s.default_value = "";
        CREATE_PTHREAD(g_tid, bsl_rwseque_set<std::string>, 1, s);
        CREATE_PTHREAD(g_tid, bsl_rwseque_get<std::string>, 20, s);

        PTHREAD_JOIN(g_tid);
    }

    /**
    **/
    void test_set_get_2()
    {
        MyParam<int> s;
        s.r.create();
        for (int i = 0; i < MAX_SIZE; ++i) s.data.push_back(i);
        s.r.assign(MAX_SIZE, -1);
        s.default_value = -1;
        CREATE_PTHREAD(g_tid, bsl_rwseque_set<int>, 10, s);

        CREATE_PTHREAD(g_tid, bsl_rwseque_get<int>, 20, s);

        PTHREAD_JOIN(g_tid);
    }


    /**
    **/
    void test_set_get_3()
    {
        MyParam<std::string> s;
        s.r.create();
        for (int i = 0; i < MAX_SIZE; ++i) s.data.push_back(rand_string());
        s.r.assign(MAX_SIZE, "");
        s.default_value = "";
        CREATE_PTHREAD(g_tid, bsl_rwseque_set<std::string>, 10, s);
        CREATE_PTHREAD(g_tid, bsl_rwseque_get<std::string>, 20, s);

        PTHREAD_JOIN(g_tid);
    }

    void test_get_clear_0()
    {
        MyParam<int> s;
        s.r.create();
        for (int i = 0; i < MAX_SIZE; ++i) s.data.push_back(60000);
        s.r.assign(MAX_SIZE, -1);
        s.default_value = -1;

        CREATE_PTHREAD(g_tid, bsl_rwseque_push<int>, 5, s);
        CREATE_PTHREAD(g_tid, bsl_rwseque_clear<int>, 1, s);
        CREATE_PTHREAD(g_tid, bsl_rwseque_get<int>, 20, s);
        PTHREAD_JOIN(g_tid);
    }


    void test_create_1()
    {
        bsl::rwseque<int> t1;
        t1.create();
        t1.create();
        t1.create();
        TS_ASSERT(t1.destroy() == 0);
    }



};
