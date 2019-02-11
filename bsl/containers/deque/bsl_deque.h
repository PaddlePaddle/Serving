/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_deque.h,v 1.9 2008/12/15 09:56:59 xiaowei Exp $ 
 * 
 **************************************************************************/
 

 
/**
 * @file deque.h
 * @author baonh(baonenghui@baidu.com)
 * @date 2008/07/27 14:22:17  
 * @version $Revision: 1.9 $ 2010/10/15 modified by zhjw(zhujianwei@baidu.com)
 * @brief 
 *  
 **/


#ifndef  __BSL_DEQUE_H_
#define  __BSL_DEQUE_H_

#include <bsl/alloc/bsl_alloc.h>
#include <bsl/utils/bsl_utils.h>
#include <bsl/exception/bsl_exception.h>

namespace bsl {

	/**
	* @brief deque的迭代器
	*   
	*/
	template<typename _Tp, typename _Ref, typename _Ptr, size_t _BuffSize = 1024UL>
	struct _deque_iterator {
		typedef _deque_iterator<_Tp, _Tp&, _Tp*, _BuffSize>				iterator;
		typedef _deque_iterator<_Tp, const _Tp&, const _Tp*, _BuffSize> const_iterator;

		typedef std::random_access_iterator_tag iterator_category;
		typedef _Tp                        value_type;
		typedef _Ptr                       pointer;
		typedef _Ref                       reference;
		typedef size_t                     size_type;
		typedef ptrdiff_t                  difference_type;
		typedef _Tp**                      _map_pointer;
		typedef _deque_iterator		   _self; 

		value_type* _cur;
		value_type* _first;
		value_type* _last;
		_map_pointer _node;

		static const size_type _BUFF_SIZE = sizeof(value_type)<_BuffSize?
			size_type(_BuffSize/sizeof(value_type)):size_type(1);
		
		_deque_iterator(_Tp* __x, _map_pointer __y)
			: _cur(__x), _first(NULL == __y ? NULL : *__y),
			  _last(NULL == __y ? NULL : *__y + _BUFF_SIZE),
			  _node(__y) {}

		/**
		 * @brief 生成空迭带器
		**/
		_deque_iterator() : _cur(NULL), _first(NULL), _last(NULL), _node(NULL) {}

		/**
		 * @brief 由另一迭带器复制
		 *
		 * @param [in] __x   : const iterator&
		**/
		_deque_iterator(const iterator& __x)
			: _cur(__x._cur),
			  _first(__x._first),
			  _last(__x._last),
			  _node(__x._node) {}

		reference operator*() const 
		{
			if(NULL == _cur){
				throw bsl::NullPointerException()<<BSL_EARG<<"can not evaluate a null iterator.";
			}
			return *_cur; 
		}

		pointer operator->() const 
		{
			return _cur;
		}

		_self& operator++() 
		{
			++_cur;
			if (_cur == _last) {
				++_node;
				_first = *_node;
				_last = _first + difference_type(_BUFF_SIZE);
				_cur = _first;
			}
			return *this;
		}

		_self operator++(int) 
		{
			_self tmp = *this;
			++*this;
			return tmp;
		}

		_self operator--() 
		{
			if (_cur == _first) {
				--_node;
				_first = *_node;
				_last = _first + difference_type(_BUFF_SIZE);
				_cur = _last;
			}
			--_cur;
			return *this;
		}

		_self operator--(int) 
		{
			_self tmp = *this;
			--*this;
			return tmp;
		} 

		_self& operator+=(difference_type __n) 
		{
			const difference_type offset = __n + (_cur - _first);
			if (offset >= 0 && offset < difference_type(_BUFF_SIZE)) {
				_cur += __n;
			} else {
				const difference_type node_offset =
					offset > 0 ? offset / difference_type(_BUFF_SIZE)
					: -difference_type((-offset - 1)
							/ _BUFF_SIZE) - 1;
				_node += node_offset;
				_first = *_node;
				_last = _first + difference_type(_BUFF_SIZE);
				_cur = _first + (offset - node_offset
						* difference_type(_BUFF_SIZE));
			}
    		return *this;
      	}

      	_self operator+(difference_type __n) const 
		{
    		_self tmp = *this;
    		return tmp += __n;
      	}

      	_self& operator-=(difference_type __n) 
		{ 
			return *this += -__n; 
		}

      	_self operator-(difference_type __n) const 
		{
    		_self tmp = *this;
    		return tmp -= __n;
      	}

		reference operator[](difference_type __n) const 
		{
			return *(*this + __n); 
		}
	};

	template<typename _Tp, typename _Ref, typename _Ptr, size_t _BuffSize>
		inline bool operator==(const _deque_iterator<_Tp, _Ref, _Ptr, _BuffSize>& __x,
				const _deque_iterator<_Tp, _Ref, _Ptr, _BuffSize>& __y)
		{ 
			return __x._cur == __y._cur; 
		}

	template<typename _Tp, typename _RefL, typename _PtrL,
		typename _RefR, typename _PtrR, size_t _BuffSize>
			inline bool operator==(const _deque_iterator<_Tp, _RefL, _PtrL, _BuffSize>& __x,
					const _deque_iterator<_Tp, _RefR, _PtrR, _BuffSize>& __y)
			{ 
				return __x._cur == __y._cur; 
			}

	template<typename _Tp, typename _Ref, typename _Ptr, size_t _BuffSize>
		inline bool operator!=(const _deque_iterator<_Tp, _Ref, _Ptr, _BuffSize>& __x,
				const _deque_iterator<_Tp, _Ref, _Ptr, _BuffSize>& __y)
		{ 
			return !(__x == __y); 
		}

	template<typename _Tp, typename _RefL, typename _PtrL,
		typename _RefR, typename _PtrR, size_t _BuffSize>
			inline bool operator!=(const _deque_iterator<_Tp, _RefL, _PtrL, _BuffSize>& __x,
					const _deque_iterator<_Tp, _RefR, _PtrR, _BuffSize>& __y)
		{ 
			return !(__x == __y);
		}

	template<typename _Tp, typename _Ref, typename _Ptr, size_t _BuffSize>
		inline bool operator<(const _deque_iterator<_Tp, _Ref, _Ptr, _BuffSize>& __x,
				const _deque_iterator<_Tp, _Ref, _Ptr, _BuffSize>& __y)
		{ 
			return (__x._node == __y._node) ? (__x._cur < __y._cur)
			: (__x._node < __y._node); 
		}

	template<typename _Tp, typename _RefL, typename _PtrL,
		typename _RefR, typename _PtrR, size_t _BuffSize>
		inline bool operator<(const _deque_iterator<_Tp, _RefL, _PtrL, _BuffSize>& __x,
					const _deque_iterator<_Tp, _RefR, _PtrR, _BuffSize>& __y)
		{ 
			return (__x._node == __y._node) ? (__x._cur < __y._cur)
			: (__x._node < __y._node); 
		}

	template<typename _Tp, typename _Ref, typename _Ptr, size_t _BuffSize>
		inline bool operator>(const _deque_iterator<_Tp, _Ref, _Ptr, _BuffSize>& __x,
				const _deque_iterator<_Tp, _Ref, _Ptr, _BuffSize>& __y)
		{ 
			return __y < __x; 
		}

	template<typename _Tp, typename _RefL, typename _PtrL,
		typename _RefR, typename _PtrR, size_t _BuffSize>
		inline bool operator>(const _deque_iterator<_Tp, _RefL, _PtrL, _BuffSize>& __x,
					const _deque_iterator<_Tp, _RefR, _PtrR, _BuffSize>& __y)
		{ 
			return __y < __x; 
		}

	template<typename _Tp, typename _Ref, typename _Ptr, size_t _BuffSize>
		inline bool
		operator<=(const _deque_iterator<_Tp, _Ref, _Ptr, _BuffSize>& __x,
				const _deque_iterator<_Tp, _Ref, _Ptr, _BuffSize>& __y)
		{ 
			return !(__y < __x); 
		}

	template<typename _Tp, typename _RefL, typename _PtrL,
		typename _RefR, typename _PtrR, size_t _BuffSize>
		inline bool operator<=(const _deque_iterator<_Tp, _RefL, _PtrL, _BuffSize>& __x,
					const _deque_iterator<_Tp, _RefR, _PtrR, _BuffSize>& __y)
		{ 
			return !(__y < __x); 
		}

	template<typename _Tp, typename _Ref, typename _Ptr, size_t _BuffSize>
		inline bool operator>=(const _deque_iterator<_Tp, _Ref, _Ptr, _BuffSize>& __x,
				const _deque_iterator<_Tp, _Ref, _Ptr, _BuffSize>& __y)
		{ 
			return !(__x < __y); 
		}

	template<typename _Tp, typename _RefL, typename _PtrL,
		typename _RefR, typename _PtrR, size_t _BuffSize>
		inline bool operator>=(const _deque_iterator<_Tp, _RefL, _PtrL, _BuffSize>& __x,
					const _deque_iterator<_Tp, _RefR, _PtrR, _BuffSize>& __y)
		{ 
			return !(__x < __y); 
			
		}

	template<typename _Tp, typename _RefL, typename _PtrL,
		typename _RefR, typename _PtrR, size_t _BuffSize>
		inline typename _deque_iterator<_Tp, _RefL, _PtrL, _BuffSize>::difference_type
		operator-(const _deque_iterator<_Tp, _RefL, _PtrL, _BuffSize>& __x,
					const _deque_iterator<_Tp, _RefR, _PtrR, _BuffSize>& __y)
		{
			return typename _deque_iterator<_Tp, _RefL, _PtrL, _BuffSize>::difference_type
				(_deque_iterator<_Tp, _RefL, _PtrL, _BuffSize>::_BUFF_SIZE)
				* (__x._node - __y._node - 1) + (__x._cur - __x._first)
				+ (__y._last - __y._cur);
		}

	template<typename _Tp, typename _Ref, typename _Ptr, size_t _BuffSize>
		inline _deque_iterator<_Tp, _Ref, _Ptr, _BuffSize>
		operator+(ptrdiff_t __n, const _deque_iterator<_Tp, _Ref, _Ptr, _BuffSize>& __x)
		{ 
			return __x + __n; 
		}



	/**
	* @brief deque
	*   
	*   双头队列， 通过一个索引表对数据块进行操作
	*   可以通过_BuffSize指定每个块的字节大小，每个块的元素大小通过 _BuffSize除元素的大小取整得到。
	*  
	*/
	template<typename _Tp, size_t _BuffSize = 1024UL, class _InnerAlloc = bsl_alloc<_Tp> >
	class deque
	{
		public:
			typedef typename _InnerAlloc::pool_type::template rebind<_Tp>::other _base;

			typedef typename _base::value_type        value_type;
			typedef typename _base::pointer           pointer;
			typedef typename _base::const_pointer     const_pointer;
			typedef typename _base::reference         reference;
		    typedef typename _base::const_reference   const_reference;
			typedef typename _base::size_type         size_type;
			typedef typename _base::difference_type   difference_type;

			typedef _deque_iterator<value_type,
					reference,
					pointer,
					_BuffSize> iterator; 

			typedef _deque_iterator<value_type,
					const_reference,
					const_pointer,
					_BuffSize> const_iterator;

			typedef std::reverse_iterator<const_iterator>   const_reverse_iterator;
			typedef std::reverse_iterator<iterator>         reverse_iterator;

		private:
			typedef typename _InnerAlloc::pool_type::template rebind<value_type*>::other 
																			_base_pointer;
			_base _node_alloc;				  /**< 内存块分配器       */
			_base_pointer _map_alloc;		  /**< 索引内存 分配器       */
			/**< 每个块的元素个数  */
			static const size_type _BUFF_SIZE = sizeof(_Tp) < _BuffSize?
				size_type(_BuffSize / sizeof(_Tp)) : size_type(1);		 
			/**< 索引表的大小初始为 _INITIAL_MAP_SIZE       */
			enum { _INITIAL_MAP_SIZE = 8UL };		  
			/**
			* @brief  指向头块和尾块的结构体
			*  
			*/
			struct impl_t {
				pointer cur;
				pointer first;		  /**< 块头部       */
				pointer last;		  /**< 块尾部       */
				pointer* map;		  /**< 指向块的指针 */
			};

			impl_t _start;			/**< 块头       */
			impl_t _finish;			/**< 块尾       */

			pointer* _map_first;		/**< 索引表头       */
			pointer* _map_last;			/**< 索引表尾      */
			size_type _map_size;		/**< 索引表大小      */

		public:

			/* *
			 * @brief 默认构造函数
			 * 无异常
			 * 不需调create
			 * */
			deque(){
				_reset();
			}

			/**
			 * @brief 构造函数
			 * @param [in] __size	: size_type
			 *
			 * 失败抛异常
			 * 不需调create
			 *
			**/
			explicit deque(size_type __size) {
				_reset();
				if (create(__size) != 0) {
					throw BadAllocException()<<BSL_EARG
						<<"create error when create deque with size "<<__size<<".";
				}
			}

			/**
			 * @brief 拷贝构造函数
			 * 失败抛异常，恢复成原来的空deque
			 * @param  [in] other	: const deque&
			 **/
			deque(const deque& other) {
				_reset();
				if (assign(other.begin(),other.end()) != 0) {
					throw BadAllocException()<<BSL_EARG
						<<"assign deque error in copy constructor.";
				}
			}
			
			/**
			 * @brief 赋值运算符，抛异常
			 *		  如果插入失败，将恢复成原deque，强异常安全保证
			 *		  如果删除失败，从末尾删除结点，不会造成资源泄漏，基本异常安全保证
			 *
			 * @param  [in] other	: const deque&
			 **/
			deque& operator=(const deque& other) {
				if (this != &other) {
					iterator first1 = begin();
					iterator last1 = end();
					const_iterator first2 = other.begin();
					const_iterator last2 = other.end();
					size_t size1 = size();
					size_t size2 = other.size();
					if (size1 < size2) {
						for (size_t i = size1; i < size2; i ++) {
							if ( push_back( other[i] ) != 0 ) {
								for (size_t j = size1; j < i; j ++) {
									pop_back();
								}
								throw BadAllocException()<<BSL_EARG<<"push_back deque error index["
									<<i<<"] size["<<size1<<"~"<<size2<<"]";
							}
						}
					} else { // size1 >= size2
						for (size_t i = size1; i > size2; -- i) {
							if ( pop_back() != 0 ) {
								throw BadAllocException()<<BSL_EARG<<"pop_back deque error index["
									<<i<<"] size["<<size1<<"~"<<size2<<"]";
							}
						}
					}
					while (first1 != last1 && first2 != last2) {
						*first1 ++ = *first2 ++;
					}
				}
				return *this;
			}
			
			/**
			 * @brief 析构函数，释放内存
			 *
			**/
			~deque()
			{
				destroy();
			}
			
			/**
			 * @brief  初始化一个deque,分配空间
			 *
			 * @param [in] __size   : 指定初始化的大小
			 * @return  成功返回0， 失败返回-1, 如果失败不能调用其它接口
			**/
			int create(size_type __size = size_type(_INITIAL_MAP_SIZE))
			{
				//先destroy的目的是避免2次连续的create出现问题
				destroy();
				size_type init_size = std::max(__size, size_type(1));
				//初始化内存池
				if (_node_alloc.create() != 0 || _map_alloc.create() != 0) {
					goto fail;
				}
				//计算需要初始化的索引大小
				init_size = (init_size - 1) / _BUFF_SIZE + 1;
				//初始化索引
				if 	(_initialize_map(std::max(init_size, size_type(_INITIAL_MAP_SIZE))) != 0) {
					goto fail;
				}
				return 0;	
			fail:
				_node_alloc.destroy();
				_map_alloc.destroy();
				return -1;	
			}

			/**
			 * @brief 将n个val的副本赋给deque
			 *
			 * @param [in] __n   : 元素个数
			 * @param [in] __val   : 需要复制的值
			 * @return  成功返回0, 失败返回-1, 如果失败会把deque清空 
			**/
			int assign(size_type __n, const value_type& __val)
			{
				clear();
				for (size_type i = 0; i < __n; ++i) {
					if (push_back(__val) != 0) {
						goto fail;
					}
				}
				return 0;
			fail:
				clear();
				return -1;
			}

			/**
			 * @brief 将区间 [__first, __last) 间的元素赋值给 deque
			 *
			 * @param [in] __first   : 迭带器头
			 * @param [in] __last   : 迭带器尾
			 * @return  成功返回0， 失败返回-1, 如果失败会把deque清空
			**/
			template<typename _InputIterator>
			int assign(_InputIterator __first, _InputIterator __last)
			{
				typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
				return _assign(__first, __last, _Integral());
			}

			template<typename _Integer>
			int _assign(_Integer __n, _Integer __val, __true_type)
			{
				return assign(size_type(__n), value_type(__val));
			}

			template<typename _InputIterator>
			int _assign(_InputIterator __first, _InputIterator __last, __false_type)
			{
				clear();
				for (_InputIterator iter = __first; iter != __last; ++iter) {
					if (push_back(*iter) != 0) {
						goto fail;
					}
				}
				return 0;
			fail:
				clear();
				return -1;
			}

			/**
			 * @brief 完全销毁deque, 需要再次create才能使用
			 *
			 * @return  成功返回0 
			**/
			int destroy()
			{
				//清空deque
				clear();
				//销毁最后的内存块
				if (NULL != _start.map) {
					_node_alloc.deallocate(_start.map[0], _BUFF_SIZE);
					_start.map = NULL;
				}
				//销毁索引
				if (_map_first != NULL) {
					_map_alloc.deallocate(_map_first, _map_size);
					_map_first = NULL;
				}
				//数据重置
				_reset();
				//销毁内存池
				_node_alloc.destroy();
				_map_alloc.destroy();
				return 0;
			}

			/**
			 * @brief 清空deque, 但不销毁内存，可以再次使用
			 *
			 * @return  成功返回0
			**/
			int clear()
			{
				while (pop_back() == 0) {
				}
				return 0;
			}

			iterator begin()
			{
				return iterator(_start.cur, _start.map);
			}

			iterator end()
			{
				return iterator(_finish.cur, _finish.map);
			}
			
			const_iterator begin() const
			{
				return const_iterator(_start.cur, _start.map);
			}

			const_iterator end() const
			{
				return const_iterator(_finish.cur, _finish.map);
			}

			reverse_iterator rbegin()
			{ 
				return reverse_iterator(end()); 
			}

			const_reverse_iterator rbegin() const
			{ 
				return const_reverse_iterator(end()); 
			}

			reverse_iterator rend() 
			{ 
				return reverse_iterator(begin()); 
			}

			const_reverse_iterator rend() const
			{ 
				return const_reverse_iterator(begin()); 
			}

			size_type max_size() const
			{ 
				return size_type(-1); 
			}

	
			/**
			 * @brief 从尾部插入元素
			 *
			 * @param [in] __x  : 插入的元素
			 * @return  成功返回0，失败返回-1
			**/
			int push_back(const value_type& __x)
			{
				//如果未创建，则创建之
				if(0 == _map_size){
					if(create() != 0){
						return -1;
					}
				}
				if (_finish.cur) {
					pointer cur = _finish.cur++;
					if (_finish.cur == _finish.last) {
						if (_finish.map == _map_last) {
							if (_new_map() != 0) {
								//失败恢复现场
								_finish.cur = cur;
								return -1;
							}
						}
						//分配新的内存块
						pointer new_node = _node_alloc.allocate(_BUFF_SIZE);
						if (NULL == new_node) {
							//失败恢复现场
							_finish.cur = cur;
							return -1;
						}
						(*(++_finish.map)) = new_node;
						_finish.first = new_node; 
						_finish.last = new_node + _BUFF_SIZE;
						_finish.cur = new_node;
					}
					bsl::bsl_construct(cur, __x);
					return 0;
				}
				return -1;
			}

			/**
			 * @brief 从头部插入元素
			 *
			 * @param [in] __x  : 插入的元素
			 * @return  成功返回0，失败返回-1
			**/
			int push_front(const value_type& __x)
			{
				//如果未创建，则创建之
				if(0 == _map_size){
					if(create() != 0){
						return -1;
					}
				}
				if (_start.cur) {
					if (_start.cur != _start.first) {
						--_start.cur;
						bsl::bsl_construct(_start.cur, __x);
						return 0;
					}
					if (_start.map == _map_first) {
						if (_new_map() != 0) {
							return -1;
						}
					}
					pointer new_node = _node_alloc.allocate(_BUFF_SIZE);
					if (NULL == new_node) {
						return -1;
					}
					*(--_start.map) = new_node;
					_start.first = new_node;
					_start.last =  new_node + _BUFF_SIZE;
					_start.cur = _start.last - 1;
					bsl::bsl_construct(_start.cur, __x);
					return 0;
				}
				return -1;
			}
			
			/**
			 * @brief 删除尾部元素
			 *
			 * @return  成功返回0， 失败返回-1
			**/
			int pop_back()
			{
				if (_start.cur != _finish.cur) {
					if (_finish.cur == _finish.first) {
						_node_alloc.deallocate(_finish.first, _BUFF_SIZE);
						_finish.first = *(--_finish.map);
						_finish.last = _finish.first + _BUFF_SIZE;
						_finish.cur = _finish.last;
					}
					--_finish.cur;
					bsl::bsl_destruct(_finish.cur);
					return 0;
				} 
				return -1;
			}

			/**
			 * @brief 删除头部元素
			 *
			 * @return  成功返回0， 失败返回-1
			**/
			int pop_front()
			{
				if (_start.cur != _finish.cur) {
					bsl::bsl_destruct(_start.cur);
					++_start.cur;
					if (_start.cur == _start.last) {
						_node_alloc.deallocate(_start.first, _BUFF_SIZE);
						_start.first = *(++_start.map);
						_start.last = _start.first + _BUFF_SIZE;
						_start.cur = _start.first;
					}
					return 0;
				} 
				return -1;
			}

			/**
			 * @brief 取头部元素的引用
			 *
			 * 如果为容器为空，则抛出异常。
			 * @return  头部元素的引用
			**/
			reference front()
			{
				//检查是否为空
				if(_start.cur == _finish.cur){
					throw bsl::InvalidOperationException()<<BSL_EARG
						<<"deque front item does not exist.";
				}
				return *_start.cur;
			}

			/**
			 * @brief 取尾部元素的引用
			 *
			 * 如果容器为空，则抛出异常
			 * @return  取尾部元素的引用
			**/
			reference back() 
			{
				//检查是否为空
				if(_start.cur == _finish.cur){
					throw bsl::InvalidOperationException()<<BSL_EARG
						<<"deque back item does not exist.";
				}
				if (_finish.cur != _finish.first) {
					return *(_finish.cur - 1);
				}else{
					return *(*(_finish.map - 1) + _BUFF_SIZE - 1);
				}
			}

			/**
			 * @brief 取头部元素的常引用
			 *
			 * 如果容器为空，则抛出异常
			 * @return  头部元素的常引用
			**/
			const_reference front() const
			{
				//检查是否为空
				if(_start.cur == _finish.cur){
					throw bsl::InvalidOperationException()<<BSL_EARG
						<<"deque front item does not exist.";
				}
				return *_start.cur;
			}

			/**
			 * @brief 取尾部元素的常引用
			 *
			 * 如果容器为空，则抛出异常
			 * @return  取尾部元素的常引用
			**/
			const_reference back() const 
			{
				//检查是否为空
				if(_start.cur == _finish.cur){
					throw bsl::InvalidOperationException()<<BSL_EARG
						<<"deque back item does not exist.";
				}

				if (_finish.cur != _finish.first) {
					return *(_finish.cur - 1);
				}else{
					return *(*(_finish.map - 1) + _BUFF_SIZE - 1);
				}
			}

			/**
			 * @brief 获取第n个的元素, 但不作长度检查
			 *
			**/
			reference operator[](size_type __n) const 
			{
				__n += _start.cur - _start.first;
				return _start.map[__n/_BUFF_SIZE][__n%_BUFF_SIZE];
			}

			/**
			 * @brief 获取第n个元素，做越界检查，但这里传的是拷贝
			 *
			 * @param [in] __n   : 位置
			 * @param [in] __val   : 存放结果的指针
			 * @return  失败返回-1, 成功返回0
			**/
			int get(const size_type &__n, value_type *__val)
			{
				if (__n >= size() || NULL == __val) {
					return -1;
				}
				bsl::bsl_construct(__val, (*this)[__n]);
				return 0;
			}

			/**
			 * @brief 设置第n个元素的值为 __val
			 *
			 * @param [in] __n   : 位置
			 * @param [in] __val   : 需要设置的值
			 * @return  失败返回-1, 成功返回0
			**/
			int set(const size_type &__n, const value_type &__val)
			{
				if (__n >= size()) {
					return -1;
				}
				bsl::bsl_construct(&((*this)[__n]), __val);
				return 0;
			}

			/**
			 * @brief deque的长度
			 *
			 * @return  deque的长度 
			**/
			size_type size() const 
			{
				if (_start.map == _finish.map) {
					return _finish.cur - _start.cur;
				} 
				return (_start.last - _start.cur) +
					(_finish.cur - _finish.first) +
					(_finish.map - _start.map - 1) * _BUFF_SIZE;
			}

			/**
			 * @brief 重新分配空间大小，并初始化为__x
			 *
			 * @param [in] __new_size   : 新的大小
			 * @param [in] __x   : 初始化的值
			 * @return  0成功，-1失败
			**/
			int resize(size_type __new_size, const value_type& __x)
			{
				size_type len = size();
				if (__new_size < size()) {
					for (size_type i = __new_size; i < len; ++i) {
						if (pop_back() < 0) {
							goto fail;
						}
					}
				} else {
					for (size_type i = len; i < __new_size; ++i) {
						if (push_back(__x) < 0) {
							goto fail;
						}
					}
				}
				return 0;
			fail:
				return -1;
				
			}

			/**
			 * @brief 重新分配空间大小
			 *
			 * @param [in] __new_size   : 新的大小
			 * @return  0成功，-1失败
			**/
			int resize(size_type __new_size)
			{
				return resize(__new_size, value_type());
			}

			/**
			 * @brief 判断是否为空
			 *
			 * @return  true为空
			**/
			bool empty() const
			{
				return (_start.cur == _finish.cur);
			}


			/**
			 * @brief 与另一个deque进行交换
			 *
			 * @param [in] __x   : 进行交换的deque
			**/
			void swap(deque& __x)
			{
				std::swap(this->_start, __x._start);
				std::swap(this->_finish, __x._finish);
				std::swap(this->_map_first, __x._map_first);
				std::swap(this->_map_last, __x._map_last);
				std::swap(this->_map_size, __x._map_size);
				std::swap(this->_map_alloc, __x._map_alloc);
				std::swap(this->_node_alloc, __x._node_alloc);
			}


			/**
			 * @brief 序列化接口
			 *
			 * @param [in] ar : 序列化的结果
			 *
			 * @return  成功返回0， 失败返回-1
			**/
			template <typename _Archive>
			int serialization(_Archive &__ar) 
			{    
				if (bsl::serialization(__ar, size()) != 0) {
					return -1;
				}
				for (iterator iter = begin(); iter != end(); ++iter) {    
					if (bsl::serialization(__ar, *iter) != 0) { 
						return -1;
					}
				}    
				return 0;
			}

			/**
			 * @brief 反序列化接口
			 *
			 * @param [in] ar : 需要反序列化的容器
			 *
			 * @return  成功返回0， 失败返回-1, 失败的同时会清空deque
			**/
			template <typename _Archive>
			int deserialization(_Archive &ar)
			{
				size_type len = size();
				if (bsl::deserialization(ar, len) != 0) {
					return -1;
				}
				clear();
				value_type tmp;
				while (len--) {
					if (bsl::deserialization(ar, tmp) != 0) {
						goto fail;
					}
					if (push_back(tmp) != 0) {
						goto fail;
					}
				}

				return 0;
			fail:
				clear();
				return -1;
			}

		private:	
			/**
			 * @brief 重置deque
			 *
			**/
			void _reset()
			{
				_map_first = NULL;
				_map_last = NULL;
				_start.cur = NULL;
				_start.first = NULL;
				_start.last = NULL;
				_start.map = NULL;
				_finish = _start;
				_map_size = 0;
			}

			/**
			 * @brief 初始化索引表
			 *
			 * @param [in] __initialize_size   : 初始化索引表的大小
			 * @return 成功返回0， 失败返回-1
			**/
			int _initialize_map(size_type __initialize_size)
			{
				//分配内存给索引
				_map_first = _map_alloc.allocate(__initialize_size);
				if (NULL == _map_first) {
					goto fail;
				}
				_map_size = __initialize_size;
				_map_last = _map_first + _map_size - 1;
				_start.map = _map_first + (_map_size>>1);
				//分配第一个内存块
				_start.first = _node_alloc.allocate(_BUFF_SIZE);
				if (NULL == _start.first) {
					goto fail;
				}
				//头尾内存块的初始化
				(*_start.map) = _start.first;
				_start.last = _start.first + _BUFF_SIZE;
				_start.cur = _start.first;
				_finish = _start;
				return 0;
			fail:
				if (_map_first != NULL) {
					_map_alloc.deallocate(_map_first, __initialize_size);
					_map_first = NULL;
				}
				_reset();
				return -1;	
			}
	
			/**
			 * @brief 索引表扩展
			 *
			 *  
			 * @return  成功返回0， 失败返回-1
			**/
			int _new_map()
			{
				//这里分两种情况，如果实际使用的空间没有占到一半，只进行移动
				//否则分配新的内存块。
				//这样处理可以节省一定的内存。
				const difference_type offset = _finish.map - _start.map + 1;
				pointer* new_start = NULL;
				if ((difference_type)_map_size > 2 * offset) {
					new_start = _map_first + ((_map_size - offset) >> 1);
					//判断是向前还是向后移
					//这里不用memmov的是因为我们测试的结果for循环的性能要跟好一些
					
					if (new_start < _start.map) {
						for (pointer* p = new_start; p < new_start + offset; ++p) {
							*p = *_start.map;
							++_start.map;
						}
					} else {
						for (pointer* p = new_start + offset - 1; p >= new_start; --p) {
							*p = *_finish.map;
							--_finish.map;
						}
					}
				} else {
					//分配新的内存快
					pointer* new_map = _map_alloc.allocate(_map_size << 1);
					if (NULL == new_map) {
						return -1;
					}
					//拷贝旧的索引
					new_start = new_map + (_map_size - (offset >> 1));
					memcpy(new_start, _start.map, offset * sizeof(pointer));
					_map_alloc.deallocate(_map_first, _map_size);
					_map_size <<= 1;
					//指针的切换
					_map_first = new_map;
					_map_last = new_map + _map_size - 1;
				}
				_start.map = new_start;
				_finish.map = new_start + offset - 1;
				return 0;
			}
						
	};

	/**
	 *  交换两个相同类型的deque
	 *
	 *  @param __x : 需要交换的deque 
	 *  @param __y : 需要交换的deque
	**/
	template<typename _Tp, size_t _BuffSize, typename _InnerAlloc>
	inline void swap(deque<_Tp, _BuffSize, _InnerAlloc>& __x,
			deque<_Tp, _BuffSize, _InnerAlloc>& __y)
	{ 
		__x.swap(__y); 
	}


} //namespace bsl

#endif  //__BSL_DEQUE_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
