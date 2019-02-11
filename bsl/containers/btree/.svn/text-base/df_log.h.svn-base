///====================================================================
//
// df_log.h - Pyramid / DFS / df-lib
//
// Copyright (C) 2008 Baidu.com, Inc.
//
// Created on 2007-12-27 by Ye Ling (yeling@baidu.com)
//
// -------------------------------------------------------------------
//
// Description
//
//    declaration of common logging lib
//
// -------------------------------------------------------------------
//
// Change Log
//
//    updated on 2008-12-12 by Ye Ling
//
////====================================================================

#ifndef __DF_LOG_H__
#define __DF_LOG_H__

/////////////////////////////////////////////////////////////////////////
//下面这些是不使用ul_log的代码..不会打印任何日志...:).
#include <time.h>

#define DF_LOG_NONE    "DF_NONE"
#define DF_LOG_FATAL   "DF_FATAL"
#define DF_LOG_WARNING "DF_WARNING"
#define DF_LOG_NOTICE  "DF_NOTICE"
#define DF_LOG_TRACE   "DF_TRACE"
#define DF_LOG_DEBUG   "DF_DEBUG"
#define DF_LOG_ALL     "DF_ALL"

#define DF_UL_LOG_NONE		0
#define DF_UL_LOG_FATAL	0x01    /**<   fatal errors */
#define DF_UL_LOG_WARNING	0x02    /**<   exceptional events */
#define DF_UL_LOG_NOTICE   0x04    /**<   informational notices */
#define DF_UL_LOG_TRACE	0x08    /**<   program tracing */
#define DF_UL_LOG_DEBUG	0x10    /**<   full debugging */
#define DF_UL_LOG_ALL		0xff    /**<   everything     */
#define DF_UL_LOG_LEVEL_COUNT 17

const char DF_LOG_LEVEL[DF_UL_LOG_LEVEL_COUNT][16] = {
    DF_LOG_NONE, DF_LOG_FATAL, DF_LOG_WARNING, DF_LOG_NONE, DF_LOG_NOTICE,
    DF_LOG_NONE, DF_LOG_NONE, DF_LOG_NONE, DF_LOG_TRACE, DF_LOG_NONE,
    DF_LOG_NONE, DF_LOG_NONE, DF_LOG_NONE, DF_LOG_NONE, DF_LOG_NONE,
    DF_LOG_NONE, DF_LOG_DEBUG
};

// DF_OPEN_LOG 用于打开日志，和ul_openlog一致，直接替换该函数调用即可
// NOTE: 请保证使用该宏打开日志
#define DF_OPEN_LOG(_logpath_, _logname_, _plogstat_, _log_size_)

// 打开日志，将warning／fatal日志也输出到debug日志中，且使用异步方式写日志文件
#define DF_OPEN_LOG_EX(_logpath_, _logname_, _plogstat_, _log_size_)

#define DF_CLOSE_LOG()

// DF_WRITE_LOG 用于系统日志的记录
// NOTE：对于数据/展现日志请勿使用
#define DF_WRITE_LOG(_loglevel_, _fmt_, args...)

#define DF_WRITE_LOG_DEBUG(_fmt_, args...)

#define DF_WRITE_LOG_EX(_loglevel_, _fmt_, args...)

// DF_WRITE_LOG_NOTICE_US 用于系统日志的记录
// 不打印文件名，函数，行号，增加微秒数
#define DF_WRITE_LOG_NOTICE_US(_loglevel_, _fmt_, args...)

// DF_WRITE_LOG_US 用于系统日志的记录,带微秒数
// NOTE：对于数据/展现日志请勿使用
#define DF_WRITE_LOG_US(_loglevel_, _fmt_, args...)

// AP_WRITE_LOG 用于系统日志的记录
// NOTE：对于数据/展现日志请勿使用
#define AP_WRITE_LOG(_loglevel_, _fmt_, args...)

// AP_WRITE_LOG_DEBUG用于系统调试日志的记录，正式发布会删除
// NOTE: 对于数据/展现日志请勿使用
#define AP_WRITE_LOG_DEBUG(_fmt_, args...)

#define AP_WRITE_LOG_EX(_loglevel_, _fmt_, args...)

// AP_WRITE_LOG_NOTICE_US 用于系统日志的记录
// 不打印文件名，函数，行号，增加微秒数
#define AP_WRITE_LOG_NOTICE_US(_loglevel_, _fmt_, args...)

// AP_WRITE_LOG_US 用于系统日志的记录,带微秒数
// NOTE：对于数据/展现日志请勿使用
#define AP_WRITE_LOG_US(_loglevel_, _fmt_, args...)

#endif // #ifndef __DF_LOG_H__

