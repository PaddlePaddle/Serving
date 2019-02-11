#!/usr/bin/env python
TEST_CLASS_NAME = 'bsl_test_shallow_copy_string'

TEST_TYPES  = [ 'normal' ]

CHAR_TYPES  = [ 'char', 'wchar_t', 'unsigned short', 'unsigned int' ]

ALLOC_TYPES = [ 'bsl::bsl_alloc', 'std::allocator' ]

TEST_METHODS= [
    'test_shallow_copy'
]

HEADER      = """
#ifndef __BSL_TEST_SHALLOW_COPY_STRING_H_
#define __BSL_TEST_SHALLOW_COPY_STRING_H_
#include <cxxtest/TestSuite.h>
#include "bsl_test_shallow_copy_string.hpp"
class bsl_test_string_main : public CxxTest::TestSuite{
public:
"""

FOOTER      = """
};
#endif  //__BSL_TEST_SHALLOW_COPY_STRING_H_
"""

FUNCTION_TEMPLATE    = """
    void test_%(test_type)s_%(escape_char_type)s_%(escape_alloc_type)s_%(escape_test_method)s(){
        return %(TEST_CLASS_NAME)s<%(char_type)s, %(alloc_type)s<%(char_type)s> >().%(escape_test_method)s();
    }
"""

def escape(s):
    return s.replace(' ','_').replace('::','_')

if __name__ == '__main__':
    print HEADER, 
    for test_type in TEST_TYPES:
        for char_type in CHAR_TYPES:
            escape_char_type = escape(char_type)
            for alloc_type in ALLOC_TYPES:
                escape_alloc_type = escape(alloc_type)
                for test_method in TEST_METHODS:
                    escape_test_method = escape(test_method)
                    print FUNCTION_TEMPLATE % locals()
    print FOOTER,

