#ifndef __TEST_H___
#define __TEST_H___

#include<cassert>
#include<typeinfo>
#include<iostream>
#include"bsl/containers/string/bsl_string.h"
#include"bsl/var/implement.h"
#include"ITestVar.h"


#define see(x) do{  \
    std::cerr<<__FILE__<<":"<<__LINE__<<": " <<(#x)<<" = "<<(x)<<" ("<<typeid(x).name()<<")"<<std::endl;\
}while(0)

#define stub() do{ \
    std::cerr<<__FILE__<<":"<<__LINE__<<": " <<__PRETTY_FUNCTION__<<": stub! "<<std::endl;\
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

#define ASSERT_NOT_THROW( expr, except )\
    do{                                 \
        bool has_thrown = false;        \
        try{                            \
            expr;                       \
        }catch( except& e ){            \
            has_thrown = true;          \
        }                               \
        assert( has_thrown );           \
    }while(0)

#endif//__TEST_H___
