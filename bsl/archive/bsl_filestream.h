/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_filestream.h,v 1.4 2008/12/15 09:56:59 xiaowei Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file bsl_filestream.h
 * @author xiaowei(com@baidu.com)
 * @date 2008/07/25 17:05:19
 * @version $Revision: 1.4 $ 
 * @brief 
 *  
 **/


#ifndef  __BSL_FILESTREAM_H_
#define  __BSL_FILESTREAM_H_

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <bsl/archive/bsl_stream.h>

namespace bsl
{
class filestream : public stream
{
	FILE *_fp;
	int _trans;
	int _trans_off;
	char *_wbuffer;
	size_t _wbuffersize;
	char *_rbuffer;
	size_t _rbuffersize;
public:
	filestream();
	~filestream();
	/**
	 * @brief 文件打开模式
	 *
	 * @param [in/out] fname   : const char* 文件名
	 * @param [in/out] mode   : const char* 文件打开模式, 
	 * 								"w" 写打开
	 * 								"r" 读打开
	 * 								不要尝试读写打开,参数跟FILE一样
	 * @return  int  0表示成功,其他失败
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/11/04 10:24:02
	**/
	int open(const char *fname, const char *mode);
	/**
	 * @brief 关闭文件
	 *
	 * @return  int 
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/11/04 10:26:05
	**/
	int close();
	/**
	 * @brief 往文件里面写数据,如果写入失败,什么都不发生
	 * 要吗写入,要吗回滚到上一个状态
	 *
	 * @param [in/out] dat   : const void* 要写的数据指针
	 * @param [in/out] size   : size_t 写的数据长度
	 * @return  int 0 表示成功
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/11/04 10:26:17
	**/
	int write(const void *dat, size_t size);
	/**
	 * @brief 往文件里面读数据,如果读入失败,什么都不发生
	 * 要吗读入,要吗回滚到上一个状态 
	 *
	 * @param [in/out] dat   : void*   
	 * @param [in/out] size   : size_t
	 * @return  int 0表示成功
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/11/04 10:28:41
	**/
	int read(void *dat, size_t size);
	/**
	 * @brief 开启一个事务
	 *
	 * @return  int 0 开启成功
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/11/04 10:29:49
	**/
	int start_trans();
	/**
	 * @brief 结束一个事务
	 *
	 * @return  int 0表示成功
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/11/04 10:30:08
	**/
	int comit_trans();
	/**
	 * @brief 放弃一个事务
	 *
	 * @param [in] trunc   : bool
	 * 					false : 不将该事务对磁盘的操作丢弃
	 * 					true : 将该事务对磁盘的操作丢弃
	 * @return  int 0表示成功
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/11/04 10:30:31
	**/
	int drop_trans(bool trunc);
	/**
	 * @brief 设置写缓冲区
	 *
	 * @param [in/out] size   : size_t 缓冲区大小为byte
	 * @return  int 成功返回0
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/11/04 10:32:12
	**/
	int setwbuffer(size_t size);
	/**
	 * @brief 设置读缓冲区
	 *
	 * @param [in/out] size   : size_t 缓冲区大小位byte
	 * @return  int 成功返回0
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/11/04 10:33:01
	**/
	int setrbuffer(size_t size);
	/**
	 * @brief 强制将数据刷到硬盘
	 *
	 * @return  int 
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/11/04 10:34:30
	**/
	int flush();
};
};


#endif  //__BSL_FILESTREAM_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
