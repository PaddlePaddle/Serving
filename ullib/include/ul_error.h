/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: ul_error.h,v 1.3 2009/12/29 03:24:00 baonh Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file ul_error.h
 * @author xiaowei(xiaowei@baidu.com)
 * @version $Revision: 1.3 $ 
 * @brief 
 *  
 **/
#ifndef _UL_ERROR_H
#define _UL_ERROR_H
enum {
    UL_SUCCESS = 0,             //处理成功
    UL_DETAIL_ERR,              //细节错误，ul内部将错误信息打印到线程缓冲区去
    UL_ALLOC_ERR,               //内存分配错误
    UL_MEMSEG,                  //内存越界
    UL_READ_ERR,                //io读失败
    UL_WRITE_ERR,               //io写失败
    UL_INVALID_FD,              //无效的句柄
    UL_NETCONNET_ERR,           //网络连接失败
    UL_INVALID_PARAM,           //无效的传入参数
    UL_OPEN_FILE_ERR,           //打开文件句柄失败
};
/**
 * @brief 设置错误号
 *
 * @param [in] err   : int  要设置的错误号代码
 * @return  int 成功返回0，其他失败
 * @retval   
 * @see 
 * @author xiaowei
 * @date 2007/12/03 21:13:41
**/
int ul_seterrno(int err);
/**
 * @brief 获取错误号
 *
 * @return  int 返回错误号代码
 * @retval   
 * @see 
 * @author xiaowei
 * @date 2007/12/03 21:14:21
**/
int ul_geterrno();
/**
 * @brief 根据错误号获取错误信息
 *
 * @return  const char* 返回错误号的错误描述
 * @param [in] err : int 错误号代号
 * @retval   
 * @see 
 * @author xiaowei
 * @date 2007/12/05 13:56:12
**/
const char *ul_geterrstr(int err);
/**
 * @brief 设置错误提示
 *
 * @param [in] format   : const char* 设置错误信息
 * @return  int 设置成功返回0，其他失败
 * @retval   
 * @see 
 * @author xiaowei
 * @date 2007/12/03 21:14:45
**/
int ul_seterrbuf(const char *format, ...);
/**
 * @brief 获取提示信息缓冲区
 *
 * @return  const char* 成功返回出错缓冲区，
 *                      失败返回""字符串
 *                      不会返回NULL指针
 * @retval   
 * @see 
 * @author xiaowei
 * @date 2007/12/03 21:15:28
**/
const char *ul_geterrbuf();
#endif
/* vim: set ts=4 sw=4 tw=100 noet: */
/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
