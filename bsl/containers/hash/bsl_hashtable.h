/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_hashtable.h,v 1.12 2009/10/14 08:24:58 chenxm Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file bsl_hashmap.h
 * @author xiaowei(com@baidu.com)
 * @date 2008/07/12 21:04:29
 * @version $Revision: 1.12 $ 2010/10/15 modified by zhjw(zhujianwei@baidu.com) 
 * @brief 
 *  
 **/


#ifndef  __BSL_HASHTABLE_H_
#define  __BSL_HASHTABLE_H_

#include <bsl/alloc/bsl_alloc.h>
#include <bsl/alloc/bsl_sample_alloc.h>
#include <string.h>
#include <bsl/containers/hash/bsl_hashutils.h>
#include <bsl/utils/bsl_utils.h>
#include <bsl/utils/bsl_debug.h>
#include <bsl/exception/bsl_exception.h>

namespace bsl
{

template <typename _Tp, class _Alloc>
struct bsl_hashtable_node_t;

template <typename _Tp, class _Alloc>
struct bsl_hashtable_node_t
{
	_Tp val;
	typedef typename _Alloc::template rebind<bsl_hashtable_node_t>::other::pointer pointer;
	pointer next;
};

template <class _Key, class _Value, class _HashFun, class _Equl, class _GetKey, class _InnerAlloc> 
class bsl_hashtable;

template <class _Key, class _Value, class _HashFun, class _Equl, class _GetKey, class _InnerAlloc> 
struct bsl_hashtable_iterator;

template <class _Key, class _Value, class _HashFun, class _Equl, class _GetKey, class _InnerAlloc> 
struct bsl_hashtable_const_iterator;

/**
 * @brief hash表迭代器实现
 */
template <class _Key, class _Value, class _HashFun, class _Equl, class _GetKey, class _InnerAlloc> 
struct bsl_hashtable_iterator
{
	typedef bsl_hashtable<_Key, _Value, _HashFun, _Equl, _GetKey, _InnerAlloc> hashtable;
	typedef bsl_hashtable_iterator<_Key, _Value, _HashFun, _Equl, _GetKey, _InnerAlloc> iterator;
	typedef bsl_hashtable_const_iterator<_Key, _Value, _HashFun, 
                                        _Equl, _GetKey, _InnerAlloc> const_iterator;
	typedef bsl_hashtable_node_t<_Value, _InnerAlloc> node_t;
	typedef typename hashtable::node_pointer node_pointer;

	typedef std::forward_iterator_tag iterator_category;
	typedef _Value value_type;
	typedef ptrdiff_t	difference_type;
	typedef value_type * pointer;
	typedef value_type & reference;

	hashtable *_ht;
	size_t _bucketpos;
	node_pointer _node;

public:
	bsl_hashtable_iterator() { 
        _ht = 0; 
        _bucketpos = 0; 
        _node = 0; 
    }
	bsl_hashtable_iterator(const iterator &iter) {
		_ht = iter._ht;
		_bucketpos = iter._bucketpos;
		_node = iter._node;
	}
    /*
    //TODO:这个构造函数语义有问题，而且暂时没有用户使用过（因为编译时会报错），以后考虑删掉
	bsl_hashtable_iterator (const const_iterator & iter) {
		_ht = iter._ht; 
		_bucketpos = iter._bucketpos;
		_node = iter._node;
	}
    */
	bsl_hashtable_iterator(hashtable *ht, size_t bp, node_pointer node) {
        _ht = ht;
		_bucketpos = bp;
		_node = node;
        if(_ht != 0){
    		while (_node == 0 && _bucketpos < _ht->_bitems) {
	    		_node = _ht->_bucket[_bucketpos];
		    	if (_node == 0) {
			    	++_bucketpos;
			    }
		    }
        }
	}  
	reference operator * () {
        if(0 == _ht){
            throw bsl::NullPointerException()<<BSL_EARG
                <<"can not dereference a null interator.";
        }
		return _ht->_sp_alloc.getp(_node)->val;
	}
	pointer operator -> () {
        if(0 == _ht){
            throw bsl::NullPointerException()<<BSL_EARG
                <<"can not evaluate using a null interator.";
        }
		return &(_ht->_sp_alloc.getp(_node)->val);
	}

	//这里可以优化
	bool operator == (const iterator & iter) const {
		return (_node == iter._node);
	}
	bool operator != (const iterator & iter) const {
		return (_node != iter._node);
	}

	iterator & operator ++ () {
        if(0 == _ht){
            throw bsl::NullPointerException()<<BSL_EARG
                <<"can not increase a null interator.";
        }
		if (_node) {
			_node = _ht->_sp_alloc.getp(_node)->next;
			if (_node) {
				return *this;
			}
		}
		for (size_t i = _bucketpos + 1; i < _ht->_bitems; ++i) {
			_node = _ht->_bucket[i];
			if (_node) {
				_bucketpos = i;
				return *this;
			}
		}
		_bucketpos = _ht->_bitems;
		return *this;
	}
	iterator operator ++ (int) {
		iterator iter = *this;
		++ *this;
		return iter;
	}
};

/**
 * @brief hashtable的只读迭代器
 */
template <class _Key, class _Value, class _HashFun, class _Equl, class _GetKey, class _InnerAlloc> 
struct bsl_hashtable_const_iterator
{
	typedef bsl_hashtable<_Key, _Value, _HashFun, _Equl, _GetKey, _InnerAlloc> hashtable;
	typedef bsl_hashtable_iterator<_Key, _Value, _HashFun, _Equl, _GetKey, _InnerAlloc> iterator;
	typedef bsl_hashtable_const_iterator<_Key, _Value, _HashFun, 
                                        _Equl, _GetKey, _InnerAlloc> const_iterator;

	typedef bsl_hashtable_node_t<_Value, _InnerAlloc> node_t;
	typedef typename hashtable::node_pointer node_pointer;
	typedef std::forward_iterator_tag iterator_category;
	typedef _Value value_type;
	typedef ptrdiff_t	difference_type;

	typedef const value_type * pointer;
	typedef const value_type & reference;

	const hashtable *_ht;
	size_t _bucketpos;
	node_pointer _node;

public:
	bsl_hashtable_const_iterator() { 
        _ht = 0; 
        _bucketpos = 0; 
        _node = 0; 
    }
	bsl_hashtable_const_iterator(const const_iterator &iter) {
		_ht = iter._ht;
		_bucketpos = iter._bucketpos;
		_node = iter._node;
	}
	bsl_hashtable_const_iterator(const iterator &iter) {
		_ht = iter._ht;
		_bucketpos = iter._bucketpos;
		_node = iter._node;
	}
	bsl_hashtable_const_iterator(const hashtable *ht, size_t bp, node_pointer node) {
        _ht = ht;
		_bucketpos = bp;
		_node = node;
        if(_ht != 0){
		    while (_node == 0 && _bucketpos < _ht->_bitems) {
			    _node = _ht->_bucket[_bucketpos];
			    if (_node == 0) {
				    ++_bucketpos;
			    }
		    }
        }
	}
	reference operator * () const {
        if(0 == _ht){
            throw bsl::NullPointerException()<<BSL_EARG
                <<"can not dereference a null interator.";
        }
		return _ht->_sp_alloc.getp(_node)->val;
	}
	pointer operator -> () const {
        if(0 == _ht){
            throw bsl::NullPointerException()<<BSL_EARG
                <<"can not evaluate using a null interator.";
        }
		return &(_ht->_sp_alloc.getp(_node)->val);
	}

	//这里可以优化
	bool operator == (const const_iterator & iter) const {
		return (_node == iter._node);
	}
	bool operator != (const const_iterator & iter) const {
		return (_node != iter._node);
	}

	const_iterator & operator ++ () {
        if(0 == _ht){
            throw bsl::NullPointerException()<<BSL_EARG
                <<"can not increase a null interator.";
        }
        if (_node) {
			_node = _ht->_sp_alloc.getp(_node)->next;
			if (_node) {
				return *this;
			}
		}
		for (size_t i = _bucketpos + 1; i < _ht->_bitems; ++i) {
			_node = _ht->_bucket[i];
			if (_node) {
				_bucketpos = i;
				return *this;
			}
		}
		_bucketpos = _ht->_bitems;
		return *this;
	}
	const_iterator operator ++ (int) {
		const_iterator iter = *this;
		++ *this;
		return iter;
	}
};

//默认bitems大小
//modified by chenyanling@baidu.com
const size_t HASHTABLE_BITEMS = 64;

/**
 * @brief 普通的hashtable
 */
template <class _Key	//hash key
	, class _Value	//存储的数据 能萃取除key值
	, class _HashFun	//hash 函数
	, class _Equl	//key值的比较函数
	, class _GetKey	//从 _Value萃取出key值的函数
	, class _InnerAlloc	//维护状态数据的内部allocator
	>
class bsl_hashtable
{
public:
	typedef bsl_hashtable<_Key, _Value, _HashFun, _Equl, _GetKey, _InnerAlloc> _Self;
	typedef _Value value_type;
	typedef _Key key_type;
	typedef value_type * pointer;

	typedef bsl_hashtable_node_t<value_type, _InnerAlloc> node_t;
	typedef typename _InnerAlloc::pool_type pool_type;

	typedef typename _InnerAlloc::template rebind<node_t>::other sample_alloc_type;
	typedef typename sample_alloc_type::pointer node_pointer;
	typedef typename pool_type::template rebind<node_pointer >::other node_p_alloc_type;

	friend struct bsl_hashtable_iterator<_Key, _Value, _HashFun, _Equl, _GetKey, _InnerAlloc>;
	friend struct bsl_hashtable_const_iterator<_Key, _Value, _HashFun, 
                                                _Equl, _GetKey, _InnerAlloc>;

public:
	sample_alloc_type _sp_alloc;
	node_p_alloc_type _np_alloc;

	node_pointer *_bucket;
	size_t _bitems;
	size_t _size;

	_HashFun _hashfun;
	_Equl _equl;
	_GetKey _getkey;

public:
	typedef bsl_hashtable_iterator<_Key, _Value, _HashFun, _Equl, _GetKey, _InnerAlloc> iterator;
	typedef bsl_hashtable_const_iterator<_Key, _Value, _HashFun, 
                                        _Equl, _GetKey, _InnerAlloc> const_iterator;

	typedef std::pair<const_iterator, const_iterator> const_iterator_pair;
	typedef std::pair<iterator, iterator> iterator_pair;
	
	iterator _end_iter;

public:
    /**
     * @brief 构造函数
     * 无异常
     * 需调create
     */
	bsl_hashtable() {
		_reset();
    }

    /**
     * @brief 带桶大小的构造函数，抛异常
     *        如果构造失败，将销毁对象
     * 不需调create
     *
     * @param  [in] bitems  : size_t hash桶的大小
     * @param  [in] hf      : const _HashFun& hash函数
     * @param  [in] eq      : const _Equl& hash结点的比较仿函数
     */
	bsl_hashtable(size_t bitems, const _HashFun& hf = _HashFun(), const _Equl& eq = _Equl()) {
        _reset();
        if (create(bitems,hf,eq) != 0) {
            destroy();
            throw BadAllocException()<<BSL_EARG
                <<"create error when create hashtable bitems "<<bitems;
        }
    }

    /**
     * @brief 拷贝构造函数, 抛异常
     *        如果构造失败，将销毁对象
     *  
     * @param  [in] other   : const _Self&
     * @return
     * @retval
     * @see
     * @author liaoshangbin
     * @data 2010/08/27 18:02:50
     **/
	bsl_hashtable(const _Self& other) {
        _reset();
        if (assign(other.begin(),other.end()) != 0) {
            destroy();
            throw BadAllocException()<<BSL_EARG<<"assign hashtable error";
        }
    }

    /**
     * @brief 赋值运算符，抛异常
     *        先获得自身的一个拷贝，如果赋值失败，将恢复成原来的hashtable
     *          
     * @param
     * @return
     * @retval
     * @see
     * @author liaoshangbin
     * @data 2010/08/27 19:56:47
     **/
	_Self & operator = (const _Self& other) {
        if (this != &other) {
            _Self self_(*this); // 先获得自身的一个拷贝
            if (assign(other.begin(),other.end()) != 0) {
                destroy();
                *this = self_;
                throw BadAllocException()<<BSL_EARG<<"assign hashtable error";
            }
        }
        return *this; 
    }

	~bsl_hashtable() {
		destroy();
	}

	/**
	 * @brief 创建hash表
	 *
	 * @param [in/out] bitems   : size_t	hash桶的大小
	 * @param [in] hf	: const _HashFun 	hash函数
	 * @param [in/out] eq   : const _Equl&	hash结点的比较仿函数
	 * @return  int 成功返回0，其他返回失败
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2008/08/12 17:09:10
	**/
	int create(size_t bitems, const _HashFun &hf = _HashFun(), const _Equl &eq = _Equl()) {
		//判断溢出
		if (bitems >= size_t(-1) / sizeof(void *)) {
			__BSL_ERROR("too large bitems, overflower");
			return -1;
		}

		if (bitems == 0) {
			__BSL_ERROR("bitems == 0");
			return -1;
		}

		if (destroy() != 0) {
			__BSL_ERROR("destroy error when create hash bitems[%lu]", (unsigned long)bitems);
			return -1;
		}
		if (recreate(bitems) != 0) {
			__BSL_ERROR("recreate error when create hash bitems[%lu]", (unsigned long)bitems);
			return -1;
		}
		_hashfun = hf;
		_equl = eq;
		return 0;
	}

    /* *
     * @brief 判断hashtable是否已初始化
     * @author zhujianwei
     * @date 2010/12/13
     * */
    bool is_created() const{
        return (_bucket != 0);
    }

	//destroy分两部分，一个有构造函数，一个没有构造函数
	int destroy() {
		//具体实现
		if (_bucket) {
			if (sample_alloc_type::recycle_space) {
				bsl::bsl_destruct(begin(), end());
			} else {
				node_pointer nd = 0;
				for (size_t i=0; i<_bitems; ++i) {
					while (_bucket[i]) {
						nd = _bucket[i];
						_bucket[i] = _sp_alloc.getp(_bucket[i])->next;
						//nd->val.~value_type();
						bsl::bsl_destruct(&(_sp_alloc.getp(nd)->val));
						_sp_alloc.deallocate(nd, 1);
					}
				}
			}
		}
		_sp_alloc.destroy();

		if (_bucket) {
                    _np_alloc.deallocate(_bucket, _bitems);
		}
		_np_alloc.destroy();
		_bitems = 0;
		_bucket = 0;

		_size = 0;
		_end_iter._bucketpos = 0;
		return 0;
	}

	iterator begin() {
		return iterator(this, 0, 0);
	}

	const_iterator begin() const {
		return const_iterator(this, 0, 0);
	}

	iterator end()  {
		return _end_iter;
	}

	const_iterator end() const {
		return const_iterator(this, _bitems, 0);
	}
	
	size_t size() const {
		return _size;
	}

	/**
	 * @brief hash表中键值等于key的个数
	 * @param 	[in/out] hashval	: const size_t		hash值
	 * @param       [in/out] k              : const key_type	键值
	 * @param       [in/out] flag           : int			相同<k,v>是否重复计数
	 * 								flag == 0，重复计数
	 * 								flag != 0，不重复计数(目前没有实现，只是留作接口)
	 * @return  	size_t		返回键值等于key的个数
	 * @author luowei
	 * @date 2012/08/27 17:11:02
	 **/
	size_t count(size_t hashval, const key_type &k, int flag = 0)
	{
		if(0 == _bitems || 0 == _bucket)
		{
			return 0;
		}
		size_t __result = 0;
		node_pointer lst = _bucket[hashval % _bitems];
		while(lst && !_equl(_getkey(_sp_alloc.getp(lst)->val),k))
		{
			lst = _sp_alloc.getp(lst)->next;
		}
		if(lst)
		{
			__result = 1;
			lst = _sp_alloc.getp(lst)->next;
			while(lst && _equl(_getkey(_sp_alloc.getp(lst)->val), k))
			{
				++__result;
				lst = _sp_alloc.getp(lst)->next;
			}
		}
		return __result;
	}

public:
	/**
	 * @brief 用其他容器对hash表进行附值操作
	 *
	 * @param [in/out] __begin   : _Iterator	迭代器的起始地址
	 * @param [in/out] __end   : _Iterator	迭代器的结束地址
	 * @return 	int 0	表示成功附值，其他失败
	 * 			如果返回出错，请调用destroy，销毁该对象
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2008/08/12 17:11:02
	**/
	template <class _Iterator>
	int assign(_Iterator __begin, _Iterator __end)
	{
		size_t bkt = 0;
		bkt = std::distance(__begin, __end) * 2;
		if (bkt < _bitems) {
			bkt = _bitems;
		}
		destroy();
		if (recreate(bkt) != 0) {
			return -1;
		}
		for (_Iterator iter = __begin; iter != __end; ++iter) {
			if (set(_hashfun(_getkey(*iter)), _getkey(*iter), *iter) == -1) {
				return -1;
			}
		}
		return 0;
	}

	/**
	 * @brief 用其他的map容器对hash表进行附值，允许重复key
	 * @param [in/out] __begin	: _Iterator	迭代器的起始地址
	 * @param [in/out] __end	: _Iterator	迭代器的结束地址
	 * @param [in/out] flag		: int			0表示不允许重复<key,value>；非0表示允许.默认为0
	 * @return  	int		返回0表示删除成功，其他表示失败
	 *
	 * @author luowei
	 * @date 2012/08/28 19:40:02
	 **/
	template <class _Iterator>
	int assign_multimap(_Iterator __begin, _Iterator __end, int flag = 0)
	{
		size_t bkt = 0;
		bkt = std::distance(__begin, __end) * 2;
		if(bkt < _bitems)
		{
			bkt = _bitems;
		}
		destroy();
		if(recreate(bkt) != 0)
		{
			return -1;
		}
		for(_Iterator iter = __begin; iter != __end; ++iter)
		{
			if (set_multimap(_hashfun(_getkey(*iter)), _getkey(*iter), (*iter).second, flag) == -1) 
			{
				return -1;
			}
		}
		return 0;
	}

	/**
	 * @brief 根据hash值，和key查找元素
	 *
	 * @param [in/out] hashval   : size_t
	 * @param [in/out] k   : const key_type&
	 * @return  value_type* 存在返回值指针,否这返回NULL
	 * @retval   
	 * @see 
	 * @note 
	 * @author xiaowei
	 * @date 2008/08/13 12:11:39
	**/
	value_type * find (size_t hashval, const key_type &k) const {
        if(0 == _bitems || 0 == _bucket){
            return NULL;
        }
		node_pointer lst = _bucket[hashval % _bitems];
		while (lst) {
			if (_equl(_getkey(_sp_alloc.getp(lst)->val), k)) {
				return &(_sp_alloc.getp(lst)->val);
			}
			lst = _sp_alloc.getp(lst)->next;
		}
		return NULL;
	}
	value_type * find (size_t hashval, const key_type &k) {
        if(0 == _bitems || 0 == _bucket){
            return NULL;
        }
		node_pointer lst = _bucket[hashval % _bitems];
		while (lst) {
			if (_equl(_getkey(_sp_alloc.getp(lst)->val), k)) {
				return &(_sp_alloc.getp(lst)->val);
			}
			lst = _sp_alloc.getp(lst)->next;
		}
		return NULL;
	}

	/**
	 * @brief 根据hash值，和key/value查找元素
	 * @param [in/out] hashval   	: size_t
	 * @param [in/out] k   		: value_type &		val
	 * @return  		Hash表为空返回-1
	 *			找到<key,val>对	返回HASH_EXIST 
	 *			否则		返回HASH_NOEXIST
	 * @author luowei
	 * @date 2012/08/28 19:11:39
	 **/
	template <typename pair_sec_type>
	int find_pair(size_t hashval, const key_type &k, const pair_sec_type &val)
	{
		if(0 ==_bitems || 0 == _bucket)
		{
			return -1;
		}

		//value值的比较函数
		std::equal_to<pair_sec_type> _val_equl;
		
		node_pointer lst = _bucket[hashval % _bitems];

		while(lst)
		{
			if(_equl(_sp_alloc.getp(lst)->val.first, k))
			{
				while(lst && _equl(_sp_alloc.getp(lst)->val.first, k))
				{
					if(_val_equl(_sp_alloc.getp(lst)->val.second, val))
					{
						return HASH_EXIST;
					}
					lst = _sp_alloc.getp(lst)->next;
				}
				break;
			}
			lst = _sp_alloc.getp(lst)->next;
		}
		return HASH_NOEXIST;
	}
	
	template <typename pair_sec_type>
	int find_pair(size_t hashval, const key_type &k, const pair_sec_type &val) const
	{
		if(0 ==_bitems || 0 == _bucket)
		{
			return -1;
		}

		//value值的比较函数
		std::equal_to<pair_sec_type> _val_equl;

		node_pointer lst = _bucket[hashval % _bitems];
		while(lst)
		{
			if(_equl(_sp_alloc.getp(lst)->val.first, k))
			{
				while(lst && _equl(_sp_alloc.getp(lst)->val.first, k))
				{
					if(_val_equl(_sp_alloc.getp(lst)->val.second, val))
					{
						return HASH_EXIST;
					}
					lst = _sp_alloc.getp(lst)->next;
				}
				break;
			}
			lst = _sp_alloc.getp(lst)->next;
		}
		return HASH_NOEXIST;
	}
	
	/**
	 * @brief 根据hashval和key在hash_multimap查找元素，返回所有的key为k的<k,v>
	 *
	 * @param [in/out] hashval   : size_t
	 * @param [in/out] k   : const key_type&
	 * @return  _const_iterator_pair		返回key对应的range的首、尾迭代器的pair
	 * @retval  
	 * @see 
	 * @note 
	 * @author luowei
	 * @date 2012/08/21 18:55:39
	 **/
	const_iterator_pair find_multimap (size_t hashval, const key_type &k) const
	{
		if(0 == _bitems || 0 == _bucket)
		{
			return const_iterator_pair(end(), end());
		}
		size_t key = hashval % _bitems;
		node_pointer __first = _bucket[key];//记录找到的第一个迭代器位置
		node_pointer __cur = NULL;
		for (;__first; __first= _sp_alloc.getp(__first)->next) 
		{
			if (_equl(_sp_alloc.getp(__first)->val.first, k))
			{
				for(__cur = _sp_alloc.getp(__first)->next; __cur; __cur= _sp_alloc.getp(__cur)->next)
				{
					if(!_equl(_sp_alloc.getp(__cur)->val.first, k))
					{
						return const_iterator_pair(const_iterator(this, key, __first),
								            const_iterator(this, key, __cur));
					}
				}
				for(size_t __m = key + 1; __m < _bitems; ++__m)
				{
					if(_bucket[__m])
					{
						return const_iterator_pair(const_iterator(this, key, __first),
								            const_iterator(this, __m, _bucket[__m]));
					}
				}
				return const_iterator_pair(const_iterator(this, key, __first), end());
			}
		}
		return const_iterator_pair(end(), end());
	}

	iterator_pair find_multimap (size_t hashval, const key_type &k) 
	{
		if(0 == _bitems || 0 == _bucket)
		{
			return iterator_pair(end(), end());
		}
		size_t key = hashval % _bitems;
		node_pointer __first = _bucket[key];//记录找到的第一个迭代器位置
		node_pointer __cur = NULL;
		for (;__first; __first= _sp_alloc.getp(__first)->next)
		{
			if (_equl(_sp_alloc.getp(__first)->val.first, k))
			{
				for(__cur = _sp_alloc.getp(__first)->next; __cur; __cur= _sp_alloc.getp(__cur)->next)
				{
					if(!_equl(_sp_alloc.getp(__cur)->val.first, k))
					{
						return iterator_pair(iterator(this, key, __first),
									iterator(this, key, __cur));
					}
				}
				for(size_t __m = key + 1; __m < _bitems; ++__m)
				{
					if(_bucket[__m])
					{
						return iterator_pair(iterator(this, key, __first),
								    	iterator(this, __m, _bucket[__m]));
					}
				}
				return iterator_pair(iterator(this, key, __first), end());
			}
		}
		return iterator_pair(end(), end());
	}

	 //根据行为产生一个copy on write的还有直接附值的hash表
	/**
	 * @brief 将val插入hash表中
	 *
	 * @param [in/out] hashval   : size_t	hash key值
	 * @param [in/out] k   : const key_type&	key 值
	 * @param [in/out] val   : const value_type&	数据值
	 * @param [in/out] flag   : int	0表示如果值存在，直接返回，非0表示替换旧值
	 * @return  int 
	 * 				返回	-1	表示set调用出错, (无法分配新结点)
	 * 			其他均表示插入成功：插入成功分下面三个状态
	 * 				返回	HASH_OVERWRITE	表示覆盖旧结点成功(在flag非0的时候返回）
	 * 				返回	HASH_INSERT_SEC	表示插入新结点成功
	 * 				返回	HASH_EXIST	表示hash表结点存在（在flag为0的时候返回)
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2008/08/12 20:56:27
	**/
	int set (size_t hashval, const key_type &k, const value_type &val, int flag = 0) {
        if(0 == _bitems || 0 == _bucket){
            return -1;
        }
		size_t key = hashval % _bitems;
		node_pointer lst = _bucket[key];
		while (lst) {
			if (_equl(_getkey(_sp_alloc.getp(lst)->val), k)) {
				if (flag) {
					_sp_alloc.getp(lst)->val = val;
					return HASH_OVERWRITE;
				}
				return HASH_EXIST;
			}
			lst = _sp_alloc.getp(lst)->next;
		}
		node_pointer node = _sp_alloc.allocate(1);
		if (0 == node) {
			return -1;
		}
		bsl::bsl_construct(&(_sp_alloc.getp(node)->val), val);
		//::new (&node->val) value_type(val);
		_sp_alloc.getp(node)->next = _bucket[key];
		_bucket[key] = node;
		++ _size;
		return HASH_INSERT_SEC;
	}

	template <typename pair_sec_type>
	int set_map(size_t hashval, const key_type &k, const pair_sec_type &val, int flag = 0) {
        if(0 == _bitems || 0 == _bucket){
            return -1;
        }
		size_t key = hashval % _bitems;
		node_pointer lst = _bucket[key];
		while (lst) {
			if (_equl(_sp_alloc.getp(lst)->val.first, k)) {
				if (flag) {
					_sp_alloc.getp(lst)->val.second = val;
					return HASH_OVERWRITE;
				}
				return HASH_EXIST;
			}
			lst = _sp_alloc.getp(lst)->next;
		}
		node_pointer node = _sp_alloc.allocate(1);
		if (node == 0) {
			return -1;
		}
		::new (&(_sp_alloc.getp(node)->val)) value_type(k, val);
		_sp_alloc.getp(node)->next = _bucket[key];
		_bucket[key] = node;
		++ _size;
		return HASH_INSERT_SEC;
	}

	/**
	 * @brief 向hash_multimap中插入<key,value>
	 * @param        [in] hashval:       const hashval           hash值
	 * @param        [in] k      :       const key_type          key值
	 * @param        [in] val    :       const value_type&       value值 
	 * @param        [in] flag   : int  flag==0,不允许重复<k,v>;flag!=0,允许重复<k,v>.默认为不允许重复 
	 * @return       int 
	 *       返回    -1      表示set调用出错, (无法分配新结点，或者hash表未create)
	 *       其他均表示插入成功：插入成功分下面2个状态
	 *                     返回    HASH_INSERT_SEC 表示插入新结点成功
	 *                     返回    HASH_EXIST      表示hash表结点存在(在flag为0的时候返回)
	 * @retval
	 * @author luowei
	 * @date 2012/08/27 11:11:02
	 * */
	template <typename pair_sec_type>
	int set_multimap(size_t hashval, const key_type &k, const pair_sec_type &val, int flag = 0)
	{
		if(0 == _bitems || 0 == _bucket)
		{
			return -1;
		}
		std::equal_to<pair_sec_type> _val_equl;
		size_t key = hashval % _bitems;
		node_pointer lst = _bucket[key];
		node_pointer front = 0;
		while(lst)
		{
			if(_equl(_sp_alloc.getp(lst)->val.first, k))
			{
				if(0 == flag)
				{
					while(lst && _equl(_sp_alloc.getp(lst)->val.first, k))
					{
						if(_val_equl(_sp_alloc.getp(lst)->val.second, val))
						{
							return HASH_EXIST;
						}
						lst = _sp_alloc.getp(lst)->next;
					}
				}
				break;
			}
			//遍历_bucket[key]链表里的元素，直到键值为k
			front = lst;
			lst = _sp_alloc.getp(lst)->next;
		}
		node_pointer node = _sp_alloc.allocate(1);
		if(0 == node)
		{
			return -1;
		}
		::new(&(_sp_alloc.getp(node)->val)) value_type(k,val);

		//在第一个键值为k的结点之前插入，若无hash值为key的结点，则直接在链表尾部插入
		if(0 == front)
		{
			_sp_alloc.getp(node)->next = _bucket[key];
			_bucket[key] = node;
		}
		else
		{
			_sp_alloc.getp(node)->next = _sp_alloc.getp(front)->next;
			_sp_alloc.getp(front)->next = node;
		}
		++_size;
		return HASH_INSERT_SEC;
	}
public:

	/**
	 * @brief 根据指定key删除结点
	 *
	 * @param [in/out] hashval   : size_t
	 * @param [in/out] k   : const key_type&
	 * @return  int 
	 * 		返回	HASH_EXIST表示结点存在并删除成功
	 * 		返回	HASH_NOEXIST表示结点不存在不用删除
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2008/08/12 21:00:58
	**/
	int erase (size_t hashval, const key_type &k) {
        if(0 == _bitems || 0 == _bucket){
            return HASH_NOEXIST;
        }
		size_t key = hashval % _bitems;
		node_pointer lst = _bucket[key];
		node_pointer front = 0;
		while (lst != 0) {
			if (_equl(_getkey(_sp_alloc.getp(lst)->val), k)) {
				if (front == 0) {
					_bucket[key] = _sp_alloc.getp(lst)->next;
				} else {
					_sp_alloc.getp(front)->next = _sp_alloc.getp(lst)->next;
				}
				bsl::bsl_destruct(&(_sp_alloc.getp(lst)->val));
				//lst->val.~value_type();
				_sp_alloc.deallocate(lst, 1 );
				-- _size;
				return HASH_EXIST;
			}
			front = lst;
			lst = _sp_alloc.getp(lst)->next;
		}
		return HASH_NOEXIST;
	}

	/**
	 * @brief 根据指定key删除hash_multimap中的结点，并返回删除的结点个数
	 *
	 * @param [in] hashval  	 : size_t
	 * @param [in] k  		 : const key_type&
	 * @return  int			 : 删除的结点个数 
	 * @retval 
	 * @see 
	 * @author xiaowei
	 * @date 2012/08/30 21:00:58
	 **/
	int erase_multimap(size_t hashval, const key_type &k)
	{
		if(0 == _bitems || 0 == _bucket)
		{
			return -1;
		}

		size_t key = hashval % _bitems;
		node_pointer lst = _bucket[key];
		node_pointer front = 0;
		node_pointer first = 0;
		node_pointer tmp = 0;
		int __result = 0;

		while (lst != 0) 
		{
			if (_equl(_sp_alloc.getp(lst)->val.first, k))
			{
				first = lst;
				while (lst != 0 && _equl(_sp_alloc.getp(lst)->val.first, k)) 
				{
					lst = _sp_alloc.getp(lst)->next;
				}
				if (front == 0) 
				{
					_bucket[key] = lst;
				}
				else
				{
					_sp_alloc.getp(front)->next = lst;
				}
				tmp = first->next;
				while(first != lst)
				{
					bsl::bsl_destruct(&(_sp_alloc.getp(first)->val));
					_sp_alloc.deallocate(first, 1 );
					-- _size;
					++__result;

					if(!tmp)
					{
						break;
					}
					first = tmp;
					tmp = _sp_alloc.getp(tmp)->next;
					
				}
				return __result;
			}
			front = lst;
			lst = _sp_alloc.getp(lst)->next; 
		}
		return 0;
	}

	/**
	 * @brief 根据指定key和指定value删除hash_multimap中的结点，并返回删除的结点个数
	 *
	 * @param [in] hashval           : size_t
	 * @param [in] k                 : const key_type&
	 * @param [in] val    		 : const value_type&       value值 
	 * @return  int
	 * 		返回	删除的结点个数 
	 * @retval
	 * @see
	 * @author luowei
	 * @date 2012/08/30 21:00:58
	 **/
	template <typename pair_sec_type>
	int erase_pair (size_t hashval, const key_type &k,  const pair_sec_type &val) 
	{
		if(0 == _bitems || 0 == _bucket)
		{
			return -1;
		}
		std::equal_to<pair_sec_type> _val_equl;
		size_t key = hashval % _bitems;
		node_pointer lst = _bucket[key];
		node_pointer front = 0;
		int __result= 0;

		while (lst != 0) 
		{
			if (_equl(_sp_alloc.getp(lst)->val.first, k)) 
			{
				if (_val_equl(_sp_alloc.getp(lst)->val.second, val))
				{
					if (0 == front) 
					{
						_bucket[key] = _sp_alloc.getp(lst)->next;
					}
					else
					{
						_sp_alloc.getp(front)->next = _sp_alloc.getp(lst)->next;
					}
					bsl::bsl_destruct(&(_sp_alloc.getp(lst)->val));
					_sp_alloc.deallocate(lst, 1 );
					--_size;

					++__result;

					if (0 == front)
					{
						lst = _bucket[key];
					}
					else
					{
						lst = _sp_alloc.getp(front)->next;
					}
					continue;
				}
			}
			front = lst;
			lst = _sp_alloc.getp(lst)->next;
		}

		return __result;

	}


public:
	template <class _Archive>
	int serialization(_Archive & ar) {
		if (bsl::serialization(ar, _bitems)) {
			return -1;
		}
		__BSL_DEBUG("write _bitems=%ld", (long)_bitems);
		if (bsl::serialization(ar, _size)) {
			return -1;
		}
		__BSL_DEBUG("write _size=%ld", (long)_size);
		if (_size > 0) {
			for (iterator iter = begin(); iter != end(); ++iter) {
				if (bsl::serialization(ar, *iter)) {
					return -1;
				}
			}
		}
		return 0;
	}

	template <class _Archive>
	int deserialization(_Archive & ar) {
		size_t bitems = 0;
		if (bsl::deserialization(ar, bitems)) {
			__BSL_ERROR("deserialization bitems error");
			return -1;
		}
		__BSL_DEBUG("bitems = %ld", (long)bitems);

		//已经被初始化过
		if (_bucket) {
			if (clear()) {
				__BSL_ERROR("clear phashtable error");
				return -1;
			}
		} else {	//没有初始化过
			if (create(bitems, _HashFun(), _Equl())) {
				__BSL_ERROR("create hashtable error");
				return -1;
			}
		}

		size_t __size = 0;
		if (bsl::deserialization(ar, __size)) {
			__BSL_ERROR("deserialization size error");
			return -1;
		}
		value_type val;
		for (size_t i=0; i<__size; ++i) {
			if (bsl::deserialization(ar, val)) {
				__BSL_ERROR("deserialization val at[%ld] error", (long)i);
				return -1;
			}
			if (set(_hashfun(_getkey(val)), _getkey(val), val) == -1) {
				__BSL_ERROR("deserialization set at[%ld] error", (long)i);
				return -1;
			}
		}

		return 0;
	}

	/**
	 * @brief 数据反串行化，允许有重复key
	 *
	 * @param 	[in/out] ar		: _Archive &
	 * @return  	int		0:正确返回，-1:错误
	 * @author luowei
	 * @date 2012/08/21 17:11:02
	**/
	template <class _Archive>
	int deserialization_multimap(_Archive & ar) 
	{
		size_t bitems = 0;
		if (bsl::deserialization(ar, bitems)) 
		{
			__BSL_ERROR("deserialization bitems error");
			return -1;
		}
		__BSL_DEBUG("bitems = %ld", (long)bitems);

		if (_bucket)
		{
			clear();
		}
		else
		{
			if (create(bitems, _HashFun(), _Equl()))
			{
				__BSL_ERROR("create hashtable error");
				return -1;
			}
		}

		size_t __size = 0;
		if (bsl::deserialization(ar, __size)) 
		{
			__BSL_ERROR("deserialization size error");
			return -1;
		}
		value_type val;
		for (size_t i=0; i<__size; ++i)
		{
			if (bsl::deserialization(ar, val))
			{
				__BSL_ERROR("deserialization val at[%ld] error", (long)i);
				return -1;
			}
			if (set_multimap(_hashfun(_getkey(val)), val.first, val.second, 1) == -1) 
			{
				printf("deserialization set at[%ld] error", (long)i);
				return -1;
			}
		}
		return 0;
	}

	int clear() {
		for (size_t i=0; i<_bitems; ++i) {
			node_pointer nd = 0;
			while (_bucket[i]) {
				nd = _bucket[i];
				_bucket[i] = _sp_alloc.getp(_bucket[i])->next;
				bsl::bsl_destruct(&_sp_alloc.getp(nd)->val);
				_sp_alloc.deallocate(nd, 1);
			}
		}
		_size = 0;
		return 0;
	}
      
protected:
	int recreate(size_t bitems) {
		_bitems = bitems;
                //默认_bitems大小
                //modified by chenyanling@baidu.com
                //2011/11/15
                if(_bitems == 0)
                {
                    _bitems = HASHTABLE_BITEMS;
                }

		_sp_alloc.create();
		_np_alloc.create();
		_bucket = _np_alloc.allocate(_bitems);
		if (_bucket == 0) {
			goto err;
		}
		memset(_bucket, 0, sizeof(node_pointer ) * _bitems);

		//初始化end 迭代器
		_end_iter._ht = this;
		_end_iter._bucketpos = _bitems;
		_end_iter._node = 0;

		_size = 0;
		return 0;
    err:
		_sp_alloc.destroy();
		_np_alloc.destroy();
		_bitems = 0;
		_end_iter._bucketpos = 0;
		return -1;
	}
private:
    void _reset(){
        _bitems = 0;
		_bucket = 0;
		_end_iter._ht = this;
		_end_iter._bucketpos = 0;
		_end_iter._node = 0;
		_size = 0;
    }
};

}

#endif  //__BSL_HASHTABLE_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
