#!/usr/bin/env python
INF = float('inf')

SUFFIX = {
    'long': 'L',
    'long long': 'LL',
    'unsigned int': 'U',
    'unsigned long': 'UL',
    'unsigned long long': 'ULL',
    'float': '.0',
    'double': '.0',
    'long double': '.0L',
    'size_t': 'UL',
    #'ptrdiff_t': 'L'
}

MIN_32 = { 
    'char': -2**7, 'signed char': -2**7, 'short': -2**15, 'int': -2**31, 'long': -2**31, 'long long': -2**63,
    'unsigned char': 0, 'unsigned short': 0, 'unsigned int': 0, 'unsigned long': 0, 'unsigned long long': 0,
    'size_t': 0, #'ptrdiff_t': -2**31
#    'float': -INF, 'double': -INF, 'long double': -INF
}

MAX_32 = {
    'char': 2**7-1, 'signed char' : 2**7-1, 'short': 2**15-1, 'int': 2**31-1, 'long': 2**31-1, 'long long': 2**63-1,
    'unsigned char': 2**8-1, 'unsigned short': 2**16-1, 'unsigned int': 2**32-1, 'unsigned long': 2**32-1, 'unsigned long long': 2**64-1,
    'size_t': 2**32-1, #'ptrdiff_t': 2**31-1
#    'float': INF, 'double': INF, 'long double': INF
}

MIN_64 = dict(MIN_32)
MIN_64['ptrdiff_t'] = MIN_64['long'] = MIN_64['long long']

MAX_64 = dict(MAX_32)
MAX_64['ptrdiff_t'] = MAX_64['long'] = MAX_64['long long']
MAX_64['size_t'] = MAX_64['unsigned long'] = MAX_64['unsigned long long']

INT_TYPES = MIN_32.keys()

FLOAT_TYPES = [ 'float', 'double', 'long double' ]

HEADER = """
#include <cassert>
#include "bsl/check_cast.h"

#include<typeinfo>
#include<iostream>
#define see(x) do{  \
    std::cerr<<__FILE__<<":"<<__LINE__<<": " <<(#x)<<" = "<<(x)<<" ("<<typeid(x).name()<<")"<<std::endl;\
}while(0)

#define ASSERT_THROW( expr, except )    \
    do{                                 \
        bool has_thrown = false;        \
        try{                            \
            expr;                       \
        }catch( except& ){              \
            has_thrown = true;          \
        }                               \
        assert( has_thrown );           \
    }while(0)


int main(){
"""

FOOTER = """
    return 0;
}
"""

MIN_TEMPLATE = """
    assert( %(min)s == bsl::check_cast<%(dest_type)s>( static_cast<%(src_type)s>(%(min)s) ) ) ;
    assert( %(min)s + %(offset)s == bsl::check_cast<%(dest_type)s>( static_cast<%(src_type)s>(%(min)s + %(offset)s) ) ) ;
"""

MIN_FLOAT_TEMPLATE = """
    {
        //avoid floating-point number errors
        %(dest_type)s value1 = static_cast<%(dest_type)s>( static_cast<%(src_type)s>( %(min)s ) );
        %(dest_type)s value2 = bsl::check_cast<%(dest_type)s>( static_cast<%(src_type)s>(%(min)s ) );
        assert( value1 == value2 );
    }
    {
        //avoid floating-point number errors
        %(dest_type)s value1 = static_cast<%(dest_type)s>( static_cast<%(src_type)s>( %(min)s + %(offset)s ) );
        %(dest_type)s value2 = bsl::check_cast<%(dest_type)s>( static_cast<%(src_type)s>(%(min)s + %(offset)s) );
        assert( value1 == value2 );
    }
"""

UNDERFLOW_TEMPLATE = """
    ASSERT_THROW( bsl::check_cast<%(dest_type)s>( static_cast<%(src_type)s>(%(min)s - %(offset)s) ), bsl::UnderflowException );
"""

MAX_TEMPLATE = """
    assert( %(max)s - %(offset)s== bsl::check_cast<%(dest_type)s>( static_cast<%(src_type)s>(%(max)s - %(offset)s) ) ) ;
    assert( %(max)s == bsl::check_cast<%(dest_type)s>( static_cast<%(src_type)s>(%(max)s) ) ) ;
"""

MAX_FLOAT_TEMPLATE = """
    {
        //avoid floating-point number errors
        %(dest_type)s value1 = static_cast<%(dest_type)s>( static_cast<%(src_type)s>( %(max)s - %(offset)s ) );
        %(dest_type)s value2 = bsl::check_cast<%(dest_type)s>( static_cast<%(src_type)s>(%(max)s - %(offset)s) );
        assert( value1 == value2 );
    }
    {
        //avoid floating-point number errors
        %(dest_type)s value1 = static_cast<%(dest_type)s>( static_cast<%(src_type)s>( %(max)s ) );
        %(dest_type)s value2 = bsl::check_cast<%(dest_type)s>( static_cast<%(src_type)s>(%(max)s ) );
        assert( value1 == value2 );
    }
"""

OVERFLOW_TEMPLATE = """
    ASSERT_THROW( bsl::check_cast<%(dest_type)s>( static_cast<%(src_type)s>(%(max)s + %(offset)s) ), bsl::OverflowException );
"""

MIN_CSTR_TEMPLATE = """
    assert( %(min)s == bsl::check_cast<%(dest_type)s>( const_cast<const char*>("%(min_str)s") ) ) ;
    assert( %(min)s == bsl::check_cast<%(dest_type)s>( const_cast<char *>("%(min_str)s") ) ) ;
"""

MIN_CSTR_FLOAT_TEMPLATE = """
    {
        //avoid floating-point number errors
        %(dest_type)s value1 = %(min)s;
        %(dest_type)s value2 = bsl::check_cast<%(dest_type)s>( const_cast<const char *>("%(min_str)s") ) ;
        %(dest_type)s value3 = bsl::check_cast<%(dest_type)s>( const_cast<char *>("%(min_str)s") ) ;
        assert( value1 == value2 );
        assert( value1 == value3 );
    }
"""

UNDERFLOW_CSTR_TEMPLATE = """
    ASSERT_THROW( bsl::check_cast<%(dest_type)s>( const_cast<const char *>("%(min_str)s") ), bsl::UnderflowException ) ;
    ASSERT_THROW( bsl::check_cast<%(dest_type)s>( const_cast<char *>("%(min_str)s") ), bsl::UnderflowException ) ;
"""

MAX_CSTR_TEMPLATE = """
    assert( %(max)s == bsl::check_cast<%(dest_type)s>( const_cast<const char *>("%(max_str)s") ) ) ;
    assert( %(max)s == bsl::check_cast<%(dest_type)s>( const_cast<char *>("%(max_str)s") ) ) ;
"""

MAX_CSTR_FLOAT_TEMPLATE = """
    {
        //avoid floating-point number errors
        %(dest_type)s value1 = %(max)s;
        %(dest_type)s value2 = bsl::check_cast<%(dest_type)s>( const_cast<const char *>("%(max_str)s") );
        %(dest_type)s value3 = bsl::check_cast<%(dest_type)s>( const_cast<char *>("%(max_str)s") );
        assert( value1 == value2 );
        assert( value1 == value3 );
    }
"""

OVERFLOW_CSTR_TEMPLATE = """
    ASSERT_THROW( bsl::check_cast<%(dest_type)s>( const_cast<const char *>("%(max_str)s") ), bsl::OverflowException );
    ASSERT_THROW( bsl::check_cast<%(dest_type)s>( const_cast<char *>("%(max_str)s") ), bsl::OverflowException );
"""


TO_BSL_STR_TEMPLATE = """
    assert( "%(res)s" == bsl::check_cast<bsl::string>( static_cast<%(src_type)s>(%(value)s) ) ) ;
"""

def literal(num):
    return str(num) + ( 'ULL' if num >= MAX_32['long long'] else '.0L' if num <= MIN_32['long long'] else 'LL' ) ;

if __name__ == "__main__":
    print HEADER
    print '#if __WORDSIZE == 32'    
    for (MIN, MAX) in [ (MIN_32, MAX_32), (MIN_64, MAX_64) ]:
        if MIN == MIN_64:
            print '#else'
        # INT_TYPES -> INT_TYPES
        for dest_type in INT_TYPES:
            for src_type in INT_TYPES:
                min = literal(MIN[dest_type])
                max = literal(MAX[dest_type]) 
                offset = '1'
                print ''.join(['\t\t//', src_type, ' -> ', dest_type])
                if MIN[src_type] <= MIN[dest_type] and MIN[dest_type] != -INF:
                    print MIN_TEMPLATE % locals()
                if MIN[src_type] < MIN[dest_type]:
                    print UNDERFLOW_TEMPLATE% locals()
                if MAX[src_type] >= MAX[dest_type] and MAX[dest_type] != INF:
                    print MAX_TEMPLATE % locals()
                if MAX[src_type] > MAX[dest_type]:
                    print OVERFLOW_TEMPLATE % locals()

        # FLOAT_TYPES -> INT_TYPES
        for dest_type in INT_TYPES:
            for src_type in FLOAT_TYPES:
                min = literal(MIN[dest_type])
                max = literal(MAX[dest_type]) 
                offset = str(MAX[dest_type]*0.001)
                print ''.join(['\t\t//', src_type, ' -> ', dest_type])
                print MIN_FLOAT_TEMPLATE % locals()
                print UNDERFLOW_TEMPLATE% locals()
                #some has Overflow problem caused by floating-point errors
                #print MAX_FLOAT_TEMPLATE % locals()
                print OVERFLOW_TEMPLATE % locals()

        #special for Overflow problems
        max = literal(MAX[dest_type]) 
        offset = '1'
        dest_type = 'float'
        for src_type in [ 'char', 'unsigned char', 'short', 'unsigned short' ]:
            print MAX_FLOAT_TEMPLATE % locals()

        dest_type = 'double'
        for src_type in INT_TYPES:
            if src_type not in [ 'long long', 'unsigned long long' ]:
                print MAX_FLOAT_TEMPLATE % locals()

        dest_type = 'long double'
        for src_type in INT_TYPES:
            if src_type not in [ 'unsigned long long' ]:
                print MAX_FLOAT_TEMPLATE % locals()


        # INT_TYPES -> FLOAT_TYPES
        # always pass
        for src_type in INT_TYPES:
            for dest_type in FLOAT_TYPES:
                min = literal(MIN[src_type])
                max = literal(MAX[src_type]) 
                offset = 1
                print MIN_FLOAT_TEMPLATE % locals()
                print MAX_FLOAT_TEMPLATE % locals()

        # FLOAT_TYPES -> FLOAT_TYPES
        for src_type in FLOAT_TYPES:
            for dest_type in FLOAT_TYPES:
                min = literal(MIN['long long'])
                max = literal(MAX['long long']) 
                offset = 1
                print MIN_FLOAT_TEMPLATE % locals()
                print MAX_FLOAT_TEMPLATE % locals()

        # C string -> INT_TYPES except CHAR_TYPES
        for dest_type in INT_TYPES:
            if dest_type not in ['char']:
                min = literal(MIN[dest_type])
                max = literal(MAX[dest_type])
                min_str = min
                max_str = max
                print MIN_CSTR_TEMPLATE % locals()
                print MAX_CSTR_TEMPLATE % locals()
                min_str = str(MIN[dest_type]) #no suffix
                max_str = str(MAX[dest_type]) #no suffix
                print MIN_CSTR_TEMPLATE % locals()
                print MAX_CSTR_TEMPLATE % locals()

                min = literal(MIN[dest_type] + 1)
                max = literal(MAX[dest_type] - 1)
                min_str = min
                max_str = max
                print MIN_CSTR_TEMPLATE % locals()
                print MAX_CSTR_TEMPLATE % locals()
                min_str = str(MIN[dest_type] + 1) #no suffix
                max_str = str(MAX[dest_type] - 1) #no suffix
                print MIN_CSTR_TEMPLATE % locals()
                print MAX_CSTR_TEMPLATE % locals()

                min = literal(MIN[dest_type] - 1)
                max = literal(MAX[dest_type] + 1)
                min_str = min
                max_str = max
                print UNDERFLOW_CSTR_TEMPLATE % locals()
                print OVERFLOW_CSTR_TEMPLATE % locals()
                min_str = str(MIN[dest_type] - 1) #no suffix
                max_str = str(MAX[dest_type] + 1) #no suffix
                print UNDERFLOW_CSTR_TEMPLATE % locals()
                print OVERFLOW_CSTR_TEMPLATE % locals()
    
        # C string -> CHAR_TYPES
        for dest_type in ['char']:            
            min = "'0'"            
            max = "'0'"
            min_str = "0"
            max_str = "0"
            print MIN_CSTR_TEMPLATE % locals()
            print MAX_CSTR_TEMPLATE % locals()

            min = "'\\0'"
            max = "'\\0'"
            min_str = "\\0"
            max_str = "\\0"
            print MIN_CSTR_TEMPLATE % locals()
            print MAX_CSTR_TEMPLATE % locals()

        # C string -> FLOAT_TYPES
        for dest_type in FLOAT_TYPES:
            min = str(MIN['int'])+'LL'
            max = str(MAX['int'])+'LL'
            min_str = min
            max_str = max
            print MIN_CSTR_FLOAT_TEMPLATE % locals()
            print MAX_CSTR_FLOAT_TEMPLATE % locals()
            min_str = str(MIN['int'])
            max_str = str(MAX['int'])
            print MIN_CSTR_FLOAT_TEMPLATE % locals()
            print MAX_CSTR_FLOAT_TEMPLATE % locals()

        # INT_TYPES -> bsl::string
        for src_type in INT_TYPES:
            if src_type not in [ 'char' ]:
                value = literal(MIN[src_type])
                res   = str(MIN[src_type])
                print TO_BSL_STR_TEMPLATE % locals()
                value = literal(MIN[src_type] + 1)
                res   = str(MIN[src_type] + 1)
                print TO_BSL_STR_TEMPLATE % locals()
                value = literal(MAX[src_type])
                res   = str(MAX[src_type])
                print TO_BSL_STR_TEMPLATE % locals()
                value = literal(MAX[src_type] - 1)
                res   = str(MAX[src_type] - 1)
                print TO_BSL_STR_TEMPLATE % locals()

        # FLOAT_TYPES -> bsl::string
        for src_type in FLOAT_TYPES:
            value = res = "-1234.5"
            print TO_BSL_STR_TEMPLATE % locals()
            value = res = "1.234e+12"
            print TO_BSL_STR_TEMPLATE % locals()

        # CHAR_TYPES -> bsl::string
        src_type = 'char'
        value = "'A'"
        res = "A"
        print TO_BSL_STR_TEMPLATE % locals()

    print '#endif'
    print FOOTER
        





