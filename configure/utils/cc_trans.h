/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: cc_trans.h,v 1.5 2009/11/10 04:50:30 scmpf Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file cc_trans.h
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/10 17:48:14
 * @version $Revision: 1.5 $ 
 * @brief 
 *  
 **/


#ifndef  __CC_TRANS_H_
#define  __CC_TRANS_H_

#include "cc_struct.h"

namespace comcfg{
	class Trans{
		public:
#if 0
			static const unsigned long long max_uint64 = 0xFFFFFFFFFFFFFFFF;
			static const unsigned long long min_uint64 = 0x0;
			static const unsigned long long max_int64 = 0x7FFFFFFFFFFFFFFF;
			static const unsigned long long min_int64 = 0x8000000000000000;
			static const unsigned long long max_uint32 = 0xFFFFFFFF;
			static const unsigned long long min_uint32 = 0x0;
			static const unsigned long long max_int32 = 0x7FFFFFFF;
			static const unsigned long long min_int32 = 0x80000000;
			static const unsigned long long max_uint16 = 0xFFFF;
			static const unsigned long long min_uint16 = 0x0;
			static const unsigned long long max_int16 = 0x7FFF;
			static const unsigned long long min_int16 = 0x8000;
#endif
		public:
			/**
			 * @brief �ж�һ���ַ����Ƿ���������ʽ
			 *
			 * @param [in] str   : const str_t&
			 * @return  bool 
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2009/03/19 22:36:03
			**/
			static bool isInteger(const str_t & str);
			/**
			 * @brief �ж�һ���ַ����Ƿ��Ǹ�������ʽ������龫�ȷ�Χ��
			 *
			 * @param [in] str   : const str_t&
			 * @return  bool 
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2009/03/19 22:36:52
			**/
			static bool isFloat(const str_t & str);
			/**
			 * @brief ��һ���ַ���ת����һ��int64
			 *
			 * @param [in] __str   : const str_t& bsl::string���͵��ַ���
			 * @param [out] buf   : int64*  ת�����
			 * @return  int 0�ɹ���ʧ�ܣ�OUTOFRANGE, NULLVALUE, FORMATERROR
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2009/03/19 22:37:22
			**/
			static int str2int64(const str_t & __str, long long * buf);
			/**
			 * @brief ��һ���ַ���ת����һ��uint64
			 *
			 * @param [in] __str   : const str_t& bsl::string���͵��ַ���
			 * @param [out] buf   : uint64* ת�����
			 * @return  int 0�ɹ���ʧ�ܣ�OUTOFRANGE, NULLVALUE, FORMATERROR
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2009/03/19 22:40:42
			**/
			static int str2uint64(const str_t & __str, unsigned long long * buf);
			/**
			 * @brief ��һ������ת���ַ����ַ������н�������\xFF, \022, \n֮����ı�ת����ʵ�ʵ��ַ���
			 *         �����������˫���ſ�ͷ��˫���Ž�β��
			 *
			 *         ��configure�ڲ�ʹ�ã��粻��Ϥ�ڲ���Ϊ�����Ƽ�ʹ�á�
			 *
			 * @param [in] __str   : const str_t& bsl::string���͵��ַ���
			 * @param [out] buf   : uint64* ת�����
			 * @return  int 0�ɹ���ʧ�ܣ�FORMATERROR
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2009/03/19 22:40:42
			**/
			static int str2str(const str_t & str, str_t * buf);
			/**
			 * @brief ��һ���ַ���ת����һ��double
			 *
			 * @param [in] __str   : const str_t& bsl::string���͵��ַ���
			 * @param [out] buf   : double* ת�����
			 * @return  int 0�ɹ���ʧ�ܣ�OUTOFRANGE, NULLVALUE, FORMATERROR
			 * @retval   
			 * @see 
			 * @author yingxiang
			 * @date 2009/03/19 22:45:33
			**/
			static int str2double(const str_t & str, double * buf);

			//����configureʹ��
			static str_t cutComment(const str_t & str);
	};
}
















#endif  //__CC_TRANS_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
