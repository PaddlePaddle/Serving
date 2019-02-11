/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_exception.cpp,v 1.8 2009/10/14 08:24:58 chenxm Exp $ 
 * 
 **************************************************************************/

/**
 * @file Exception.cpp
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/08/04 16:44:56
 * @version $Revision: 1.8 $ 
 * @brief 
 *  
 **/

#include<execinfo.h>
#include<typeinfo>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include"bsl_exception.h"

namespace bsl{

    size_t Exception::_s_stack_trace_level = Exception::DEFAULT_STACK_TRACE_LEVEL;
    const char * const ExceptionBase::DEFAULT_FILE       = "<unknown-file>";
    const char * const ExceptionBase::DEFAULT_FUNCTION   = "<unknown-function>";
    const char * const Exception::DEFAULT_LINE_DELIMITER = "<CR>";
    char               Exception::_s_line_delimiter[Exception::_S_LINE_DELIMITER_SIZE]  = "<CR>";

    const char * to_cstring(exception_level_t level){
        if ( level >= EXCEPTION_LEVEL_CORE_DUMPED ){
            return EXCEPTION_LEVEL_CORE_DUMPED_CSTRING;
        }
        if ( level >= EXCEPTION_LEVEL_FATAL ){
            return EXCEPTION_LEVEL_FATAL_CSTRING;
        }
        if ( level >= EXCEPTION_LEVEL_WARNING ){
            return EXCEPTION_LEVEL_WARNING_CSTRING;
        }
        if ( level >= EXCEPTION_LEVEL_NOTICE ){
            return EXCEPTION_LEVEL_NOTICE_CSTRING;
        }
        if ( level >= EXCEPTION_LEVEL_TRACE ){
            return EXCEPTION_LEVEL_TRACE_CSTRING;
        }
        if ( level >= EXCEPTION_LEVEL_DEBUG ){
            return EXCEPTION_LEVEL_DEBUG_CSTRING;
        }
        if ( level >= EXCEPTION_LEVEL_SILENT ){
            return EXCEPTION_LEVEL_SILENT_CSTRING;
        }
        return EXCEPTION_LEVEL_UNKNOWN_CSTRING;
    }    

    //not considered inlining
    void Exception::core_dump(){
        int tmp = 0;
        tmp = 0 / tmp;  // core dumped
    }
    
    const char * Exception::all() const{
        if ( _all.empty() ){
            if ( _name.empty() ){
                demangle( _name, typeid(*this).name() );
            }
            _all.reserve( 160 + _msg.size() + _stack.size() ); 
            _all<<"exception["<<_name<<"] was thrown at["<<_s_line_delimiter<<'\t'<<_arg._file<<':'<<_arg._line<<':'<<_arg._function
                <<_s_line_delimiter<<"] with message["<<_msg<<"] stack:"<<_s_line_delimiter<<_stack;
        }
        return _all.c_str();
    }

}//namespace bsl
/* vim: set ts=4 sw=4 sts=4 tw=100 */
