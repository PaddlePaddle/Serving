/***************************************************************************
 * 
 * Copyright (c) 2009 Baidu.com, Inc. All Rights Reserved
 * $Id: test_var_utils.cpp,v 1.3 2010/04/28 12:45:33 scmpf Exp $ 
 * 
 **************************************************************************/
 
 
#include "bsl/var/implement.h"
#include "bsl/var/utils.h"
#include "bsl/var/assign.h"
#include "bsl/var/var_traits.h"

 
/**
 * @file test_var_utils.cpp
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2009/05/07 15:35:38
 * @version $Revision: 1.3 $ 
 * @brief 
 *  
 **/

#include<typeinfo>
#include<iostream>
#define see(x) do{  \
    std::cerr<<__FILE__<<":"<<__LINE__<<": " <<(#x)<<" = "<<(x)<<" ("<<typeid(x).name()<<")"<<std::endl;\
}while(0)

void test_dump_to_string(){
    bsl::var::Int32 i32=123;
    bsl::var::Int64 i64=-100000000000000LL;
    bsl::var::String str= "hello, \"\'123";
    bsl::var::Array arr;
    bsl::var::Dict dict;
    bsl::var::Bool var_true;
    bsl::var::ShallowRaw  raw("ABC\1\2\0",6);
    bsl::var::Double dbl(123.45e78);
    dict["self"] = dict;
    dict["arr"]  = arr;
    dict["null"] = bsl::var::Null::null;
    dict["bool"] = var_true;
    dict["raw"]  = raw;
    dict["double"]=dbl;
    arr[0] = i32;
    arr[1] = i64;
    arr[2] = str;
    
    {
        printf("i32, 0: %s\n", i32.dump(0).c_str() );
    }
    {
        printf("i32, 999: %s\n", i32.dump(999).c_str() );
    }
    {
        printf("dict, 0: %s\n", dict.dump(0).c_str() );
    }
    {
        printf("dict, 1: %s\n", dict.dump(1).c_str() );
    }
    {
        printf("dict, 3: %s\n", dict.dump(3).c_str() );
    }
    {
        bsl::string res;
        dump_to_string( i32, res, 0, "<CR>" );
        printf("i32, 0: %s<CR>", res.c_str() );
    }
    {
        bsl::string res;
        dump_to_string( i32, res, 999, "<CR>" );
        printf("i32, 999: %s<CR>", res.c_str() );
    }
    {
        bsl::string res;
        dump_to_string( dict, res, 0, "<CR>" );
        printf("dict, 0: %s<CR>", res.c_str() );
    }
    {
        bsl::string res;
        dump_to_string( dict, res, 1, "<CR>" );
        printf("dict, 1: %s<CR>", res.c_str() );
    }
    {
        bsl::string res;
        dump_to_string( dict, res, 3, "<CR>" );
        printf("dict, 3: %s<CR>", res.c_str() );
    }
    {
        bsl::string res;
        dump_to_string( i32, res, 0, "\n" );
        printf("i32, 0: %s\n", res.c_str() );
    }
    {
        bsl::string res;
        dump_to_string( i32, res, 999, "\n" );
        printf("i32, 999: %s\n", res.c_str() );
    }
    {
        bsl::string res;
        dump_to_string( dict, res, 0, "\n" );
        printf("dict, 0: %s\n", res.c_str() );
    }
    {
        bsl::string res;
        dump_to_string( dict, res, 1, "\n" );
        printf("dict, 1: %s\n", res.c_str() );
    }
    {
        bsl::string res;
        dump_to_string( dict, res, 3, "\n" );
        printf("dict, 3: %s\n", res.c_str() );
    }
}

void test_assign(){
    bsl::var::Int32 i32(123);
    bsl::var::Int64 i64(456);
    bsl::var::String str("hello");
    //array
    bsl::var::Array arr;
    bsl::var::assign( arr, i32, i64 );
    assert( arr.size() == 2 );
    assert( arr[0].to_int32() == 123 );
    assert( arr[1].to_int64() == 456 );
    bsl::var::assign( arr, str );
    assert( arr.size() == 1 );
    assert( arr[0].to_string() == "hello" );

    //dict
    bsl::var::Dict dict;
    bsl::var::assign( dict, "haha", i32, "hehe", i64 );
    assert( dict.size() == 2 );
    assert( dict["haha"].to_int32() == 123 );
    assert( dict["hehe"].to_int64() == 456 );
    bsl::var::assign( dict, "crazy", str );
    assert( dict.size() == 1 );
    assert( dict["crazy"].to_string() == "hello" );

}
template<typename T>
void test_traits_type(){
    bsl::var::Number<T> v; 
    assert(v.get_mask() == bsl::var::var_traits<T>::MASK);
}

void test_traits(){
    test_traits_type<signed char>();
    test_traits_type<unsigned char>();
    test_traits_type<signed short>();
    test_traits_type<unsigned short>();
    test_traits_type<signed int>();
    test_traits_type<unsigned int>();
    test_traits_type<signed long long>();
    test_traits_type<unsigned long long>();
}

int main(){
    test_dump_to_string();
    test_assign();
    test_traits();
    return 0;
}
/* vim: set ts=4 sw=4 sts=4 tw=100 */
