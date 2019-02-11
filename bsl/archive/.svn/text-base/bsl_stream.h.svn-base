/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_stream.h,v 1.2 2008/09/25 08:06:51 xiaowei Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file bsl_stream.h
 * @author xiaowei(com@baidu.com)
 * @date 2008/09/01 14:25:20
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/


#ifndef  __BSL_STREAM_H_
#define  __BSL_STREAM_H_


namespace bsl
{
class stream
{
public:
	virtual ~stream() {};
	virtual int close() = 0;
	virtual int write(const void *dat, size_t size) = 0;
	virtual int read(void *dat, size_t size) = 0;
	virtual int start_trans() = 0;
	virtual int comit_trans() = 0;
	virtual int drop_trans(bool) = 0;
	virtual int flush() = 0;
};
};















#endif  //__BSL_STREAM_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
