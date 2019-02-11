/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_phashmap.h,v 1.2 2008/12/15 09:56:59 xiaowei Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file phashmap.h
 * @author xiaowei(com@baidu.com)
 * @date 2008/08/05 21:23:54
 * @version $Revision: 1.2 $ 2010/10/15 modified by zhjw(zhujianwei@baidu.com)
 * @brief 
 *  
 **/


#ifndef  __BSL_PHASHMAP_H_
#define  __BSL_PHASHMAP_H_

#include <bsl/containers/hash/bsl_phashtable.h>

namespace bsl
{

template <class _Key, /*��Ϊhashkey������*/
		 class _Value,	/*��Ϊhash value������*/
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
class phashmap : public bsl_phashtable<_Key, std::pair<_Key, _Value>, _HashFun, _Equl, 
	bsl::pair_first<std::pair<_Key, _Value> >, _InnerAlloc>
{
public:
	typedef std::pair<_Key, _Value> _Pair;
	typedef phashmap<_Key, _Value, _HashFun, _Equl, _InnerAlloc> _Self;
	typedef bsl_phashtable<_Key, _Pair, _HashFun, _Equl, bsl::pair_first<_Pair>, _InnerAlloc> _Base;
	typedef typename _Base::iterator iterator;
	typedef typename _Base::const_iterator const_iterator;

	typedef _Value value_type;
	typedef _Key key_type;

private:
    /* * 
     *  @beirf �������캯��
     **/
    phashmap(const _Self &);            //����
    /* *
     * @brief ��ֵ�����
     * */
	_Self & operator = (const _Self &); //����

public:

    /**
     * @brief Ĭ�Ϲ��캯��
     * ���쳣
     * �����create������
     **/
    phashmap(){}

    /**
     * @brief ��Ͱ��С�Ĺ��캯��
     *  ���ʧ�����쳣
     *  �������create
     *
     * @param   [in] bitems     : size_t hashͰ����
     * @param   [in] rwlsize    : size_t ��д������
     * @param   [in] hf         : const _HashFun& hash����
     * @param   [in] eq         : const _Equl& �ȽϺ���
     **/
    phashmap(size_t bitems, size_t rwlsize = 0,
            const _HashFun &hf = _HashFun(),
            const _Equl &eq = _Equl()):
        _Base(bitems, rwlsize, hf, eq){
    }

	/**
	 * create�����ο�phashtable�ķ���,ֱ�Ӽ̳�
	 * destroy����Ҳֱ�Ӽ̳�
	 */


	/**
	 * @brief ����ָ��key��ȡvalueֵ
	 *
	 * @param [in/out] k   : const key_type&	ָ���Ĳ���key
	 * @param [in/out] val   : value_type*	��ȡ��valueֵ�Ĵ洢��ַ
	 * 				���value��Ϊ�գ���*val��ֵ���ҵ�ֵ
	 * @return  int 
	 *				���� HASH_EXIST		��ʾhashֵ����
	 *				���� HASH_NOEXIST	��ʾhashֵ������
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2008/08/12 21:17:52
	**/
	int get(const key_type &k, value_type *val = 0) const {
		return _Base::get_map(k, val);
	}
	int get(const key_type &k, value_type *val = 0) {
		return _Base::get_map(k, val);
	}
    
    /**
	 * @brief ����ָ��key��ȡvalue��Ӧ�ûص�����������ԭֵ
     * ������Ҫԭֵ������NULLָ��
     * _ValOpFunc�����Ǻ���ָ�룬Ҳ������functor����������Ϊ(_Second*, void*)
	 **/
    template <typename _ValOpFunc> 
    int get(const key_type &k, value_type *old_val, _ValOpFunc val_op_func, void *args = 0) {
		return _Base::get_map(k, old_val, val_op_func, args);
	}

	/**
	 * @brief ��key �� value �� ���� hash����
	 *
	 * @param [in/out] k   : const key_type&	keyֵ	
	 * @param [in/out] val   : const value_type&	valueֵ
	 * @param [in/out] flag   : int	
	 * 				flag ����0�����ֵ����ֱ�ӷ��أ�
	 * 				��0��ʾ�����ֵ���ڣ��滻��ֵ
	 * @return  int 	
	 *  			����	-1	��ʾset���ó���, (�޷������½��)
	 * 			��������ʾ����ɹ�������ɹ�����������״̬
	 * 				����	HASH_OVERWRITE	��ʾ���Ǿɽ��ɹ�(��flag��0��ʱ�򷵻�)
	 * 				����	HASH_INSERT_SEC	��ʾ�����½��ɹ�
	 * 				����	HASH_EXIST	��ʾhash�������(��flagΪ0��ʱ�򷵻�)
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2008/08/12 21:20:49
	**/
	int set(const key_type &k, const value_type &val, int flag = 0) {
		return _Base::set(k, _Pair(k, val), flag);
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


#endif  //__PHASHMAP_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
