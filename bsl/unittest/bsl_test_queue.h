#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <deque>
#include <vector>
#include <algorithm>
#include <queue>
#include <stack>

#include <bsl/deque.h>
#include <bsl/string.h>
#include <bsl/list.h>
#include <bsl/pool.h>
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
    bool operator < (const MyClass & __x) const{
        return ((strcmp(name, __x.name) < 0) || 
                ((strcmp(name, __x.name) == 0) &&
                (strcmp(value, __x.value) < 0)));
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


template<typename Tp>
Tp rand_value();

template<>
int rand_value<int>(){
    return rand();
}

template<>
std::string rand_value<std::string>(){
    return rand_string();
}

template<>
MyClass rand_value<MyClass>(){
    MyClass ret;
    std::string name = rand_string();
    std::string value = rand_string();
    memcpy(ret.name, name.c_str(), name.size());
    memcpy(ret.value, value.c_str(), value.size());
    return ret;
}


template<typename Tp, typename Container>
void test_queue(){
    typedef Tp val_type;
    typedef std::queue<val_type, Container> Queue;

    const int N = 10;
    const int M = 5;
    std::vector<val_type> vec(N);
    for(int i = 0; i < N; ++i){
        vec[i] = rand_value<Tp>();
    }

    //定义
    Queue q;
    TS_ASSERT(q.size() == 0);
     
    //push
    for(int i = 0; i < N; ++i){
        q.push(vec[i]);
        TS_ASSERT(q.back() == vec[i]);
        TS_ASSERT(q.front() == vec[0]);
    }
    TS_ASSERT((int)q.size() == N);

    //pop (N-M) items
    for(int i = 0; i < N - M; ++i){
        TS_ASSERT(q.front() == vec[i]);
        TS_ASSERT(q.back() == vec[N - 1]);
        q.pop();
    }
    TS_ASSERT((int)q.size() == M);

    //const queue reference
    const Queue& cq = q;
    TS_ASSERT(cq.empty() == false);
    TS_ASSERT((int)cq.size() == M);

    //front item refenrnce
    const val_type& v1 = cq.front();
    val_type& v2 = q.front();
    TS_ASSERT(v1 == v2);
    
    //back item refenrnce
    const val_type& v3 = cq.back();
    val_type& v4 = q.back();
    TS_ASSERT(v3 == v4);

    //copy conqruct
    Queue q2(q);
    for(int i = 0; i < M; ++i){
        TS_ASSERT(q2.front() == vec[N - M + i]);
        TS_ASSERT(q2.back() == vec[N - 1]);
        q2.pop();
    }   
    TS_ASSERT((int)q2.size() == 0);
    TS_ASSERT(q2.empty() == true);

    //q has not been changed.
    TS_ASSERT((int)q.size() == M);
    TS_ASSERT(q.empty() == false);

    //assignment
    Queue q3;
    q3 = q;
    for(int i = 0; i < M; ++i){
        TS_ASSERT(q3.front() == vec[N - M + i]);
        TS_ASSERT(q3.back() == vec[N - 1]);
        q3.pop();
    } 
    TS_ASSERT((int)q3.size() == 0);
    TS_ASSERT(q3.empty() == true);
    
    //q has not been changed.
    TS_ASSERT((int)q.size() == M);
    TS_ASSERT(q.empty() == false);
    return;
}

template<typename Tp, typename Container>
void test_priority_queue(){
    typedef Tp val_type;
    typedef std::priority_queue<val_type, Container> Queue;

    const int N = 10;
    const int M = 5;
    std::vector<val_type> vec(N);
    for(int i = 0; i < N; ++i){
        vec[i] = rand_value<Tp>();
    }

    //定义
    Queue q;
    TS_ASSERT(q.size() == 0);
     
    //push
    for(int i = 0; i < N; ++i){
        q.push(vec[i]);
        std::sort(vec.begin(), vec.begin() + i + 1);
        TS_ASSERT(q.top() == vec[i]);
    }
    TS_ASSERT((int)q.size() == N);

    std::sort(vec.begin(), vec.end());
    //pop (N-M) items
    for(int i = 0; i < N - M; ++i){ 
        TS_ASSERT(q.top() == vec[N - 1 - i]);
        q.pop();
    }
    TS_ASSERT((int)q.size() == M);

    //const queue reference
    const Queue& cq = q;
    TS_ASSERT(cq.empty() == false);
    TS_ASSERT((int)cq.size() == M);

    //top item refenrnce
    const val_type& v1 = cq.top();
    const val_type& v2 = q.top();
    TS_ASSERT(v1 == v2);

    //copy conqruct
    Queue q2(q);
    for(int i = 0; i < M; ++i){
        q2.pop();
    }   
    TS_ASSERT((int)q2.size() == 0);
    TS_ASSERT(q2.empty() == true);

    //q has not been changed.
    TS_ASSERT((int)q.size() == M);
    TS_ASSERT(q.empty() == false);

    //assignment
    Queue q3;
    q3 = q;
    for(int i = 0; i < M; ++i){
        q3.pop();
    } 
    TS_ASSERT((int)q3.size() == 0);
    TS_ASSERT(q3.empty() == true);
    
    //q has not been changed.
    TS_ASSERT((int)q.size() == M);
    TS_ASSERT(q.empty() == false);
    return;
}



class bsl_test_queue : public CxxTest::TestSuite
{
public:
    //queue
    void test_queue_with_int_deque(){
        test_queue<int, bsl::deque<int> >();
    }
    void test_queue_with_string_deque(){
        test_queue<std::string, bsl::deque<std::string> >();
    }
    void test_queue_with_myclass_deque(){
        test_queue<MyClass, bsl::deque<MyClass> >();
    }
    
    //priority_queue
    void test_priority_queue_with_int_deque(){
        test_priority_queue<int, bsl::deque<int> >();
    }
    void test_priority_queue_with_string_deque(){
        test_priority_queue<std::string, bsl::deque<std::string> >();
    }
    void test_priority_queue_with_myclass_deque(){
        test_priority_queue<MyClass, bsl::deque<MyClass> >();
    }

    void test_queue_with_int_list(){
        test_queue<int, bsl::list<int> >();
    }
    void test_queue_with_string_list(){
        test_queue<std::string, bsl::list<std::string> >();
    }
    void test_queue_with_myclass_list(){
        test_queue<MyClass, bsl::list<MyClass> >();
    }
};
