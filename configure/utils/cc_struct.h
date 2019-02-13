/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: cc_struct.h,v 1.6 2009/11/10 04:50:30 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file cc_struct.h
 * @author yingxiang(com@baidu.com)
 * @date 2008/11/26 20:57:17
 * @version $Revision: 1.6 $ 
 * @brief 
 *  
 **/


#ifndef  __CC_STRUCT_H_
#define  __CC_STRUCT_H_

#include <stdlib.h>

#include <string>
#include <list>
#include <map>
#include "bsl/containers/string/bsl_string.h"

namespace comcfg{


	
	typedef unsigned char comcfg_type_uchar;		  /**< uchar       */
	typedef signed short comcfg_type_int16;		  /**< int16       */
	typedef unsigned short comcfg_type_uint16;		  /**< uint16       */
	typedef signed int comcfg_type_int32;		  /**< int32       */
	typedef unsigned int comcfg_type_uint32;		  /**< uint32       */
	typedef signed long long comcfg_type_int64;		  /**< int64       */
	typedef unsigned long long comcfg_type_uint64;		  /**< uing64       */
	typedef std::string std_string;		  /**< std::string       */
	typedef bsl::string bsl_string;		  /**< bsl::string       */
#define CC_CHAR "char"		  /**< char，出现在约束文件中的类型，下同       */
#define CC_UCHAR "uchar"		  /**< uchar       */
#define CC_INT16 "int16_t"		  /**< int16       */
#define CC_UINT16 "uint16_t"		  /**< uint16       */
#define CC_INT32 "int32_t"		  /**< int32       */
#define CC_UINT32 "uint32_t"		  /**< uint32       */
#define CC_INT64 "int64_t"		  /**< int64       */
#define CC_UINT64 "uint64_t"		  /**< uing64       */
#define CC_FLOAT "float"		  /**< float       */
#define CC_DOUBLE "double"		  /**< double       */
#define CC_STRING "string"		  /**< 实际上是bsl::string       */
	typedef bsl::string str_t;		  /**< 标准的内部string类型       */
	typedef int ErrCode;		  /**< 错误号       */

}



#endif  //__CC_STRUCT_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
