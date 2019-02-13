/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: init.cpp,v 1.3 2009/02/13 15:25:31 yingxiang Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file init.cpp
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/21 16:04:17
 * @version $Revision: 1.3 $ 
 * @brief 
 *  
 **/

#include "cc_log.h"
#include "ul_log.h"

namespace comcfg{
	const int NO_LOG = 0x7fff;
	int Log::_fatal = UL_LOG_FATAL;
	int Log::_warning = UL_LOG_WARNING;
	int Log::_notice = NO_LOG;
	int Log::_trace = NO_LOG;
	int Log::_debug = NO_LOG;
}



















/* vim: set ts=4 sw=4 sts=4 tw=100 */
