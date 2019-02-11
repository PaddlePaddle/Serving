#!/usr/bin/env python
# vim: set fileencoding=utf-8
# bsl_test_auto_buffer.h generator
HEADER = """
#ifndef  __BSL_TEST_AUTO_BUFFER_H_
#define  __BSL_TEST_AUTO_BUFFER_H_
#include <cxxtest/TestSuite.h>
#include <bsl/AutoBuffer.h>
#include <bsl/pool/bsl_xcompool.h>
#include<typeinfo>
#include<iostream>
#define see(x) do{  \
    std::cerr<<__FILE__<<":"<<__LINE__<<": " <<(#x)<<" = "<<(x)<<" ("<<typeid(x).name()<<")"<<std::endl;\
}while(0)

#define CXM_ASSERTF( expr, fmt, arg... ) do{   \
    static char __CXM_MSG__[1024];  \
    TSM_ASSERT( (snprintf( __CXM_MSG__, 1024, fmt, ##arg ), __CXM_MSG__ ), expr )  \
}while(0)

bsl::xmempool g_xmempool;
char g_membuf[1024];

class test_AutoBuffer_main : public CxxTest::TestSuite {
public:
"""

FOOTER = """
};
#endif  //__BSL_TEST_AUTO_BUFFER_H_
"""

TEST_DATA = [
   #[ "{type}", {value} ],or
   #[ "{type}", {literal_value}", {string_value} ]
    [ "bool", "true" ],
    [ "bool", "false" ],

    [ "char", "'c'", "c" ],
    
    [ "int", "0" ],
    [ "int", "123" ],
    [ "int", "-456" ],

    [ "double", "0" ],
    [ "double", "-1e+100" ],

    [ "const char * ", '""', "" ],
    [ "const char * ", '"hello world"', "hello world" ],
    [ "const char * ", '"込込"', "込込" ],

    [ "wchar_t", "L'c'", "c" ],
    [ "const wchar_t *", 'L"hello"', "hello" ],

    [ "unsigned char", "99", "99" ],
    [ "signed char", "127", "127" ],
    [ "signed char", "-128", "-128" ]
]

TEST_TEMPLATE = """
    void test_single_%(func_name)s(){
        bsl::AutoBuffer buf(%(buf_size)d);
        TS_ASSERT( buf.empty() );
        TS_ASSERT( !buf.truncated() );

        if ( %(buf_size)d != 0 ){
            TS_ASSERT( !buf.full() );
        }else{
            TS_ASSERT( buf.full() );
        }

%(operation)s


        if ( 0 != strcmp( result, buf.c_str() ) ){
            see(buf.capacity());
            see(buf.size());
            see(strlen(result));
            see(buf.c_str());
            see(result);
        }
        TS_ASSERT( buf.size()  == strlen(result) );
        TS_ASSERT( buf.capacity() >= buf.size() );
        TS_ASSERT( 0 == strcmp( buf.c_str(), result ) );
        TS_ASSERT( !buf.truncated() );

        if ( buf.size() != 0 ){
            TS_ASSERT( !buf.empty() );
        }else{
            TS_ASSERT( buf.empty() );
        }

        if ( buf.size() != buf.capacity() ){
            TS_ASSERT( !buf.full() );
        }else{
            TS_ASSERT( buf.full() );
        }

        size_t old_capacity = buf.capacity();
        buf.clear();
        TS_ASSERT( buf.size() == 0 );
        TS_ASSERT( old_capacity == buf.capacity() );
        TS_ASSERT( buf.empty() );
        if ( old_capacity ){
            TS_ASSERT( !buf.full() );
        }

    }
"""

TEST_TEMPLATE2 = """
    void test_single_%(func_name)s(){
        // init pool
        bsl::xcompool g_xcompool;
        g_xcompool.create();
        bsl::AutoBuffer buf(g_xcompool,%(buf_size)d);
        TS_ASSERT( buf.empty() );
        TS_ASSERT( !buf.truncated() );

        if ( %(buf_size)d != 0 ){
            TS_ASSERT( !buf.full() );
        }else{
            TS_ASSERT( buf.full() );
        }

%(operation)s


        if ( 0 != strcmp( result, buf.c_str() ) ){
            see(buf.capacity());
            see(buf.size());
            see(strlen(result));
            see(buf.c_str());
            see(result);
        }
        TS_ASSERT( buf.size()  == strlen(result) );
        TS_ASSERT( buf.capacity() >= buf.size() );
        TS_ASSERT( 0 == strcmp( buf.c_str(), result ) );
        TS_ASSERT( !buf.truncated() );

        if ( buf.size() != 0 ){
            TS_ASSERT( !buf.empty() );
        }else{
            TS_ASSERT( buf.empty() );
        }

        if ( buf.size() != buf.capacity() ){
            TS_ASSERT( !buf.full() );
        }else{
            TS_ASSERT( buf.full() );
        }

        size_t old_capacity = buf.capacity();
        buf.clear();
        TS_ASSERT( buf.size() == 0 );
        TS_ASSERT( old_capacity == buf.capacity() );
        TS_ASSERT( buf.empty() );
        if ( old_capacity ){
            TS_ASSERT( !buf.full() );
        }

    }
"""


TEST_TEMPLATE3 = """
    void test_single_%(func_name)s(){
        // init pool
        g_xmempool.create(g_membuf,sizeof(g_membuf));
        bsl::AutoBuffer buf(g_xmempool,%(buf_size)d);
        TS_ASSERT( buf.empty() );
        TS_ASSERT( !buf.truncated() );

        if ( %(buf_size)d != 0 ){
            TS_ASSERT( !buf.full() );
        }else{
            TS_ASSERT( buf.full() );
        }

%(operation)s


        if ( 0 != strcmp( result, buf.c_str() ) ){
            see(buf.capacity());
            see(buf.size());
            see(strlen(result));
            see(buf.c_str());
            see(result);
        }
        TS_ASSERT( buf.size()  == strlen(result) );
        TS_ASSERT( buf.capacity() >= buf.size() );
        TS_ASSERT( 0 == strcmp( buf.c_str(), result ) );
        TS_ASSERT( !buf.truncated() );

        if ( buf.size() != 0 ){
            TS_ASSERT( !buf.empty() );
        }else{
            TS_ASSERT( buf.empty() );
        }

        if ( buf.size() != buf.capacity() ){
            TS_ASSERT( !buf.full() );
        }else{
            TS_ASSERT( buf.full() );
        }

        size_t old_capacity = buf.capacity();
        buf.clear();
        TS_ASSERT( buf.size() == 0 );
        TS_ASSERT( old_capacity == buf.capacity() );
        TS_ASSERT( buf.empty() );
        if ( old_capacity ){
            TS_ASSERT( !buf.full() );
        }
        g_xmempool.clear();

    }
"""


OPERATION_TEMPLATES = [
"""
        %(type)s value = %(literal)s;
        buf.push(value);
        const char * result = "%(string)s";
""",

"""
        %(type)s value = %(literal)s;
        buf<<value;
        const char * result = "%(string)s";
""",

"""
        %(type)s value = %(literal)s;
        buf.push(value).push(value).push(value);
        const char * result = "%(string)s%(string)s%(string)s";
""",

"""
        %(type)s value = %(literal)s;
        buf<<value<<value<<value;;
        const char * result = "%(string)s%(string)s%(string)s";
"""
]

SPECIAL_OPERATIONS = [
"""
        buf.pushf( "abc[%d]\\n%s", 123, "acumon" );
        const char * result = "abc[123]\\nacumon";
""",

"""
        buf.pushf( "abc[%d]\\n%s", 123, "acumon" ).pushf( "abc[%d]\\n%s", 123, "acumon" ).pushf( "abc[%d]\\n%s", 123, "acumon" );
        const char * result = "abc[123]\\nacumonabc[123]\\nacumonabc[123]\\nacumon";
""",

"""
        buf.push( 3, 'a');
        const char * result = "aaa";
""",

"""
        buf.push( 0, 'a');
        const char * result = "";
""",

"""
        buf.push( "auto_buffer", 4 );
        const char * result = "auto";
""",

"""
        buf.push( "auto_buffer", 0 );
        const char * result = "";
""",


"""
        buf.push( "", 0 );
        const char * result = "";
""",

"""
        buf.push( "abc", 2 );
        const char * result = "ab";
""",

"""
        buf.push( (const char *)(NULL) );
        const char * result = "";
""",

"""
        buf.push( (const char *)(NULL), 100 );
        const char * result = "";
""",

"""
        buf << char(0);
        const char * result = "";
""",

"""
        buf.push( char(0) );
        const char * result = "";
""",

"""
        buf.push( 10, char(0) );
        const char * result = "";
""",

"""
        buf.push( 123 );
        buf.clear();
        const char * result = "";
""",

"""
        buf.clear();
        const char * result = "";
""",

"""
        bsl::AutoBuffer buf1;
        buf1.push(123);
        buf.transfer_from(buf1);
        const char * result = "123";
""",

"""
        bsl::AutoBuffer buf1;
        buf.transfer_from(buf1);
        const char * result = "";
""",

"""
        buf.push(123);
        bsl::AutoBuffer buf1;
        buf1.transfer_from(buf);
        const char * result = "";
""",


"""
        bsl::AutoBuffer buf1;
        buf1.push(123);
        buf.transfer_from(buf1);
        buf1.push(123);
        buf.swap(buf1);
        const char * result = "123";
""",

"""
        bsl::AutoBuffer buf1;
        buf.transfer_from(buf1);
        buf1.push(123);
        buf.swap(buf1);
        buf1.transfer_from(buf);
        const char * result = "";
""",


"""
        buf.push(123);
        bsl::AutoBuffer buf1;
        buf1.transfer_from(buf);
        buf.push(123);
        const char * result = "123";
"""



]

def get_id_iter():
    id = 0;
    while True:
        #turn 123 in to 'bcd'
        yield ''.join(map(lambda ch: chr(int(ch)+ord('a')), list(str(id))))
        id = id + 1


if __name__ == "__main__":
    print HEADER

    id_iter = get_id_iter()
    for buf_size in [0, 1, 100]:
        for data in TEST_DATA:
            type, literal, string = data[0], data[1], len(data) > 2 and data[2] or data[1]
            for operation_tmpl in OPERATION_TEMPLATES:
                operation = operation_tmpl % vars()
                func_name = id_iter.next()
                print TEST_TEMPLATE % vars()

        for operation in SPECIAL_OPERATIONS:
            func_name = id_iter.next()
            print TEST_TEMPLATE % vars()
    
    for buf_size in [0, 1, 100]:   
        for data in TEST_DATA:
            type, literal, string = data[0], data[1], len(data) > 2 and data[2] or data[1]
            for operation_tmpl in OPERATION_TEMPLATES:
                operation = operation_tmpl % vars()
                func_name = id_iter.next()
                print TEST_TEMPLATE2 % vars()

        for operation in SPECIAL_OPERATIONS:
            func_name = id_iter.next()
            print TEST_TEMPLATE2 % vars()

    
    for buf_size in [0, 1, 100]:   
        for data in TEST_DATA:
            type, literal, string = data[0], data[1], len(data) > 2 and data[2] or data[1]
            for operation_tmpl in OPERATION_TEMPLATES:
                operation = operation_tmpl % vars()
                func_name = id_iter.next()
                print TEST_TEMPLATE3 % vars()

        for operation in SPECIAL_OPERATIONS:
            func_name = id_iter.next()
            print TEST_TEMPLATE3 % vars()


    print FOOTER
