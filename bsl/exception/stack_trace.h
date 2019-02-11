/**
 * @file stack_trace.h
 * @author zhujianwei(zhujianwei@baidu.com)
 * @date 2010/12/27
 * @version $Revision: 1.0 
 * @brief 
 *  
 **/

#ifndef  __BSL_STACK_TRACE_H__
#define  __BSL_STACK_TRACE_H__

#include "bsl/AutoBuffer.h"

namespace bsl{

    /**
     * @brief 在运行时打印栈信息的工具函数
     * 在连接目标文件时，必须加上-rdynamic参数
     *
     * @param [in] buf   : AutoBuffer& 
     * @param [in] total_level   : size_t 打出的总层
     * @param [in] begin_level   : size_t 跳过栈顶的begin_level层,默认值为不打印stack_trace()本身
     * @return  void 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2008/08/12 20:55:34 / modified by zhujianwei at 2010/12/27
    **/
    void stack_trace( AutoBuffer& buf, size_t total_level, size_t begin_level, size_t max_level,
            const char* line_delimiter );

    /**
     * @brief demangle
     *
     * @param [in] buf   : AutoBuffer&
     * @param [in] mangled_name   : const char*
     * @return  void 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2009/02/04 16:33:13
    **/
    void demangle( AutoBuffer& buf, const char * mangled_name );

}
//namespace bsl 
#endif  //__BSL_STACK_TRACE_H__

