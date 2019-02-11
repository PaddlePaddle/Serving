/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_binarchive.h,v 1.5 2008/12/15 09:56:59 xiaowei Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file bsl_binarchive.h
 * @author xiaowei(com@baidu.com)
 * @date 2008/07/25 17:00:04
 * @version $Revision: 1.5 $ 
 * @brief 
 *  
 **/


#ifndef  __BSL_BINARCHIVE_H_
#define  __BSL_BINARCHIVE_H_

#include <bsl/utils/bsl_debug.h>
#include <stdlib.h>
#include <string.h>
#include <bsl/archive/bsl_stream.h>

namespace bsl
{
class binarchive
{
protected:
	bsl::stream &_fd;
public:
	binarchive(bsl::stream & fd) : _fd(fd) {}
	~binarchive() {}
		
	/**
	 * @brief 从bsl stream 流中读取一个实体数据
	 *
	 * @param [out] tp   : _Tp* 非NULL指针
	 * @return  int 成功返回0,其他失败
	 * @retval  tp传NULL 会core
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/11/04 10:17:39
	**/
	template <typename _Tp>
	int read(_Tp *tp) {
		_fd.start_trans();
		int ret = bsl::deserialization(*this, *tp);
		if (ret == 0) {
			_fd.comit_trans();
		} else {
			_fd.drop_trans(false);
		}
		return ret;
	}

	/**
	 * @brief 往stream里面塞数据
	 *
	 * @param [in] tp   : const _Tp& 需要写入的实体数据
	 * @return int  成功返回0,其他失败
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/11/04 10:19:50
	**/
	template <typename _Tp>
	int write(const _Tp & tp) {
		_fd.start_trans();
		int ret = bsl::serialization(*this, tp);
		if (ret == 0) {
			ret = _fd.comit_trans();
			if (ret != 0) {
				_fd.drop_trans(true);
			}
		} else {
			_fd.drop_trans(true);
		}
		return ret;
	}

public:
	/**
	 * @brief 往stream里面写入size大的数据
	 *
	 * @param [in/out] dat   : const void*
	 * @param [in/out] size   : size_t
	 * @return 0 表示成功
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/11/04 10:21:07
	**/
	int push(const void *dat, size_t size) {
		//参数检查让底层去做
		return _fd.write(dat, size);
	}

	/**
	 * @brief 往stream里面读数据
	 *
	 * @param [in/out] dat   : void*
	 * @param [in/out] size   : size_t
	 * @return  int 0表示成功,其他失败
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/11/04 10:23:02
	**/
	int pop(void *dat, size_t size) {
		return _fd.read(dat, size);
	}
};

}
#endif  //__BSL_BINARCHIVE_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
