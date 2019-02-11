/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_hashmap.h,v 1.4 2009/08/24 06:24:49 scmpf Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file hashmap.h
 * @author xiaowei(com@baidu.com)
 * @date 2008/07/13 18:45:23
 * @version $Revision: 1.4 $ 2010/10/15 modified by zhjw(zhujanwei@baidu.com)
 * @brief 实现管理key value对的hash表，
 * 	线程不安全
 *  
 **/


#ifndef  __BSL_HASHMAP_H_
#define  __BSL_HASHMAP_H_

#include <bsl/containers/hash/bsl_hashtable.h>

namespace bsl
{

/**
 * @brief hashmap
 */
template <class _Key, /*作为hashkey的类型*/
		 class _Value,	/*作为hash value的类型*/
		 /**
		  * hash 函数的仿函数，比如
		  * struct xhash {
		  * 	inline size_t operator () (const _Key &_1);
		  * };
		  **/
		 class _HashFun = xhash<_Key>,
		 /**
		  * 判断两个key相等的仿函数
		  * 比如 struct equal {
		  * 	inline bool operator () (const _Tp &_1, const _Tp &_2) const ;
		  * };
		  */
		 class _Equl = std::equal_to<_Key>,	
		 /**
		  * 空间分配器，默认的空间分配器能够高效率的管理小内存，防止内存碎片
		  * 但是在容器生命骑内不会释放申请的内存
		  *
		  * bsl_alloc<_Key>做内存分配器，可以在容器生命期内释放内存，
		  * 但是不能有效防止内存碎片
		  */
		 class _InnerAlloc = bsl_sample_alloc<bsl_alloc<_Key>, 256>
		>
class hashmap
{
public:
    /**
     * @brief 键
     */
	typedef _Key key_type;
	/**
     * @brief 值
     */
    typedef _Value value_type;
	//typedef bsl::pair<_Key, _Value> _Pair;	//将key value打包，作为底层容器的存储value
	/**
     * @brief 	//将key value打包，作为底层容器的存储value
     */
    typedef std::pair<_Key, _Value> _Pair;
    /**
     * @brief hashmap别名
     */
	typedef hashmap<_Key, _Value, _HashFun, _Equl, _InnerAlloc> _Self;
	/**
     * @brief hashtable别名
     */
    typedef bsl_hashtable<_Key, _Pair, _HashFun, _Equl, pair_first<_Pair>, _InnerAlloc> hash_type;
	/**
     * @brief 内部的hashtable
     */
    hash_type _ht;	//底层hash表
public:
    /**
     * @brief 迭代器
     */
	typedef typename hash_type::iterator iterator;
    /**
     * @brief 只读迭代器
     */
	typedef typename hash_type::const_iterator const_iterator;
    
public:

    /**
     * @brief 默认构造函数
     * 不抛异常
     * 需调用create
     **/
    hashmap(){}

    /**
     * @brief 带桶大小的构造函数
     * 失败时会抛出异常
     * 不需调create
     **/
    explicit hashmap(size_t bitems, const _HashFun& hf = _HashFun(), const _Equl& eq = _Equl())
        : _ht(bitems, hf, eq){}

    /**
     * @brief 拷贝构造函数
     * 失败时会抛出异常
     **/
    hashmap(const _Self& other): _ht(other._ht){}

    /**
     * @brief 赋值运算符
     * 失败会抛出异常
     **/
    _Self& operator=(const _Self& other){
        _ht = other._ht;
        return *this;
    }

	/**
	 * @brief 返回迭代器的起始地址
	 *
	 * @return  iterator 
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2008/08/12 21:13:26
	**/
	iterator begin() {
		return _ht.begin(); 
	}
    /**
     * @brief 返回只读迭代器的起始地址
     */
	const_iterator begin() const {
		return _ht.begin();
	}

	/**
	 * @brief 返回迭代器的结束地址
	 *
	 * @return  iterator 
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2008/08/12 21:13:39
	**/
	iterator end() { 
		return _ht.end(); 
	}
    /**
     * @brief 返回只读迭代器的结束地址
     */
	const_iterator end() const {
		return _ht.end();
	}

	/**
	 * @brief 返回hash表目前有多少个元素
	 *
	 * @return  size_t 当前的元素个数
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2008/08/12 21:14:05
	**/
	size_t size() const { 
		return _ht.size(); 
	}

	/**
	 * @brief 创建一张hashmap
	 *
	 * @param [in/out] bitems   : int	设置hash桶的大小
	 * @param [in/out] hf   : const _HashFun& 设置hash函数
	 * @param [in/out] eq   : const _Equl&	设置相等比较函数
	 * @return  int 返回	0		表示创建成功
	 * 				返回	其他	表示创建失败
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2008/08/12 21:12:50
	**/
	int create(size_t bitems, const _HashFun &hf = _HashFun(), const _Equl & eq = _Equl()) {
        return _ht.create(bitems, hf, eq);
	}

    /* *
     * @brief 判断hashmap是否已create
     * @author zhujianwei
     * @date 2010/12/13
     * */
    bool is_created() const{
        return _ht.is_created();
    }

	/**
	 * @brief 销毁hash表
	 *
	 * @return  int 返回0表示删除成功，其他表示删除失败
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2008/08/12 21:16:39
	**/
	int destroy() {
		return _ht.destroy();
	}

	/**
	 * @brief 根据指定key获取value值
	 *
	 * @param [in/out] k   : const key_type&	指定的查找key
	 * @param [in/out] val   : value_type*	获取的value值的存储地址
	 * 				如果value不为空，将*val附值查找的值
	 * @return  int 
	 *				返回 HASH_EXIST		表示hash值存在
	 *				返回 HASH_NOEXIST	表示hash值不存在
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2008/08/12 21:17:52
	**/
	int get(const key_type &k, value_type *val=0) const {
		_Pair *ptr = _ht.find(_ht._hashfun(k), k);
		if (ptr == NULL) {
			return HASH_NOEXIST;
		}
		if (val) {
			*val = ptr->second;
		}
		return HASH_EXIST;
	}
    /**
     * @brief 根据指定key获取value值
     */
	int get(const key_type &k, value_type *val=0) {
		_Pair *ptr = _ht.find(_ht._hashfun(k), k);
		if (ptr == NULL) {
			return HASH_NOEXIST;
		}
		if (val) {
			*val = ptr->second;
		}
		return HASH_EXIST;
	}

	/**
	 * @brief 根据指定的key获取value指针
	 *
	 * @param [in/out] k   : const key_type&
	 * @return  _Pair*  不存在返回NULL
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/09/04 14:21:43
	**/
	_Pair * find (const key_type &k) const {
		return _ht.find(_ht._hashfun(k), k);
	}

    /**
     * @brief 根据指定的key获取value指针
     */
	_Pair * find (const key_type &k) {
		return _ht.find(_ht._hashfun(k), k);
	}

	/**
	 * @brief 将key 和 value 对 插入 hash表中
	 *
	 * @param [in/out] k   : const key_type&	key值	
	 * @param [in/out] val   : const value_type&	value值
	 * @param [in/out] flag   : int	
	 * 				flag ＝＝0，如果值存在直接返回，
	 * 				非0表示，如果值存在，替换旧值
	 * @return  int 	
	 *  			返回	-1	表示set调用出错, (无法分配新结点)
	 * 			其他均表示插入成功：插入成功分下面三个状态
	 * 				返回	HASH_OVERWRITE	表示覆盖旧结点成功(在flag非0的时候返回)
	 * 				返回	HASH_INSERT_SEC	表示插入新结点成功
	 * 				返回	HASH_EXIST	表示hash表结点存在(在flag为0的时候返回)
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2008/08/12 21:20:49
	**/
	int set(const key_type &k, const value_type &val, int flag = 0) {
		return _ht.set_map(_ht._hashfun(k), k, val, flag);
	}

	/**
	 * @brief 根据指定的key删除结点
	 *
	 * @param [in/out] k   : const key_type&
	 * @return  int 
	 * 		返回	HASH_EXIST表示结点存在并删除成功
	 * 		返回	HASH_NOEXIST表示结点不存在不用删除
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2008/08/12 21:24:58
	**/
	int erase(const key_type &k) {
		return _ht.erase(_ht._hashfun(k), k);
	}

	/**
	 * @brief 将其他容器对hash表进行附值
	 *
	 * @param [in/out] __begin   : _Iterator	迭代器的起始地址
	 * @param [in/out] __end   : _Iterator		迭代器的结束地址
	 * @return int 	0	表示附值成功，其他表示失败
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2008/08/12 21:26:00
	**/
	template <class _Iterator>
	int assign(_Iterator __begin, _Iterator __end) {
		return _ht.assign(__begin, __end);
	}

    /**
     * @brief hashmap数据串行化
     */
	template <class _Archive>
	int serialization(_Archive &ar) {
		return bsl::serialization(ar, _ht);
	}
    
    /**
     * @brief hashmap数据反串行化
     */
	template <class _Archive>
	int deserialization(_Archive &ar) {
		return bsl::deserialization(ar, _ht);
	}
    /**
     * @brief 清空hashmap
     */
	int clear() {
		return _ht.clear();
	}
};

};

#endif  //__HASHMAP_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
