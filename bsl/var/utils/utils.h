/***************************************************************************
 * 
 * Copyright (c) 2009 Baidu.com, Inc. All Rights Reserved
 * $Id: utils.h,v 1.3 2010/09/08 $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file utils.h
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2010/09/08 modified by Zhu Jianwei
 * @version $Revision: 1.3 $ 
 * @brief 
 *  
 **/
#ifndef  __UTILS_H_
#define  __UTILS_H_
#include "bsl/var/IVar.h"
#include "bsl/var/assign.h"
#include "bsl/var/var_traits.h"
#include "bsl/var/IRef.h"

namespace bsl{
    namespace var{
        /**
         * @brief 递归打印IVar数据到字符串
         *
         * @param [in] var   : const IVar&              被打印的IVar根结点
         * @param [in] res   : IVar::string_type&       打印到的字符串
         * @param [in] verbose_level   : size_t         递归最大层数
         * @param [in] line_delimiter   : const char*   分行符
         * @param [in] per_indent   : size_t            每次缩进字节数
         * @param [in] total_indent   : size_t          总缩进字节数
         * @return  void 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/05/14 16:50:12
        **/
        void dump_to_string(const IVar& var, IVar::string_type& res, size_t verbose_level, const char *line_delimiter, size_t per_indent = 4, size_t total_indent = 0 );

        /**
         * @brief 递归、缩进打印IVar到STDOUT，GDB调试专用
         *
         * @param [in] var   : const IVar&
         * @param [in] verbose_level   : size_t
         * @return  void 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/05/14 16:54:52
        **/
        void print( const IVar& var, size_t verbose_level );      
    }
}
#endif  //__UTILS_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
