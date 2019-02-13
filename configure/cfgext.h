/***************************************************************************
 * 
 * Copyright (c) 2009 Baidu.com, Inc. All Rights Reserved
 * $Id: cfgext.h,v 1.2 2009/03/23 03:14:52 yingxiang Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file cfgext.h
 * @author yingxiang(com@baidu.com)
 * @date 2009/03/10 21:39:54
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/


#ifndef  __CFGEXT_H_
#define  __CFGEXT_H_

#include "utils/cc_utils.h"

namespace comcfg{
/**
 * @brief ����һ��Լ���ļ����Զ�����һ���������ļ�
 *        ����ʹ�ã�����RD����һ�������ļ�ģ��
 *
 * @param [in] range_file   : const char* Լ���ļ���ַ
 * @return  bsl::string ���������ļ����ı����ݡ�ֱ��write��ĳ���ļ����ɡ�
 *
 * 			������ʱ�׳�ConfigException�쳣
 *
 * @retval   
 * @see 
 * @author yingxiang
 * @date 2009/03/10 21:42:44
**/
bsl::string autoConfigGen(const char * range_file);









}//namespace comcfg






#endif  //__CFGEXT_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
