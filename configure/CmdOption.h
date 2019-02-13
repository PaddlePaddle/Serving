/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: CmdOption.h,v 1.6 2010/04/13 09:59:41 scmpf Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file CmdOption.h
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/29 21:47:53
 * @version $Revision: 1.6 $ 
 * @brief 
 *  
 **/


#ifndef  __CMDOPTION_H_
#define  __CMDOPTION_H_

#include "ConfigGroup.h"
#include "utils/cc_utils.h"

namespace comcfg{
	/**
	 * @brief 与getopt行为不一致的地方：
	 *         如果optstring = "a:b:"
	 *         当：./run -a -b c d
	 *         optstring会把 -b作为-a的参数
	 *         而CmdOption会把c, d堆栈处理，把c给-b，把d给-a
	 *
	 */
	class CmdOption : public ConfigGroup{
		public:
			/**
			 * @brief 命令行参数初始化（不可以重复调用）
			 *
			 * @param [in] argc   : int 参数个数
			 * @param [in] argv   : char*  const* 参数
			 * @param [in] optstring   : const char* 选项信息，与getopt相同
			 * @return  int 0为成功，其它为失败
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2008/12/30 01:47:13
			**/
			int init(int argc, char * const * argv, const char * optstring);
			/**
			 * @brief 命令行参数中是否存在某个选项
			 *
			 * @return  bool 
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2008/12/30 01:48:08
			**/
			bool hasOption(const char *) const;
			/**
			 * @brief 末尾参数个数（注意：不是总的参数个数）
			 * 		如：argc = 5, argv = {"./run", "-a", "xxx", "-h", "yyy", "zzz"}, optstring = "a:h"
			 * 		那么这里的argSize = 3 (分别是./run, yyy和zzz)，其中xxx是-a的参数
			 *
			 * @return  size_t 参数个数
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2008/12/30 01:49:16
			**/
			size_t argSize() const;
			/**
			 * @brief 获取第n个参数
			 *
			 * @param [in] n   : int 参数位置（0 ~ argSize-1）
			 * @return  const char* 
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2008/12/30 01:51:34
			**/
			const char * getArg(int n) const;

			typedef int (*hook_t)(CmdOption * );
			/**
			 * @brief 注册参数钩子，如果存在option参数，runHook时就执行相应的钩子
			 * 			一个option可注册多个钩子
			 *
			 * @param [in] option   : const char*
			 * @param [in] hook   : hook_t
			 * @return  int 
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2008/12/30 01:52:09
			**/
			int registHook(const char * option, hook_t hook);
			/**
			 * @brief 运行钩子
			 *
			 * @return  int 无意义（runHook不管钩子的运行结果）
			 * 				如关心钩子的运行结果，请在钩子中以异常抛出，自行处理
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2008/12/30 01:52:53
			**/
			int runHook();
			virtual  ConfigUnit & operator= (ConfigUnit & unit) {
				return ConfigUnit::operator=(unit);
			}
			CmdOption();
		protected:
			static const int CHAR_TABLE_SIZE = 256;
			char ch[CHAR_TABLE_SIZE];
			static const char CMD_NONE = 0;
			static const char CMD_OPT0 = 1;
			static const char CMD_OPT1 = 2;
			static const char CMD_OPT2 = 3;
			std::vector <str_t> arg;
			std::multimap <str_t, hook_t> hkmap;
			typedef std::pair<str_t, hook_t> pair_t;
			typedef std::multimap<str_t, hook_t> :: iterator hkitr_t;
			typedef std::pair<hkitr_t, hkitr_t> hk_pair_t;

			int setOptString(const char *);
	};
}












#endif  //__CMDOPTION_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
