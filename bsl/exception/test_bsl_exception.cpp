/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: test_bsl_exception.cpp,v 1.8 2009/10/14 08:24:59 chenxm Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file test_bsl_exception.cpp
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/12/10 10:56:44
 * @version $Revision: 1.8 $ 
 * @brief 
 *  
 **/

#include "bsl_exception.h"
#include <cassert>
#include <memory>   //for std::bad_alloc @ g++ 2.96

#include<typeinfo>
#include<iostream>
#define see(x) do{  \
    std::cerr<<__FILE__<<":"<<__LINE__<<": " <<(#x)<<" = "<<(x)<<" ("<<typeid(x).name()<<")"<<std::endl;\
}while(0)


int main(){
    bsl::Exception::set_stack_trace_level(10);

    bsl::AutoBuffer stack;
    //bsl::Exception::stack_trace( stack );
    //see( stack.c_str() );

    try{
        throw bsl::UnknownException()<<123<<"abc"<<true<<3.14159<<bsl::EXCEPTION_LEVEL_FATAL<<BSL_EARG;

    }catch(bsl::UnknownException& e){
        see(e.name());
        see(e.what());
        see(e.file());
        see(e.line());
        see(e.function());
        see(e.level());
        see(e.level_str());
        see(e.stack());
    }

    try{
        try{
            throw bsl::NullPointerException()<<123;
        }catch( bsl::Exception& e ){
            size_t sz=0;
            e<<456<<sz;
            see( e.what() );
            throw;
        }
    }catch(bsl::NullPointerException& e){
        see( e.what() );
    }

    class WrongException: public bsl::Exception {};  //wrong inheritance
    try{
        throw WrongException()<<BSL_EARG;
    }catch( bsl::Exception& e ){
        see( e.what() );
        see( e.level_str() );
    }
    try{
        try{
            try{
                throw bsl::NullPointerException()<<"haha";
            }catch(bsl::NullPointerException& e){
                assert( 0 == strcmp( e.what(), "haha" ) );
                throw bsl::StdException(e);
            }
        }catch(std::exception& e){
            see( e.what() );
            assert( 0 == strcmp( e.what(), "haha" ) );
            throw;
        }
    }catch(bsl::StdException& e){
        see(e.what());
    }
    try{
        try{
            std::bad_alloc ba;
            see( ba.what());
            throw ba;
        }catch(std::bad_alloc& e){
            see( e.what() );
            throw;
        }
    }catch(std::bad_alloc& e){
        see( e.what() );
    }

/*
    std::bad_alloc ba;
    bsl::StdException se(ba);
    see(se.name());
    see(se.what());
    */
    return 0;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 */
