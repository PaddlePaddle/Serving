/***************************************************************************
 * 
 * Copyright (c) 2010 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file ConfigReloader.h
 * @author zhang_rui(com@baidu.com)
 * @date 2010-1-21
 * @brief 
 *  
 **/


#ifndef CONFIGRELOADER_H_
#define CONFIGRELOADER_H_

#include "Configure.h"

namespace comcfg {

typedef int (*key_change_cb_t)(const ConfigUnit &keyold, \
		const ConfigUnit &keynew, void * prm);    /**<  �������������Ļص���������    */

typedef struct _load_param_t {
	bsl::string path;    /**<  ·��    */
	bsl::string filename;    /**< �����ļ�   */
	bsl::string rangename;  /**< Լ���ļ�   */
	int version;  /**< �汾   */
}load_param_t ;

typedef struct _config_monitor_t {
	key_change_cb_t callback;
	void * param;
} config_monitor_t;

class ConfigReloader {
	typedef bsl::hashmap <bsl::string, config_monitor_t *> config_monmap_t;
	typedef bsl::hashmap <bsl::string, config_monitor_t *> :: iterator config_monmap_itr_t;
	typedef bsl::hashmap <bsl::string, config_monitor_t *> :: _Pair* config_monmap_pr_t;
private:
	int _finit; /**< ��ʼ����־   */
	load_param_t _load_param;  /**< load ����   */
	Configure * _config[2];  /**<  configure����ԭ�ļ��͸����ļ�    */
	int _config_curid;  /**< load ����   */
	time_t _last_modify;  /**< ����޸�ʱ��   */
	std::vector <config_monitor_t *> _filecblist;  /**<  �����ļ�����Ļص�    */
	bsl::hashmap <bsl::string, config_monitor_t *> _chrcbmap;  /**<  �洢��ص��ַ�key�ͻص�    */
	bsl::hashmap <bsl::string, config_monitor_t *> _strcbmap;  /**<  �洢��ص��ַ���key�ͻص�    */
	bsl::hashmap <bsl::string, config_monitor_t *> _intcbmap;  /**<  �洢��ص�����key�ͻص�    */
	bsl::hashmap <bsl::string, config_monitor_t *> _uint64cbmap;  /**<  �洢��صĸ���key�ͻص�    */
	bsl::hashmap <bsl::string, config_monitor_t *> _floatcbmap;  /**<  �洢��صĸ���key�ͻص�    */
	bsl::hashmap <bsl::string, config_monitor_t *> _groupcbmap;  /**<  �洢��ص�group�ͻص�    */

	/**
	 * @brief ����һ����ػص���map��
	 *
	 * @param [in] key   : const char* �������ȫ·��
	 * @param [in] proc   : key_change_cb_t ����ص�����
	 * @param [in] prm : ����
	 * @return  int  0��ʾ�ɹ���-1Ϊʧ��
	 **/
	int _add_key_monitor(config_monmap_t * map, const char * key, key_change_cb_t proc, void * prm);
	/**
	 * @brief �ͷŴ洢������
	 *
	 * @param [in] map   : bsl::hashmap
	 * @return  void
	 **/
	void _free_map(bsl::hashmap <bsl::string, config_monitor_t *> *map);
	/**
	 * @brief �ͷŴ洢������
	 *
	 * @param [in] map   : bsl::hashmap
	 * @return  void
	 **/
	void _free_list(std::vector <config_monitor_t *> *list);

	/**
	 * @brief ��������configure�ļ�
	 * @return  int 0��ʾ�ɹ� ����ʧ��
	 **/
	int reload();
	/**
	 * @brief ���������ֵ��
	 * @return  int 0��ʾ����û�м���>=1��ʾ���÷����˱仯��������֪ͨ��-1��ʾ����
	 **/
	int check();
public :
	/**
	 * @brief �趨Ҫ��ص������ļ�
	 *
	 * @param [in] path   : const char* �����ļ���·��
	 * @param [in] conf   : const char* �����ļ���
	 * @param [in] range   : const char* Լ���ļ���Լ���ļ������и��¼��
	 * @param [in] version   : int �ļ���ʽ�İ汾����δʹ�ã�
	 * @return  int  0��ʾ�ɹ�������Ϊʧ��
	 **/
	int init(const char * path, const char * conf, const char * range = NULL, int version = CONFIG_VERSION_1_0);
	/**
	 * @brief ��������ļ��Ƿ����仯��
	 *
	 * @return  int  0��ʾ����û�в�������������ʾ���÷����˱仯��������֪ͨ�Ļص�������-1��ʾ���º���������ݴ����Լ������
	 * -2��ʾ��������
	 **/
	int monitor();
	/**
	 * @brief ����Ҫ��ص�����key
	 *
	 * @param [in] key   : const char* �������ȫ·��
	 * @param [in] proc   : key_change_cb_t ����ص�����
	 * @return  int  0��ʾ�ɹ���-1Ϊʧ��
	 **/
	int add_key_int(const char * key, key_change_cb_t proc, void * prm);
	/**
	 * @brief ����Ҫ��ص�uint64 key
	 *
	 * @param [in] key   : const char* �������ȫ·��
	 * @param [in] proc   : key_change_cb_t ����ص�����
	 * @return  int  0��ʾ�ɹ���-1Ϊʧ��
	 **/
	int add_key_uint64(const char * key, key_change_cb_t proc, void * prm);
	/**
	 * @brief ����Ҫ��صĸ�������key
	 *
	 * @param [in] key   : const char* �������ȫ·��
	 * @param [in] proc   : key_change_cb_t ����ص�����
	 * @return  int  0��ʾ�ɹ���-1Ϊʧ��
	 **/
	int add_key_float(const char * key, key_change_cb_t proc, void * prm);
	/**
	 * @brief ����Ҫ��ص��ַ�key
	 *
	 * @param [in] key   : const char* �������ȫ·��
	 * @param [in] proc   : key_change_cb_t ����ص�����
	 * @return  int  0��ʾ�ɹ���-1Ϊʧ��
	 **/
	int add_key_char(const char * key, key_change_cb_t proc, void * prm);
	/**
	 * @brief ����Ҫ��ص��ַ���key
	 *
	 * @param [in] key   : const char* �������ȫ·��
	 * @param [in] proc   : key_change_cb_t ����ص�����
	 * @return  int  0��ʾ�ɹ���-1Ϊʧ��
	 **/
	int add_key_string(const char * key, key_change_cb_t proc, void * prm);
	/**
	 * @brief ���Ӽ��[group]�Ļص�
	 *
	 * @param [in] group   : const char* �������ȫ·��
	 * @param [in] proc   : key_change_cb_t ����ص�����
	 * @return  int  0��ʾ�ɹ���-1Ϊʧ��
	 **/
	int add_group_monitor(const char * group, key_change_cb_t proc, void * prm);
	/**
	 * @brief ���Ӽ�������ļ��Ļص�
	 *
	 * @param [in] proc   : key_change_cb_t ����ص�����
	 * @return  int  0��ʾ�ɹ���-1Ϊʧ��
	 **/
	int add_file_monitor(key_change_cb_t proc, void * prm);
	/**
	 * @brief ��ȡ��ǰ��configure
	 **/
	Configure * get_config();

	ConfigReloader();
	~ConfigReloader();
};



}//namespace comcfg


#endif /* CONFIGRELOADER_H_ */
