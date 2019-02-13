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

typedef int (*file_change_cb_t)(const char * filename, void * prm);    /**<  �����ļ�����Ļص���������    */

typedef struct _config_filemonitor_t {
	file_change_cb_t callback;  /**<  �����ļ�����Ļص�����    */
	void * param; /**<  ����    */
	time_t _last_modify;  /**< ����޸�ʱ��   */
} config_filemonitor_t;

class FileReloader {
	typedef bsl::hashmap <bsl::string, config_filemonitor_t *> config_filemonmap_t;
private :
	config_filemonmap_t _filemap;
public :
	/**
	 * @brief ��������ļ��Ƿ����仯��
	 *
	 * @return  int  0��ʾû�в�������1��ʾ���÷����˱仯��������֪ͨ��-1��ʾ�ļ�statʧ�ܣ����ܱ�ɾ����
	 **/
	int monitor();
	/**
	 * @brief ���Ӽ�������ļ��Ļص�
	 *
	 * @param [in] proc   : file_change_cb_t ����ص�����
	 * @return  int  0��ʾ�ɹ���-1Ϊʧ��
	 **/
	int add_file_monitor(const char * filename, file_change_cb_t proc, void * prm);

	/**
	 * @brief ���캯��
	 **/
	FileReloader(){
		 _filemap.create(256);
	}

	/**
	 * @brief ��������
	 **/
	~FileReloader();
};

}//namespace comcfg

#endif /* FILERELOADER_H_ */
