/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: test_ShallowCopystring.cpp,v 1.3 2009/06/15 06:29:05 chenxm Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file test_ShallowCopyString.cpp
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/08/08 10:31:44
 * @version $Revision: 1.3 $ 
 * @brief 
 *  
 **/

#include <cassert>
#include <iostream>
#include "bsl/ShallowCopyString.h"

int main(){
    //todo: more tests, even more tests!
    const char *str = "hello world";
    size_t len = strlen(str);
    bsl::ShallowCopyString ss = str;
    assert( 0 == strcmp(ss.c_str(), str) );
    assert( ss.length() == len );
    assert( ss.size() == len );
    assert( ss.capacity() == len );
    assert( ss == str );
    assert( str == ss );
    assert( ss == bsl::ShallowCopyString(str) );
    assert( ss == bsl::ShallowCopyString(ss) );
    
    std::cout<<bsl::ShallowCopyString("Hello world! I am ShallowCopyString!!!")<<std::endl;

    return 0;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 */
