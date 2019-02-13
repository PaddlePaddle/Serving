/***************************************************************************
 * 
 * Copyright (c) 2009 Baidu.com, Inc. All Rights Reserved
 * $Id: cfgflag.h,v 1.3 2009/08/13 07:23:01 scmpf Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file init.h
 * @author xiaowei(com@baidu.com)
 * @date 2009/02/12 13:57:53
 * @version $Revision: 1.3 $ 
 * @brief 
 *  
 **/


#ifndef  __XAUTOFRAME_H_
#define  __XAUTOFRAME_H_

#include <bsl/containers/string/bsl_string.h>
#include "Configure.h"
#include "CmdOption.h"

namespace comcfg
{

extern const char *DEFHELP;

class Flag
{

	typedef void (*fun_t)();

	bsl::string _modname;
	bsl::string _cfpath;
	bsl::string _cffile;
	bsl::string _cfrange;

	comcfg::CmdOption _opt;
	comcfg::Configure _cfg;

	fun_t _version;
	fun_t _help;
public:
	/**
	 * @brief 初始化类
	 *
	 * @param [in/out] ver   : fun_t	显示版本的函数
	 * @param [in/out] help   : fun_t	显示帮助的函数
	 * @see 
	 * @author xiaowei
	 * @date 2009/03/09 15:26:51
	**/
	Flag(fun_t ver=Flag::default_version, fun_t help=Flag::default_help);

	virtual ~Flag();

	/**
	 * @brief 根据void main(int argc, void **argv)初始化flag
	 *
	 * @param [in/out] argc   : int    
	 * @param [in/out] argv   : char**
	 * @param [in/out] optstr   : const char*	与CmdOption的第三个参数一致
	 * @return  int 成功返回0，其他失败
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2009/03/09 15:28:22
	**/
	virtual int init(int argc, char **argv, const char *optstr=NULL);

	/**
	 * @brief 返回获取命令行参数的控制句柄
	 *
	 * @return  const comcfg::CmdOption& 
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2009/03/09 15:29:36
	**/
	inline const comcfg::CmdOption & opt() const { return _opt; }
	/**
	 * @brief  返回获取命令行参数的控制句柄 
	 *
	 * @return  comcfg::CmdOption& 
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2009/03/09 15:30:23
	**/
	inline comcfg::CmdOption & opt() { return _opt; }
	/**
	 * @brief 根据key返回这个key对应的命令行参数值
	 *
	 * @param [in/out] str   : const char* 指定的key
	 * @return  const comcfg::ConfigUnit& 
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2009/03/09 15:31:41
	**/
	inline const comcfg::ConfigUnit & opt(const char *str) const { return _opt[str]; }

	/**
	 * @brief  返回配置文件句柄
	 *
	 * @return  const comcfg::Configure& 
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2009/03/09 15:30:42
	**/
	inline const comcfg::Configure & cfg() const { return _cfg; }
	/**
	 * @brief 返回配置文件句柄
	 *
	 * @return  comcfg::Configure& 
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2009/03/09 15:31:16
	**/
	inline comcfg::Configure & cfg() { return _cfg; }

	/**
	 * @brief 根据key返回指定的配置文件项目
	 *
	 * @param [in/out] str   : const char*
	 * @return  const comcfg::ConfigUnit& 
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2009/03/09 15:32:25
	**/
	inline const comcfg::ConfigUnit & cfg(const char *str) const { return _cfg[str]; }

	/**
	 * @brief 设置配置路径
	 *
	 * @param [in/out] cfpath   : const bsl::string&
	 * @return  void 
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2009/03/09 15:43:21
	**/
	inline void set_cfpath(const bsl::string &__cfpath) { _cfpath = __cfpath; }
	/**
	 * @brief 返回配置路径
	 *
	 * @return  const char* 
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2009/03/09 15:43:34
	**/
	inline const char * cfpath() const { return _cfpath.c_str(); }

	/**
	 * @brief 设置配置文件名
	 *
	 * @param [in/out] cffile   : const bsl::string&
	 * @return  void 
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2009/03/09 15:43:42
	**/
	inline void set_cffile(const bsl::string &__cffile) { _cffile = __cffile; }
	/**
	 * @brief 返回配置文件名
	 *
	 * @return  const char* 
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2009/03/09 15:43:50
	**/
	inline const char * cffile() const { return _cffile.c_str(); }

	/**
	 * @brief 设置约束文件名
	 *
	 * @param [in/out] cfrange   : const bsl::string&
	 * @return  void 
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2009/03/09 15:44:00
	**/
	inline void set_cfrange(const bsl::string &__cfrange) { _cfrange = __cfrange; }
	/**
	 * @brief 返回约束文件名
	 *
	 * @return  const char* 
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2009/03/09 15:44:13
	**/
	inline const char *cfrange() const { return _cfrange.c_str(); }

	/**
	 * @brief 设置配置文件信息
	 *
	 * @param [in/out] path   : const char*	配置文件路径
	 * @param [in/out] file   : const char*	配置文件名
	 * @param [in/out] range   : const char*	约束文件名
	 * @return  void 
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2009/03/09 15:44:23
	**/
	void set_confpath(const char *path, const char *file, const char *range=NULL);

	/**
	 * @brief 设置显示版本函数
	 *
	 * @param [in/out] fun   : fun_t
	 * @return  void 
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2009/03/09 15:50:01
	**/
	inline void set_version(fun_t fun) { _version = fun; }
	/**
	 * @brief 设置显示帮助函数
	 *
	 * @param [in/out] fun   : fun_t
	 * @return  void 
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2009/03/09 15:50:13
	**/
	inline void set_help(fun_t fun) { _help = fun; }

	/**
	 * @brief 读取配置
	 *
	 * @return  int 成功返回0，其他失败
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2009/03/09 15:50:21
	**/
	virtual int loadconfig();
	/**
	 * @brief 根据指定的配置文件名读取配置
	 *
	 * @param [in/out] path   : const char* 配置路径
	 * @param [in/out] conf   : const char*	配置文件名
	 * @return  int 
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2009/03/09 15:51:05
	**/
	virtual int loadconfig(const char *path, const char *conf);

protected:
	static void default_version();
	static void default_help();

	/**
	 * @brief 根据配置文件名，获取默认的约束文件名
	 *
	 * @param [in/out] file   : const bsl::string&
	 * @return  bsl::string 
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2009/03/09 15:52:10
	**/
	bsl::string get_defrange(const bsl::string &file);
	bsl::string get_defconf(const bsl::string &mod);
	void auto_def();
	bool file_exist(const char *fn);
	bsl::string fullpath(const bsl::string &path, const bsl::string &file);
};
}

#endif  //__XAUTOFRAME_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
