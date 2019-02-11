/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: test_ResourcePool.cpp,v 1.5 2010/04/28 12:45:33 scmpf Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file test_ResourcePool.cpp
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/08/06 20:23:01
 * @version $Revision: 1.5 $ 
 * @brief 
 *  
 **/
//#define NGUI_CONFIG_DO_NOT_COPY_FOR_DEFAULT_CONSTRUCTION
//#define NG_TEST_DEBUG
#include<new>
#include<iostream>
#include<cassert>
#include<cstdarg>
#include "bsl/ResourcePool.h"
#include "bsl/pool/bsl_debugpool.h"
#include "bsl/pool/bsl_xcompool.h"
#include "bsl/var/implement.h"

using namespace std;

#include<typeinfo>
#include<iostream>
using namespace std;
#define see(x) do{  \
    cerr<<__FILE__<<":"<<__LINE__<<": " <<(#x)<<" = "<<(x)<<" ("<<typeid(x).name()<<")"<<endl;\
}while(0)

#define ASSERT_THROW( expr, except )    \
    do{                                 \
        bool has_thrown = false;        \
        try{                            \
            expr;                       \
        }catch( except& e ){            \
            has_thrown = true;          \
        }                               \
        assert( has_thrown );           \
    }while(0)
/**
* @brief 好用的测试类
*  
*  
*/
class C{
public:
    static int s_default_constructor_count;
    static int s_copy_constructor_count;
    static int s_int_constructor_count;
    static int s_destructor_count;
    static int alive_count(){
        return  s_default_constructor_count
            +   s_copy_constructor_count
            +   s_int_constructor_count
            -   s_destructor_count;
    }
    static void print_count(){
        see( s_default_constructor_count );
        see( s_copy_constructor_count );
        see( s_int_constructor_count );
        see( s_destructor_count );
        see( alive_count() );
    }

    static void clear_count(){
        s_default_constructor_count = 0;
        s_copy_constructor_count    = 0;
        s_int_constructor_count     = 0;
        s_destructor_count          = 0;
    }

    C(){
#ifdef NG_TEST_DEBUG
        cerr<<"C() this ="<<this<<endl;
#endif
        s_default_constructor_count ++;
    }
    ~C(){
#ifdef NG_TEST_DEBUG
        cerr<<"~C() this ="<<this<<endl;
#endif
        s_destructor_count ++;
    }
    C( const C& other){
#ifdef NG_TEST_DEBUG
        cerr<<"C(const C&) this ="<<this<<" other ="<<(void*)&other<<endl;
#endif
        s_copy_constructor_count ++;
    }

    C(int i){
#ifdef NG_TEST_DEBUG
        cerr<<"C(int i) this = "<<this<<" i ="<<i<<endl;
#endif
        s_int_constructor_count ++;
    }

};

int C::s_default_constructor_count  = 0;
int C::s_copy_constructor_count     = 0;
int C::s_int_constructor_count      = 0;
int C::s_destructor_count           = 0;

#include<cassert>
/*
void test___bsl::NgAllocImpl(){
    cout<<endl<<"enter "<<__func__<<"():"<<endl;

    __bsl::NgAllocImpl impl10(10);// a multiple of sizeof(size_t) is better
    impl10.allocate(10);
    impl10.allocate(0);
    impl10.allocate(9);
    impl10.allocate(8);
    impl10.allocate(1);
    impl10.allocate(123);

    __bsl::NgAllocImpl impl20(20);
    impl20.allocate(10);
    impl20.allocate(0);
    impl20.allocate(9);
    impl20.allocate(8);
    impl20.allocate(1);
    impl20.allocate(123);

    typedef __bsl::NgAllocImpl::config_t conf_t;
    conf_t config; //取默认值
    assert( config.first_chunk_size == conf_t::DEFAULT_FIRST_CHUNK_SIZE );
    assert( config.chunk_size       == conf_t::DEFAULT_CHUNK_SIZE );
    assert( config.alignment        == conf_t::DEFAULT_ALIGNMENT );

    __bsl::NgAllocImpl impl_conf(config);
    //todo: 增加对__bsl::NgAllocImpl(const __bsl::NgAllocImpl::config_t&)生成的对象的测试

}
*/
void test_construct_and_destroy(){
    cout<<endl<<"enter "<<__func__<<"():"<<endl;

    C::clear_count();
    //test default constructor
    char buf[sizeof(C)];
    void *p = buf;
    bsl::construct<C>(p);
    
    assert( C::s_default_constructor_count == 1);
    assert( C::alive_count() == 1 );
    bsl::destroy<C>(p);
    assert( C::s_destructor_count == 1 );
    assert( C::alive_count() == 0 );

    //test copy constructor
    char buf2[sizeof(C)];
    void *q = buf2;
    bsl::construct<C,C>(q,p);
    int aa = 4;
    bsl::construct<C,int>(q,&aa); //call C::C(int)
    assert( C::s_copy_constructor_count == 1 );
    assert( C::s_int_constructor_count == 1 );
    assert( C::alive_count() == 2 );

    bsl::destroy<C>(q);
    assert( C::s_destructor_count == 2 );
    assert( C::alive_count() == 1 );
}

void test_attach(){
    C::clear_count();

    cout<<endl<<"enter "<<__func__<<"():"<<endl;

    {
        bsl::ResourcePool rp;
        //attach a file
        FILE * file = fopen("test_ResourcePool","r");
        assert(file);
        cout<<"file "<<file<<" opened"<<endl;
        rp.attach( file, bsl::bsl_fclose );

        //attach some memory
        void * memory = malloc(7336);
        assert(memory);
        cout<<"memory "<<memory<<" malloced"<<endl;
        rp.attach( memory, bsl::bsl_free );

        //atach an object;
        C *pc = new C;
        rp.attach( pc, bsl::bsl_delete<C> );

        C *pc_arr = new C[2];
        rp.attach( pc_arr, bsl::bsl_delete_array<C> );
        
    }// rp destroyed here, “没有rp，就什么都没有了”

    assert( C::s_default_constructor_count == 3 );
    assert( C::s_destructor_count == 3 );
    assert( C::alive_count() == 0 ); 
}

void test_create(){
    cout<<endl<<"enter "<<__func__<<"():"<<endl;

    C::clear_count();
    {
        //test create()
        bsl::ResourcePool rp;
        C& c = rp.create<C>();
        //test create(arg)
        rp.create<C>(c);
        //test create(arg) with int
        rp.create<C>(123);
        C* pc = rp.createp<C>(456);
        assert(NULL != pc);
    }// rp destroyed here, “没有rp，就什么都没有了”
    assert( C::s_default_constructor_count == 1 );
    assert( C::s_copy_constructor_count == 1 );
    assert( C::s_int_constructor_count == 2 );
    assert( C::s_destructor_count == 4 );
    assert( C::alive_count() == 0 );

    //POD
    {
        bsl::ResourcePool rp;
        void * p = rp.create<void *>();
        void * p1 = rp.create<void *>(p);
        assert( p1 == p );
        void * p2 = rp.create<void *, void *>(p);
        assert( p2 == p );
        const char * str = "str";
        const void * p3 = rp.create<const void *, const char *>(str);
        assert( p3 == str );
        //注意： void * p4 = rp.create<void *, char *>(str); 是不可以的
        //因为const char *＆ 不能转化成void *

    }// rp destroyed here

}

void test_create_array(){
    cout<<endl<<"enter "<<__func__<<"():"<<endl;
    C::clear_count();
    {    
        //test create()
        bsl::ResourcePool rp;
        C* c_arr = rp.create_array<C>(2);// default = 1 copy = 2 int = 0 destruct = 1 

        //test create(arg)
        C* c_arr2 = rp.create_array<C>(3, c_arr[0]); // default = 1 copy = 5 int = 0 destruct = 1 

        //test create(arg) with arg is int
        C* c_arr3 = rp.create_array<C>(4, 7336 );// default = 1 copy = 5 int = 4 destruct = 1 

        // you can dump them casually
        c_arr = NULL;
        c_arr2 = NULL;
        c_arr3 = NULL;

    }// rp destroyed here, “没有rp，就什么都没有了”

#ifdef NGUI_CONFIG_DO_NOT_COPY_FOR_DEFAULT_CONSTRUCTION
    //这个没什么好说的，正常版本
    assert( C::s_default_constructor_count == 2 );
    assert( C::s_copy_constructor_count == 3 );
    assert( C::s_int_constructor_count  == 4 );
    assert( C::alive_count() == 0 );
#else
    //这个看请面的注释
    assert( C::s_default_constructor_count == 1 );
    assert( C::s_copy_constructor_count == 5 );
    assert( C::s_int_constructor_count  == 4 );
    assert( C::alive_count() == 0 );
#endif
    
    //pod;
    {
        bsl::ResourcePool rp;
        void ** p_arr = rp.create_array<void *>(1);
        void ** p1_arr = rp.create_array<void *>(3,p_arr[0]);
        char str[] = "str";
        const void ** p2_arr = rp.create_array<const void *,const char *>(3,str);
        for( int i = 0; i< 3; ++ i ){
            assert(p2_arr[i] == str );
        }
        void ** p3_arr = rp.create_array<void *, void *>(7,(void*)str);
        for( int i = 0; i< 7; ++ i ){
            assert(p3_arr[i] == str );
        }

    }

}

void test_clone(){
    cout<<endl<<"enter "<<__func__<<"():"<<endl;

    C src[3] = { C(123), C(7336), C(9394) };
    C::clear_count();
    {
        bsl::ResourcePool rp;
        
        //test clone(arg)
        rp.clone<C>(src[1]);
        rp.clone(src[1]);
        //test clone_array()
        C* pc3 = rp.clone_array(src, 3);
        assert(NULL != pc3);
        //test create_raw()
       void * p1 = rp.create_raw( 123 );
        assert(NULL != p1);
        void * p2 = rp.clone_raw( p1, 123 );
        assert(NULL != p2);
        //test create_cstring()
        char * str = rp.clone_cstring( "没有rp，就什么都没有了" );
        assert( 0 == strcmp( str, "没有rp，就什么都没有了" ) ); 
    }// rp destroyed here, “没有rp，就什么都没有了”
    assert( C::s_default_constructor_count == 0 );
    assert( C::s_copy_constructor_count == 5 );
    assert( C::s_int_constructor_count == 0 );
    assert( C::s_destructor_count == 5 );
    assert( C::alive_count() == 0 );

}
#include<vector>
#include<map>
#include<set>
#include<list>
#include<deque>

/*
void test_allocator(){
    cout<<endl<<"enter "<<__func__<<"():"<<endl;

    // 编译测试
    vector<int,bsl::NgAlloc<int> > v;
    map<double, char*, bsl::NgAlloc<double> >m;
    set<char, bsl::NgAlloc<char> > s;
    list<int, bsl::NgAlloc<int> > l;
    deque<int, bsl::NgAlloc<int> > di;
    deque<vector<int, bsl::NgAlloc<int> >, bsl::NgAlloc<vector<int, bsl::NgAlloc<int> > > > dv;

    // todo: 功能测试
}
*/

void test_with_mempool(){
    {
        bsl::debugpool pool;
        bsl::ResourcePool rp(pool);
        rp.create<C>();
        rp.create_array<C>(123);
    }

    {
        bsl::debugpool pool;
        bsl::ResourcePool rp(pool);
        bsl::pool_allocator<char> alloc = rp.get_allocator();
        char *p = alloc.allocate(2);
        alloc.deallocate(p, 2);
        bsl::pool_allocator<C> alloc2 = rp.get_allocator();
        C* pc = alloc2.allocate(1);
        alloc2.deallocate(pc, 1);
    }

    {
        bsl::debugpool pool;
        bsl::ResourcePool rp(pool);
        char * p = static_cast<char *>(rp.create_raw(1000));
        p[999] = 'A';
    }

    {
        bsl::debugpool pool;
        bsl::ResourcePool rp(pool);
        const char * cs = "iwegoewgewobjojqwnn q qoeqbn w%$%#^@$#@^@$%@!@$&&*%$*%$";
        assert( 0 == strcmp( cs, rp.clone_cstring(cs) ) );
        assert( 0 == strcmp( "", rp.clone_cstring("") ) );
        assert( 0 == strncmp( cs, rp.clone_cstring(cs, strlen(cs)), strlen(cs) ) );
        assert( 0 == strncmp( cs, rp.clone_cstring(cs, 5), 5 ) );
        assert( '\0' == * rp.clone_cstring(cs, 0) ) ;
    }

    {
        bsl::xcompool pool;
        pool.create();
        bsl::ResourcePool rp(pool);
        bsl::var::String str = 
            rp.create<bsl::var::String>(bsl::var::String::allocator_type( &rp.get_mempool() ));
        str = "hello word";
        printf("%s\n",str.c_str());
    }    
}

class EException: public bsl::BasicException<EException, bsl::Exception>{};

class E{
public:
    E(){
        throw  EException()<<BSL_EARG;
    }
    E(int){
        throw EException()<<BSL_EARG;
    }

    E(std::nothrow_t){
    }

    E(const E&){
        throw EException();
    }
};

class BufPool: public bsl::mempool{
public:
    BufPool( void* buf, size_t size )
        :_map(), _buf(static_cast<char*>(buf)), _count(0), _size(size) {}
    
    virtual void * malloc(size_t size){
        if ( _count + size > _size ){
            throw bsl::BadAllocException()<<BSL_EARG<<"need:"<<size<<" left:"<<_size - _count;
        }
        char * res = _buf + _count;
        _count += size;
        _map[res] = _size;
        return res;
    }

    virtual void free( void *p, size_t len ){
        std::map<char*, size_t>::iterator iter = _map.find(static_cast<char*>(p));
        if ( iter == _map.end() || p != iter->first || len != iter->second ){
            throw bsl::BadArgumentException()<<BSL_EARG<<"p:"<<p<<" len:"<<len;
        }
        //do nothing
    }
private:
    std::map<char*, size_t> _map;
    char * _buf;
    size_t _count;
    size_t _size;
};

void test_except(){
    {
        bsl::ResourcePool rp;
        ASSERT_THROW( rp.create<E>(), EException );
        assert( NULL == rp.createp<E>() );
    }
    {

        bsl::ResourcePool rp;
        const E& ee = rp.create<E>(std::nothrow);
        ASSERT_THROW( rp.create<E>(ee), EException );
        assert( NULL == rp.createp<E>(ee) );
    }

    {
        BufPool throw_pool(NULL, 0);
        bsl::ResourcePool rp(throw_pool);
        
        ASSERT_THROW( rp.create<int>(), bsl::BadAllocException );
        ASSERT_THROW( rp.create<C>(123), bsl::BadAllocException );
        assert( NULL == rp.createp<int>() );
        assert( NULL == rp.createp<C>(123) );

        C *pc = new C();
        ASSERT_THROW( rp.attach( pc, bsl::bsl_delete<C> ), bsl::BadAllocException );
    }
}

void test_valist(const char * format, ...) {
    va_list ap;
    va_start(ap,format);
    bsl::ResourcePool rp;
    const char * str = rp.vcrcprintf( format, ap );
    printf("%s\n",str);
    const char * str2 = rp.vcrcprintf_hint( 5, format, ap );
    printf("%s\n",str2);
    va_end(ap);
    assert( 0 == strcmp( str, rp.clone_cstring(str2) ) );
    return ;
}

void test_print() {
    test_valist("%d,%d,%s,%lf",200,3,"wahaha",1.0);
    bsl::ResourcePool rp;
    const char * str2 = rp.crcprintf("%d,%d,%s,%lf",200,3,"wahaha",1.0);
    const char * str3 = rp.crcprintf_hint(5,"%d,%d,%s,%lf",200,3,"wahaha",1.0);
    assert( 0 == strcmp(str2,rp.clone_cstring(str3)) );
}

int main(){
    try{
        test_construct_and_destroy();
        test_attach();
        test_create();
        test_create_array();
        test_clone();
        test_with_mempool();
        test_except();
        test_print();
    }catch(bsl::Exception& e){
        fprintf( stderr, "Exception: %s", e.what() );
    }
    return 0;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 */
