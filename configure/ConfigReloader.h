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
		const ConfigUnit &keynew, void * prm);    /**<  处理配置项变更的回调函数类型    */

typedef struct _load_param_t {
	bsl::string path;    /**<  路径    */
	bsl::string filename;    /**< 配置文件   */
	bsl::string rangename;  /**< 约束文件   */
	int version;  /**< 版本   */
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
	int _finit; /**< 初始化标志   */
	load_param_t _load_param;  /**< load 参数   */
	Configure * _config[2];  /**<  configure对象，原文件和更新文件    */
	int _config_curid;  /**< load 参数   */
	time_t _last_modify;  /**< 最近修改时间   */
	std::vector <config_monitor_t *> _filecblist;  /**<  处理文件变更的回调    */
	bsl::hashmap <bsl::string, config_monitor_t *> _chrcbmap;  /**<  存储监控的字符key和回调    */
	bsl::hashmap <bsl::string, config_monitor_t *> _strcbmap;  /**<  存储监控的字符串key和回调    */
	bsl::hashmap <bsl::string, config_monitor_t *> _intcbmap;  /**<  存储监控的整数key和回调    */
	bsl::hashmap <bsl::string, config_monitor_t *> _uint64cbmap;  /**<  存储监控的浮点key和回调    */
	bsl::hashmap <bsl::string, config_monitor_t *> _floatcbmap;  /**<  存储监控的浮点key和回调    */
	bsl::hashmap <bsl::string, config_monitor_t *> _groupcbmap;  /**<  存储监控的group和回调    */

	/**
	 * @brief 增加一个监控回调到map中
	 *
	 * @param [in] key   : const char* 配置项的全路径
	 * @param [in] proc   : key_change_cb_t 处理回调函数
	 * @param [in] prm : 参数
	 * @return  int  0表示成功，-1为失败
	 **/
	int _add_key_monitor(config_monmap_t * map, const char * key, key_change_cb_t proc, void * prm);
	/**
	 * @brief 释放存储的数据
	 *
	 * @param [in] map   : bsl::hashmap
	 * @return  void
	 **/
	void _free_map(bsl::hashmap <bsl::string, config_monitor_t *> *map);
	/**
	 * @brief 释放存储的数据
	 *
	 * @param [in] map   : bsl::hashmap
	 * @return  void
	 **/
	void _free_list(std::vector <config_monitor_t *> *list);

	/**
	 * @brief 重新载入configure文件
	 * @return  int 0表示成功 其他失败
	 **/
	int reload();
	/**
	 * @brief 检查各个监控值。
	 * @return  int 0表示配置没有检查项，>=1表示配置发生了变化并进行了通知，-1表示错误。
	 **/
	int check();
public :
	/**
	 * @brief 设定要监控的配置文件
	 *
	 * @param [in] path   : const char* 配置文件的路径
	 * @param [in] conf   : const char* 配置文件名
	 * @param [in] range   : const char* 约束文件，约束文件不进行更新检查
	 * @param [in] version   : int 文件格式的版本（暂未使用）
	 * @return  int  0表示成功，其余为失败
	 **/
	int init(const char * path, const char * conf, const char * range = NULL, int version = CONFIG_VERSION_1_0);
	/**
	 * @brief 检查配置文件是否发生变化。
	 *
	 * @return  int  0表示配置没有不安化，正数表示配置发生了变化并进行了通知的回调个数，-1表示更新后的配置内容错误或约束错误。
	 * -2表示其他错误
	 **/
	int monitor();
	/**
	 * @brief 增加要监控的整数key
	 *
	 * @param [in] key   : const char* 配置项的全路径
	 * @param [in] proc   : key_change_cb_t 处理回调函数
	 * @return  int  0表示成功，-1为失败
	 **/
	int add_key_int(const char * key, key_change_cb_t proc, void * prm);
	/**
	 * @brief 增加要监控的uint64 key
	 *
	 * @param [in] key   : const char* 配置项的全路径
	 * @param [in] proc   : key_change_cb_t 处理回调函数
	 * @return  int  0表示成功，-1为失败
	 **/
	int add_key_uint64(const char * key, key_change_cb_t proc, void * prm);
	/**
	 * @brief 增加要监控的浮点类型key
	 *
	 * @param [in] key   : const char* 配置项的全路径
	 * @param [in] proc   : key_change_cb_t 处理回调函数
	 * @return  int  0表示成功，-1为失败
	 **/
	int add_key_float(const char * key, key_change_cb_t proc, void * prm);
	/**
	 * @brief 增加要监控的字符key
	 *
	 * @param [in] key   : const char* 配置项的全路径
	 * @param [in] proc   : key_change_cb_t 处理回调函数
	 * @return  int  0表示成功，-1为失败
	 **/
	int add_key_char(const char * key, key_change_cb_t proc, void * prm);
	/**
	 * @brief 增加要监控的字符串key
	 *
	 * @param [in] key   : const char* 配置项的全路径
	 * @param [in] proc   : key_change_cb_t 处理回调函数
	 * @return  int  0表示成功，-1为失败
	 **/
	int add_key_string(const char * key, key_change_cb_t proc, void * prm);
	/**
	 * @brief 增加监控[group]的回调
	 *
	 * @param [in] group   : const char* 配置项的全路径
	 * @param [in] proc   : key_change_cb_t 处理回调函数
	 * @return  int  0表示成功，-1为失败
	 **/
	int add_group_monitor(const char * group, key_change_cb_t proc, void * prm);
	/**
	 * @brief 增加监控整个文件的回调
	 *
	 * @param [in] proc   : key_change_cb_t 处理回调函数
	 * @return  int  0表示成功，-1为失败
	 **/
	int add_file_monitor(key_change_cb_t proc, void * prm);
	/**
	 * @brief 获取当前的configure
	 **/
	Configure * get_config();

	ConfigReloader();
	~ConfigReloader();
};



}//namespace comcfg


#endif /* CONFIGRELOADER_H_ */
