/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_rwhashset.h,v 1.11 2009/08/24 06:24:49 scmpf Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file bsl_rwhashset.h
 * @author xiaowei(com@baidu.com)
 * @date 2008/07/28 22:16:54
 * @version $Revision: 1.11 $  2010/10/15 modified by zhjw(zhujianwei@baidu.com) 
 * @brief 
 *  
 **/


#ifndef  __BSL_RWHASHSET_H_
#define  __BSL_RWHASHSET_H_

#include <pthread.h>
#include <bsl/containers/hash/bsl_hashset.h>

namespace bsl
{

template <class _Key, /*×÷ÎªhashkeyµÄÀàĞÍ*/
		 /**
		  * hash º¯ÊıµÄ·Âº¯Êı£¬±ÈÈç
		  * struct xhash {
		  * 	inline size_t operator () (const _Key &_1);
		  * };
		  **/
		 class _HashFun = xhash<_Key>,
		 /**
		  * ÅĞ¶ÏÁ½¸ökeyÏàµÈµÄ·Âº¯Êı
		  * ±ÈÈç struct equal {
		  * 	inline bool operator () (const _Tp &_1, const _Tp &_2);
		  * };
		  */
		 class _Equl = std::equal_to<_Key>,	
		 /**
		  * ¿Õ¼ä·ÖÅäÆ÷£¬Ä¬ÈÏµÄ¿Õ¼ä·ÖÅäÆ÷ÄÜ¹»¸ßĞ§ÂÊµÄ¹ÜÀíĞ¡ÄÚ´æ£¬·ÀÖ¹ÄÚ´æËéÆ¬
		  * µ«ÊÇÔÚÈİÆ÷ÉúÃüÆïÄÚ²»»áÊÍ·ÅÉêÇëµÄÄÚ´æ
		  *
		  * bsl_alloc<_Key>×öÄÚ´æ·ÖÅäÆ÷£¬¿ÉÒÔÔÚÈİÆ÷ÉúÃüÆÚÄÚÊÍ·ÅÄÚ´æ£¬
		  * µ«ÊÇ²»ÄÜÓĞĞ§·ÀÖ¹ÄÚ´æËéÆ¬
		  */
		 class _InnerAlloc = bsl_sample_alloc<bsl_alloc<_Key>, 256>
		>
class bsl_rwhashset
{	
public:
	typedef _Key key_type;

	typedef bsl_rwhashset<_Key, _HashFun, _Equl, _InnerAlloc> _Self;
	typedef bsl_hashtable<_Key, _Key, _HashFun, _Equl, param_select<_Key>, _InnerAlloc> hash_type;
	typedef typename _InnerAlloc::pool_type::template rebind<BSL_RWLOCK_T>::other lockalloc_type;
	typedef typename hash_type::node_pointer node_pointer;

	BSL_RWLOCK_T *_locks;
	pthread_mutex_t _lock;
	size_t _lsize;
	lockalloc_type _alloc;
	hash_type _ht;

private:
    /** ¿½±´¹¹Ôìº¯Êı */
	bsl_rwhashset (const _Self &);          //½ûÓÃ
    /** ¸³ÖµÔËËã·û */
	_Self & operator = (const _Self &);     //½ûÓÃ

public:

    /* *
     * @brief Ä¬ÈÏ¹¹Ôìº¯Êı
     * ÎŞÒì³£
     * Ğèµ÷create
     * */
	bsl_rwhashset() {
		_locks = 0;
		_lsize = 0;
	}

    /**
     * @brief ´øÍ°´óĞ¡µÄÏß³Ì°²È«ÈİÆ÷
     *        Èç¹û¹¹ÔìÊ§°Ü£¬½«Ïú»Ù¶ÔÏó
     *ÅÅ×Òì³£
     * ²»Ğèµ÷create
     **/
    bsl_rwhashset(size_t bitems, size_t lsize = 0,
                const _HashFun &hf = _HashFun(),
                const _Equl &eq = _Equl()) {
        _locks = 0;
        _lsize = 0;
        if (create(bitems,lsize,hf,eq) != 0) {
            destroy();
            throw BadAllocException()<<BSL_EARG
                <<"create error when create rwhashset bitems "<<bitems;
        }
    }

	~bsl_rwhashset() {
		destroy();
	}

	BSL_RWLOCK_T & get_rwlock(size_t key) const {
		return _locks[key % _ht._bitems % _lsize];
	}

	/**
	 * @brief ´´½¨Ò»ÕÅÏß³Ì°²È«µÄhash±í
	 *
	 * @param [in/out] bitems   : size_t	hashÍ°¸öÊı
	 * @param [in/out] lsize   : size_t		¶ÁĞ´Ëø¸öÊı
	 * @param [in/out] hf   : const _HashFun&	hashº¯Êı
	 * @param [in/out] eq   : const _Equl&		±È½Ïº¯Êı
	 * @return  int ·µ»Ø 0±íÊ¾´´½¨³É¹¦£¬ÆäËûÊ§°Ü
	 * @retval  Ïß³Ì²»°²È«
	 * @see 
	 * @author xiaowei
	 * @date 2008/08/12 21:41:08
	**/
	int create(size_t bitems, size_t lsize = 0, 
			const _HashFun &hf = _HashFun(), 
			const _Equl &eq = _Equl()) {
		if (bitems >= size_t(-1) / sizeof(void *)) {
			__BSL_ERROR("too large bitems, overflower");
			return -1;
		}
		destroy();

		if (lsize > bitems) {
			_lsize = bitems;
		} else {
			_lsize = lsize;
		}
		if (_lsize <= 0) {
			_lsize  = (1 + (size_t)((float)(0.05)*(float)bitems));
			if (_lsize > 100) {
				_lsize = 100;
			}
		}

		_alloc.create();

		_locks = _alloc.allocate(_lsize);
		if (_locks == 0) return -1;
		for(size_t i=0; i<_lsize; ++i) {
			BSL_RWLOCK_INIT(_locks+i);
		}

		pthread_mutex_init(&_lock, NULL);
		return _ht.create(bitems, hf, eq);
	}

    /* *
     * @brief ÅĞ¶ÏrwhashsetÊÇ·ñÒÑcreate
     * @author zhujianwei
     * @date 2010/12/13
     * */
    bool is_created() const{
        return _ht.is_created();
    }

	/**
	 * @brief Ïú»Ùhash±í
	 *
	 * @return  int ·µ»Ø0±íÊ¾É¾³ı³É¹¦£¬ÆäËû±íÊ¾É¾³ıÊ§°Ü
	 * @retval  Ïß³Ì²»°²È«
	 * @see 
	 * @author xiaowei
	 * @date 2008/08/12 21:16:39
	**/
	int destroy() {
		if (_locks) {
			_alloc.deallocate(_locks, _lsize);
			pthread_mutex_destroy(&_lock);
		}
		_alloc.destroy();
		_locks = NULL;
		_lsize = 0;
		return _ht.destroy();
	}

	/**
	 * @brief ²éÑ¯key_typeÊÇ·ñ´æÔÚ
	 *
	 * @param [in/out] k   : const key_type&	Ö¸¶¨µÄ²éÕÒkey
	 * @return  int 
	 *				·µ»Ø HASH_EXIST		±íÊ¾hashÖµ´æÔÚ
	 *				·µ»Ø HASH_NOEXIST	±íÊ¾hashÖµ²»´æÔÚ
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2008/08/12 21:17:52
	**/
	int get(const key_type &k) const {
		size_t key = _ht._hashfun(k);
		BSL_RWLOCK_T &__lock = get_rwlock(key);
		BSL_RWLOCK_RDLOCK(&__lock);
		int ret = HASH_EXIST;
		if (_ht.find(key, k) == NULL) {
			ret = HASH_NOEXIST;
		}
		BSL_RWLOCK_UNLOCK(&__lock);
		return ret;
	}
	int get(const key_type &k) {
		size_t key = _ht._hashfun(k);
		BSL_RWLOCK_T &__lock = get_rwlock(key);
		BSL_RWLOCK_RDLOCK(&__lock);
		int ret = HASH_EXIST;
		if (_ht.find(key, k) == NULL) {
			ret = HASH_NOEXIST;
		}
		BSL_RWLOCK_UNLOCK(&__lock);
		return ret;
	}

	/**
	 * @brief ½«key²åÈëhash±í
	 *
	 * @param [in/out] k   : const key_type&
	 * @return  int 
	 * 			·µ»Ø	-1±íÊ¾setµ÷ÓÃ³ö´í
	 * 			·µ»Ø	HASH_EXIST	±íÊ¾hash½áµã´æÔÚ
	 * 			·µ»Ø	HASH_INSERT_SEC	±íÊ¾²åÈë³É¹¦
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2008/08/12 21:33:10
	**/
	int set(const key_type &k) {
		size_t key = _ht._hashfun(k);
		pthread_mutex_lock(&_lock);

		BSL_RWLOCK_T &__lock = get_rwlock(key);
		BSL_RWLOCK_WRLOCK(&__lock);
		int ret = _ht.set(key, k, k);
		BSL_RWLOCK_UNLOCK(&__lock);

		pthread_mutex_unlock(&_lock);
		return ret;
	}

	/**
	 * @brief ¸ù¾İÖ¸¶¨µÄkeyÉ¾³ı½áµã
	 *
	 * @param [in/out] k   : const key_type&
	 * @return  int 
	 * 		·µ»Ø	HASH_EXIST±íÊ¾½áµã´æÔÚ²¢É¾³ı³É¹¦
	 * 		·µ»Ø	HASH_NOEXIST±íÊ¾½áµã²»´æÔÚ²»ÓÃÉ¾³ı
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2008/08/12 21:24:58
	**/
	int erase(const key_type &k) {
		size_t key = _ht._hashfun(k);
		pthread_mutex_lock(&_lock);

		BSL_RWLOCK_T &__lock = get_rwlock(key);
		BSL_RWLOCK_WRLOCK(&__lock);
		int ret = _ht.erase(key, k);
		BSL_RWLOCK_UNLOCK(&__lock);

		pthread_mutex_unlock(&_lock);
		return ret;
	}


	/**
	 * @brief ½«Êı¾İÇå¿Õ£¬Ïß³Ì°²È«
	 *
	 * @return  int 
	 * @retval   
	 * @see 
	 * @author xiaowei
	 * @date 2008/08/12 22:17:48
	**/
	int clear() {
		//¶ÁĞ´ËøÃ¿±»³õÊ¼»¯,Ã»ÓĞ±»create
		if (_locks == NULL) {
			return 0;
		}
		pthread_mutex_lock(&_lock);
		for (size_t i=0; i<_ht._bitems; ++i) {
			BSL_RWLOCK_T &__lock = get_rwlock(i);
			BSL_RWLOCK_WRLOCK(&__lock);
			//typename hash_type::node_t *nd = NULL;
			node_pointer nd = 0;
			while (_ht._bucket[i]) {
				nd = _ht._bucket[i];
				_ht._bucket[i] = _ht._sp_alloc.getp(_ht._bucket[i])->next;
				bsl::bsl_destruct(&_ht._sp_alloc.getp(nd)->val);
				_ht._sp_alloc.deallocate(nd, 1);
			}
			BSL_RWLOCK_UNLOCK(&__lock);
		}
		_ht.clear();//Ê²Ã´¶¼·¢Éú£¬Ö»Çå¿ÕÒ»ÏÂ±êÖ¾Î»
		pthread_mutex_unlock(&_lock);
		return 0;
	}

	size_t size() const {
		return _ht.size();
	}

	template <class _Iterator>
	int assign(_Iterator __begin, _Iterator __end) {
		return _ht.assign(__begin, __end);
	}

};

};

#endif  //__BSL_RWHASHSET_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
