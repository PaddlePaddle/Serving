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
 * @brief �µ�Configure��
 * 			����������μ���
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
	 * @brief ���þ������ʵ��һ��ȫ�ֵ�[GLOBAL]������
	 *
	 * Configure��immutable�ģ������޷��޸ģ��޷�����
	 *  ����load, rebuild, loadIVar�����ӿڻ���
	 * Ҫ����load�����봴��һ���µ�Configure������в��� 
	 *
	 */
	class Configure : public ConfigGroup{
		public:
			/**
			 * @brief ��ָ�����ļ������������ļ���Լ���ļ�
			 *
			 * @param [in] path   : const char* �����ļ���·��
			 * @param [in] conf   : const char* �����ļ���
			 * @param [in] range   : const char* Լ���ļ�
			 * @param [in] version   : int �ļ���ʽ�İ汾����δʹ�ã�
			 * @return  int  0��ʾ�ɹ�������Ϊʧ��
			 * 				bsl::CONSTRAINT_ERROR  Լ�����������������Լ���������Ӵ˴���
			 * 				bsl::CONFIG_ERROR �����ļ���������
			 * 				bsl::ERROR ��������
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2008/12/30 01:36:54
			 **/
			int load(const char * path, const char * conf, const char * range = NULL, int version = CONFIG_VERSION_1_0);
			/**
			 * @brief ��ָ�����ļ������������ļ���Լ���ļ���������Ƿ���group�������������ӡ����־
			 * 			Ĭ����Ϊ��load()�ӿ���ͬ
			 *
			 * @param [in] path   : const char* �����ļ���·��
			 * @param [in] conf   : const char* �����ļ���
			 * @param [in] range   : const char* Լ���ļ�
			 * @param [in] dupLevel  : enum GROUP_DUP_LEVEL ָ���������ȼ�������ֵ֮һ
			 *				0: GROUP_DUP_LEVEL0 ������Ƿ�����������ԭ�ӿ���Ϊ��ͬ������ӡ
			 *				1: GROUP_DUP_LEVEL1 ����һ��group�Ƿ�����
			 *				2: GROUP_DUP_LEVEL2 ������������group�Ƿ�����
			 *				3: GROUP_DUP_LEVEL3 ��¼����group��
			 * @param [in] version   : int �ļ���ʽ�İ汾����δʹ�ã�
			 * @return  int  0��ʾ�ɹ�������ֵΪʧ��
			 * 				bsl::CONSTRAINT_ERROR  Լ�����������������Լ���������Ӵ˴���
			 * 				bsl::CONFIG_ERROR �����ļ���������
			 * 				bsl::ERROR ��������
			 *              DUPLICATED_GROUP ������������GROUP_DUP_LEVEL0��GROUP_DUP_LEVEL3�£������򷵻�0
			 * @retval   
			 * @see 
			 * @author linjieqiong 
			 * @date 20012/12/28 11:36:54
			 **/
			int load_ex(const char * path, const char * conf, const char * range = NULL,
					int dupLevel = GROUP_DUP_LEVEL0, int version = CONFIG_VERSION_1_0);
			/**
			 * @brief ��ָ�����ļ������������ļ���Լ���ļ����������δ��Լ������������ӡ������
			 *
			 * @param [in] path   : const char* �����ļ���·��
			 * @param [in] conf   : const char* �����ļ���
			 * @param [in] range   : const char* Լ���ļ������ָ����Լ���ļ�������ڻ���ӡ��δ��Լ����������
			 * @param [in] version   : int �ļ���ʽ�İ汾����δʹ�ã�
			 * @return  int  0��ʾ�ɹ�������Ϊʧ��
			 * 				bsl::CONSTRAINT_ERROR  Լ�������󣨲�����Լ��������δ��Լ��������������������Լ���������Ӵ˴���
			 * 				bsl::CONFIG_ERROR �����ļ���������
			 * 				bsl::ERROR ��������
			 * @retval   
			 * @see 
			 * @author linjieqiong
			 * @date 2013/01/06 21:36:54
			 **/
			int load_ex2(const char * path, const char * conf, const char * range = NULL, int version = CONFIG_VERSION_1_0);
			/**
			 * @brief �����ݴ��л���һ���ֽ��������Է�������
			 * 			���������ֽ���ֱ��д������ļ���������ֱ��load����������Ҫ��ȡ����rebuild
			 * @note ��apiֻ�����л�ԭʼ�����ļ������ivar�����ܷ�ӳ���ڴ��ж�configure������޸�
			 *			�������л�ʵʱ���ݣ���ʹ��dump_ex()
			 *
			 * @param [out] size_t * : ������ֽ�������
			 * @return  char* ������ֽ���
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2008/12/30 01:38:35
			 **/
			char * dump(size_t *);

			/**
			 * @brief ���л�configure�����ʵʱ�ڴ�����
			 *
			 * @param [in] buf	: dump_ex()�����λ��
			 * @param [in] bufsize	: buf�ĳ���
			 * @param [in] restoreComment	: const int
			 *			0 RC_COMMENT ��ԭ�����ļ��е�ע�͵����
			 *			1 RC_NO_COMMENT ����ԭ�����ļ��е�ע�����ݵ����
			 *			2 RC_POSITION ��ԭ��ͬʱ����������������ļ��е�λ�ã���ʽ: 
			 *					#this is the original comment
			 *					#[FILE:LINE]
			 *					[group]
			 *					#[FILE:LINE]
			 *					key : value
			 *
			 * @return int д��buf�����ݳ���
			 * 			����������󣬷���-1
			 *			���buf����������ʵ�ʳ��ȵ��෴��
			 *			�Զ����\0
			 *
			 * @author linjieqiong
			 * @version 1.2.9
			 * @date 2013/04/24 14:56:31
			 */
			int dump_ex(char *buf, const size_t bufsize,  const int restoreComment = 0);

			/**
			 * @brief ����dump���ֽ�������Configure
			 * 		�����ֽ����ؽ�һ��Configure�Ĳ����ǣ�
			 * 		1��getRebuildBuffer(size) ��ȡһ���ڲ�������
			 * 		2�����ֽ����е����ݿ��������������
			 * 		3������rebuild�ؽ�����
			 *
			 * 		���ñ��ӿڻ�������е����ݡ�
			 *
			 * @param [in] size   : size_t �ֽ�������
			 * @return  char* ������
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2008/12/30 01:39:38
			 **/
			char * getRebuildBuffer(size_t size);
			/**
			 * @brief ���ݻ�õ��ֽ����ؽ�Configure����
			 *
			 * @return  int 0Ϊ�ɹ�������Ϊʧ��
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2008/12/30 01:43:05
			 **/
			int rebuild();
			/**
			 * @brief ������������ļ���������ʱ�䣨������$include���������ļ�
			 *        $include ���Ʋ���Ϊ��MAX_INCLUDE_DEPTH
			 *        ��ִ��lastConfigModify()��Ӧȷ��ԭ���������ļ���û�б�ɾ��
			 *
			 * @return  time_t ���������ļ��е�������ʱ��
			 *                 ����0��ʾ����
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2009/03/10 14:23:47
			**/
			time_t lastConfigModify();
			/**
			 * @brief ��һ��IVar���������ļ�������
			 *        ���ӿ���load/rebuild����
			 *        ���IVar������һ��Dict����
			 *
			 * @return  int 0�ɹ�������ʧ��
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2009/03/11 17:29:56
			**/
			int loadIVar(const bsl::var::IVar & );
			/**
			 * @brief ��Լ���ļ����һ��
			 * @param [in] range   : const char* Լ���ļ�
			 * @param [in] version   : int �ļ���ʽ�İ汾����δʹ�ã�
			 * @return  int  0��ʾ�ɹ�������Ϊʧ��
			 * 				bsl::CONSTRAINT_ERROR  Լ��������
			 * 				bsl::ERROR ��������
			 **/
			int check_once(const char * range, int version = CONFIG_VERSION_1_0);
			/**
			 * @brief ���conf�ļ�������unit�Ƿ�Լ�����������δ��Լ��������ӡ�����ش���
			 *
			 * @return  int 0 �ɹ��������Ǵ����
			 * 				bsl::CONSTRAINT_ERROR Լ��������
			 * @author linjieqiong
			 * @date 2013/01/07 17:32:10
 			 **/
			int checkConstraint();
			/**
			 * @brief ��ȡһ��key��Լ�������б�ע��group��array���͵�Լ�������������õ�
			 * @param [in] key_path : const char* ��key��conf�ļ��е�����·�� 
			 * @param [in] range   : const char* Լ���ļ���
			 * 				����ǿգ��Ƚ���Լ����飻
			 *				���Ϊ�գ���ʹ�����һ��load�ຯ����check_once()������ʹ�õ�Լ���ļ�
			 *
			 * @return  int 0 �ɹ��������Ǵ����
			 * 				bsl::CONSTRAINT_ERROR  Լ��������
			 * 				bsl::ERROR ��������
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

			struct ReaderNode{//ÿһ��conf�ļ�
				str_t filename;
				Reader * reader;
				int level;
			};
			std::vector <ReaderNode *> _readers;
			str_t _path;
			Reader * _cur_reader;//��ǰ�ڴ�����ļ�
			int _cur_level;//��ǰ�ļ������($include�Ĳ��)
			char * _dump_buffer;
			size_t _dump_size;
			confIDL::idl_t * _idl;
			void changeSection(str_t str);
			void pushPair(const str_t& key, const str_t& value);
			
			ConfigGroup* _section;
			//depth��һ�����Ա���
			int _depth;
	};
}



#endif  //__CONFIGURE_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
