


/**
 * @file bsl_test_string.hpp
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/09/12 17:29:08
 * @version $Revision: 1.10 $ 
 * @brief 
 *  
 **/
#ifndef  __BSL_TEST_SHALLOW_COPY_STRING_HPP_
#define  __BSL_TEST_SHALLOW_COPY_STRING_HPP_
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
class bsl_test_shallow_copy_string{
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
    bsl_test_shallow_copy_string()
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

    ~bsl_test_shallow_copy_string(){
        free( _cs_0 );
        free( _cs_1 );
        free( _cs_0_add_1 );
        free( _cs_2 );
        free( _cs_3 );
        free( _cs_4 );
        free( _cs_5 );
        free( _empty_cs );
    }
        

    void test_shallow_copy()
    {

        const size_t char_size = sizeof(_CharT);
        _Alloc all;
        {
            string_t s(_cs_0, 3,all,true);
            TS_ASSERT( s.size() == 3 );
            TS_ASSERT( 0 == memcmp( s.c_str(), _cs_0, 3*char_size ) );
            // TS_ASSERT( s.c_str()[3] == '\0' );
        }

        {
            string_t s(_cs_0, 3,all,false);
            TS_ASSERT( s.size() == 3 );            
            TS_ASSERT( 0 == memcmp( s.c_str(), _cs_0, 3*char_size ) );
            TS_ASSERT( s.c_str()[3] == '\0' );
        }

        /*                {
                    string_t a(_cs_0);
            string_t s(a,all,false);
            TS_ASSERT( s.size() == a.size() );
            TS_ASSERT( 0 == memcmp( s.c_str(), a.c_str(), a.size() ));
            TS_ASSERT( s.c_str()[s.size()]== '\0' );
        }
        
        
        {
            string_t a(_cs_0);
            string_t s(a,all,true);
            TS_ASSERT( s.size() == a.size() );
            TS_ASSERT( 0 == memcmp( s.c_str(), a.c_str(), a.size() ));
            //TS_ASSERT( s.c_str()[s.size()]== '\0' );
        }
        */
        {
            string_t a(_cs_0);
            string_t s(a);
            TS_ASSERT( s.size() == a.size() );
            TS_ASSERT( 0 == memcmp( s.c_str(),a.c_str(), s.size() ) );
            TS_ASSERT( s.c_str() != a.c_str() );
        }

        {
            string_t a(_cs_0, 3,all,true);
            string_t s(a);
            TS_ASSERT( s.size() == a.size() );
            TS_ASSERT( 0 == memcmp( s.c_str(),a.c_str(), s.size() ) );
            TS_ASSERT( s.c_str() == a.c_str() );
        }


    }
    
};

#endif  //__BSL_TEST_STRING_HPP_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
