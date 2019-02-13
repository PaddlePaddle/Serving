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
	 * @brief ��ʼ����
	 *
	 * @param [in/out] ver   : fun_t	��ʾ�汾�ĺ���
	 * @param [in/out] help   : fun_t	��ʾ�����ĺ���
	 * @see 
	 * @author xiaowei
	 * @date 2009/03/09 15:26:51
	**/
	Flag(fun_t ver=Flag::default_version, fun_t help=Flag::default_help);

	virtual ~Flag();

	/**
	 * @brief ����void main(int argc, void **argv)��ʼ��flag
	 *
	 * @param [in/out] argc   : int    
	 * @param [in/out] argv   : char**
	 * @param [in/out] optstr   : const char*	��CmdOption�ĵ���������һ��
	 * @return  int �ɹ�����0������ʧ��
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2009/03/09 15:28:22
	**/
	virtual int init(int argc, char **argv, const char *optstr=NULL);

	/**
	 * @brief ���ػ�ȡ�����в����Ŀ��ƾ��
	 *
	 * @return  const comcfg::CmdOption& 
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2009/03/09 15:29:36
	**/
	inline const comcfg::CmdOption & opt() const { return _opt; }
	/**
	 * @brief  ���ػ�ȡ�����в����Ŀ��ƾ�� 
	 *
	 * @return  comcfg::CmdOption& 
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2009/03/09 15:30:23
	**/
	inline comcfg::CmdOption & opt() { return _opt; }
	/**
	 * @brief ����key�������key��Ӧ�������в���ֵ
	 *
	 * @param [in/out] str   : const char* ָ����key
	 * @return  const comcfg::ConfigUnit& 
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2009/03/09 15:31:41
	**/
	inline const comcfg::ConfigUnit & opt(const char *str) const { return _opt[str]; }

	/**
	 * @brief  ���������ļ����
	 *
	 * @return  const comcfg::Configure& 
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2009/03/09 15:30:42
	**/
	inline const comcfg::Configure & cfg() const { return _cfg; }
	/**
	 * @brief ���������ļ����
	 *
	 * @return  comcfg::Configure& 
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2009/03/09 15:31:16
	**/
	inline comcfg::Configure & cfg() { return _cfg; }

	/**
	 * @brief ����key����ָ���������ļ���Ŀ
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
	 * @brief ��������·��
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
	 * @brief ��������·��
	 *
	 * @return  const char* 
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2009/03/09 15:43:34
	**/
	inline const char * cfpath() const { return _cfpath.c_str(); }

	/**
	 * @brief ���������ļ���
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
	 * @brief ���������ļ���
	 *
	 * @return  const char* 
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2009/03/09 15:43:50
	**/
	inline const char * cffile() const { return _cffile.c_str(); }

	/**
	 * @brief ����Լ���ļ���
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
	 * @brief ����Լ���ļ���
	 *
	 * @return  const char* 
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2009/03/09 15:44:13
	**/
	inline const char *cfrange() const { return _cfrange.c_str(); }

	/**
	 * @brief ���������ļ���Ϣ
	 *
	 * @param [in/out] path   : const char*	�����ļ�·��
	 * @param [in/out] file   : const char*	�����ļ���
	 * @param [in/out] range   : const char*	Լ���ļ���
	 * @return  void 
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2009/03/09 15:44:23
	**/
	void set_confpath(const char *path, const char *file, const char *range=NULL);

	/**
	 * @brief ������ʾ�汾����
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
	 * @brief ������ʾ��������
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
	 * @brief ��ȡ����
	 *
	 * @return  int �ɹ�����0������ʧ��
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2009/03/09 15:50:21
	**/
	virtual int loadconfig();
	/**
	 * @brief ����ָ���������ļ�����ȡ����
	 *
	 * @param [in/out] path   : const char* ����·��
	 * @param [in/out] conf   : const char*	�����ļ���
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
	 * @brief ���������ļ�������ȡĬ�ϵ�Լ���ļ���
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
