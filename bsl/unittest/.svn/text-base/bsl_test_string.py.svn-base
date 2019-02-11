#!/usr/bin/env python
TEST_CLASS_NAME = 'bsl_test_string'

TEST_TYPES  = [ 'normal' ]

CHAR_TYPES  = [ 'char', 'wchar_t', 'unsigned short', 'unsigned int' ]

ALLOC_TYPES = [ 'bsl::bsl_alloc', 'std::allocator' ]

TEST_METHODS= [
    'test_ctors',
    'test_c_str',
    'test_empty',
    'test_size',
    'test_length',
    'test_capacity',
    'test_operator_square',
    'test_clear',
    'test_reserve',
    'test_swap',
    'test_operator_assign_to_null',
    'test_operator_assign_to_cstring',
    'test_operator_assign_to_string',
    'test_operator_eq_eq',
    'test_operator_not_eq',
    'test_operator_lt',
    'test_operator_gt',
    'test_operator_lt_eq',
    'test_operator_gt_eq',
    'test_operator_shift',
    'test_serialization',
    'test_append_str',
    'test_append_sub_str',
    'test_append_cstr',
    'test_append_sub_cstr',
    'test_append_n_char',
    'test_append_range',
    'test_push_back',
    'test_appendf',
    'test_find',
    'test_rfind'
]

HEADER      = """
#ifndef __BSL_TEST_STRING_H_
#define __BSL_TEST_STRING_H_
#include <cxxtest/TestSuite.h>
#include "bsl_test_string.hpp"
class bsl_test_string_main : public CxxTest::TestSuite{
public:
"""

FOOTER      = """
};
#endif  //__BSL_TEST_STRING_H_
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

