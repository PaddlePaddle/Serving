/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: IBinarySerializer.h,v 1.3 2009/06/15 06:29:05 chenxm Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file IBinarySerializer.h
 * @author duchuanying(duchuanying@baidu.com)
 * @date 2008/11/01 21:12:18
 * @version $Revision: 1.3 $ 
 * @brief 
 *  
 **/


#ifndef  __BSL_VAR_IBINARY_SERIALIZER_H__
#define  __BSL_VAR_IBINARY_SERIALIZER_H__

#include "bsl/var/IVar.h"
namespace bsl{
namespace var{
	class IBinarySerializer{
	public:
		virtual ~IBinarySerializer(){}

		virtual size_t serialize(const IVar& var, void* buf, size_t max_size) = 0;
	};
}}   //namespace bsl::var

#endif  //__BSL_VAR_IBINARY_SERIALIZER_H__

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
