/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_filestream.h,v 1.4 2008/12/15 09:56:59 xiaowei Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file bsl_filestream.h
 * @author xiaowei(com@baidu.com)
 * @date 2008/07/25 17:05:19
 * @version $Revision: 1.4 $ 
 * @brief 
 *  
 **/


#ifndef  __BSL_FILESTREAM_H_
#define  __BSL_FILESTREAM_H_

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <bsl/archive/bsl_stream.h>

namespace bsl
{
class filestream : public stream
{
	FILE *_fp;
	int _trans;
	int _trans_off;
	char *_wbuffer;
	size_t _wbuffersize;
	char *_rbuffer;
	size_t _rbuffersize;
public:
	filestream();
	~filestream();
	/**
	 * @brief �ļ���ģʽ
	 *
	 * @param [in/out] fname   : const char* �ļ���
	 * @param [in/out] mode   : const char* �ļ���ģʽ, 
	 * 								"w" д��
	 * 								"r" ����
	 * 								��Ҫ���Զ�д��,������FILEһ��
	 * @return  int  0��ʾ�ɹ�,����ʧ��
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/11/04 10:24:02
	**/
	int open(const char *fname, const char *mode);
	/**
	 * @brief �ر��ļ�
	 *
	 * @return  int 
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/11/04 10:26:05
	**/
	int close();
	/**
	 * @brief ���ļ�����д����,���д��ʧ��,ʲô��������
	 * Ҫ��д��,Ҫ��ع�����һ��״̬
	 *
	 * @param [in/out] dat   : const void* Ҫд������ָ��
	 * @param [in/out] size   : size_t д�����ݳ���
	 * @return  int 0 ��ʾ�ɹ�
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/11/04 10:26:17
	**/
	int write(const void *dat, size_t size);
	/**
	 * @brief ���ļ����������,�������ʧ��,ʲô��������
	 * Ҫ�����,Ҫ��ع�����һ��״̬ 
	 *
	 * @param [in/out] dat   : void*   
	 * @param [in/out] size   : size_t
	 * @return  int 0��ʾ�ɹ�
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/11/04 10:28:41
	**/
	int read(void *dat, size_t size);
	/**
	 * @brief ����һ������
	 *
	 * @return  int 0 �����ɹ�
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/11/04 10:29:49
	**/
	int start_trans();
	/**
	 * @brief ����һ������
	 *
	 * @return  int 0��ʾ�ɹ�
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/11/04 10:30:08
	**/
	int comit_trans();
	/**
	 * @brief ����һ������
	 *
	 * @param [in] trunc   : bool
	 * 					false : ����������Դ��̵Ĳ�������
	 * 					true : ��������Դ��̵Ĳ�������
	 * @return  int 0��ʾ�ɹ�
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/11/04 10:30:31
	**/
	int drop_trans(bool trunc);
	/**
	 * @brief ����д������
	 *
	 * @param [in/out] size   : size_t ��������СΪbyte
	 * @return  int �ɹ�����0
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/11/04 10:32:12
	**/
	int setwbuffer(size_t size);
	/**
	 * @brief ���ö�������
	 *
	 * @param [in/out] size   : size_t ��������Сλbyte
	 * @return  int �ɹ�����0
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/11/04 10:33:01
	**/
	int setrbuffer(size_t size);
	/**
	 * @brief ǿ�ƽ�����ˢ��Ӳ��
	 *
	 * @return  int 
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/11/04 10:34:30
	**/
	int flush();
};
};


#endif  //__BSL_FILESTREAM_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
