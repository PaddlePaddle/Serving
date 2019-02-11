#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <deque>
#include <vector>
#include <algorithm>
#include <queue>
#include <stack>

#include <bsl/containers/deque/bsl_deque.h>
#include <bsl/containers/string/bsl_string.h>
#include <bsl/containers/list/bsl_list.h>
#include <bsl/containers/slist/bsl_slist.h>

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


template<typename Tp, typename Container >
void test_stack(){
    typedef Tp val_type;
    typedef std::stack<val_type, Container> Stack;

    const int N = 10;
    const int M = 5;
    std::vector<val_type> vec(N);
    for(int i = 0; i < N; ++i){
        vec[i] = rand_value<Tp>();
    }
    //∂®“Â
    Stack st;
    TS_ASSERT(st.size() == 0);
    //push
    for(int i = 0; i < N; ++i){
        st.push(vec[i]);
    }
    TS_ASSERT((int)st.size() == N);
    //pop
    for(int i = N - 1; i >= M; --i){
        TS_ASSERT(st.top() == vec[i]);
        st.pop();
    }
    TS_ASSERT((int)st.size() == M);
    TS_ASSERT(st.top() == vec[M - 1]);
   
    //const reference
    const Stack& cst = st;
    TS_ASSERT(cst.empty() == false);
    TS_ASSERT((int)cst.size() == M);

    //top refenrnce
    const val_type& v1 = cst.top();
    val_type& v2 = st.top();
    TS_ASSERT(v1 == v2);

    //copy construct
    Stack st2(st);
    for(int i = M - 1; i >= 0; --i){
        TS_ASSERT(st2.top() == vec[i]);
        st2.pop();
    }   
    TS_ASSERT((int)st2.size() == 0);
    TS_ASSERT(st2.empty() == true);
    TS_ASSERT((int)st.size() == M);
    TS_ASSERT(st.empty() == false);
    //assignment
    Stack st3;
    st3 = st;
    for(int i = M - 1; i >= 0; --i){
        TS_ASSERT(st3.top() == vec[i]);
        st3.pop();
    }   
    TS_ASSERT((int)st3.size() == 0);
    TS_ASSERT(st3.empty() == true);
    TS_ASSERT((int)st.size() == M);
    TS_ASSERT(st.empty() == false);
    return;
}

class bsl_test_stack : public CxxTest::TestSuite
{
public:
    void test_stack_with_int_deque(){
        test_stack<int, bsl::deque<int> >();
    }
    void test_stack_with_string_deque(){
        test_stack<std::string, bsl::deque<std::string> >();
    }
    void test_stack_with_myclass_deque(){
        test_stack<MyClass, bsl::deque<MyClass> >();
    }
    void test_stack_with_int_list(){
        test_stack<int, bsl::list<int> >();
    }
    void test_stack_with_string_list(){
        test_stack<std::string, bsl::list<std::string> >();
    }
    void test_stack_with_myclass_list(){
        test_stack<MyClass, bsl::list<MyClass> >();
    }
};
