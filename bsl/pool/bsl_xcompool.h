/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_xcompool.h,v 1.5 2009/01/06 14:05:00 xiaowei Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file bsl_xcompool.h
 * @author xiaowei(com@baidu.com)
 * @date 2008/12/08 17:46:07
 * @version $Revision: 1.5 $ 
 * @brief 
 *  
 **/


#ifndef  __BSL_XCOMPOOL_H_
#define  __BSL_XCOMPOOL_H_


#include "bsl_xmempool.h"
#include <set>

namespace bsl
{
class xcompool : public mempool
{
	size_t _maxblksiz;
	void * _buffer;
	size_t _bufsiz;
	xmempool _pool;

	typedef std::set<void *> SET;
	SET _set;
public:
	xcompool() {
		_maxblksiz = 0;
		_buffer = NULL;
	}
	~xcompool() {
		destroy();
	}
	/**
	 * @brief ��ʼ���ڴ��
	 *
	 * @param [in/out] maxblk   : size_t ÿ��slab������ڴ��С
	 * @param [in/out] bmin   : size_t	slab�������С�ڴ浥Ԫ
	 * @param [in/out] bmax   : size_t	slab���������ڴ浥Ԫ
	 * @param [in/out] rate   : float	slab�ĵ�������
	 * @return  int  �ɹ�����0, ����ʧ��
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/12/24 14:07:04
	**/
	int create (size_t maxblk = 1<<20, 
			size_t bmin = sizeof(void *), size_t bmax = (1<<16), 
			float rate = 2.0f);

	/**
	 * @brief �����ڴ��
	 *
	 * @return  void 
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/12/24 14:08:27
	**/
	void destroy();

	void * malloc (size_t size);

	void free (void *ptr, size_t size);

	/**
	 * @brief ����pool������ڴ�
	 *
	 * @return  void 
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/12/24 14:08:53
	**/
	void clear ();

};
};



#endif  //__BSL_XCOMPOOL_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
