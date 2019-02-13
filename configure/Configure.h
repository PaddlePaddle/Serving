/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: Configure.h,v 1.15 2010/04/13 09:59:41 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file Configure.h
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/21 17:59:12
 * @version $Revision: 1.15 $ 
 * @brief 新的Configure库
 * 			更多资料请参见：
 * 			http://com.baidu.com/twiki/bin/view/Main/LibConfig
 *  
 **/


#ifndef  __CONFIGURE_H_
#define  __CONFIGURE_H_

#include "ConfigGroup.h"
#include "sys/uio.h"
#include <vector>

#define CONFIG_GLOBAL "GLOBAL"
#define CONFIG_INCLUDE "$include"

namespace confIDL{
	struct idl_t;
}
namespace comcfg{
	const int MAX_INCLUDE_DEPTH = 1;
	class Reader;
	enum{
		CONFIG_VERSION_1_0,	//Config Version 1.0
	};
	/**
	 * @brief 配置句柄，其实是一个全局的[GLOBAL]配置组
	 *
	 * Configure是immutable的，即：无法修改，无法复用
	 *  所以load, rebuild, loadIVar三个接口互斥
	 * 要重新load，必须创建一个新的Configure句柄进行操作 
	 *
	 */
	class Configure : public ConfigGroup{
		public:
			/**
			 * @brief 从指定的文件中载入配置文件和约束文件
			 *
			 * @param [in] path   : const char* 配置文件的路径
			 * @param [in] conf   : const char* 配置文件名
			 * @param [in] range   : const char* 约束文件
			 * @param [in] version   : int 文件格式的版本（暂未使用）
			 * @return  int  0表示成功，其余为失败
			 * 				bsl::CONSTRAINT_ERROR  约束检查错误（如果不关心约束，可无视此错误）
			 * 				bsl::CONFIG_ERROR 配置文件解析错误
			 * 				bsl::ERROR 其它错误
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2008/12/30 01:36:54
			 **/
			int load(const char * path, const char * conf, const char * range = NULL, int version = CONFIG_VERSION_1_0);
			/**
			 * @brief 从指定的文件中载入配置文件和约束文件，并检查是否有group重名情况，最后打印到日志
			 * 			默认行为与load()接口相同
			 *
			 * @param [in] path   : const char* 配置文件的路径
			 * @param [in] conf   : const char* 配置文件名
			 * @param [in] range   : const char* 约束文件
			 * @param [in] dupLevel  : enum GROUP_DUP_LEVEL 指定的重名等级，下列值之一
			 *				0: GROUP_DUP_LEVEL0 不检测是否有重名，与原接口行为相同，不打印
			 *				1: GROUP_DUP_LEVEL1 检测第一级group是否重名
			 *				2: GROUP_DUP_LEVEL2 检测所有深度下group是否重名
			 *				3: GROUP_DUP_LEVEL3 记录所有group名
			 * @param [in] version   : int 文件格式的版本（暂未使用）
			 * @return  int  0表示成功，其他值为失败
			 * 				bsl::CONSTRAINT_ERROR  约束检查错误（如果不关心约束，可无视此错误）
			 * 				bsl::CONFIG_ERROR 配置文件解析错误
			 * 				bsl::ERROR 其它错误
			 *              DUPLICATED_GROUP 有组重名，在GROUP_DUP_LEVEL0和GROUP_DUP_LEVEL3下，正常则返回0
			 * @retval   
			 * @see 
			 * @author linjieqiong 
			 * @date 20012/12/28 11:36:54
			 **/
			int load_ex(const char * path, const char * conf, const char * range = NULL,
					int dupLevel = GROUP_DUP_LEVEL0, int version = CONFIG_VERSION_1_0);
			/**
			 * @brief 从指定的文件中载入配置文件和约束文件，如果存在未被约束的配置项，则打印并报错
			 *
			 * @param [in] path   : const char* 配置文件的路径
			 * @param [in] conf   : const char* 配置文件名
			 * @param [in] range   : const char* 约束文件，如果指定了约束文件，则会在会后打印出未被约束的配置项
			 * @param [in] version   : int 文件格式的版本（暂未使用）
			 * @return  int  0表示成功，其余为失败
			 * 				bsl::CONSTRAINT_ERROR  约束检查错误（不满足约束，存在未被约束的配置项）（如果不关心约束，可无视此错误）
			 * 				bsl::CONFIG_ERROR 配置文件解析错误
			 * 				bsl::ERROR 其它错误
			 * @retval   
			 * @see 
			 * @author linjieqiong
			 * @date 2013/01/06 21:36:54
			 **/
			int load_ex2(const char * path, const char * conf, const char * range = NULL, int version = CONFIG_VERSION_1_0);
			/**
			 * @brief 将数据串行化到一个字节流，可以发网网络
			 * 			如果将这个字节流直接写入磁盘文件，不可以直接load回来，而需要读取后再rebuild
			 * @note 本api只能序列化原始配置文件镜像或ivar，不能反映在内存中对configure对象的修改
			 *			如需序列化实时内容，请使用dump_ex()
			 *
			 * @param [out] size_t * : 输出的字节流长度
			 * @return  char* 输出的字节流
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2008/12/30 01:38:35
			 **/
			char * dump(size_t *);

			/**
			 * @brief 序列化configure对象的实时内存内容
			 *
			 * @param [in] buf	: dump_ex()输出的位置
			 * @param [in] bufsize	: buf的长度
			 * @param [in] restoreComment	: const int
			 *			0 RC_COMMENT 还原配置文件中的注释到输出
			 *			1 RC_NO_COMMENT 不还原配置文件中的注释内容到输出
			 *			2 RC_POSITION 还原的同时标记配置项在配置文件中的位置，格式: 
			 *					#this is the original comment
			 *					#[FILE:LINE]
			 *					[group]
			 *					#[FILE:LINE]
			 *					key : value
			 *
			 * @return int 写入buf的数据长度
			 * 			如果发生错误，返回-1
			 *			如果buf不够，返回实际长度的相反数
			 *			自动添加\0
			 *
			 * @author linjieqiong
			 * @version 1.2.9
			 * @date 2013/04/24 14:56:31
			 */
			int dump_ex(char *buf, const size_t bufsize,  const int restoreComment = 0);

			/**
			 * @brief 根据dump的字节流创建Configure
			 * 		根据字节流重建一个Configure的步骤是：
			 * 		1，getRebuildBuffer(size) 获取一个内部缓冲区
			 * 		2，将字节流中的数据拷贝到这个缓冲区
			 * 		3，调用rebuild重建数据
			 *
			 * 		调用本接口会清空现有的数据。
			 *
			 * @param [in] size   : size_t 字节流长度
			 * @return  char* 缓冲区
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2008/12/30 01:39:38
			 **/
			char * getRebuildBuffer(size_t size);
			/**
			 * @brief 根据获得的字节流重建Configure数据
			 *
			 * @return  int 0为成功，其他为失败
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2008/12/30 01:43:05
			 **/
			int rebuild();
			/**
			 * @brief 检查所有配置文件的最后更新时间（包括被$include）的配置文件
			 *        $include 限制层数为：MAX_INCLUDE_DEPTH
			 *        在执行lastConfigModify()，应确保原来的配置文件都没有被删除
			 *
			 * @return  time_t 所有配置文件中的最后更新时间
			 *                 返回0表示出错
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2009/03/10 14:23:47
			**/
			time_t lastConfigModify();
			/**
			 * @brief 从一个IVar载入配置文件的数据
			 *        本接口与load/rebuild互斥
			 *        这个IVar必须是一个Dict类型
			 *
			 * @return  int 0成功，其余失败
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2009/03/11 17:29:56
			**/
			int loadIVar(const bsl::var::IVar & );
			/**
			 * @brief 用约束文件检查一次
			 * @param [in] range   : const char* 约束文件
			 * @param [in] version   : int 文件格式的版本（暂未使用）
			 * @return  int  0表示成功，其余为失败
			 * 				bsl::CONSTRAINT_ERROR  约束检查错误
			 * 				bsl::ERROR 其它错误
			 **/
			int check_once(const char * range, int version = CONFIG_VERSION_1_0);
			/**
			 * @brief 检查conf文件中所有unit是否被约束，如果存在未被约束的项，则打印并返回错误
			 *
			 * @return  int 0 成功，其他是错误号
			 * 				bsl::CONSTRAINT_ERROR 约束检查错误
			 * @author linjieqiong
			 * @date 2013/01/07 17:32:10
 			 **/
			int checkConstraint();
			/**
			 * @brief 获取一个key的约束函数列表，注意group和array类型的约束函数是无作用的
			 * @param [in] key_path : const char* 该key在conf文件中的完整路径 
			 * @param [in] range   : const char* 约束文件，
			 * 				如果非空，先进行约束检查；
			 *				如果为空，将使用最近一次load类函数或check_once()函数所使用的约束文件
			 *
			 * @return  int 0 成功，其他是错误号
			 * 				bsl::CONSTRAINT_ERROR  约束检查错误
			 * 				bsl::ERROR 其它错误
			 * 
			 * @author linjieqiong
			 * @date 2013/01/10 10:50:00 
 			 **/
			int printKeyConstraint(const char *key_path, const char *range = NULL);

			virtual  ConfigUnit & operator= (ConfigUnit & unit) {
				return ConfigUnit::operator=(unit);
			}

			Configure();
			~Configure();
		protected:

			int load_1_0(const char * path, const char * conf, const char *range);
			void pushSubReader(const char * conf, int level = -1);
			friend class Reader;
			friend class Constraint;
			friend class ConfigGroup;
			friend class ConfigUnit;
			friend struct dump_conf;
			friend int cb_dumpConf(ConfigUnit *, void *);

			struct ReaderNode{//每一个conf文件
				str_t filename;
				Reader * reader;
				int level;
			};
			std::vector <ReaderNode *> _readers;
			str_t _path;
			Reader * _cur_reader;//当前在处理的文件
			int _cur_level;//当前文件的深度($include的层次)
			char * _dump_buffer;
			size_t _dump_size;
			confIDL::idl_t * _idl;
			void changeSection(str_t str);
			void pushPair(const str_t& key, const str_t& value);
			
			ConfigGroup* _section;
			//depth是一个调试变量
			int _depth;
	};
}



#endif  //__CONFIGURE_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
