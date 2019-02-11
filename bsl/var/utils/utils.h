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
         * @brief �ݹ��ӡIVar���ݵ��ַ���
         *
         * @param [in] var   : const IVar&              ����ӡ��IVar�����
         * @param [in] res   : IVar::string_type&       ��ӡ�����ַ���
         * @param [in] verbose_level   : size_t         �ݹ�������
         * @param [in] line_delimiter   : const char*   ���з�
         * @param [in] per_indent   : size_t            ÿ�������ֽ���
         * @param [in] total_indent   : size_t          �������ֽ���
         * @return  void 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2009/05/14 16:50:12
        **/
        void dump_to_string(const IVar& var, IVar::string_type& res, size_t verbose_level, const char *line_delimiter, size_t per_indent = 4, size_t total_indent = 0 );

        /**
         * @brief �ݹ顢������ӡIVar��STDOUT��GDB����ר��
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
