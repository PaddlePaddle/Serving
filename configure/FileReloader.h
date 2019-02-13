/***************************************************************************
 * 
 * Copyright (c) 2010 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file FileReloader.h
 * @author zhang_rui(com@baidu.com)
 * @date 2010-2-1
 * @brief 
 *  
 **/


#ifndef FILERELOADER_H_
#define FILERELOADER_H_

#include <map>
#include "butil/logging.h"
#include "bsl/containers/string/bsl_string.h"
#include "bsl/containers/hash/bsl_hashmap.h"

namespace comcfg {

typedef int (*file_change_cb_t)(const char * filename, void * prm);    /**<  处理文件变更的回调函数类型    */

typedef struct _config_filemonitor_t {
	file_change_cb_t callback;  /**<  处理文件变更的回调函数    */
	void * param; /**<  参数    */
	time_t _last_modify;  /**< 最近修改时间   */
} config_filemonitor_t;

class FileReloader {
	typedef bsl::hashmap <bsl::string, config_filemonitor_t *> config_filemonmap_t;
private :
	config_filemonmap_t _filemap;
public :
	/**
	 * @brief 检查配置文件是否发生变化。
	 *
	 * @return  int  0表示没有不安化，1表示配置发生了变化并进行了通知，-1表示文件stat失败，可能被删除。
	 **/
	int monitor();
	/**
	 * @brief 增加监控整个文件的回调
	 *
	 * @param [in] proc   : file_change_cb_t 处理回调函数
	 * @return  int  0表示成功，-1为失败
	 **/
	int add_file_monitor(const char * filename, file_change_cb_t proc, void * prm);

	/**
	 * @brief 构造函数
	 **/
	FileReloader(){
		 _filemap.create(256);
	}

	/**
	 * @brief 析构函数
	 **/
	~FileReloader();
};

}//namespace comcfg

#endif /* FILERELOADER_H_ */
