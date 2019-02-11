#!/usr/bin/env python
# bsl_test_shared_buffer.h generator
HEADER = """
#ifndef  __BSL_TEST_EXCEPTION_H_
#define  __BSL_TEST_EXCEPTION_H_
#include <cxxtest/TestSuite.h>
#include "bsl/exception/bsl_exception.h"
#include "bsl_test_exception_prepare.h_"

class test_exception : public CxxTest::TestSuite {
public:
"""

FOOTER = """
};
#endif  //__BSL_TEST_EXCEPTION_H_
"""

EXCEPTIONS = [
    "UnknownException",
    "BadCastException",
#   "OutOfBoundException",
#   "KeyNotFoundException",
#   "KeyAlreadyExistException",
#   "BadAllocException",
#   "BadArgumentException",
#   "NullPointerException",
#   "BadFormatStringException",
#   "UninitializedException",
#   "NotImplementedException",
#   "InvalidOperationException",
#   "OverflowException",
#   "UnderflowException",
#   "ParseErrorException",
]

INFO_TEST_TEMPLATE = """
    void test_%(exception)s_info_%(id)s(){
        int line;
        try{
            line = __LINE__, throw bsl::%(exception)s()%(arg_cmd)s%(level_cmd)s%(msg_cmd)s;
        }catch( bsl::%(catch_exception)s& ex ){
            const char * what = "%(what)s";
            bsl::exception_level_t level = bsl::%(level)s;
            const char * level_str  = "%(level_str)s";

            TS_ASSERT( 0 == strcmp( ex.what(), what ) );
#if __GNUC__ <= 2
            TS_ASSERT( 0 != strstr( ex.name(), "%(exception)s" ) );
#else
            TS_ASSERT( 0 == strcmp( ex.name(), "bsl::%(exception)s" ) );
#endif
            TS_ASSERT( 0 != strstr( ex.stack(), __func__ ) );

            TS_ASSERT( 0 == strcmp( ex.file(), __FILE__ ) );
            TS_ASSERT( 0 == strcmp( ex.function(), __PRETTY_FUNCTION__ ) );
            TS_ASSERT( ex.line() == line );

            TS_ASSERT( ex.level() == level );
            TS_ASSERT( 0 == strcmp( ex.level_str(), level_str ) );

        }
    }
"""

DEPTH_TEST_TEMPLATE = """
    void test_%(exception)s_depth_%(id)s(){
        try{
            %(func_name)s<bsl::%(exception)s>(%(depth)d);
        }catch( bsl::%(catch_exception)s& ex ){

            TS_ASSERT( strlen( ex.what() ) == 1000000 || 0 == strcmp( ex.what(), throw_func_what ) );
#if __GNUC__ <= 2
            TS_ASSERT( 0 != strstr( ex.name(), "%(exception)s" ) );
#else
            TS_ASSERT( 0 == strcmp( ex.name(), "bsl::%(exception)s" ) );
#endif
            TS_ASSERT( 0 != strstr( ex.stack(), "%(func_name)s" ) );

            TS_ASSERT( 0 == strcmp( ex.file(), "bsl_test_exception_prepare.h_" ) ); //test BSL_EARG

            TS_ASSERT( ex.level() == throw_func_level );

        }
    }
"""
E2NO_TEST_TEMPLATE = """
    void test_%(exception)s_e2no_%(id)s(){
        {
            int errno = 123;
            BSL_E2NO( errno, call_nothrow() );
            assert( errno == 0 );
        }
        {
            int errno = 123;
            BSL_E2NO( errno, call_throw<bsl::%(exception)s>() );
            assert( errno == -1 );
        }
        try{
            call_throw<bsl::%(exception)s>();
        }catch(bsl::Exception& e){
            see(e.what());
        }
    
        size_t old_level = bsl::Exception::set_stack_trace_level(1);
        printf("bsl::Exception::set_stack_trace_level(1)\\n");
        {
            int errno = 123;
            BSL_E2NO( errno, call_throw<bsl::%(exception)s>() );
            assert( errno == -1 );
        }
        assert( 1 == bsl::Exception::set_stack_trace_level(0) );
        printf("bsl::Exception::set_stack_trace_level(0)\\n");
        {
            int errno = 123;
            BSL_E2NO( errno, call_throw<bsl::%(exception)s>() );
            assert( errno == -1 );
        }
        assert( bsl::Exception::set_stack_trace_level(old_level) == 0 );
        printf("bsl::Exception::set_stack_trace_level(%%d)\\n", int(old_level));
        {
            int errno = 123;
            BSL_E2NO( errno, throw std::out_of_range("this is message") );
            assert( errno == -1 );
        }
        {
            int errno = 123;
            BSL_E2NO( errno, call_throw_int(456) );
            assert( errno == -1 );
        }
        {
            int errno = 123;
            BSL_E2NO( errno, errno = 789 );
            assert( errno == 0 );
        }
    }
"""

TEST_DATA = [{
    'msg_cmd':".push(true)",
    'what':   "true"
},{
    'msg_cmd':".push(false)",
    'what':   "false"
},{
    'msg_cmd':".push(true).push(false).push(true)",
    'what':   "truefalsetrue"
},{
    'msg_cmd':".push(123)",
    'what':   "123"
},{
    'msg_cmd':".push(123).push(-456).push(+789).push(0)",
    'what':   "123-4567890"
},{
    'msg_cmd':".push(123.456).push(-1.2e+100).push(3.14e-50)",
    'what':   "123.456-1.2e+1003.14e-50"
},{
    'msg_cmd':".push('a').push(char(0)).push('b').push(4,'x')",
    'what':   "abxxxx"
},{
    'msg_cmd':'.push("abc")',
    'what':   "abc"
},{
    'msg_cmd':'.push("")',
    'what':   ""
},{
    'msg_cmd':'.push("abc").push("").push("def")',
    'what':   "abcdef"
},{
    'msg_cmd':'.pushf("%d%s",123,"abc").push(true)',
    'what':   "123abctrue"
},{
    'msg_cmd':"<<true",
    'what':   "true"
},{
    'msg_cmd':"<<false",
    'what':   "false"
},{
    'msg_cmd':"<<true<<false<<true",
    'what':   "truefalsetrue"
},{
    'msg_cmd':"<<123",
    'what':   "123"
},{
    'msg_cmd':"<<123<<-456<<+789<<0",
    'what':   "123-4567890"
},{
    'msg_cmd':"<<123.456<<-1.2e+100<<3.14e-50",
    'what':   "123.456-1.2e+1003.14e-50"
},{
    'msg_cmd':"<<'a'<<char(0)<<'b'",
    'what':   "ab"
},{
    'msg_cmd':'<<"abc"',
    'what':   "abc"
},{
    'msg_cmd':'<<""',
    'what':   ""
},{
    'msg_cmd':'<<"abc"<<""<<"def"',
    'what':   "abcdef"
}]

# not testing CORE_DUMP & DEFAULT
LEVELS = [
    'EXCEPTION_LEVEL_DEFAULT',
    'EXCEPTION_LEVEL_SILENT',
#   'EXCEPTION_LEVEL_DEBUG',
#   'EXCEPTION_LEVEL_TRACE',
#   'EXCEPTION_LEVEL_NOTICE',
#   'EXCEPTION_LEVEL_WARNING',
#   'EXCEPTION_LEVEL_FATAL',
]

def get_id_iter():
    id = 0;
    while True:
        #turn 123 in to 'bcd'
        yield ''.join(map(lambda ch: chr(int(ch)+ord('a')), list(str(id))))
        id = id + 1

if __name__ == "__main__":
    id_iter = get_id_iter()

    print HEADER
    #info test
    for exception in EXCEPTIONS:
        for data in TEST_DATA:
            arg_cmd = '<<BSL_EARG' if '<<' in data['msg_cmd'] else '.push(BSL_EARG)'
            for level in LEVELS:
                level_cmd =  '<<bsl::' + level if '<<' in data['msg_cmd'] else ''.join(['.push(bsl::', level, ')'])
                for catch_exception in [ exception, 'Exception' ]:
                    id      = id_iter.next()
                    msg_cmd = data['msg_cmd']
                    what    = data['what']
                    level_str= 'EXCEPTION_LEVEL_DEBUG' if level == 'EXCEPTION_LEVEL_DEFAULT' else level

                    print INFO_TEST_TEMPLATE % vars()

    #depth test
    for exception in EXCEPTIONS:
        for func_name in [ 'throw_func_push', 'throw_func_shift' ]:
            for depth in [ 1, 3, 10000 ]:
                for catch_exception in [ exception, 'Exception' ]:
                    id = id_iter.next()
                    print DEPTH_TEST_TEMPLATE % vars()

    for exception in EXCEPTIONS:
        id = id_iter.next()
        print E2NO_TEST_TEMPLATE % vars()
                
    print FOOTER
