/**
 * @file stack_trace.cpp
 * @author zhujianwei(zhujianwei@baidu.com)
 * @date 2010/12/27
 * @version $Revision: 1.0 
 * @brief 
 *  
 **/

#include<execinfo.h>
#include<typeinfo>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include"bsl/AutoBuffer.h"
#include"stack_trace.h"

namespace bsl{

#if defined(__cplusplus) && (!defined(__GXX_ABI_VERSION) || __GXX_ABI_VERSION < 100) //for g++ 2.96 on nasdaq
    void stack_trace( AutoBuffer& buf, size_t total_level, size_t begin_level, size_t max_level,
            const char* line_delimiter ){
        if ( begin_level >= max_level || total_level == 0 ){
            //begin level is too large or no need to print
            return;
        }
        if ( begin_level + total_level > max_level ){
            total_level = max_level - begin_level;
        }

        void * array[begin_level + total_level];
        int levels = backtrace(array, begin_level + total_level);   //levels是可以拿到的层数
        char ** symbols = backtrace_symbols(array, levels);
        if ( symbols == NULL ){
            //无法获取符号信息
            return ;
        }
        for (int i = begin_level; i < levels; i++) {
            buf.push("  ").push(symbols[i]).push(line_delimiter);
        }

        free(symbols);
    }

    void demangle( AutoBuffer& buf, const char * mangled_name ){
        //TODO: 增加能兼容g++2.96的demangle版本。
        if ( mangled_name ){
            buf.push(mangled_name);
        }
    }

#else //for newer versions, like g++ 3.4.5 on Friday, which supports C++ ABI
#include <cxxabi.h>

    void stack_trace( AutoBuffer& buf, size_t total_level, size_t begin_level, size_t max_level,
            const char * line_delimiter){
        if ( begin_level >= max_level || total_level == 0 ){
            //begin level is too large or no need to print
            return;
        }
        if ( begin_level + total_level > max_level ){
            total_level = max_level - begin_level;
        }

        void * array[begin_level + total_level];
        int levels = backtrace(array, int(begin_level + total_level));   //levels是可以拿到的层数
        char ** symbols = backtrace_symbols(array, levels);
        if ( symbols == NULL ){
            //无法获取符号信息
            return;
        }
        char *func = NULL;
        size_t size = 0;
        int status;

        for (int i = int(begin_level); i < levels; i++) {

            buf.push('\t');
            char * mangled_func = strchr( symbols[i], '(' );
            char * offset_pos = NULL;
            char * frame_ptr = NULL;

            if ( mangled_func != NULL ){
                ++ mangled_func;
                
                offset_pos = strchr( mangled_func, '+' );
                if ( offset_pos != NULL ){
                    *offset_pos = 0;
                    ++offset_pos;
                    
                    frame_ptr = strchr( offset_pos, '[' );
                    // if func == NULL, or size is not enough, func will be malloc/realloced.
                    // new_func is returned, size is set to new size.
                    // new_func can be NULL, then status can't be 0( meaning success)
                    // then you MUST free func!
                    char * new_func = abi::__cxa_demangle(mangled_func,func,&size,&status);
                    if ( new_func ){
                        func = new_func;
                    }
                    //func is NULL(status != 0) or malloced by __cxa_demangle now

                    //output
                    buf.push( symbols[i], mangled_func-symbols[i]-1 ).push(' ');
                    if ( status == 0 ){
                        buf.push( func );
                    }else{
                        buf.push( mangled_func ).push('(').push(')');
                    }

                    if ( frame_ptr != NULL ){
                        buf.push(' ').push(frame_ptr);
                    }

                    buf.push(line_delimiter);
                }else{
                    buf.push(symbols[i]).push(line_delimiter);
                }
            }else{
                buf.push(symbols[i]).push(line_delimiter);
            }

        }        
        if ( func ){
            free(func);
        }
        free(symbols);
    }

    void demangle( AutoBuffer& buf, const char * mangled_name ){
        size_t size = 0;
        int    status = 0;

        // demangled name is returned, size is set to new size.
        char * demangled_name  = abi::__cxa_demangle(mangled_name,NULL, &size,&status);
        // now name can be NULL, then status can't be 0( meaning success)

        if ( status != 0 ){
            if(demangled_name){
                free(demangled_name);
            }

            buf.push(mangled_name);
        }else{

            buf.push(demangled_name);
            free(demangled_name);
        }
    }
#endif 
} //namespace bsl

