/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_rwseque.h,v 1.11 2009/04/07 06:35:53 xiaowei Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file rwseque.h
 * @author baonh(baonenghui@baidu.com)
 * @date 2008/07/27 14:22:17
 * @version $Revision: 1.11 $ 2010/10/15 modified by zhjw(zhujianwei@baidu.com)
 * @brief 
 *  
 **/


#ifndef  __BSL_RWSEQUE_H_
#define  __BSL_RWSEQUE_H_

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif 

#include <pthread.h>

#include <bsl/alloc/bsl_alloc.h>
#include <bsl/utils/bsl_utils.h>
#include <bsl/exception/bsl_exception.h>

namespace bsl {
	/**
	* @brief bsl_deque�ĵ�����
	*  
	*  ʹ�õ��������߳ʰ�ȫ���⣬����֮���Ա�����������Ҫ��Ϊ���ܹ���stl���㷨��һ������
	*/
	
	template<typename _Tp, typename _Ref, typename _Ptr, size_t _BuffSize = 1024UL>
	struct _rwseque_iterator {
		typedef _rwseque_iterator<_Tp, _Tp&, _Tp*, _BuffSize>				iterator;
		typedef _rwseque_iterator<_Tp, const _Tp&, const _Tp*, _BuffSize> const_iterator;

		typedef std::random_access_iterator_tag iterator_category;
		typedef _Tp                        value_type;
		typedef _Ptr                       pointer;
		typedef _Ref                       reference;
		typedef size_t                     size_type;
		typedef ptrdiff_t                  difference_type;
		typedef _Tp**                      _map_pointer;
		typedef _rwseque_iterator		   _self; 

		value_type* _cur;
		value_type* _first;
		value_type* _last;
		_map_pointer _node;

		static const size_type _BUFF_SIZE = sizeof(value_type)<_BuffSize?
			size_type(_BuffSize/sizeof(value_type)):size_type(1UL);
		

		//��_map_pointerΪ��ʱ��_first��_last��Ϊ��
		_rwseque_iterator(_Tp* __x, _map_pointer __y)
			: _cur(__x), _first(NULL == __y ? NULL : *__y),
			  _last(NULL == __y ? NULL : *__y + _BUFF_SIZE),
			  _node(__y) {}

		/**
		 * @brief ���ɿյ�����
		**/
		_rwseque_iterator() : _cur(NULL), _first(NULL), _last(NULL), _node(NULL) {}

		/**
		 * @brief ����һ����������
		 *
		 * @param [in] __x   : const iterator&
		**/
		_rwseque_iterator(const iterator& __x)
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
		inline bool operator==(const _rwseque_iterator<_Tp, _Ref, _Ptr, _BuffSize>& __x,
				const _rwseque_iterator<_Tp, _Ref, _Ptr, _BuffSize>& __y)
		{ 
			return __x._cur == __y._cur; 
		}

	template<typename _Tp, typename _RefL, typename _PtrL,
		typename _RefR, typename _PtrR, size_t _BuffSize>
			inline bool operator==(const _rwseque_iterator<_Tp, _RefL, _PtrL, _BuffSize>& __x,
					const _rwseque_iterator<_Tp, _RefR, _PtrR, _BuffSize>& __y)
			{ 
				return __x._cur == __y._cur; 
			}

	template<typename _Tp, typename _Ref, typename _Ptr, size_t _BuffSize>
		inline bool operator!=(const _rwseque_iterator<_Tp, _Ref, _Ptr, _BuffSize>& __x,
				const _rwseque_iterator<_Tp, _Ref, _Ptr, _BuffSize>& __y)
		{ 
			return !(__x == __y); 
		}

	template<typename _Tp, typename _RefL, typename _PtrL,
		typename _RefR, typename _PtrR, size_t _BuffSize>
			inline bool operator!=(const _rwseque_iterator<_Tp, _RefL, _PtrL, _BuffSize>& __x,
					const _rwseque_iterator<_Tp, _RefR, _PtrR, _BuffSize>& __y)
			{ 
				return !(__x == __y);
			}

	template<typename _Tp, typename _Ref, typename _Ptr, size_t _BuffSize>
		inline bool operator<(const _rwseque_iterator<_Tp, _Ref, _Ptr, _BuffSize>& __x,
				const _rwseque_iterator<_Tp, _Ref, _Ptr, _BuffSize>& __y)
		{ 
			return (__x._node == __y._node) ? (__x._cur < __y._cur)
			: (__x._node < __y._node); 
		}

	template<typename _Tp, typename _RefL, typename _PtrL,
		typename _RefR, typename _PtrR, size_t _BuffSize>
			inline bool operator<(const _rwseque_iterator<_Tp, _RefL, _PtrL, _BuffSize>& __x,
					const _rwseque_iterator<_Tp, _RefR, _PtrR, _BuffSize>& __y)
			{ 
				return (__x._node == __y._node) ? (__x._cur < __y._cur)
				: (__x._node < __y._node); 
	
			}

	template<typename _Tp, typename _Ref, typename _Ptr, size_t _BuffSize>
		inline bool
		operator>(const _rwseque_iterator<_Tp, _Ref, _Ptr, _BuffSize>& __x,
				const _rwseque_iterator<_Tp, _Ref, _Ptr, _BuffSize>& __y)
		{ return __y < __x; }

	template<typename _Tp, typename _RefL, typename _PtrL,
		typename _RefR, typename _PtrR, size_t _BuffSize>
			inline bool
			operator>(const _rwseque_iterator<_Tp, _RefL, _PtrL, _BuffSize>& __x,
					const _rwseque_iterator<_Tp, _RefR, _PtrR, _BuffSize>& __y)
			{ return __y < __x; }

	template<typename _Tp, typename _Ref, typename _Ptr, size_t _BuffSize>
		inline bool
		operator<=(const _rwseque_iterator<_Tp, _Ref, _Ptr, _BuffSize>& __x,
				const _rwseque_iterator<_Tp, _Ref, _Ptr, _BuffSize>& __y)
		{ return !(__y < __x); }

	template<typename _Tp, typename _RefL, typename _PtrL,
		typename _RefR, typename _PtrR, size_t _BuffSize>
			inline bool
			operator<=(const _rwseque_iterator<_Tp, _RefL, _PtrL, _BuffSize>& __x,
					const _rwseque_iterator<_Tp, _RefR, _PtrR, _BuffSize>& __y)
			{ return !(__y < __x); }

	template<typename _Tp, typename _Ref, typename _Ptr, size_t _BuffSize>
		inline bool
		operator>=(const _rwseque_iterator<_Tp, _Ref, _Ptr, _BuffSize>& __x,
				const _rwseque_iterator<_Tp, _Ref, _Ptr, _BuffSize>& __y)
		{ return !(__x < __y); }

	template<typename _Tp, typename _RefL, typename _PtrL,
		typename _RefR, typename _PtrR, size_t _BuffSize>
			inline bool
			operator>=(const _rwseque_iterator<_Tp, _RefL, _PtrL, _BuffSize>& __x,
					const _rwseque_iterator<_Tp, _RefR, _PtrR, _BuffSize>& __y)
			{ return !(__x < __y); }

	template<typename _Tp, typename _RefL, typename _PtrL,
		typename _RefR, typename _PtrR, size_t _BuffSize>
			inline typename _rwseque_iterator<_Tp, _RefL, _PtrL, _BuffSize>::difference_type
			operator-(const _rwseque_iterator<_Tp, _RefL, _PtrL, _BuffSize>& __x,
					const _rwseque_iterator<_Tp, _RefR, _PtrR, _BuffSize>& __y)
			{
				return typename _rwseque_iterator<_Tp, _RefL, _PtrL, _BuffSize>::difference_type
					(_rwseque_iterator<_Tp, _RefL, _PtrL, _BuffSize>::_BUFF_SIZE)
					* (__x._node - __y._node - 1) + (__x._cur - __x._first)
					+ (__y._last - __y._cur);
			}

	template<typename _Tp, typename _Ref, typename _Ptr, size_t _BuffSize>
		inline _rwseque_iterator<_Tp, _Ref, _Ptr, _BuffSize>
		operator+(ptrdiff_t __n, const _rwseque_iterator<_Tp, _Ref, _Ptr, _BuffSize>& __x)
		{ return __x + __n; }


	template<typename _Tp, size_t _BuffSize = 1024UL, class _InnerAlloc = bsl_alloc<_Tp> >
	class rwseque
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


			typedef _rwseque_iterator<value_type,
					reference,
					pointer,
					_BuffSize> iterator; 

			typedef _rwseque_iterator<value_type,
					const_reference,
					const_pointer,
					_BuffSize> const_iterator;

			typedef std::reverse_iterator<const_iterator>   const_reverse_iterator;
			typedef std::reverse_iterator<iterator>         reverse_iterator;

		private:

			typedef typename _InnerAlloc::pool_type::template rebind<value_type*>::other 
																					_base_pointer;
			typedef typename _InnerAlloc::pool_type::template rebind<pthread_rwlock_t>::other 
																					_base_lock; 
			typedef typename _InnerAlloc::pool_type::template rebind<pthread_rwlock_t*>::other 
																				_base_lock_list; 

			_base         _node_alloc;
			_base_pointer _map_alloc;
			_base_lock    _locks_alloc;
			_base_lock_list _locks_list_alloc;

			static const size_type _BUFF_SIZE = sizeof(_Tp) < _BuffSize?
				size_type(_BuffSize / sizeof(_Tp)) : size_type(1);

			enum { _INITIAL_MAP_SIZE = 8 };		/**< ��ʼ��map��С       */
			enum { _MAP_LIST_SIZE = 64 };		/**< map���ӵĵȼ���ÿ��size����       */
			enum { _LOCK_NUM = 8 };				/**< _LOCK_NUM ���������ڴ��ʹ��һ����д��       */

			pointer _last;			/**< ���һ��Ԫ�ص�ָ��λ��   */
			size_type _finish;		/**< ���һ��Ԫ�ص�ʵ��ƫ��       */
			size_type _size;		/**< Ԫ�صĳ���       */
			size_type _lsize;
			size_type _locks_map_size;		  /**< ���ĸ���      */

			pthread_rwlock_t** _locks;
			pointer* _map;
			size_type _map_size;		  /**< map�Ĵ�С       */

			pthread_rwlock_t** _locks_list[_MAP_LIST_SIZE];
			pointer* _map_list[_MAP_LIST_SIZE];
			size_type _cur_list;
			
			/**
			 * @brief ������
			 */
			pthread_mutex_t _up_locks;
			/**
			 * @brief ��ʼ�����»������ı��,true��ʾ���³ɹ�
			 */
			bool _lock_init;

		private:

			/**
			 * @brief �Ⱥ���Ϊ˽�У���֧�ֿ������죬��Ҫ����ʹ��assign
			 * @note ��֧��= ,ͬʱҲ��֧�� std::swap
			 */
			rwseque& operator=(const rwseque&);
			rwseque(const rwseque&);

		public:

			/* *
			 * @brief Ĭ�Ϲ��캯��
			 * ���쳣
			 * ��Ҫ��create
			 * */
			rwseque(){
				_reset();
				_lock_init = false;
			}

			/**
			 * @brief ���캯��
			 * @param  [in] __size	: size_type rwseque��С
			 * �������ʧ�ܣ������쳣
			 * �����create
			 */
			explicit rwseque(size_type __size) 
			{
				_reset();
				_lock_init = false;
				if (create(__size) != 0) {
					throw BadAllocException()<<BSL_EARG
						<<"create error when create rwseque with map size "<<__size<<".";
				}
			}		

			~rwseque()
			{
				destroy();
			}

			/**
			 * @brief ��ʼ��һ��rwseque,����ռ�
			 * @param [in] __size	: ָ����ʼ���Ĵ�С
			 * @return �ɹ�����0�� ʧ�ܷ���-1, ���ʧ�ܲ��ܵ��������ӿ�
			 */
			int create(size_type __size = size_type(_INITIAL_MAP_SIZE))
			{
				destroy();
				_lsize = _BUFF_SIZE * _LOCK_NUM;
				size_type init_size = std::max(__size, size_type(1));
				if (_node_alloc.create() != 0 || 
					_map_alloc.create() != 0 ||
					_locks_alloc.create() != 0 ||
					_locks_list_alloc.create() != 0) {
					goto fail;
				}
				init_size = (init_size - 1)/_BUFF_SIZE + 1;
				if 	(_initialize_map(std::max(init_size,
					size_type(_INITIAL_MAP_SIZE)), _LOCK_NUM) != 0) {
					goto fail;
				}
				return 0;	
			fail:
				_lsize = 0;
				_node_alloc.destroy();
				_map_alloc.destroy();
				_locks_alloc.destroy();
				_locks_list_alloc.destroy();
				return -1;	
			}

			/* *
			 * @brief �ж�rwseque�Ƿ���create
			 * @author zhujianwei
			 * @date 2010/12/13
			 * */
			bool is_created() const{
				return (_lock_init && _locks && _map);
			}

			int assign(size_type __n, const value_type& __val)
			{
				if (!_lock_init) {
					return -1;
				}
				mutexlock_monitor up_lock(&_up_locks);
				return _assign(size_type(__n), value_type(__val));
			}

			/**
			 * @brief ������ [__first, __last) ���Ԫ�ظ�ֵ�� d
			 *
			 * @param [in] __first   : ������ͷ
			 * @param [in] __last   : ������β
			 * @return  �ɹ�����0�� ʧ�ܷ���-1, ���ʧ�ܻ��deque���
			**/
			template<typename _InputIterator>
			int assign(_InputIterator __first, _InputIterator __last)
			{
				if (!_lock_init) {
					return -1;
				}
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
				mutexlock_monitor up_lock(&_up_locks);
				_clear();
				for (_InputIterator iter = __first; iter != __last; ++iter) {
					if (_push_back(*iter) != 0) {
						goto fail;
					}
				}
				return 0;
			fail:
				_clear();
				return -1;
			}


			int destroy()
			{
				_clear();
				//���������ڴ��
				if (NULL != _map) {
					if (NULL != _map[0]) {
						_node_alloc.deallocate(_map[0], _BUFF_SIZE);
					}
					//���ٶ�д��
					for (size_type i = 0; i < _locks_map_size; ++i) {
						if (NULL != _locks[i]) {
							pthread_rwlock_destroy(_locks[i]);
							_locks_alloc.deallocate(_locks[i], 1);
						}
					}
				}
				//����map_list��_lock_list	
				for (size_type list_size = _map_size;
						(list_size > 0) ;
						(list_size >>= 1), (_locks_map_size >>= 1), --_cur_list) {
					if (NULL != _map_list[_cur_list]) {
						//����map_list
						_map_alloc.deallocate(_map_list[_cur_list], list_size);
						//����lock_list
						_locks_list_alloc.deallocate(_locks_list[_cur_list], _locks_map_size);

					}
					if (0 == _cur_list) {
						break;
					}
				}
				
				_reset();
				//�����ڴ��
				_node_alloc.destroy();
				_map_alloc.destroy();
				_locks_alloc.destroy();
				_locks_list_alloc.destroy();
				//����ȫ�ֻ�����
				if (_lock_init) {
					pthread_mutex_destroy(&_up_locks);
					_lock_init = false;
				}
				return 0;
			}

			int clear()
			{
				if (!_lock_init) {
					return -1;
				}
				mutexlock_monitor up_lock(&_up_locks);
				return _clear();
			}

			size_type max_size() const
			{ 
				return size_type(-1); 
			}

			iterator begin()
			{
				return iterator((NULL == _map ? NULL : _map[0]), _map);
			}

			iterator end()
			{
				return iterator(_last, _map + _finish);
			}
			
			const_iterator begin() const
			{
				return const_iterator((NULL == _map ? NULL : _map[0]), _map);
			}

			const_iterator end() const
			{
				return const_iterator(_last, _map + _finish);
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

			int push_back(const value_type& __x)
			{
				if (!_lock_init) {
					return -1;
				}
				mutexlock_monitor up_lock(&_up_locks);
				return _push_back(__x);
			}

			int pop_back()
			{
				if (!_lock_init) {
					return -1;
				}
				mutexlock_monitor up_lock(&_up_locks);
				return _pop_back();
			}

			int get(const size_type &__n, value_type *__val)
			{
				if (!_lock_init) {
					return -1;
				}
				//����Ҫ��ֵ�������ö��߳�__n��ֵ��仯�� volatile���ⱻ�Ż���
				volatile size_type n = __n;
				if (n >= _size || NULL == __val) {
					return -1;
				}
				pthread_rwlock_t &plock = *(_locks[n/_lsize]);
				pthread_rwlock_rdlock(&plock);
				int ret = 0;
				if (n < _size) {
					bsl::bsl_construct(__val, _map[n/_BUFF_SIZE][n%_BUFF_SIZE]);
				} else {
					ret = -1;
				}

				pthread_rwlock_unlock(&plock);
				return ret;
			}

			int set(const size_type &__n, const value_type &__val)
			{
				if (!_lock_init) {
					return -1;
				}
				//����Ҫ��ֵ�������ö��߳�__n��ֵ��仯, volatile���ⱻ�Ż���
				volatile size_type n = __n;
				if (n >= _size) {
					return -1;
				}
				mutexlock_monitor up_lock(&_up_locks);
				int ret = 0;
				pthread_rwlock_t &plock = *(_locks[n/_lsize]);
				pthread_rwlock_wrlock(&plock);
				if (n < _size) {
					bsl::bsl_construct(&(_map[n/_BUFF_SIZE][n%_BUFF_SIZE]), __val);
				} else {
					ret = -1;
				}
				pthread_rwlock_unlock(&plock);
				return ret;
			}

			size_type size() const 
			{
				return _size;
			}

			bool empty() const
			{
				return (0 == _size);
			}

			template <typename _Archive>
			int serialization(_Archive &__ar) 
			{    
				if (!_lock_init) {
					return -1;
				}
				if (bsl::serialization(__ar, _size) < 0) {
					return -1;
				}
				mutexlock_monitor up_lock(&_up_locks);
				for (const_iterator iter = begin(); iter != end(); ++iter) {    
					if (bsl::serialization(__ar, *iter) < 0) { 
						return -1;
					}
				}
				return 0;
			}

			template <typename _Archive>
			int deserialization(_Archive &__ar)
			{
				if (!_lock_init) {
					return -1;
				}
				mutexlock_monitor up_lock(&_up_locks);
				size_type len = size();
				if (bsl::deserialization(__ar, len) < 0) {
					return -1;
				}
				_clear();
				value_type tmp;
				while (len--) {
					if (bsl::deserialization(__ar, tmp) < 0) {
						goto fail;
					}
					if (_push_back(tmp) < 0) {
						goto fail;
					}
				}
				return 0;
			fail:
				_clear();
				return -1;
			}

		private:

			/**
			 * @brief ����squeue���ڲ�����
			 *
			**/
			void _reset()
			{
				_finish = 0;
				_last = NULL;
				_map = NULL;
				_locks = NULL;
				_cur_list = 0;
				_map_size = 0;
				_size = 0;
				_lsize = 0;
				_locks_map_size = 0;
			}

			/* *
			 * @brief ��ʼ��������
			 * @return ����0��ʾ�ɹ���-1��ʾʧ��
			 * */
			int _initialize_map(size_type __initialize_size, size_type __lock_num)
			{
				_map = _map_alloc.allocate(__initialize_size);
				if (NULL == _map) {
					goto fail;
				}

				_map[0] = _node_alloc.allocate(_BUFF_SIZE);
				if (NULL == _map[0]) {
					goto fail;
				}

				_locks_map_size = (__initialize_size-1)/__lock_num+1;
				//��ʼ����, ����ռ�
				_locks = _locks_list_alloc.allocate(_locks_map_size);
				if (NULL == _locks) {
					goto fail;
				}

				//���ĳ�ʼ��
				if (_initialize_locks(_locks, _locks + _locks_map_size) != 0) {
					goto fail;
				}

				//��ʼ�����»�����
				if (!_lock_init) {
					if (pthread_mutex_init(&_up_locks, NULL) < 0) {
						goto fail;
					}
					_lock_init = true;
				}
				//λ�ñ�ǳ�ʼ��
				_last = _map[_finish];
				_finish = 0;
				_size = 0;
				_cur_list = 0;
				_map_list[0] = _map;
				_locks_list[0] = _locks;
				_map_size = __initialize_size;
				return 0;
			fail:
				if (NULL != _map) {
					if (NULL != _map[0]) {
						_node_alloc.deallocate(_map[0], _BUFF_SIZE);
					}
					_map_alloc.deallocate(_map, __initialize_size);
				}

				if (NULL != _locks) {
					_locks_list_alloc.deallocate(_locks, _locks_map_size);
				}
				_locks_map_size = 0;
				_reset();
				
				return -1;	
			}

			
			/**
			 * @brief ��ʼ����д��
			 *
			 * @param [in] __locks_first   : ���б��ͷ��
			 * @param [in] __locks_last   : ���б��β��
			 * @return �ɹ�����0��ʧ�ܷ���-1 
			**/
			int _initialize_locks(pthread_rwlock_t **__locks_first, 
					pthread_rwlock_t **__locks_last)
			{
				for (pthread_rwlock_t **plock = __locks_first;
						plock != __locks_last; ++plock) {
					*plock = _locks_alloc.allocate(1);
					if (NULL == *plock || pthread_rwlock_init(*plock, NULL) < 0) {
						for (pthread_rwlock_t **lock_destroy = __locks_first;
							lock_destroy != plock; ++lock_destroy) {
							if (NULL != *lock_destroy) {
								pthread_rwlock_destroy(*lock_destroy);
								_locks_alloc.deallocate(*lock_destroy, 1);
							}
						}
						if (*plock != NULL) {
							_locks_alloc.deallocate(*plock, 1);
						}
						return -1;
					}
				}
				
				return 0;
			}

			/**
			 * @brief ����������
			 * @return �ɹ�����0��ʧ�ܷ���-1
			 * */
			int _new_map()
			{
				pthread_rwlock_t **new_locks = NULL;
				//�µ������б�
				pointer* new_map = _map_alloc.allocate(_map_size<<1);
				if (NULL == new_map) {
					goto fail;
				}
				//�µ����б�
				new_locks = _locks_list_alloc.allocate(_locks_map_size<<1);
				if (NULL == new_locks) {
					goto fail;
				}
				//���Ƶ��µ�λ��
				memcpy(new_map, _map, _map_size * sizeof(new_map[0]));
				memcpy(new_locks, _locks, _locks_map_size * sizeof(new_locks[0]));
				//�������г�ʼ��
				if (_initialize_locks(new_locks + _locks_map_size, 
							new_locks + (_locks_map_size<<1)) != 0) { 
					goto fail;
				}
				
				//map_list����
				++_cur_list;
				_map_list[_cur_list] = new_map;
				_locks_list[_cur_list] = new_locks;
				//�滻�ɵ�_map
				_map = new_map;
				_locks = new_locks;
				_map_size <<= 1;
				_locks_map_size <<=1;
				return 0;

			fail:
				//ʧ�������ڴ�
				if (NULL != new_map) {
					_map_alloc.deallocate(new_map, _map_size<<1);
				}

				if (NULL != new_locks) {
					_locks_list_alloc.deallocate(new_locks, _locks_map_size<<1);
				}
				return -1;
			}

			/** 
			 * @brief ��β������Ԫ��
			 * @return �ɹ�����0��ʧ�ܷ���-1
			 * */
			int _push_back(const value_type& __x)
			{
				if (NULL == _map) {
					goto fail;
				}

				if (_last !=  _map[_finish] + _BUFF_SIZE - 1) {
					bsl::bsl_construct(_last, __x);
					++_last;
					++_size;
				} else {
					if (_finish + 1 == _map_size) {
						if (_new_map() < 0) {
							goto fail;
						}
					}
					pointer new_node = _node_alloc.allocate(_BUFF_SIZE);
					if (NULL == new_node) {
						goto fail;
					}
					bsl::bsl_construct(_last, __x);
					++_finish;
					_map[_finish] = new_node;
					_last = new_node;
					++_size;
				}
				return 0;
			fail:
				return -1;
			}

			/* *
			 * @brief ����β��Ԫ��
			 * @return �ɹ�����0��ʧ�ܷ���-1
			 * ������Ϊ�գ��򷵻�ʧ��
			 * */
			int _pop_back()
			{
				if (0 == _size) {
					return -1;
				}

				pthread_rwlock_t &plock = *_locks[(_size - 1)/_lsize];
				pthread_rwlock_wrlock(&plock);
				--_size;
				pthread_rwlock_unlock(&plock);
				if (_last == _map[_finish]) {
					_node_alloc.deallocate(_map[_finish], _BUFF_SIZE);
					--_finish;
					_last = _map[_finish] + _BUFF_SIZE;
				}
				--_last;
				bsl::bsl_destruct(_last);
				return 0;
			}

			int _assign(size_type __n, const value_type& __val)
			{
				_clear();
				for (size_type i = 0; i < __n; ++i) {
					if (_push_back(__val) != 0) {
						goto fail;
					}
				}
				return 0;
			fail:
				_clear();
				return -1;
			}

			/* *
			 * @brief �������Ԫ��
			 * @return �ɹ�����0��ʧ�ܷ���-1
			 * */
			int _clear()
			{
				//���С��0���Ѿ�Ϊ����û�б�Ҫ��pop��
				while (_pop_back() == 0) {
				}
				return 0;
			}
	};


} //namespace bsl


#endif  //__BSL_RWSEQUE_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
