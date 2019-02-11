/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_phashset.h,v 1.2 2008/12/15 09:56:59 xiaowei Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file phashset.h
 * @author xiaowei(com@baidu.com)
 * @date 2008/08/13 22:05:17
 * @version $Revision: 1.2 $  2010/10/15 modified by zhjw(zhujianwei@baidu.com)
 * @brief 
 *  
 **/


#ifndef  __BSL_PHASHSET_H_
#define  __BSL_PHASHSET_H_

#include <bsl/containers/hash/bsl_phashtable.h>

namespace bsl
{

template <class _Key, /*��Ϊhashkey������*/
		 /**
		  * hash �����ķº���������
		  * struct xhash {
		  * 	inline size_t operator () (const _Key &_1);
		  * };
		  **/
		 class _HashFun = xhash<_Key>,
		 /**
		  * �ж�����key��ȵķº���
		  * ���� struct equal {
		  * 	inline bool operator () (const _Tp &_1, const _Tp &_2);
		  * };
		  */
		 class _Equl = std::equal_to<_Key>,	
		 /**
		  * �ռ��������Ĭ�ϵĿռ�������ܹ���Ч�ʵĹ���С�ڴ棬��ֹ�ڴ���Ƭ
		  * �����������������ڲ����ͷ�������ڴ�
		  *
		  * bsl_alloc<_Key>���ڴ���������������������������ͷ��ڴ棬
		  * ���ǲ�����Ч��ֹ�ڴ���Ƭ
		  */
		 class _InnerAlloc = bsl_sample_alloc<bsl_alloc<_Key>, 256>
		>
class phashset : public bsl_phashtable<_Key, _Key, _HashFun, _Equl, 
	bsl::param_select<_Key>, _InnerAlloc>
{
public:
	typedef phashset<_Key, _HashFun, _Equl, _InnerAlloc> _Self;
	typedef bsl_phashtable<_Key, _Key, _HashFun, _Equl, bsl::param_select<_Key>, _InnerAlloc> _Base;
	typedef typename _Base::iterator iterator;
	typedef typename _Base::const_iterator const_iterator;
	typedef _Key value_type;
	typedef _Key key_type;
private:
	/**
	 * @brief �������캯��
	 * */
	phashset(const _Self&);				//����
	/**
	 * @brief ��ֵ�����
	 * */
	_Self & operator = (const _Self &);	//����

public:
 
	/**
	 * @brief Ĭ�Ϲ��캯��
	 * ���쳣
	 * ���create
	**/
	phashset(){}

	/**
	 * @brief ��Ͱ��С�Ĺ��캯��
	 * ���쳣
	 * �����create
	 * @param   [in] bitems     : size_t hashͰ����
	 * @param   [in] rwlsize    : size_t ��д������
	 * @param   [in] hf         : const _HashFun& hash����
	 * @param   [in] eq         : const _Equl& �ȽϺ���
	 **/
	phashset(size_t bitems, size_t rwlsize = 0,
			const _HashFun &hf = _HashFun(), 
			const _Equl &eq = _Equl())
			:_Base(bitems, rwlsize, hf, eq){
	}

	/**
	 * create�����ο�phashtable�ķ���,ֱ�Ӽ̳�
	 * destroy����Ҳֱ�Ӽ̳�
	 */

	/**
	 * @brief ��ѯkey_type�Ƿ����
	 *
	 * @param [in/out] k   : const key_type&	ָ���Ĳ���key
	 * @return  int 
	 *				���� HASH_EXIST		��ʾhashֵ����
	 *				���� HASH_NOEXIST	��ʾhashֵ������
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2008/08/12 21:17:52
	**/
	int get(const key_type &k) const {
		return _Base::get(k);
	}
	int get(const key_type &k) {
		return _Base::get(k);
	}

	/**
	 * @brief ��key���� hash����
	 *
	 * @param [in/out] k   : const key_type&	keyֵ	
	 * @return  int 	
	 *  			����	-1	��ʾset���ó���, (�޷������½��)
	 * 			��������ʾ����ɹ�������ɹ�����������״̬
	 * 				����	HASH_INSERT_SEC	��ʾ�����½��ɹ�
	 * 				����	HASH_EXIST	��ʾhash�������(��flagΪ0��ʱ�򷵻�)
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2008/08/12 21:20:49
	**/
	int set(const key_type &k) {
		return _Base::set(k, k);
	}

	/**
	 * @brief ����ָ����keyɾ�����
	 *
	 * @param [in/out] k   : const key_type&
	 * @return  int 
	 * 		����	HASH_EXIST��ʾ�����ڲ�ɾ���ɹ�
	 * 		����	HASH_NOEXIST��ʾ��㲻���ڲ���ɾ��
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2008/08/12 21:24:58
	**/
	int erase(const key_type &k) {
		return _Base::erase(k);
	}

};
	
};















#endif  //__PHASHSET_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
