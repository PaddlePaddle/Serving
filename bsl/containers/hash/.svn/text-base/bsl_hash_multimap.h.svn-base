/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_hashmap.h,v 1.4 2009/08/24 06:24:49 scmpf Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file hash_multimap.h
 * @author luoweii(sat@baidu.com)
 * @date 2012/09/04 14:45:23
 * @brief 实现管理key value对的hash表，
 * 	线程不安全
 *  
 **/


#ifndef  __BSL_HASH_MULTIMAP_H_
#define  __BSL_HASH_MULTIMAP_H_

#include <bsl/containers/hash/bsl_hashtable.h>

namespace bsl
{

/**
 * @brief hash_multimap
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
class hash_multimap
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
	typedef hash_multimap<_Key, _Value, _HashFun, _Equl, _InnerAlloc> _Self;
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

    /**
     * @brief 迭代器pair
     */
	typedef std::pair<iterator, iterator> iterator_pair;

    /**
     * @brief 只读迭代器pair
     */
	typedef std::pair<const_iterator, const_iterator> const_iterator_pair;
    
public:

    /**
     * @brief 默认构造函数
     * 不抛异常
     * 需调用create
     **/
    hash_multimap(){}

    /**
	* @brief 带桶大小的构造函数
	* 失败时会抛出异常
	* 不需调create
     **/
	explicit hash_multimap(size_t bitems, const _HashFun& hf = _HashFun(), const _Equl& eq = _Equl())
	{
		if (_ht.create(bitems,hf,eq) != 0)
		{
			_ht.destroy();
			throw BadAllocException()<<BSL_EARG
				<<"create error when create hash_multimap bitems "<<bitems;
		}
	}

    /**
     * @brief 拷贝构造函数
     * 失败时会抛出异常
     **/
    	hash_multimap(const _Self& other)
    	{
		if (_ht.assign_multimap(other.begin(),other.end(), 1) != 0) 
		{
			_ht.destroy();
			throw BadAllocException()<<BSL_EARG<<"assign hash_multimap error";
		}
	}

    /**
     * @brief 赋值运算符
     * 失败会抛出异常
     **/
	_Self& operator=(const _Self& other)
	{
		if (this != &other) 
		{
			if (assign(other.begin(),other.end(), 1) != 0) 
			{
				_ht.destroy();
				throw BadAllocException()<<BSL_EARG<<"assign hash_multitable error";
			}
		}
		return *this; 
        }

	/**
	 * @brief 返回迭代器的起始地址
	 *
	 * @return  iterator 
	 * @retval   
	 * @see 
	 * @author luowei
	 * @date 2012/09/04 21:13:26
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
	 * @author luowei
	 * @date 2012/09/04 21:13:39
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
	 * @author luowei
	 * @date 2012/09/04 21:14:05
	**/
	size_t size() const { 
		return _ht.size(); 
	}

	/**
	 * @brief 返回hash表目前键值为K有多少个元素
	 * @param [in/out] k   : const key_type&
	 * @param [in/out] flag: int   	
	 * 		标记是否考虑重复元素：
	 * 			若flag==0，不考虑重复因素，即重复元素仍计数；
	 * 			若flag!=0，则考虑重复因素，重复元素不计数(目前只有接口，不提供支持)
	 * @return  size_t 当前的元素个数
	 * @retval
	 * @see
	 * @author luowei
	 * @date 2012/09/04 21:14:05
	**/
       	size_t count(const key_type &k, int flag = 0)
       	{
       		return _ht.count(_ht._hashfun(k), k, flag);
       	}	

	/**
	 * @brief 创建一张hash_multimap
	 *
	 * @param [in/out] bitems   : int	设置hash桶的大小
	 * @param [in/out] hf   : const _HashFun& 设置hash函数
	 * @param [in/out] eq   : const _Equl&	设置相等比较函数
	 * @return  int 返回	0		表示创建成功
	 * 				返回	其他	表示创建失败
	 * @retval   
	 * @see 
	 * @author luowei
	 * @date 2012/09/04 21:12:50
	**/
	int create(size_t bitems, const _HashFun &hf = _HashFun(), const _Equl & eq = _Equl()) {
		return _ht.create(bitems, hf, eq);
	}

    /* *
     * @brief 判断hash_multimap是否已create
     * @author luowei
     * @date 2012/09/04
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
	 * @author luowei
	 * @date 2012/09/04 21:16:39
	**/
	int destroy() {
		return _ht.destroy();
	}

	/**
	 * @brief 根据指定key/value查找
	 *
	 * @param [in/out] k   : const key_type&	指定的查找key
	 * @param [in/out] val   : value_type&		value
	 * @return  		未找到返回HASH_NOEXIST
	 *			找到则返回HASH_EXIST
	 * @retval   
	 * @see 
	 * @author luowei
	 * @date 2012/08/22 21:17:52
	**/
	int find(const key_type &k, const value_type &val) const 
	{
		return _ht.find_pair(_ht._hashfun(k), k, val);
	}
	/**
	 * @brief 根据指定key/value查找
	**/
	int find(const key_type &k, const value_type &val) {
		return _ht.find_pair(_ht._hashfun(k), k, val);
	}

	/**
	 * @brief 根据key查找元素，返回所有的key为k的<k,v>的迭代器首尾pair
	 *
	 * @param [in/out] k   : const key_type&
	 * @return  _const_iterator_pair  		hash表空，返回<end(),end()>
	 *								不存在返回<end(),end()>,
	 *								查找成功返回range的<头迭代器,尾迭代器的下一个迭代器 >
	 * @retval   
	 * @see 
	 * @note 
	 * @author luowei
	 * @date 2012/8/22 14:21:43
	**/
	const_iterator_pair get (const key_type &k) const {
		return _ht.find_multimap(_ht._hashfun(k), k);
	}

    /**
     * @brief 根据key查找元素，返回所有的key为k的<k,v>的迭代器pair
     */
	iterator_pair get (const key_type &k) {
		return _ht.find_multimap(_ht._hashfun(k), k);
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
	 * 			其他均表示插入成功：插入成功分下面两个状态
	 * 				返回	HASH_INSERT_SEC	表示插入新结点成功
	 * 				返回	HASH_EXIST	表示hash表结点存在(在flag为0的时候返回)
	 * @retval   
	 * @see 
	 * @author luowei
	 * @date 2012/09/04 21:20:49
	**/
	int set(const key_type &k, const value_type &val, int flag = 0) {
		return _ht.set_multimap(_ht._hashfun(k), k, val, flag);
	}

	/**
	 * @brief 根据指定的key删除结点
	 *
	 * @param [in/out] k   : const key_type&
	 * @return  int 
	 * 		返回	删除结点的个数
	 * @retval   
	 * @see 
	 * @author luowei
	 * @date 2012/09/04 21:24:58
	**/
	int erase(const key_type &k) {
		return _ht.erase_multimap(_ht._hashfun(k), k);
	}

	/**
	 * @brief 根据指定的<key,value>删除结点
	 *
	 * @param [in/out] k   : const key_type&
	 * @return  int 
	 * 		返回	删除结点的个数	
	 * @retval   
	 * @see 
	 * @author luowei
	 * @date 2012/9/04 21:24:58
	**/
	int erase_pair(const key_type &k, const value_type &val) {
		return _ht.erase_pair(_ht._hashfun(k), k, val);
	}

	/**
	 * @brief 将其他容器对hash表进行附值
	 *
	 * @param [in/out] __begin   : _Iterator	迭代器的起始地址
	 * @param [in/out] __end   : _Iterator		迭代器的结束地址
	 * @return int 	0	表示附值成功，其他表示失败
	 * @retval   
	 * @see 
	 * @author luowei
	 * @date 2012/09/04 21:26:00
	**/
	template <class _Iterator>
	int assign(_Iterator __begin, _Iterator __end, int flag = 0) {
		return _ht.assign_multimap(__begin, __end, flag);
	}

    /**
     * @brief hash_multimap数据串行化
     */
	template <class _Archive>
	int serialization(_Archive &ar) {
		return bsl::serialization(ar, _ht);
	}
    
    /**
     * @brief hash_multimap数据反串行化
     */
	template <class _Archive>
	int deserialization(_Archive &ar) {
		return _ht.deserialization_multimap(ar);
	}
    /**
     * @brief 清空hash_multsh_multimapimap
     */
	int clear() {
		return _ht.clear();
	}
};


}
#endif  //__HASH_MULTIMAP_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */

