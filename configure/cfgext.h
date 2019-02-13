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
 * @brief 根据一个约束文件，自动生成一个空配置文件
 *        线下使用，便于RD生成一份配置文件模板
 *
 * @param [in] range_file   : const char* 约束文件地址
 * @return  bsl::string 返回配置文件的文本内容。直接write到某个文件即可。
 *
 * 			当出错时抛出ConfigException异常
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
