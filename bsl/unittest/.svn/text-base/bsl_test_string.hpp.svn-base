

/**
 * @file bsl_test_string.hpp
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/09/12 17:29:08
 * @version $Revision: 1.10 $ 
 * @brief 
 *  
 **/
#ifndef  __BSL_TEST_STRING_HPP_
#define  __BSL_TEST_STRING_HPP_
#include <string>
#include <list>

#include <bsl/archive/bsl_serialization.h>
#include <bsl/archive/bsl_filestream.h>
#include <bsl/archive/bsl_binarchive.h>
#include <bsl/containers/string/bsl_string.h>
#include <bsl/alloc/bsl_alloc.h>
#include <sstream>

#include<typeinfo>
#include<iostream>
#define see(x) do{  \
    std::cerr<<__FILE__<<":"<<__LINE__<<": " <<(#x)<<" = "<<(x)<<" ("<<typeid(x).name()<<")"<<std::endl;\
}while(0)

/**
 * @brief 好用的测试宏
 *  
 *  输出表达式只有在测试表达式失败时才会被求值
 *
 *  example: CXM_ASSERTF( 3 + 3 != 6, "%s: 3 + 3 != %d", "stupid", 6 );
 */
#define CXM_ASSERTF( expr, fmt, arg... ) do{   \
    static char __CXM_MSG__[1024];  \
    TSM_ASSERT( (snprintf( __CXM_MSG__, 1024, fmt, ##arg ), __CXM_MSG__ ), expr )  \
}while(0)


/**
 * @brief
 *
 *  把表达式的名和值都输出，作为CXM_ASSERT的辅助宏使用
 *  
 */
#define ECHO(x) " "<<(#x)<<" = "<<(x)<<" "
/**
 * @brief 好用的测试宏
 *  
 *  输出表达式只有在测试表达式失败时才会被求值
 *
 *  需要#include <sstream>
 *  example: CXM_ASSERT( 3 + 3 != 6, <<"test: "<<ECHO(3+3) )
 */
#define CXM_ASSERT( expr, output_seq ) do{  \
    std::stringstream __CXM_MSG__;  \
    TSM_ASSERT( ( (__CXM_MSG__ output_seq), __CXM_MSG__.str().c_str() ), expr ) \
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


template <class _CharT, class _Alloc>
class bsl_test_string{
private:
    typedef bsl::basic_string<_CharT,_Alloc> string_t;
    _CharT *_cs_0;
    const size_t  _cs_0_len;
    const size_t  _cs_0_size;
    _CharT* _cs_1;
    _CharT* _cs_2;
    _CharT* _cs_3;
    _CharT* _cs_4;
    _CharT* _cs_5;
    _CharT* _empty_cs;
    _CharT* _cs_0_add_1;

    static _CharT * malloc_clone( const char * str ){
        size_t len = strlen(str);
        _CharT * p = static_cast<_CharT*>(malloc( (len+1)*sizeof(_CharT) ));
        for( size_t i = 0; i<=len; ++i){
            p[i] = str[i];
        }
        
        return p;
    }
 
public:
    bsl_test_string()
        : _cs_0(malloc_clone("bsl_string")), 
        _cs_0_len(strlen("bsl_string")),
        _cs_0_size(_cs_0_len * sizeof(_CharT)),
        _cs_1(malloc_clone("acumon")),
        _cs_2(malloc_clone("ACUMON")),
        _cs_3(malloc_clone("acumo")),
        _cs_4(malloc_clone("acumon")),   // == _cs_1
        _cs_5(malloc_clone("pudding")),
        _empty_cs(malloc_clone("")),
        _cs_0_add_1(malloc_clone("bsl_stringacumon"))
    {
    }

    ~bsl_test_string(){
        free( _cs_0 );
        free( _cs_1 );
        free( _cs_0_add_1 );
        free( _cs_2 );
        free( _cs_3 );
        free( _cs_4 );
        free( _cs_5 );
        free( _empty_cs );
    }
        

    void test_ctors(){

        const size_t char_size = sizeof(_CharT);
        // default ctor
        {
            string_t s;
            TS_ASSERT( s.size() == 0);
            TS_ASSERT( s.c_str() != NULL );
            TS_ASSERT( s.c_str()[0] == '\0' );
        }

        // bsl_string( const _CharT * )
        {
            string_t s(_cs_0);
            TS_ASSERT( s.size() == _cs_0_len );
            TS_ASSERT( 0 == memcmp( s.c_str(), _cs_0, _cs_0_size ));
            TS_ASSERT( s.c_str()[s.size()]== '\0' );
        }


        // bsl_basic_string( const_pointer str, size_type len ) throw(/*BadAllocException*/)
        {
            string_t s(_cs_0, 3);
            TS_ASSERT( s.size() == 3 );
            TS_ASSERT( 0 == memcmp( s.c_str(), _cs_0, 3*char_size ) );
            TS_ASSERT( s.c_str()[3] == '\0' );
        }

        // bsl_basic_string( const bsl_basic_string& other, size_type begin_pos ) throw(/*BadAllocException*/)
        {
            string_t _s=_cs_0; //patch for g++2.96
            string_t s(_s, 2);   //s = "l_string" 
            TS_ASSERT( s.size() == _cs_0_len -2 );
            TS_ASSERT( 0 == memcmp( s.c_str(), _cs_0+2, (_cs_0_len-2)*char_size ) );
            TS_ASSERT( s.c_str()[_cs_0_len-2] == '\0' );
        }


        // bsl_basic_string( const bsl_basic_string& other, size_type begin_pos, size_type sub_str_len ) throw(/*BadAllocException*/)
        {
            string_t _s=_cs_0; //patch for g++2.96
            string_t s(_s, 1, 4);   //s = "sl_s" 
            TS_ASSERT( s.size() == 4 );
            TS_ASSERT( 0 == memcmp( s.c_str(), _cs_0+1, 4*char_size ) );
            TS_ASSERT( s.c_str()[4] == '\0' );
        }

        // bsl_basic_string( const_pointer cstr, size_type cstr_len, const allocator_type& = allocator_type() ) throw(/*BadAllocException*/)
        {
            string_t s(_cs_0, 2);   //s = "ac" 
            TS_ASSERT( s.size() == 2 );
            TS_ASSERT( 0 == memcmp( s.c_str(), _cs_0, 2*char_size ) );
            TS_ASSERT( s.c_str()[2] == '\0' );
        }

        // bsl_basic_string( const_pointer cstr, const allocator_type& = allocator_type() ) throw(/*BadAllocException*/)
        {
            string_t s(_cs_0);   
            TS_ASSERT( s.size() == _cs_0_len  );   
            TS_ASSERT( 0 == memcmp( s.c_str(), _cs_0, _cs_0_size ) );
            TS_ASSERT( s.c_str()[s.size()] == '\0' );
        }

        // bsl_basic_string( size_type n, value_type chr, const allocator_type& = allocator_type() )
        {
            _CharT cs2[] = { 'a','a','a','a' };
            size_t cs2_len = sizeof(cs2)/sizeof(*cs2);
            string_t s(cs2_len, 'a');
            TS_ASSERT( s.size() == cs2_len );
            TS_ASSERT( 0 == memcmp( s.c_str(), cs2, sizeof(cs2 )) );
            TS_ASSERT( s.c_str()[s.size()] == '\0' );
        }

        // template<class InputIterator> bsl_basic_string( InputIterator __begin, InputIterator __end, const allocator_type& = allocator_type() )
        {
            string_t s(_cs_0,_cs_0+_cs_0_len);
            TS_ASSERT( s.size() == _cs_0_len );
            TS_ASSERT( 0 == memcmp( s.c_str(), _cs_0, _cs_0_size ));
            TS_ASSERT( s.c_str()[s.size()]== '\0' );
        }
    }
    // getters
    void test_c_str(){ 
        //empty c_str()
        {
            string_t s;
            TS_ASSERT( s.c_str() != NULL );
            TS_ASSERT( s.c_str()[0] == '\0' );
        }


        //string from const _CharT *
        {
            string_t s(_cs_0);
            TS_ASSERT( s.size() == _cs_0_len );
            TS_ASSERT( 0 == memcmp( s.c_str(), _cs_0, _cs_0_size ));
            TS_ASSERT( s.c_str() != _cs_0 );
            TS_ASSERT( s.c_str()[s.size()]== '\0' );
        }

        //string from another string
        {
            string_t _s(_cs_0), s(_s);
            TS_ASSERT( s.size() == _cs_0_len );
            TS_ASSERT( 0 == memcmp( s.c_str(), _cs_0, _cs_0_size ));
            TS_ASSERT( s.c_str() != _s.c_str() );
            TS_ASSERT( s.c_str()[s.size()]== '\0' );

        }


    }

    void test_empty() {
        //string from default ctor
        {
            string_t s;
            TS_ASSERT( s.empty() );
        }

        //string after clear()
        {
            string_t s(_cs_0);
            s.clear();
            TS_ASSERT( s.empty() );
        }

        //string after =""
        {
            string_t s(_cs_0);
            s = _empty_cs;
            TS_ASSERT( s.empty() );
        }

    }

    void test_size() {
        //empty string size
        {
            string_t s;
            TS_ASSERT( s.size() == 0 );
        }

        //string from cstring
        {
            string_t s(_cs_0);
            TS_ASSERT( s.size() == _cs_0_len );
        }

        //string after =
        {
            _CharT cs2[] = { 'h', 'i', '\0' };
            string_t s(_cs_0);
            s = cs2;
            TS_ASSERT( s.size() == sizeof(cs2)/sizeof(*cs2) -1);
            
            s = _empty_cs;
            TS_ASSERT( s.size() == 0 );
        }

    }

    void test_length() {
        //empty string length
        {
            string_t s;
            TS_ASSERT( s.length() == 0 );
        }

        //string from cstring
        {
            string_t s(_cs_0);
            TS_ASSERT( s.length() == _cs_0_len );
        }

        //string after =
        {
            _CharT cs2[] = { 'h', 'i', '\0' };
            string_t s(_cs_0);
            s = cs2;
            TS_ASSERT( s.length() == sizeof(cs2)/sizeof(*cs2) -1);
            
            s = _empty_cs;
            TS_ASSERT( s.length() == 0 );
        }

     }

    /**
     * @brief 
     *
     * @return  size_type 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2008/09/10 21:38:24
     **/
    void test_capacity(){
        //empty string
        {
            TS_ASSERT( string_t().capacity() == 0 );
        }

        //normal string
        {
            TS_ASSERT( string_t(_cs_0).capacity() >= string_t(_cs_0).length() );
        }

    }

    void test_operator_square(){

        //normal string
        {
            string_t s = _cs_0;
            for( size_t i = 0; i < _cs_0_len; ++ i ){
                TS_ASSERT( s[i] == _cs_0[i] );
            }
        }

        //const string
       {
            const string_t s = _cs_0;
            for( size_t i = 0; i < _cs_0_len; ++ i ){
                TS_ASSERT( s[i] == _cs_0[i] );
            }
        } 

       //test writing
       {
           const _CharT cs2[] = { 'l','i','v','e','\0' };
           string_t s(cs2);
           s[0] = cs2[3]; s[1] = cs2[2]; s[2] = cs2[1]; s[3] = cs2[0];
           const _CharT expected[] = { 'e', 'v', 'i', 'l', '\0' };
           TS_ASSERT( 0 == memcmp( s.c_str(), expected, sizeof(expected) ));
       }

    }

    // setters
    void test_clear(){
        //empty string
        {
            string_t s;
            s.clear();
            TS_ASSERT( s.c_str()[0] == 0 );
            TS_ASSERT( s.size() == 0 );
            TS_ASSERT( s == string_t() );

        }

        //normal string
        {
            string_t s(_cs_0);
            s.clear();
            TS_ASSERT( s.c_str()[0] == 0 );
            TS_ASSERT( s.size() == 0 );
            TS_ASSERT( s == string_t() );
            TS_ASSERT( s.capacity() == string_t(_cs_0).capacity() );

        }

    }

    void test_reserve(){
        //empty string
        {
            string_t s;
            TS_ASSERT( s.capacity() == 0 );
            s.reserve(1);
            TS_ASSERT( s.capacity() == 1 );
            s.reserve(123);
            TS_ASSERT( s.capacity() == 123 );
            s.reserve(100);
            TS_ASSERT( s.capacity() == 123 );   //ignore request for smaller capacity
        }

        //normal string
        {
            string_t s(_cs_0);
            s.reserve(1);   //ignored
            TS_ASSERT( s == string_t(_cs_0) );
            TS_ASSERT( s.capacity() == _cs_0_len );
            s.reserve(100);
            TS_ASSERT( s == string_t(_cs_0) );
            TS_ASSERT( s.capacity() == std::max(size_t(100),_cs_0_len) );

        }

    }

    void test_swap(){
        //swap method
        {
            string_t s1(_cs_1), s2(_cs_2);
            const _CharT *p1=s1.c_str(), *p2=s2.c_str();
            size_t len1=s1.size(), len2=s2.size(),cap1=s1.capacity(),cap2=s2.capacity();
            s1.swap(s2);
            TS_ASSERT( s1.c_str() == p2 );
            TS_ASSERT( s1.size() == len2 );
            TS_ASSERT( s1.capacity() == cap2 );
            TS_ASSERT( s2.c_str() == p1 );
            TS_ASSERT( s2.size() == len1 );
            TS_ASSERT( s2.capacity() == cap1 );
            
        }

        //std::swap()
        //不推荐使用！这个复杂度高达O(s1.size()+s2.size())！
        {
            string_t s1(_cs_1), s2(_cs_2);
            const _CharT *p1=s1.c_str(), *p2=s2.c_str();
            size_t len1=s1.size(), len2=s2.size();
            std::swap(s1,s2);
            TS_ASSERT( s1.c_str() != p2 );      //因为发生了深复制
            TS_ASSERT( s1 == string_t(_cs_2) ); //但是字面值是相等的
            TS_ASSERT( s1.size() == len2 );     
            TS_ASSERT( s1.capacity() == len2 ); //因为字符串是重新构造的，因此会"trim to size"
            TS_ASSERT( s2.c_str() != p1 );      //因为发生了深复制
            TS_ASSERT( s2 == string_t(_cs_1) );
            TS_ASSERT( s2.size() == len1 );
            TS_ASSERT( s2.capacity() == len1 );
            
        }
    }

    /**
     * @brief deprecated bsl::string don't accept NULL any more, and will throw NullPointerException in the future
     *
     * @return  void 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2008/11/09 01:00:48
    **/
    void test_operator_assign_to_null(){

        /*
         //empty string to null
        {
            string_t s;
            s = NULL;
            TS_ASSERT( s == string_t() );
            TS_ASSERT( s.size() == 0 );
            TS_ASSERT( s.capacity() == 0 );
            TS_ASSERT( s.c_str()[0] == 0 );
        }
        
        //normal string to null
        {
            string_t s(_cs_0);
            s = NULL;
            TS_ASSERT( s == string_t() );
            TS_ASSERT( s.size() == 0 );
            TS_ASSERT( s.capacity() == _cs_0_len );
            TS_ASSERT( s.c_str()[0] == 0 );
        }
        
        */
    }

    void test_operator_assign_to_cstring(){
        //empty string to empty string
        {
            string_t s;
            s = _empty_cs;
            TS_ASSERT( s == string_t() );
            TS_ASSERT( s.size() == 0 );
            TS_ASSERT( s.capacity() == 0 );
            TS_ASSERT( s.c_str()[0] == 0 );
        }
        //empty string to normal string
       {
            string_t s;
            s = _cs_0;
            TS_ASSERT( s == string_t(_cs_0) );
            TS_ASSERT( s.size() == _cs_0_len );
            TS_ASSERT( s.capacity() == _cs_0_len );
        } 
        //normal string to empty string
        {
            string_t s(_cs_0);
            s = _empty_cs;
            TS_ASSERT( s == string_t() );
            TS_ASSERT( s.size() == 0 );
            TS_ASSERT( s.capacity() == _cs_0_len );
            TS_ASSERT( s.c_str()[0] == 0 );
        }

        //normal string to normal string
        {
            char _cs2[] = "hello world!";
            char _cs3[] = "no ResourcePool, no rp!";
            char _cs4[] = "no money no talk";
            _CharT * cs2 = malloc_clone(_cs2);
            _CharT * cs3 = malloc_clone(_cs3);
            _CharT * cs4 = malloc_clone(_cs4);
            size_t cs2_len = sizeof(_cs2)/sizeof(*_cs2)-1;
            size_t cs3_len = sizeof(_cs3)/sizeof(*_cs3)-1;
            size_t cs4_len = sizeof(_cs4)/sizeof(*_cs4)-1;

            string_t s(_cs_0);
            s = cs2;
            TS_ASSERT( s == string_t(cs2) );
            TS_ASSERT( s.size() == cs2_len );
            TS_ASSERT( s.capacity() == cs2_len );
            TS_ASSERT( s.c_str()[cs2_len] == 0 );

            s = cs3;
            TS_ASSERT( s == string_t(cs3) );
            TS_ASSERT( s.size() == cs3_len );
            TS_ASSERT( s.capacity() == cs3_len );
            TS_ASSERT( s.c_str()[cs3_len] == 0 );

            s = cs4;
            TS_ASSERT( s == string_t(cs4) );
            TS_ASSERT( s.size() == cs4_len );
            TS_ASSERT( s.capacity() == cs4_len );
            TS_ASSERT( s.c_str()[cs4_len] == 0 );

            free( cs2 );
            free( cs3 );
            free( cs4 );
        }

       
        //empty string to self
        {
            string_t s(_empty_cs);
            s = s.c_str();
            TS_ASSERT( s == string_t() );
            TS_ASSERT( s.size() == 0 );
            TS_ASSERT( s.capacity() == 0 );
            TS_ASSERT( s.c_str()[0] == 0 );
        }
        
        //normal string to self
       {
            string_t s(_cs_0);
            s = s.c_str();
            TS_ASSERT( s == string_t(_cs_0) );
            TS_ASSERT( s.size() == _cs_0_len );
            TS_ASSERT( s.capacity() == _cs_0_len );
            TS_ASSERT( s.c_str()[_cs_0_len] == 0 );
        } 
    }

    void test_operator_assign_to_string(){
        //empty string to empty string
        {
            string_t s;
            s = string_t();
            TS_ASSERT( s == string_t() );
            TS_ASSERT( s.size() == 0 );
            TS_ASSERT( s.capacity() == 0 );
            TS_ASSERT( s.c_str()[0] == 0 );
        }
        //empty string to normal string
       {
            string_t s;
            s = string_t(_cs_0);
            TS_ASSERT( s == string_t(_cs_0) );
            TS_ASSERT( s.size() == _cs_0_len );
            TS_ASSERT( s.capacity() == _cs_0_len );
        } 

        //normal string to empty string
        {
            string_t s(_cs_0);
            s = string_t();
            TS_ASSERT( s == string_t() );
            TS_ASSERT( s.size() == 0 );
            TS_ASSERT( s.capacity() == _cs_0_len );
            TS_ASSERT( s.c_str()[0] == 0 );
        }

        //normal string to normal string
        {
            char _cs2[] = "hello world!";
            char _cs3[] = "no ResourcePool, no rp!";
            char _cs4[] = "no money no talk";
            _CharT * cs2 = malloc_clone(_cs2);
            _CharT * cs3 = malloc_clone(_cs3);
            _CharT * cs4 = malloc_clone(_cs4);
            size_t cs2_len = sizeof(_cs2)/sizeof(*_cs2)-1;
            size_t cs3_len = sizeof(_cs3)/sizeof(*_cs3)-1;
            size_t cs4_len = sizeof(_cs4)/sizeof(*_cs4)-1;
            string_t s2 = cs2;
            string_t s3 = cs3;
            string_t s4 = cs4;

            string_t s(_cs_0);
            s = s2;
            TS_ASSERT( s == string_t(cs2) );
            TS_ASSERT( s.size() == cs2_len );
            TS_ASSERT( s.capacity() == cs2_len );//cs2_len > _cs_0_len
            TS_ASSERT( s.c_str()[cs2_len] == 0 );

            s = s3;
            TS_ASSERT( s == string_t(cs3) );
            TS_ASSERT( s.size() == cs3_len );
            TS_ASSERT( s.capacity() == cs3_len );//cs3_len > cs2_len
            TS_ASSERT( s.c_str()[cs3_len] == 0 );

            s = s4;
            TS_ASSERT( s == string_t(cs4) );
            TS_ASSERT( s.size() == cs4_len );
            TS_ASSERT( s.capacity() == cs3_len );//cs4_len < cs3_len
            TS_ASSERT( s.c_str()[cs4_len] == 0 );

            free( cs2 );
            free( cs3 );
            free( cs4 );
        }

        //empty string to self
        {
            string_t s(_empty_cs);
            s = s;
            TS_ASSERT( s == string_t() );
            TS_ASSERT( s.size() == 0 );
            TS_ASSERT( s.capacity() == 0 );
            TS_ASSERT( s.c_str()[0] == 0 );
        }
        
        //normal string to self
       {
            string_t s(_cs_0);
            s = s;
            TS_ASSERT( s == string_t(_cs_0) );
            TS_ASSERT( s.size() == _cs_0_len );
            TS_ASSERT( s.capacity() == _cs_0_len );
            TS_ASSERT( s.c_str()[_cs_0_len] == 0 );
        } 
    }


    void test_operator_eq_eq(){
        _CharT cs2[] = {'b','s','l','_','s','t','r','i','n','g','\0'};
        _CharT cs3[] = {'n','o','t',' ','e','q','\0'};
        //string and string
        {
            TS_ASSERT( string_t() == string_t() );
            TS_ASSERT( string_t(_cs_0) == string_t(_cs_0) );
            TS_ASSERT( string_t(_cs_0) == string_t(cs2) );
            TS_ASSERT( !(string_t(_cs_0) == string_t(cs3)) );
        }

        //string and cstring
        {
            TS_ASSERT( string_t() == _empty_cs );
            TS_ASSERT( string_t(_cs_0) == string_t(_cs_0) );
            TS_ASSERT( string_t(_cs_0) == string_t(cs2) );
            TS_ASSERT( !( string_t(_cs_0) == string_t(cs3)) );
        }

        //cstring and string
        {
            TS_ASSERT( _empty_cs == string_t() );
            TS_ASSERT( string_t(_cs_0) == string_t(_cs_0) );
            TS_ASSERT( string_t(cs2) == string_t(_cs_0) );
            TS_ASSERT( !( string_t(cs3) == string_t(_cs_0)) );
        }
    }

    void test_operator_not_eq(){
        _CharT cs2[] = {'b','s','l','_','s','t','r','i','n','g','\0'};
        _CharT cs3[] = {'n','o','t',' ','e','q','\0'};
        //string and string
        {
            TS_ASSERT( !(string_t() != string_t()) );
            TS_ASSERT( !(string_t(_cs_0) != string_t(_cs_0)) );
            TS_ASSERT( !(string_t(_cs_0) != string_t(cs2)) );
            TS_ASSERT( string_t(_cs_0) != string_t(cs3) );
        }

        //string and cstring
        {
            TS_ASSERT( !(string_t() != _empty_cs) );
            TS_ASSERT( !(string_t(_cs_0) != string_t(_cs_0)) );
            TS_ASSERT( !(string_t(_cs_0) != string_t(cs2)) );
            TS_ASSERT( string_t(_cs_0) != string_t(cs3) );
        }

        //cstring and string
        {
            TS_ASSERT( !(_empty_cs != string_t()) );
            TS_ASSERT( !(string_t(_cs_0) != string_t(_cs_0)) );
            TS_ASSERT( !(string_t(cs2) != string_t(_cs_0)) );
            TS_ASSERT( string_t(cs3) != string_t(_cs_0) );
        }
    }

    void test_operator_lt(){
        //empty string
        {
            TS_ASSERT( !(string_t() < string_t()) );
            TS_ASSERT( string_t() < string_t(_cs_1) );
            TS_ASSERT( string_t() < string_t(_cs_2) );
        }

        //normal string
        {
            TS_ASSERT( string_t(_cs_2) < string_t(_cs_1) );
            TS_ASSERT( !(string_t(_cs_1) < string_t(_cs_2)) );

            TS_ASSERT( string_t(_cs_3) < string_t(_cs_1) );
            TS_ASSERT( !(string_t(_cs_1) < string_t(_cs_3)) );

            TS_ASSERT( !(string_t(_cs_4) < string_t(_cs_1)) );
            TS_ASSERT( !(string_t(_cs_1) < string_t(_cs_4)) );

            TS_ASSERT( string_t(_cs_1) < string_t(_cs_5) );
            TS_ASSERT( !(string_t(_cs_5) < string_t(_cs_1)) );
        }

    }

    void test_operator_gt(){
        //empty string
        {
            string_t empty_str1, empty_str2;    //modified because of a bug of g++2.96
            TS_ASSERT( !(empty_str1 > empty_str1) );
            TS_ASSERT( !(empty_str1 > empty_str2) );
            TS_ASSERT( !(empty_str1 > string_t(_cs_1)) );
            TS_ASSERT( !(empty_str1 > string_t(_cs_2)) );
            TS_ASSERT( !(empty_str2 > string_t(_cs_3)) );
            TS_ASSERT( !(empty_str2 > string_t(_cs_4)) );
        }

        //normal string
        {
            TS_ASSERT( !(string_t(_cs_2) > string_t(_cs_1)) );
            TS_ASSERT( string_t(_cs_1) > string_t(_cs_2));

            TS_ASSERT( !(string_t(_cs_3) > string_t(_cs_1)) );
            TS_ASSERT( string_t(_cs_1) > string_t(_cs_3));

            TS_ASSERT( !(string_t(_cs_4) > string_t(_cs_1)) );
            TS_ASSERT( !(string_t(_cs_1) > string_t(_cs_4)) );

            TS_ASSERT( !(string_t(_cs_1) > string_t(_cs_5)) );
            TS_ASSERT( string_t(_cs_5) > string_t(_cs_1));
        }

    }

    void test_operator_lt_eq(){
        //empty string
        {
            TS_ASSERT( string_t() <= string_t() );
            TS_ASSERT( string_t() <= string_t(_cs_1) );
            TS_ASSERT( string_t() <= string_t(_cs_2) );
        }

        //normal string
        {
            TS_ASSERT( string_t(_cs_2) <= string_t(_cs_1) );
            TS_ASSERT( !(string_t(_cs_1) <= string_t(_cs_2)) );

            TS_ASSERT( string_t(_cs_3) <= string_t(_cs_1) );
            TS_ASSERT( !(string_t(_cs_1) <= string_t(_cs_3)) );

            TS_ASSERT( string_t(_cs_4) <= string_t(_cs_1) );
            TS_ASSERT( string_t(_cs_1) <= string_t(_cs_4) );

            TS_ASSERT( string_t(_cs_1) <= string_t(_cs_5) );
            TS_ASSERT( !(string_t(_cs_5) <= string_t(_cs_1)) );
        }

    }

    void test_operator_gt_eq(){
        //empty string
        {
            TS_ASSERT( string_t() >= string_t() );
            TS_ASSERT( !(string_t() >= string_t(_cs_1)) );
            TS_ASSERT( !(string_t() >= string_t(_cs_2)) );
        }

        //normal string
        {
            TS_ASSERT( !(string_t(_cs_2) >= string_t(_cs_1)) );
            TS_ASSERT( string_t(_cs_1) >= string_t(_cs_2));

            TS_ASSERT( !(string_t(_cs_3) >= string_t(_cs_1)) );
            TS_ASSERT( string_t(_cs_1) >= string_t(_cs_3));

            TS_ASSERT( string_t(_cs_4) >= string_t(_cs_1) );
            TS_ASSERT( string_t(_cs_1) >= string_t(_cs_4) );

            TS_ASSERT( !(string_t(_cs_1) >= string_t(_cs_5)) );
            TS_ASSERT( string_t(_cs_5) >= string_t(_cs_1));
        }

    }

    void test_operator_shift(){
#if __GNUC__ >= 3
        //empty string
        {
            std::basic_stringstream<_CharT> ss;
            ss<<string_t();
            TS_ASSERT( ss.str() == std::basic_string<_CharT>() );
        }

        //normal string
        {
            std::basic_stringstream<_CharT> ss;
            ss<<string_t(_cs_1);
            TS_ASSERT( 0 == memcmp(ss.str().c_str(), _cs_1, ss.str().size() * sizeof(_CharT) ) );
        }

        {
            std::basic_stringstream<_CharT> ss;
            ss<<string_t(_cs_2);
            TSM_ASSERT( ss.str().c_str(), 0 == memcmp(ss.str().c_str(), _cs_2, ss.str().size() * sizeof(_CharT) ) );
        }

        {
            std::basic_stringstream<_CharT> ss;
            ss<<string_t(_cs_3);
            TS_ASSERT( 0 == memcmp(ss.str().c_str(), _cs_3, ss.str().size() * sizeof(_CharT) ) );
        }

        {
            std::basic_stringstream<_CharT> ss;
            ss<<string_t(_cs_4);
            TS_ASSERT( 0 == memcmp(ss.str().c_str(), _cs_4, ss.str().size() * sizeof(_CharT) ) );
        }

        {
            std::basic_stringstream<_CharT> ss;
            ss<<string_t(_cs_5);
            TS_ASSERT( 0 == memcmp(ss.str().c_str(), _cs_5, ss.str().size() * sizeof(_CharT) ) );
        }
            
#endif
    }

    void test_serialization(){
        static const char * const DATA_FILE_NAME = "bsl_test_string.data";
        //empty string
        {
            //串行化
            {
                bsl::filestream fs;
                TS_ASSERT( 0 == fs.open(DATA_FILE_NAME, "w") );
                bsl::binarchive ar(fs);
                ar.write(string_t());
                ar.write(string_t(_empty_cs));
                fs.close();
            }
            //反串行化
            {
                bsl::filestream fs;
                TS_ASSERT( 0 == fs.open(DATA_FILE_NAME, "r") );
                bsl::binarchive ar(fs);
                string_t s1(_cs_1);
                ar.read(&s1);
                TS_ASSERT( s1 == string_t() );

                string_t s2(_cs_2);
                ar.read(&s2);
                TS_ASSERT( s2 == string_t() );
                fs.close();
            }
        }

         //normal string
        {
            //串行化
            {
                bsl::filestream fs;
                TS_ASSERT( 0 == fs.open(DATA_FILE_NAME, "w") );
                bsl::binarchive ar(fs);
                TS_ASSERT( 0==ar.write(string_t(_cs_1)) );
                TS_ASSERT( 0==ar.write(string_t(_cs_2)) );
                TS_ASSERT( 0==ar.write(string_t(_cs_3)) );
                TS_ASSERT( 0==ar.write(string_t(_cs_4)) );
                TS_ASSERT( 0==ar.write(string_t(_cs_5)) );
                
                fs.close();
            }
            //反串行化
            {
                bsl::filestream fs;
                TS_ASSERT( 0 == fs.open(DATA_FILE_NAME, "r") );
                bsl::binarchive ar(fs);
                string_t s;

                TS_ASSERT( 0 == ar.read(&s) );
                TS_ASSERT( s == _cs_1 );
                TS_ASSERT( 0 == ar.read(&s) );
                TS_ASSERT( s == _cs_2 );
                TS_ASSERT( 0 == ar.read(&s) );
                TS_ASSERT( s == _cs_3 );
                TS_ASSERT( 0 == ar.read(&s) );
                TS_ASSERT( s == _cs_4 );
                TS_ASSERT( 0 == ar.read(&s) );
                TS_ASSERT( s == _cs_5 );

                fs.close();
            }
        }

    }

    void test_append_str(){
        //"" + "" == ""
        {
            assert( string_t().append(string_t()) == string_t() ); 
            assert( string_t(_empty_cs).append(string_t()) == string_t() ); 
            assert( string_t().append(string_t(_empty_cs)) == string_t() ); 
            assert( string_t(_empty_cs).append(string_t(_empty_cs)) == string_t(_empty_cs) ); 

        }

        // "..." + "" == "..."
        {
            assert( string_t(_cs_0).append(string_t()) == string_t(_cs_0) ); 
            assert( string_t().append(string_t(_cs_1)) == string_t(_cs_1) ); 

        }

        // "xxx" + "..." == "xxx..."
        {
            assert( string_t(_cs_0).append(string_t(_cs_1)) == string_t( _cs_0_add_1 ) );

        }
    }
    void test_append_sub_str(){
        //"" + "" == ""
        {
            assert( string_t().append(string_t(), 0, 0) == string_t() ); 
            assert( string_t(_empty_cs).append(string_t(), 0, 1) == string_t() );
            assert( string_t(_empty_cs).append(string_t(), 10, 0) == string_t() );
            assert( string_t(_empty_cs).append(string_t(), 100, 100) == string_t() );

            assert( string_t().append(string_t(_cs_0), _cs_0_len, 0) == string_t() );
            assert( string_t().append(string_t(_cs_0), _cs_0_len, 1) == string_t() );
            assert( string_t().append(string_t(_cs_0), _cs_0_len +1, 0) == string_t() );
            assert( string_t().append(string_t(_cs_0), _cs_0_len +1, 1) == string_t() );

            assert( string_t().append( string_t(_cs_1), 0, 0 ) == string_t() );
            assert( string_t().append( string_t(_cs_1), 1, 0 ) == string_t() );
            assert( string_t().append( string_t(_cs_1), 3, 0 ) == string_t() );
        }

        // "..." + "" == "..."
        {
            assert( string_t(_cs_0).append(string_t(), 0, 0) == string_t(_cs_0) ); 
            assert( string_t(_cs_1).append(string_t(), 0, 1) == string_t(_cs_1) );
            assert( string_t(_cs_2).append(string_t(_empty_cs), 10, 0) == string_t(_cs_2) );
            assert( string_t(_cs_3).append(string_t(_empty_cs),100,100) == string_t(_cs_3) );

            assert( string_t(_cs_4).append(string_t(_cs_0), _cs_0_len, 0) == string_t(_cs_4) );
            assert( string_t(_cs_5).append(string_t(_cs_0), _cs_0_len, 1) == string_t(_cs_5) );
            assert( string_t(_cs_0).append(string_t(_cs_0), _cs_0_len +1, 0) == string_t(_cs_0) );
            assert( string_t(_cs_1).append(string_t(_cs_0), _cs_0_len +1, 1) == string_t(_cs_1) );

            assert( string_t(_cs_2).append( string_t(_cs_1), 0, 0 ) == string_t(_cs_2) );
            assert( string_t(_cs_3).append( string_t(_cs_1), 1, 0 ) == string_t(_cs_3) );
            assert( string_t(_cs_4).append( string_t(_cs_1), 3, 0 ) == string_t(_cs_4) );

            assert( string_t().append(string_t(_cs_0), 0, 1) == string_t(_cs_0, 1) );
            assert( string_t(_empty_cs).append(string_t(_cs_1), 1, 2) == string_t(_cs_1+1, 2) );
            assert( string_t().append(string_t(_cs_0), 0,string_t::npos) == string_t(_cs_0) );
            assert( string_t(_empty_cs).append(string_t(_cs_1), 1) == string_t(_cs_1+1) );
        }

        // "xxx" + "..." == "xxx..."
        {
            assert( string_t(_cs_0).append(string_t(_cs_1), 0 ) == string_t( _cs_0_add_1 ) );
            assert( string_t(_cs_0).append(string_t(_cs_1), 0, 100 ) == _cs_0_add_1 );

            _CharT * ans;
            ans = malloc_clone("bsl_stringcu");
            assert( string_t(_cs_0).append(string_t(_cs_1), 1, 2 ) == ans );
            free(ans);

            ans = malloc_clone("bsl_stringcumon");
            assert( string_t(_cs_0).append(string_t(_cs_1), 1, 100 ) == ans );
            free(ans);

            ans = malloc_clone("bsl_stringcumon");
            assert( string_t(_cs_0).append(string_t(_cs_1), 1, 5 ) == ans );
            free(ans);

            ans = malloc_clone("bsl_stringcumon");
            assert( string_t(_cs_0).append(string_t(_cs_1), 1, 6 ) == ans );
            free(ans);

        }
    }
    void test_append_cstr(){
        //"" + "" == ""
        {
            assert( string_t().append(_empty_cs)== string_t() ); 
            assert( string_t(_empty_cs).append(_empty_cs) == string_t() ); 

        }

        // "..." + "" == "..."
        {
            assert( string_t(_cs_0).append(_empty_cs) == string_t(_cs_0) ); 
            assert( string_t(_empty_cs).append(_cs_1) == string_t(_cs_1) ); 

        }

        // "xxx" + "..." == "xxx..."
        {
            assert( string_t(_cs_0).append(_cs_1) == string_t( _cs_0_add_1 ) );

        }
    }
    void test_append_sub_cstr(){
        //"" + "" == ""
        {
            assert( string_t().append(_empty_cs, size_t(0)) == string_t() ); 
            //assert( string_t(_empty_cs).append(_empty_cs, 1) == string_t() ); 
            //assert( string_t(_empty_cs).append(_empty_cs, 100) == string_t(_empty_cs) ); 

            assert( string_t().append( _cs_1 + 0, size_t(0) ) == string_t() );
            assert( string_t().append( _cs_2 + 1, size_t(0) ) == string_t() );
            assert( string_t().append( _cs_3 + 3, size_t(0) ) == string_t() );
        }

        // "..." + "" == "..."
        {
            assert( string_t(_cs_0).append(_empty_cs, size_t(0) ) == string_t(_cs_0) ); 
            //assert( string_t(_cs_1).append(_empty_cs, 1) == string_t(_cs_1) ); 

            assert( string_t(_cs_2).append( _cs_1 + 0, size_t(0) ) == string_t(_cs_2) );
            assert( string_t(_cs_3).append( _cs_2 + 1, size_t(0) ) == string_t(_cs_3) );
            assert( string_t(_cs_4).append( _cs_3 + 3, size_t(0) ) == string_t(_cs_4) );

            assert( string_t().append(_cs_0, 1) == string_t(_cs_0, 1) );
            assert( string_t(_empty_cs).append(_cs_1+1, 2) == string_t(_cs_1+1, 2) );
            //assert( string_t().append(_cs_0, string_t::npos) == string_t(_cs_0) );
        }

        // "xxx" + "..." == "xxx..."
        {
            assert( string_t(_cs_0).append(_cs_1 ) == string_t( _cs_0_add_1 ) );
            //assert( string_t(_cs_0).append(_cs_1, 100 ) == string_t( _cs_0_add_1 ) );

            _CharT * ans;
            ans = malloc_clone("bsl_stringcu");
            assert( string_t(_cs_0).append(_cs_1 + 1, 2 ) == ans );
            free(ans);

            ans = malloc_clone("bsl_stringcumon");
            //assert( string_t(_cs_0).append(_cs_1 + 1, 100 ) == ans );
            free(ans);

            ans = malloc_clone("bsl_stringcumon");
            assert( string_t(_cs_0).append(_cs_1 + 1, 5 ) == ans );
            free(ans);

            //ans = malloc_clone("bsl_stringcumon");
            //assert( string_t(_cs_0).append(_cs_1 + 1, 6 ) == ans );
            //free(ans);

        }
    }
    void test_append_n_char(){
        //"" + "" == ""
        {
            assert( string_t().append(size_t(0), _CharT('a')) == string_t() ); 
            assert( string_t(_empty_cs).append(size_t(0), _CharT('b')) == string_t() ); 

        }

        // "..." + "" == "..."
        {
            assert( string_t(_cs_0).append(size_t(0), _CharT('c')) == string_t(_cs_0) ); 

        }

        // "xxx" + "..." == "xxx..."
        {
            _CharT * ans;
            ans = malloc_clone("bsl_stringa");
            assert( string_t(_cs_0).append(size_t(1), _CharT('a')) == ans );
            free(ans);

            ans = malloc_clone("bsl_stringbbbb");
            assert( string_t(_cs_0).append(size_t(4), _CharT('b')) == ans );
            free(ans);

            ans = malloc_clone("aaabbbb");
            assert( string_t(3,_CharT('a')).append(size_t(4), _CharT('b')) == ans );
            free(ans);

        }
    }
    void test_append_range(){
        const char char_str[] = "Obama";
        std::list<_CharT> li(char_str, char_str + sizeof(char_str) -1);
        std::string ss(char_str);
        bsl::string bs(char_str);
        string_t string_t_bs(li.begin(), li.end());
        
        //"" + "" == ""
        {
            assert( string_t().append(_cs_0, _cs_0) == string_t() ); 
            std::string empty_s;
            assert( string_t(_empty_cs).append(empty_s.begin(), empty_s.end()) == string_t() ); 
            assert( string_t().append(li.begin(), li.begin()) == string_t() ); 
            assert( string_t(_empty_cs).append(ss.end(), ss.end()) == string_t(_empty_cs) ); 

        }

        // "..." + "" == "..."
        {
            assert( string_t(_cs_0).append(char_str, char_str) == string_t(_cs_0) ); 
            assert( string_t().append(li.begin(), li.end()) == string_t_bs ); 
            assert( string_t().append(ss.begin(), ss.end()) == string_t_bs ); 

        }

        // "xxx" + "..." == "xxx..."
        {
            std::basic_string<_CharT> ss1(_cs_1);
            string_t bs1(_cs_1);
            assert( string_t(_cs_0).append(_cs_1, _cs_1 + 6) == string_t( _cs_0_add_1 ) );
            assert( string_t(_cs_0).append(ss1.begin(), ss1.end()) == string_t( _cs_0_add_1 ) );

        }

    }
    void test_push_back(){
        {
            string_t s;
            _CharT c('a');
            s.push_back(c);
            assert( s == string_t(&c,1) );
        }
        
        {
            string_t s(_cs_3);
            s.push_back('n');
            assert( s == string_t(_cs_1) );
        }

    }

    void test_appendf(){
        {
            //no look up
            const char * fmt = "abc";
            string_t s;
            s.reserve(10);
            s.appendf( fmt );
            assert( s == string_t(fmt,fmt+3) );
        }
        {
            string_t s;
            const char *fmt = "char:%c int:%d char_str:%s wchar_str:%ls\n" ;
            _CharT * ans = malloc_clone( "char:c int:123 char_str:bsl::string! wchar_str:Acumon!\n" );
            s.appendf(fmt, 'c', 123, "bsl::string!", L"Acumon!" );
            assert( s == ans );
            free( ans );
        }
    }

    void test_find(){
        assert( string_t(_cs_0).find( _cs_0 ) == 0 );
        assert( string_t(_cs_1).find( _cs_3 ) == 0 );
        assert( string_t(_cs_0).find( _cs_1 ) == string_t::npos );

        const char* cs1 = "bslacumo";
        const char* cs2 = "acumon";
        string_t s1(cs1, cs1+strlen(cs1));
        string_t s2(cs2, cs2+strlen(cs2));
        assert( s1.find(s2) == string_t::npos );
        assert( s1.find(s2.c_str(), 0, s2.size() - 1) == 3 );
        assert( s1.find(s2.c_str(), 3, s2.size() - 1) == 3 );
        assert( s1.find(s2.c_str(), 4, s2.size() - 1) == string_t::npos );

        assert( s1.find(s1[5]) == 5 );
        assert( s1.find(s1[5],5) == 5 );
        assert( s1.find(s1[5],6) == string_t::npos);
    }
    
    void test_rfind(){
        assert( string_t(_cs_0).rfind( _cs_0 ) == 0 );
        assert( string_t(_cs_1).rfind( _cs_3 ) == 0 );
        assert( string_t(_cs_0).rfind( _cs_1 ) == string_t::npos );

        const char* cs1 = "bslacumo";
        const char* cs2 = "acumon";
        string_t s1(cs1, cs1+strlen(cs1));
        string_t s2(cs2, cs2+strlen(cs2));
        assert( s1.rfind(s2) == string_t::npos );
        assert( s1.rfind(s2.c_str(), 0, s2.size() - 1) == string_t::npos );
        assert( s1.rfind(s2.c_str(), 3, s2.size() - 1) == 3 );
        assert( s1.rfind(s2.c_str(), 4, s2.size() - 1) == 3 );

        assert( s1.rfind(s1[5]) == 5 );
        assert( s1.rfind(s1[5],5) == 5 );
        assert( s1.rfind(s1[5],6) == 5 );
        assert( s1.rfind(s1[5],4) == string_t::npos);

        const char* cs3 = "123b4567";
        const char* cs4 = "3b4";
        string_t str(cs3, cs3+strlen(cs3));
        string_t tmp(cs4, cs4+strlen(cs4));

        assert( str.rfind(tmp) == 2 );
        assert( str.rfind(tmp, 3) == 2 );

    }
    
};

#endif  //__BSL_TEST_STRING_HPP_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
