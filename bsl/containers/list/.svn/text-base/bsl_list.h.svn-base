/* *
 * @brief 双向链表
 * @author yufan
 * @version  2010/10/15 modified by zhjw(zhujianwei@baidu.com)
 *
 * */

#ifndef _BSL_LIST_H__
#define _BSL_LIST_H__

#include <bsl/utils/bsl_utils.h>
#include <bsl/alloc/bsl_alloc.h>
#include <bsl/alloc/bsl_sample_alloc.h>
#include <bsl/archive/bsl_serialization.h> 
#include <bsl/exception/bsl_exception.h>

namespace bsl
{

template <typename _Tp, typename _Alloc>
struct list_node_t
{
	typedef _Tp value_type;
	typedef list_node_t<_Tp, _Alloc> node_t;
	typedef typename _Alloc::template rebind<node_t>::other node_alloc_t;
	typedef typename node_alloc_t::pointer node_pointer;
	node_pointer next;
	node_pointer prev;
	value_type val;
};

template <class _Tp, class _Alloc > class list;

/**
 * @brief list只读迭代器
 */
template<class _Tp, class _Ptr, class _Ref, class _Alloc>
class list_const_iterator
{
public:
	typedef std::bidirectional_iterator_tag iterator_category;
	typedef _Tp value_type;    
	typedef _Ptr pointer;
	typedef _Ref reference;
        typedef ptrdiff_t difference_type;
private:
	typedef list_const_iterator<_Tp, _Ptr, _Ref, _Alloc> self;
	typedef list<_Tp, _Alloc> list_t;
	typedef typename list_t::iterator iterator;
	typedef list_node_t<value_type, _Alloc> node_t;
	typedef typename _Alloc::template rebind<node_t>::other node_alloc_t;
	typedef typename node_alloc_t::pointer node_pointer;
	typedef typename node_alloc_t::const_pointer node_const_pointer;
public:
	/**
	 * @brief 
	 * 建立一个未初始化的迭代器
	 * @see 
	 * @author yufan
	 * @date 2008/08/07 11:22:25
	**/
	list_const_iterator()
	{
		_node = 0;
	}
	/**
	 * @brief 
	 * 从另一个迭代器复制，指向统一份内容 
	 * @param [in] __x   : const self& 要拷贝的迭代器
	 * @see 
	 * @author yufan
	 * @date 2008/08/07 11:22:36
	**/
	list_const_iterator(const self & __x)
	{
		_node = __x._node;
	}
	list_const_iterator(const iterator & __x)
	{
		_node = __x._node;
	}
	reference operator *() const
	{
		return _node->val;
	}
	bool operator == (const self & __iter) const
	{
		return (_node == __iter._node);
	}
	bool operator != (const self & __iter) const
	{
		return (_node != __iter._node);
	}
	pointer operator ->() const
	{
		return &_node->val;
	}
	self & operator ++ ()
	{
		_node = _node->next;
		return *this;
	}
	self operator ++ (int)
	{
		self iter = *this;
		++ *this;
		return iter;
	}
	self & operator --()
	{
		_node = _node->prev;
		return *this;
	}
	self operator --(int)
	{
		self iter = *this;
		-- *this;
		return iter;
	}
	/**
	 * @brief 
	 * 从一个指向节点的内存指针建立迭代器，内部使用
	 * @param [in] __x   : node_pointer
	 * @see 
	 * @author yufan
	 * @date 
	**/
	list_const_iterator(node_pointer __x)
	{
		_node = __x;
	}
	
	list_const_iterator(node_const_pointer __x)
	{
		_node = const_cast<node_pointer>(__x);
	}
private:
	node_pointer _node;
};

/**
 * @brief list迭代器
 */
template<class _Tp, class _Ptr, class _Ref, class _Alloc>
class list_iterator
{
public:
	typedef std::bidirectional_iterator_tag iterator_category;
	typedef _Tp value_type;
	typedef _Ptr pointer;
	typedef _Ref reference;
        typedef ptrdiff_t difference_type;
	typedef const _Tp * const_pointer;	//常空间地址指针
	typedef const _Tp & const_reference;	//常空间地址指针
private:
	typedef list_iterator<_Tp, _Ptr, _Ref, _Alloc> self;
	typedef list<_Tp, _Alloc> list_t;
	typedef list_node_t<value_type, _Alloc> node_t;
	typedef typename _Alloc::template rebind<node_t>::other node_alloc_t;
	typedef typename node_alloc_t::pointer node_pointer;
	typedef typename node_alloc_t::const_pointer node_const_pointer;
	friend class list<_Tp, _Alloc>;
public:
	/**
	 * @brief 
	 * 建立一个未初始化的迭代器
	 * @see 
	 * @author yufan
	 * @date 2008/08/07 11:22:25
	**/
	list_iterator()
	{
        _list = 0;
		_node = 0;
	}
	/**
	 * @brief 
	 * 从另一个迭代器复制，指向统一份内容 
	 * @param [in] __x   : const self& 要拷贝的迭代器
	 * @see 
	 * @author yufan
	 * @date 2008/08/07 11:22:36
	**/
	list_iterator(const self & __x)
	{
        _list = __x._list;
		_node = __x._node;
	}
	reference operator *() const
	{
		return _node->val;
	}
	bool operator == (const self & __iter) const
	{
		return (_node == __iter._node);
	}
	bool operator != (const self & __iter) const
	{
		return (_node != __iter._node);
	}
	pointer operator ->() const
	{
		return &_node->val;
	}
	self & operator ++ ()
	{
		_node = _node->next;
		return *this;
	}
	self operator ++ (int)
	{
		self iter = *this;
		++ *this;
		return iter;
	}
	self & operator --()
	{
		_node = _node->prev;
		return *this;
	}
	self operator --(int)
	{
		self iter = *this;
		-- *this;
		return iter;
	}
	/**
	 * @brief 
	 * 从一个指向节点的内存指针建立迭代器，内部使用
	 * @param [in] __x   : node_pointer
	 * @see 
	 * @author yufan
	 * @date 
	**/
	list_iterator(list_t* __l, node_pointer __x)
	{
        _list = __l;
		_node = __x;
	}
	
	list_iterator(list_t* __l, node_const_pointer __x)
	{
        _list = const_cast<list_t*>(__l);
		_node = const_cast<node_pointer>(__x);
	}
public:
    node_pointer _node;
private:
    list_t* _list;
};

/**
 * @brief list反向只读迭代器
 */
template<class _Tp, class _Ptr, class _Ref, class _Alloc>
class list_const_reverse_iterator
{
public:
	typedef std::bidirectional_iterator_tag iterator_category;
	typedef _Tp value_type;
	typedef _Ptr pointer;
	typedef _Ref reference;
        typedef ptrdiff_t difference_type;
private:
	typedef list_const_reverse_iterator<_Tp, _Ptr, _Ref, _Alloc> self;
	typedef list<_Tp, _Alloc> list_t;
	typedef typename list_t::reverse_iterator reverse_iterator;
	typedef list_node_t<value_type, _Alloc> node_t;
	typedef typename _Alloc::template rebind<node_t>::other node_alloc_t;
	typedef typename node_alloc_t::pointer node_pointer;
	typedef typename node_alloc_t::const_pointer node_const_pointer;
	friend class list<_Tp, _Alloc>;
public:
	list_const_reverse_iterator()
	{
		_node = 0;
	}
	list_const_reverse_iterator(const reverse_iterator & __x)
	{
		_node = __x._node;
	}
	list_const_reverse_iterator(const self& __x)
	{
		_node = __x._node;
	}
	reference operator *() const
	{
		return _node->val;
	}
	bool operator == (const self & __iter) const
	{
		return (_node == __iter._node);
	}
	bool operator != (const self & __iter) const
	{
		return (_node != __iter._node);
	}
	pointer operator ->() const
	{
        return &_node->val;
	}
	self & operator -- ()
	{
		_node = _node->next;
		return *this;
	}
	self operator -- (int)
	{
		self iter = *this;
		-- *this;
		return iter;
	}
	self & operator ++()
	{
		_node = _node->prev;
		return *this;
	}
	self operator ++(int)
	{
		self iter = *this;
		++ *this;
		return iter;
	}
	/**
	 * @brief 
	 *
	 * @param [in/out] __x   : node_pointer
	 * @see 
	 * @author yufan
	 * @date 2008/08/07 11:21:37
	**/
	list_const_reverse_iterator(node_pointer __x)
	{
		_node = __x;
	}
	list_const_reverse_iterator(node_const_pointer __x)
	{
		_node = const_cast<node_pointer>(__x);
	}
private:
    node_pointer _node;
};
/**
 * @brief list反向迭代器
 */
template<class _Tp, class _Ptr, class _Ref, class _Alloc>
class list_reverse_iterator
{
public:
	typedef std::bidirectional_iterator_tag iterator_category;
	typedef _Tp value_type;
	typedef _Ptr pointer;
	typedef _Ref reference;
        typedef ptrdiff_t difference_type;
	typedef list_const_reverse_iterator<_Tp, const _Tp *, 
                                        const _Tp &, _Alloc> const_reverse_iterator;
private:
	typedef list_reverse_iterator<_Tp, _Ptr, _Ref, _Alloc> self;
	typedef list<_Tp, _Alloc> list_t;
	typedef list_node_t<value_type, _Alloc> node_t;
	typedef typename _Alloc::template rebind<node_t>::other node_alloc_t;
	typedef typename node_alloc_t::pointer node_pointer;
	typedef typename node_alloc_t::const_pointer node_const_pointer;
	friend class list<_Tp, _Alloc>;
public:
	list_reverse_iterator()
	{
        _list = 0;
		_node = 0;
	}
	list_reverse_iterator(const self& __x)
	{
        _list = __x._list;
		_node = __x._node;
	}
	reference operator *() const
	{
		return _node->val;
	}
	bool operator == (const self & __iter) const
	{
		return (_node == __iter._node);
	}
	bool operator != (const self & __iter) const
	{
		return (_node != __iter._node);
	}
	pointer operator ->() const
	{
		return &_node->val;
	}
	self & operator -- ()
	{
        _node = _node->next;
		return *this;
	}
	self operator -- (int)
	{
		self iter = *this;
		-- *this;
		return iter;
	}
	self & operator ++()
	{
        _node = _node->prev;
		return *this;
	}
	self operator ++(int)
	{
		self iter = *this;
		++ *this;
		return iter;
	}
	/**
	 * @brief 
	 *
	 * @param [in/out] __x   : node_pointer
	 * @see 
	 * @author yufan
	 * @date 2008/08/07 11:21:37
	**/
	list_reverse_iterator(list_t* __l, node_pointer __x)
	{
        _list = __l;
		_node = __x;
	}
	list_reverse_iterator(list_t* __l, node_const_pointer __x)
	{
        _list = const_cast<list_t*>(__l);
		_node = const_cast<node_pointer>(__x);
	}
public:
    node_pointer _node;
private:
    list_t* _list;
};

/**
 * @brief list双向链表
 */
template <class _Tp, class _Alloc = bsl_sample_alloc<bsl_alloc<_Tp>, 256> >
class list
{
public:
	typedef _Tp value_type;	//数据类型定义
	typedef _Tp *  pointer;	//空间地址指针
	typedef const _Tp * const_pointer;	//常空间地址指针
	typedef _Tp & reference; 
	typedef const _Tp & const_reference; 
	typedef size_t size_type;
	typedef long difference_type;
	typedef list_iterator<value_type, pointer, reference, _Alloc> iterator;
	typedef list_const_iterator<value_type, const_pointer, 
                                            const_reference, _Alloc> const_iterator;
	typedef list_reverse_iterator<value_type, pointer, reference, _Alloc> reverse_iterator;
	typedef list_const_reverse_iterator<value_type, const_pointer, 
                                            const_reference, _Alloc> const_reverse_iterator;
protected:
	typedef list_node_t<_Tp, _Alloc> node_t;
	typedef typename _Alloc::template rebind<node_t>::other node_alloc_t;
	typedef typename node_alloc_t::pointer node_pointer;
	typedef typename node_alloc_t::const_pointer node_const_pointer;
	//typedef typename node_alloc_t::reference node_reference;
	//typedef typename node_alloc_t::const_reference node_const_reference;
	//friend class list_iterator<value_type, pointer, reference, _Alloc>;
	//friend class list_iterator<value_type, const_pointer, const_reference, _Alloc>;
	//friend class list_reverse_iterator<value_type, pointer, reference, _Alloc>;
	//friend class list_reverse_iterator<value_type, const_pointer, const_reference, _Alloc>;
	
private:
	struct list_node_place_holder_t
	{
		node_pointer next;
		node_pointer prev;
		//由于链表的根节点在程序中被多次调用，
		//为了避免在程序中显式将node_holder_t*转换为node_t*, 定义以下的自动类型转换
		operator node_pointer ()
		{
			return reinterpret_cast<node_pointer>(this);
		}
		operator node_const_pointer () const
		{
			return reinterpret_cast<node_const_pointer>(this);
		}
	};
	typedef list_node_place_holder_t node_holder_t;
	typedef list<_Tp,_Alloc> self;
	node_holder_t _root;
	node_alloc_t _node_allocator;
    size_type _size;
    
public:

    /* *
     *  @brief list默认构造函数
     *  无异常
     *  不需调create
     * */
    list(){
        _reset();
        create();
    }

    /**
	 * @brief 
	 * 拷贝构造函数
     * 如果拷贝失败，抛出出异常，恢复成原来的空list
     *
	 * @see 
	 * @author yufan
	 * @date 2008/09/04 14:59:47
	**/
	list(const self& other)
	{
        _reset();
        create();
        if (assign(other.begin(),other.end()) != 0) {
            throw BadAllocException()<<BSL_EARG<<"assign list error";
        }
	}

	/**
	 * @brief  赋值函数，抛异常
     * 如果插入失败，将恢复成原list，强异常安全保证
     * 如果删除失败，从末尾删除结点，不会造成资源泄漏，基本异常安全保证
	 * @return  self operator 
	 * @retval   
	 * @see 
	 * @author yufan
	 * @date 2008/09/04 15:00:16
	**/
	self& operator = (const self& other)
	{
        if (this != &other) {
            size_t size1    = size(); // size()的时间复杂度为O(1)
            size_t size2    = other.size();
            if (size2 > size1) { // this中需要添加结点
                // other反向遍历，this从前插入结点
                const_reverse_iterator rfirst2 = other.rbegin();
                for (size_t i = size1; i < size2; i ++) {
                    if (push_front( *rfirst2 ) != 0) {
                        for (size_t j = size1; j < i; j ++) {
                            pop_front();
                        }
                        throw BadAllocException()<<BSL_EARG
                            <<"push_front list error size["<<size1<<"~"<<size2<<"]";
                    }
                    ++ rfirst2;
                }
                // size = 0无需调整头结点
                if (size1 != 0) {
                    reverse_iterator rfirst1 = rbegin();
                    for (size_t i = 0; i < size1; i ++) {
                        *rfirst1 ++ = *rfirst2 ++;
                    }
                    // rfirst1为实际begin()的前一位 
                    _root.prev->next = _root.next;
                    _root.next->prev = _root.prev;
                    
                    _root.next = rfirst1._node->next;
                    _root.prev = rfirst1._node;

                    rfirst1._node->next->prev = _root;
                    rfirst1._node->next = _root;
                }
            } else { // size1 >= size2
                // 从末尾删除结点
                for (size_t i = size1; i > size2; -- i) {
                    if ( pop_back() != 0 ) {
                        throw BadAllocException()<<BSL_EARG
                            <<"pop_back list error index["<<i<<"] size["<<size1<<"~"<<size2<<"]";
                    }
                }
                iterator first1         = begin();
                iterator last1          = end();
                const_iterator first2   = other.begin();
                const_iterator last2    = other.end();
                while ( first1 != last1 && first2 != last2 ) {
                    *first1 ++ = *first2 ++;
                }
            }       
        }
		return *this; 
	}

	/**
	 * @brief 
	 * 建立链表的内部结构和内存分配器
     * 如果push_back一些元素再create将会造成内存泄露
     *
	 * @return  int 成功返回0，其他返回-1
	 * @retval   成功返回0，其他返回-1
	 * @see 
	 * @author yufan
	 * @date 
	**/
	int create()
	{
		_node_allocator.create(); //allocator的无参数create总是成功
        return 0;
	}

	/**
	 * @brief 
	 * 删除链表并清除占用的内存
	 * @return  int 
	 * @retval  成功返回0，其他返回-1
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 18:12:44
	**/
	int destroy()
	{
		this->clear();
		_node_allocator.destroy();
		return 0;

	}
	/**
	 * @brief 
	 * 析构函数
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:22:36
	**/
	~list()
	{
		destroy();
	}
	/**
	 * @brief 
	 * 从__x中复制节点，如果__x是其本身，则跳过
	 * @param [in] __x   : self&
	 * @return  int 
	 * @retval   成功返回0，其他返回-1
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:22:47
	**/
	int assign (const self & __x)
	{
		if (this == &__x)
		{
			return 0;
		}
		else
		{
			return assign(__x.begin(), __x.end());
		}
	}

	/**
	 * @brief 
	 * 从迭代其中复制链表，迭代器可以是指向其自身的
	 * @param [in] __begin   : InputIterator 开始节点
	 * @param [in] __end   : InputIterator 指向结束节点（不包括该节点）
	 * @return  template <typename InputIterator>	 	int 
	 * @retval   成功返回0，其他返回-1
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:22:25
	**/
	template <typename InputIterator>	
	int assign(InputIterator __begin, InputIterator __end)
	{
		node_holder_t tmp_root;
		tmp_root.prev = tmp_root;
		node_pointer cur = tmp_root;
		InputIterator tp = __begin;
        size_type n = 0;
		while(tp != __end)
		{
			node_pointer tmp = _node_allocator.allocate(1);
			if (!tmp) {
				goto err1;
            }
			bsl::bsl_construct(&tmp->val, *tp);
			tmp->prev = cur;
			cur->next = tmp;
			cur = tmp;
			++tp;
            ++n;
		}
		cur->next = tmp_root;
		tmp_root.prev = cur;
		clear();
		_root.next = tmp_root.next;
		tmp_root.next->prev = _root;
		_root.prev = tmp_root.prev;
		tmp_root.prev->next = _root;
        _size = n;
		return 0;
	err1:
		while(cur->prev != tmp_root)
		{
			node_pointer tmp = cur->prev;
			bsl::bsl_destruct(&(cur->val));
			_node_allocator.deallocate(cur, 1);
			cur = tmp;

		}
		return -1;
	}

	/**
	 * @brief 
	 * 清除全部节点
	 * @return  int 
	 * @retval   成功返回0，其他返回-1
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:23:02
	**/
	int clear()
	{
		node_pointer cur = _root.next;
		while(cur != _root)
		{
			node_pointer tmp = cur->next;
			bsl::bsl_destruct(&(cur->val));
			_node_allocator.deallocate(cur, 1);
			cur = tmp;
            --_size;
		}
		_root.next = _root.prev = _root;
        _size = 0;
		return 0;
	}

	/**
	 * @brief 
	 * 返回容器是否包含内容
	 * @return  bool 
	 * @retval  容器为空返回true，否则返回false
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:22:55
	**/
	bool empty() const
	{
		return _root.prev == _root;
	}

	/**
	 * @brief 
	 * 从头部增加新节点
	 * @param [in] __x   : const value_type& 被加入的元素
	 * @return  int 
	 * @retval   成功返回0，其他返回-1
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:23:14
	**/
	inline int push_front(const value_type &__x)
	{
		node_pointer tmp = _node_allocator.allocate(1);
		if (!tmp) {
			goto err;
        }
		bsl::bsl_construct(&tmp->val, __x);
		tmp->next = _root.next;
		tmp->prev = _root;
		_root.next->prev = tmp;
		_root.next = tmp;
        ++_size;
		return 0;
	err:
		return -1;
	}

	/**
	 * @brief 
	 * 删除头节点
	 * @return  int 
	 * @retval   成功返回0，其他返回-1
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:22:14
	**/
	int pop_front()
	{
		return erase(begin());
		/*
		if(_root.next == _root)
		{
			return -1;
		}
		else
		{
			node_pointer tmp = _root.next;
			_root.next = tmp->next;
			tmp->next->prev = _root;
			bsl::bsl_destruct(&(tmp->val));
			_node_allocator.deallocate(tmp, 1);
			return 0;
		}*/
	}

	/**
	 * @brief 
	 * 从尾部增加节点
	 * @param [in] __x   : const value_type&
	 * @return  int 
	 * @retval   成功返回0，其他返回-1
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:22:07
	**/
	int push_back(const value_type &__x)
	{
		node_pointer tmp = _node_allocator.allocate(1);
		if (!tmp) {
			goto err;
        }
		bsl::bsl_construct(&tmp->val, __x);
		tmp->prev = _root.prev;
		tmp->next = _root;
		_root.prev->next = tmp;
		_root.prev = tmp;
        ++_size;
		return 0;
	err:
		return -1;
	}

	/**
	 * @brief 
	 * 删除尾节点
	 * @return  int 
	 * @retval   成功返回0，其他返回-1
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:21:57
	**/
	int pop_back()
	{
    	if(_root.prev == _root)
		{
			return -1;
		}
		else
		{
			node_pointer tmp = _root.prev;
			_root.prev = tmp->prev;
			tmp->prev->next = _root;
			bsl::bsl_destruct(&(tmp->val));
			_node_allocator.deallocate(tmp, 1);
            --_size;
			return 0;
		}
	}

	/**
	 * @brief 
	 * 在__i指向的元素前，增加由__s和__e指定的节点
	 * @param [in] __i   : iterator
	 * @param [in] __s   : InputIterator
	 * @param [in] __e   : InputIterator
	 * @return  template <typename InputIterator> 	int 
	 * @retval   成功返回0，其他返回-1
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:22:03
	**/
	template <typename _InputIterator>
	inline int insert( iterator __i,  _InputIterator __s, _InputIterator __e)
	{
		typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
		return(_aux_insert(__i, __s, __e, _Integral()));
	}
	template<typename InputIterator> 
	inline int _aux_insert(iterator __i, InputIterator __s, InputIterator __e,
			__true_type)
	{
		return _insert(__i, __s, __e);
	}
	template<typename InputIterator> 
	int _aux_insert(iterator __i, InputIterator __s, InputIterator __e, 
			__false_type)
	{
        if ( __s == __e ) {
            return 0;
        }

       	InputIterator tp = __s;
		node_holder_t tmp_root;
		tmp_root.prev = tmp_root;
		node_pointer cur = tmp_root;
        size_type n = 0;
		while(tp!=__e)
		{
			node_pointer tmp = _node_allocator.allocate(1);
			if (!tmp) {
				goto err;
            }
			bsl::bsl_construct(&tmp->val, *tp);
			tmp->prev = cur;
			cur->next = tmp;
			cur = tmp;
			++tp;
            ++n;
		}
		__i._node->prev->next = tmp_root.next;
		tmp_root.next->prev = __i._node->prev;
		cur->next = __i._node;
		__i._node->prev = cur;
        __i._list->_size += n;
		return 0;
	err:
		while(cur->prev != tmp_root)
		{
			node_pointer tmp = cur->prev;
			bsl::bsl_destruct(&(cur->val));
			_node_allocator.deallocate(cur, 1);
			cur = tmp;
		}
		return -1;
	}
	/**
	 * @brief 
	 * 在由__i指向的接点之前，增加__n个值为__x的元素
	 * @param [in] __i   : iterator 指向插入位置
	 * @param [in] __n   : size_type 插入元素的数量
	 * @param [in] __x   : const value_type& 待插入的元素
	 * @return  int 
	 * @retval   成功返回0，其他返回-1
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:21:39
	**/

	inline int insert( iterator __i, size_type __n, const value_type &__x)
	{
		return _insert(__i, __n, __x);
	}
	int _insert( iterator __i, size_type __n, const value_type &__x)
	{
        if ( __n == 0 ) { 
            return 0;
        }
       
		node_holder_t tmp_root;
		tmp_root.prev = tmp_root;
		node_pointer cur = tmp_root;
		size_type tcount = __n;
		while(tcount)
		{
			node_pointer tmp = _node_allocator.allocate(1);
			if (!tmp) {
				goto err;
            }
			bsl::bsl_construct(&tmp->val, __x);
			tmp->prev = cur;
			cur->next = tmp;
			cur = tmp;
			tcount--;
		}
		__i._node->prev->next = tmp_root.next;
		tmp_root.next->prev = __i._node->prev;
		cur->next = __i._node;
		__i._node->prev = cur;
        __i._list->_size += __n;
		return 0;
	err:
		while(cur->prev != tmp_root)
		{
			node_pointer tmp = cur->prev;
			bsl::bsl_destruct(&(cur->val));
			_node_allocator.deallocate(cur, 1);
			cur = tmp;

		}
		return -1;
	}
    
	/**
	 * @brief 
	 * 在节点__i之前，增加__x节点
	 * @param [in] __i   : iterator
	 * @param [in] __x   : const value_type&
	 * @return  int 
	 * @retval   成功返回0，其他返回-1
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:21:48
	**/
	int insert( iterator __i, const value_type &__x)
	{
		node_pointer tmp = _node_allocator.allocate(1);
		if (!tmp) {
			goto err;
        }
		bsl::bsl_construct(&tmp->val, __x);
		tmp->next = __i._node;
		tmp->prev = __i._node->prev;
		__i._node->prev->next = tmp;
		__i._node->prev = tmp;
        __i._list->_size += 1;
		return 0;
	err:
		return -1;
	}

	/**
	 * @brief 
	 * 和__x容器交换内容
	 * @param [in/out] __x   : self&
	 * @return  int 
	 * @retval   成功返回0，其他返回-1
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:21:42
	**/
	int swap( self & __x)
	{
		_node_allocator.swap(__x._node_allocator);
		std::swap(_root, __x._root);
		if(_root.next != __x._root)
		{
			_root.next->prev = _root;
			_root.prev->next = _root;
		}
		else
		{
			_root.next = _root;
			_root.prev = _root;
		}
		if ( __x._root.next != _root)
		{
			__x._root.next->prev = __x._root;
			__x._root.prev->next = __x._root;
		}
		else
		{
			__x._root.next = __x._root;
			__x._root.prev = __x._root;
		}
        std::swap(_size, __x._size);
		//check();
		//__x.check();
		return 0;
	}


	/**
	 * @brief 
	 * 删除__s和__e之间的节点
	 * @param [in] __s   : iterator
	 * @param [in] __e   : iterator
	 * @return  int 
	 * @retval   成功返回0，其他返回-1
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:21:35
	**/
	int erase ( iterator __s, iterator __e)
	{
		node_pointer prev = __s._node->prev;
		node_pointer cur = __s._node;
		node_pointer t_end = __e._node;
        size_type n = 0;
		while(cur != t_end)
		{
			node_pointer tmp = cur->next;
			bsl::bsl_destruct(&(cur->val));
			_node_allocator.deallocate(cur, 1);
			cur = tmp;
            ++n;
		}
		prev->next = t_end;
		t_end->prev = prev;
        __s._list->_size -= n;
        return 0;
	}

	/**
	 * @brief 
	 * 删除 __x指向的节点
	 * @param [in] __x   : iterator
	 * @return  int 
	 * @retval   成功返回0，其他返回-1
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 20:22:22
	**/
	int erase ( iterator __x)
	{
		if(_root.next == _root || __x._node == (node_pointer)&_root) {
			return -1;
        }
		node_pointer cur = __x._node;
		cur->next->prev = cur->prev;
		cur->prev->next = cur->next;
		bsl::bsl_destruct(&(cur->val));
		_node_allocator.deallocate(cur, 1);
        __x._list->_size -= 1;
		return 0;
	}
	
	/**
	 * @brief 
	 * 返回头的常引用
	 * @return  const_reference 
	 * @retval   
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:21:04
	**/
	inline const_reference front() const
	{
		if(_root.next == _root) {
			throw bsl::InvalidOperationException()<<BSL_EARG
                <<"no front item in empty list.";
        }
		return _root.next->val;
	}
	
	/**
	 * @brief 
	 * 返回头元素的引用
	 * @return  reference 
	 * @retval   
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:21:11
	**/
	inline reference front()
	{
		if(_root.next == _root) {
            throw bsl::InvalidOperationException()<<BSL_EARG
                <<"no front item in empty list.";
        }
		return _root.next->val;
	}
    
	/**
	 * @brief 
	 * 返回尾节点的常引用
	 * @return  const_reference 
	 * @retval   
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:21:16
	**/
	inline const_reference back() const
	{
		if(_root.next == _root) {
            throw bsl::InvalidOperationException()<<BSL_EARG
                <<"no back item in empty list.";
        }
		return _root.prev->val;
	}
    
	/**
	 * @brief 
	 * 返回尾节点的引用
	 * @return  reference 
	 * @retval   
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:21:21
	**/
	inline reference back()
	{
		if(_root.next == _root) {
            throw bsl::InvalidOperationException()<<BSL_EARG
                <<"no front item in empty list.";
        }
		return _root.prev->val;
	}

	/**
	 * @brief 
	 * 返回指向头节点的常迭代器
	 * @return  const_iterator 
	 * @retval   
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:21:27
	**/
	inline const_iterator begin() const
	{
		return const_iterator(_root.next);
	}

	/**
	 * @brief 
	 * 返回指向头节点的迭代器
	 * @return  iterator 
	 * @retval   
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:20:55
	**/
	inline iterator begin()
	{
		return iterator(this, _root.next);
	}

	/**
	 * @brief 
	 * 返回指向尾节点的常迭代器
	 * @return  const_iterator 
	 * @retval   
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:20:52
	**/
	inline const_iterator end() const
	{
		return const_iterator(_root);
	}

	/**
	 * @brief 
	 * 返回指向尾节点的迭代器
	 * @return  iterator 
	 * @retval   
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:20:50
	**/
	inline iterator end()
	{
		return iterator(this, _root);
	}

	/**
	 * @brief 
	 * 返回指向首部的反向常迭代器
	 * @return  const_reverse_iterator 
	 * @retval   
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:20:47
	**/
	inline const_reverse_iterator rbegin() const
	{
		return const_reverse_iterator(_root.prev);
	}

	/**
	 * @brief 
	 * 返回指向首节点的反向常迭代器
	 * @return  reverse_iterator 
	 * @retval   
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:20:40
	**/
	inline reverse_iterator rbegin()
	{
		return reverse_iterator(this, _root.prev);
	}

	/**
	 * @brief 
	 * 返回指向尾节点的反向常迭代器
	 * @return  const_reverse_iterator 
	 * @retval   
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:20:38
	**/
	inline const_reverse_iterator rend() const
	{
		return const_reverse_iterator(_root);
	}
	/**
	 * @brief 
	 * 返回指向尾节点的反向迭代器
	 * @return  reverse_iterator 
	 * @retval   
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:20:33
	**/
	inline reverse_iterator rend()
	{
		return reverse_iterator(this, _root);
	}

	/**
	 * @brief 
	 * 返回容器包含的节点数量
	 * @return  size_type 
	 * @retval   
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:20:30
	**/
	size_type size() const
	{
        return _size;
	}

	/**
	 * @brief 
	 * 返回容器所能容纳的最大节点数量
	 * @return  size_type 
	 * @retval   
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:20:27
	**/
	inline size_type max_size() const
	{
		return (size_type)(-1);
	}

	/*
	 * @brief 
	 * 通过增加或删除尾部节点，使容器的节点数量等于__n
	 * @param [in] __n   : size_type 所需要的容器大小
	 * @param [in] __x   : value_type 如果需要扩大容器，用于插入的元素
	 * @return  int 
	 * @retval  成功返回0，其他返回-1 
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:20:24
	**/
	int resize(size_type __n, value_type __x = value_type())
	{
		size_type tp = size();
		if(tp > __n)
		{
			for(difference_type i = 0; i < (difference_type)( tp-__n); i++)
			{
				if(pop_back()) {
					goto err;
                }
			}
		}
		else
		{
			for(difference_type i=0; i < (difference_type)( __n-tp); i++)
			{
				if(push_back(__x)) {
					goto err;
                }
			}
		}
		return 0;
	err:
		return -1;
	}
    
	/**
	 * @brief 
	 * 将__l中的全部节点插入到__x指向的结点之前，并从__l中删除,
     * __l必须是一个不同于__x所在的容器
	 * @param [in/out] __x   : iterator
	 * @param [in/out] __l   : self&
	 * @return  int 
	 * @retval   
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:20:10
	**/
	int splice(iterator __x, self & __l)
	{
		if(__l._root.prev != __l._root.next)
		{
            size_type n = __l.size();
			_node_allocator.merge(__l._node_allocator);
			node_pointer cur = __x._node;
			cur->prev->next = __l._root.next;
			__l._root.next->prev = cur->prev;
			cur->prev = __l._root.prev;
			__l._root.prev->next = cur;
			__l._root.next = __l._root.prev = __l._root;
            __x._list->_size += n;
            __l._size = 0;
		}
		return 0;
	}
	
#if 0
	/**
	 * @brief 
	 * 将__l中__p指向的节点删除，并插入到当前容器的__x指向的节点之间
	 * @param [in/out] __x   : iterator
	 * @param [in/out] __l   : self&
	 * @param [in/out] __p   : iterator
	 * @return  int 
	 * @retval   
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:20:14
	**/
	int splice(iterator __x, self & __l, iterator __p)
	{

		if(__p != __l.end())
		{
			node_pointer cur = __x._node;
			node_pointer target = __p._node;
			target->prev->next = target->next;
			target->next->prev = target->prev;
			cur->prev->next = target;
			target->prev = cur->prev;
			cur->prev = target;
			target->next = cur;
		}
		return 0;
	}

	/**
	 * @brief 
	 * 将__l中[__p,__e)区间的节点从__l中删除，并插入到__x节点之前。
	 * @param [in/out] __x   : iterator
	 * @param [in/out] __l   : self&
	 * @param [in/out] __p   : iterator
	 * @param [in/out] __e   : iteratot
	 * @return  int 
	 * @retval   
	 * @see 
	 * @author yufan
	 * @date 2008/08/15 11:54:23
	**/
	int splice(iterator __x, self & __l, iterator __p, iterator __e)
	{

		if (__p != __e)
		{
			node_pointer cur = __x._node;
			node_pointer end = __e._node->prev;
			node_pointer target = __p._node;

			target->prev->next = __e._node;
			__e._node->prev = target->prev;

			
			cur->prev->next = target;
			target->prev = cur->prev;

			cur->prev = end;
			end->next = cur;
			target->next->prev = target->prev;
		}
		return 0;
	}
#endif
    
	/**
	 * @brief 
	 * 从链表中删除与__x值相同的实例
	 * @param [in] __x   : const value_type&
	 * @return  int 
	 * @retval  成功返回0，其他返回-1 
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 15:54:50
	**/
	inline int remove(const value_type & __x)
	{
		return remove_if(std::bind2nd(std::equal_to<_Tp>(), __x));
	}
	/**
	 * @brief 
	 * 根据条件函数删除元素
	 * @param [in] __p   : Predicate
	 * @return  template<class Predicate>  	int 
	 * @retval   成功返回0，其他返回-1
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 15:56:19
	**/
	template<class Predicate> 
	int remove_if(Predicate __p) 
	{
		node_pointer cur = _root.next;
		while(cur != _root)
		{
			if(__p(cur->val))
			{
				node_pointer tmp = cur;
				cur = cur->next;
				tmp->prev->next = tmp->next;
				tmp->next->prev = tmp->prev;
				bsl::bsl_destruct(&(tmp->val));
				_node_allocator.deallocate(tmp, 1);
                --_size;
			}
			else
			{
				cur = cur->next;
			}
		}
		return 0;
	}
    
	/**
	 * @brief 
	 * 将链表反序排列
	 * @return  int 
	 * @retval   成功返回0，其他返回-1
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:07:48
	**/
	int reverse()
	{
		node_pointer cur = _root.next;
		_root.next = _root.prev;
		_root.prev = cur;
		while(cur != _root)
		{
			node_pointer tmp = cur->next;
			cur->next = cur->prev;
			cur->prev = tmp;
			cur = tmp;
		}
		return 0;
	}

	/**
	 * @brief 
	 * 序列化
	 * @param [in] ar   : _Archive&
	 * @return  template <typename _Archive> 	int 序列化方法提供类
	 * @retval   成功返回0，其他返回-1
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:19:29
	**/
	template <typename _Archive>
	int serialization(_Archive &ar)
	{
		size_t l = size();
		if(bsl::serialization(ar, l)) {
			return -1;
        }
		for(iterator i=begin(); i!=end(); i++)
		{
			if(bsl::serialization(ar, *i)) {
				return -1;
            }
		}
		return 0;
	}

	/**
	 * @brief 
	 * 反序列化
	 * @param [in] ar   : _Archive&
	 * @return  template <typename _Archive> 	int 反序列化方法提供类
	 * @retval   成功返回0，其他返回-1
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:19:47
	**/
	template <typename _Archive>
	int deserialization(_Archive &ar)
	{
		size_t l;
		clear();
		if(bsl::deserialization(ar, l)) {
			return -1;
        }
		iterator cur = begin();
		for(size_t i=0; i<l; i++)
		{
			value_type tmp;
			if(bsl::deserialization(ar, tmp)) {
				goto err;
            }
			if(insert(cur, tmp)) {
				goto err;
            }
		}
		return 0;
err:
		clear();
		return -1;
	}
	
	/**
	 * @brief 
	 * 将两个已排序的链表合并，合并后，__other将为空
	 * @param [in/out] __other   : self&
	 * @return  int 
	 * @retval   
	 * @see 
	 * @author yufan
	 * @date 2008/08/19 20:56:03
	**/
	int merge(self & __other)
	{
		return merge(__other, std::less<_Tp>());
	}

	/**
	 * @brief 
	 * 将两个已排序的链表合并，合并后，__other将为空
	 * @param [in/out] __other   : self&
	 * @param [in/out] __p   : _Predicator
	 * @return  template <typename _Predicator> 	int 
	 * @retval   
	 * @see 
	 * @author yufan
	 * @date 2008/08/19 20:56:06
	**/
	template <typename _Predicator>
	int merge(self & __other, _Predicator __p)
	{
		if(_root.next != _root && __other._root.next != __other._root)
		{
            size_type other_size = __other.size();
			_node_allocator.merge(__other._node_allocator);
			_root.prev->next = 0;
			__other._root.prev->next = 0;
			_root.next = _merge(
                    (node_pointer)(_root.next), 
                    (node_pointer)(__other._root.next), 
                    __p
                    );
			__other._root.prev = __other._root.next = __other._root;
			_relink();
            _size += other_size;
            __other._size = 0;
			return 0;
		}
		else if(__other._root.next != __other._root)
		{
			return swap(__other);
		}
		else
		{
			return 0;
		}
	}
#define __BSL_LIST_STACK_DEPTH 64
	/**
	 * @brief 
	 * 按递增顺序排序
	 * @return  int 
	 * @retval  成功返回0，其他返回-1 
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:08:57
	**/
	int sort()
	{
		return sort(std::less<_Tp>());
	}
	/**
	 * @brief 
	 * 根据仿函数返回的结果对链表排序
	 * @param [in/out] _p   : Predicate& 仿函数
	 * @return 	int 
	 * @retval  成功返回0，其他返回-1
	 * @see 
	 * @author yufan
	 * @date 2008/08/18 14:34:00
	**/
	template <typename Predicate>	
	int sort(Predicate _p = std::less<_Tp>())
	{
		if( _root.next == _root) {
			return 0;
        }
		node_pointer cur = _root.next;
		node_pointer counter[__BSL_LIST_STACK_DEPTH ];
		for(int i=0; i < __BSL_LIST_STACK_DEPTH ; i++)
		{
			counter[i] = 0;
		}
	//	int fill=0;
		while( cur != _root)
		{
			node_pointer tmp = cur;
			cur = cur->next;
			tmp->next = 0;
			int i = 0;
			while(counter[i] != 0)
			{
				tmp = _merge(tmp,counter[i], _p);
				counter[i]=0;
				i++;
			}
			counter[i] = tmp;
	//		if(i > fill)
	//			fill = i;
		}
		for(int i = 1; i < __BSL_LIST_STACK_DEPTH ; i++)
		{
			counter[i] = _merge(counter[i-1], counter[i], _p);
		}
		_root.next = counter[__BSL_LIST_STACK_DEPTH - 1];
		_relink();
		return 0;
	}
private:

    void _reset(){
	    _root.next = _root.prev = _root;
        _size = 0;
    }

	void _relink()
	{
		node_pointer t_begin = _root;
		do
		{
			t_begin->next->prev = t_begin;
			t_begin = t_begin->next;
		}
		while(t_begin->next != 0);
		t_begin->next = _root;
		_root.prev = t_begin;
	}
	template <typename _Predicate>
	inline node_pointer _merge(node_pointer list1, node_pointer list2, const _Predicate &_p)
	{
		if(0 == list2) {
			return list1;
        }
		if(0 == list1) {
			return list2;
        }
		volatile node_holder_t tmp_root;
		node_pointer tp = (node_pointer)&tmp_root;
		node_pointer a = list1;
		node_pointer b = list2;
		while(true)
		{
			if(_p(a->val,b->val))
			{
				tp->next = a;
				if(a->next != 0)
				{
					tp = a;
					a = a->next;
				}
				else
				{
					a->next = b;
					return tmp_root.next;
				}

			}
			else
			{
				tp->next = b;
				if(b->next != 0)
				{
					tp = b;
					b = b->next;
				}
				else
				{
					b->next = a;
					return tmp_root.next;
				}
			}
		}
	}
	/**
	 * @brief 
	 * 检查链表结构完整性，如前后指针是否一致，是否存在环, 用于DEBUG
	 * @return  void 
	 * @retval   
	 * @see 
	 * @author yufan
	 * @date 2008/08/11 16:09:32
	**/
	void check()
	{
		if(_root.next == _root.prev) {
			return;
        }
		node_pointer tmp = _root.next;
		while(tmp != _root)
		{
			if(tmp->prev == 0)
			{
				*(int*)0=1;
			}
			if(tmp->next->prev != tmp)
			{
				*(int *)0=1;
			}
			tmp->prev = 0;
			tmp = tmp->next;
		}
		tmp = _root.next;
		tmp->prev = _root;
		while(tmp != _root)
		{
			tmp->next->prev = tmp;
			tmp = tmp->next;
		}
	}
};

}


#endif
