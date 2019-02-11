/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: test_check_cast.cpp,v 1.2 2009/03/09 04:56:42 xiaowei Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file test_check_cast.cpp
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/11/11 13:44:18
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/
#include <climits>
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

/*
 * 2010.8.24 以下case在gcc4编译的bsl库，gcc3编译的测试代码下，无法捕捉到异常
 *           gcc4编译的bsl库，gcc4编译的测试代码
 *           gcc3编译的bsl库，gcc3编译的测试代码，使用正常
 *           目前QA的测试框架还无法在gcc4下编译通过
 *   terminate called after throwing an instance of 'bsl::OverflowException'
 *
 */

void test_short_to_const_char() {
    char val[1000];
    for (int i = 0; i < 1000; i ++) {
        val[i] = '1';
    }
    val[999] = '\0';
    try {
        short a = bsl::check_cast<short,const char*>(val);
        printf("%hd\n",a);
    } catch (bsl::Exception& e) {
        printf("%s\n",e.all());
    }
}

void test_to_char(){
    {
        //char
        assert( 'a' == bsl::check_cast<char>('a') );

        //int
        assert( '\0'== bsl::check_cast<char>(0) );
        assert( char(127) == bsl::check_cast<char>(127) );
        assert( char(-128)== bsl::check_cast<char>(-128) );
        ASSERT_THROW( bsl::check_cast<char>(-129), bsl::UnderflowException );
        ASSERT_THROW( bsl::check_cast<char>(128), bsl::OverflowException );

        //unsigned char
        assert( 'c' == bsl::check_cast<char>((unsigned char)('c')));
        assert( char(127) == bsl::check_cast<char>((unsigned char)(127)));
        ASSERT_THROW( bsl::check_cast<char>(((unsigned char)128)), bsl::OverflowException );

        //double
        assert( char(127) == bsl::check_cast<char>(127.0) );
        assert( char(-128)== bsl::check_cast<char>(-128.0) );
        ASSERT_THROW( bsl::check_cast<char>(-129.0), bsl::UnderflowException );
        ASSERT_THROW( bsl::check_cast<char>(128.0), bsl::OverflowException );

        //cstring
        assert( char(0) == bsl::check_cast<char>("") );
        assert( char('c') == bsl::check_cast<char>("cat") );

    }
}

void test_to_short(){
}

void test_to_int(){
    //char
    assert( 0 == bsl::check_cast<int>('\0') );
    assert( -1 == bsl::check_cast<int>(char(-1)));

    //unsigned char
    assert( 0 == bsl::check_cast<int>((unsigned char)0));
    assert( 255 == bsl::check_cast<int>((unsigned char)255));

    //int
    assert( INT_MIN == bsl::check_cast<int>(INT_MIN) );
    assert( INT_MAX == bsl::check_cast<int>(INT_MAX) );

    //long
    if ( sizeof(long) > 4 ){
        assert( INT_MIN == bsl::check_cast<int>(long(INT_MIN)) );
        assert( INT_MAX == bsl::check_cast<int>(long(INT_MAX)) );
        ASSERT_THROW( bsl::check_cast<int>(long(INT_MIN)-1), bsl::UnderflowException );
        ASSERT_THROW( bsl::check_cast<int>(long(INT_MAX)+1), bsl::OverflowException );
    }else{
        assert( INT_MIN == bsl::check_cast<int>(long(INT_MIN)) );
        assert( INT_MAX == bsl::check_cast<int>(long(INT_MAX)) );
    }

    //long long
    assert( INT_MIN == bsl::check_cast<int>((long long)(INT_MIN)) );
    assert( INT_MAX == bsl::check_cast<int>((long long)(INT_MAX)) );
    ASSERT_THROW( bsl::check_cast<int>((long long)(INT_MIN)-1), bsl::UnderflowException );
    ASSERT_THROW( bsl::check_cast<int>((long long)(INT_MAX)+1), bsl::OverflowException );

    //double
    assert( INT_MIN == bsl::check_cast<int>(double(INT_MIN)) );
    assert( INT_MAX == bsl::check_cast<int>(double(INT_MAX)) );
    ASSERT_THROW( bsl::check_cast<int>(double(INT_MIN)-1), bsl::UnderflowException );
    ASSERT_THROW( bsl::check_cast<int>(double(INT_MAX)+1), bsl::OverflowException );

    //cstring

}

void test_to_size_t(){
    assert( ULONG_MAX == bsl::check_cast<size_t>((long long)ULONG_MAX) );
    assert( INT_MAX  == bsl::check_cast<size_t>(int(INT_MAX)) );

}
void test_to_long(){
}

void test_to_long_long(){
}

void test_to_float(){
}

void test_to_double(){
}

void test_to_long_double(){
}
void test_interpret(){
    while(true){
        char buf[1024];
        while(fgets(buf, 1024, stdin)){
            try{
                printf("result: %lld\n", bsl::check_cast<long long>(buf) );
                //bsl::check_cast<int>(int(1));
            }catch(bsl::Exception& e){
                printf("error: %s\n",e.what());
            }
        }
    }
}

int main(){
    test_short_to_const_char();
    test_to_char();
    test_to_short();
    test_to_int();
    test_to_long();
    test_to_long_long();
    test_to_float();
    test_to_double();
    test_to_long_double();
    printf("ok...\n");
    //test_interpret();
    return 0;
}
/* vim: set ts=4 sw=4 sts=4 tw=100 */
