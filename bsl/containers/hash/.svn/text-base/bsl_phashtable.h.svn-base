/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: bsl_phashtable.h,v 1.11 2010/02/24 09:19:45 scmpf Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file bsl_phashtable.h
 * @author xiaowei(com@baidu.com)
 * @date 2008/08/05 11:19:36
 * @version $Revision: 1.11 $  2010/10/15 modified by zhjw(zhujianwei@baidu.com)
 * @brief 
 *  
 **/


#ifndef  __BSL_PHASHTABLE_H_
#define  __BSL_PHASHTABLE_H_
#include <bsl/containers/hash/bsl_hashtable.h>
#include <bsl/containers/hash/bsl_rwhashset.h>
#include <bsl/archive/bsl_serialization.h>
#include <bsl/exception/bsl_exception.h>
#include <vector>
#include <pthread.h>

namespace bsl
{

enum {
    PHASH_NORMAL = 0,	//正常状态
    PHASH_START_CHECKPOINT,	//开始CHECKPOINTER
    PHASH_END_CHECKPOINT,		//checkpoint 完之后内存整理
};

/**
 * @brief 线程安全的hashtable
 */
template <class _Key	//hash key
          , class _Value	//存储的数据 能萃取除key值
          , class _HashFun	//hash 函数
          , class _Equl	//key值的比较函数
          , class _GetKey	//从 _Value萃取出key值的函数
          , class _InnerAlloc	//维护状态数据的内部allocator
          >
class bsl_phashtable 
{

  public:
    /**
     * @brief 自身的别名
     */
    typedef bsl_phashtable<_Key, _Value, _HashFun, _Equl, _GetKey, _InnerAlloc> _Self;
    /**
     * @brief 内部使用的hashtable
     */
    typedef bsl_hashtable<_Key, _Value, _HashFun, _Equl, _GetKey, _InnerAlloc> _Base;
    /**
     * @brief 迭代器
     */
    typedef typename _Base::iterator iterator;
    /**
     * @brief 只读迭代器
     */
    typedef typename _Base::const_iterator const_iterator;
    /**
     * @brief 值
     */
    typedef _Value value_type;
    /**
     * @brief 键
     */
    typedef _Key key_type;
    /**
     * @brief 键值的指针类型
     */
    typedef value_type * pointer;

    typedef typename _Base::node_pointer node_pointer;
    typedef typename _Base::node_t node_t;
    typedef typename _InnerAlloc::pool_type pool_type;

    struct deleteNode {
        node_pointer lst;
        size_t key;
    };
    typedef std::vector<deleteNode> deleteVec;

    BSL_RWLOCK_T * _rwlocks;
    size_t _rwlocksize;
    size_t _tmpsize;	//保存在dump的时候有多少个结点差
    node_pointer *_tmplist;	//临时用来检索的拉链
    volatile int _flag;	//标志当前状态
    typedef typename pool_type::template rebind<BSL_RWLOCK_T>::other lockalloc_type;
    //typedef typename pool_type::template rebind<node_pointer >::other nodealloc_type;
    
    /**
     * @brief 内部的hashtable 
     * 
     */
    _Base _base;
    lockalloc_type _lockalloc;
    //nodealloc_type _nodealloc;

    bool _mlockcreat;
    pthread_mutex_t _cplock;
    pthread_mutex_t _uplock;
    mutable pthread_mutex_t _hslock;//用来在_end_checkpoint和_get时锁住_hashset和_tmplist
    typedef bsl_rwhashset<pointer, xhash<pointer>, std::equal_to<pointer>, _InnerAlloc> _Set;
    typedef typename pool_type::template rebind<_Set>::other setalloc_type;
    _Set *_hashset;
    setalloc_type _setalloc;
  protected:
    /**
     * @brief 申请读写锁
     */
    int alloc_rwlocks(size_t bitems, size_t lsize) {
        if (lsize > bitems) {
            _rwlocksize = bitems;
        } else {
            _rwlocksize = lsize;
        }
        if (_rwlocksize <= 0) {
            _rwlocksize  = (1 + (size_t)((float)(0.05)*(float)bitems));
            if (_rwlocksize > 100) {
                _rwlocksize = 100;
            }
        }
        _rwlocks = _lockalloc.allocate(_rwlocksize);
        if (_rwlocks == 0) {
            return -1;
        }
        //__BSL_DEBUG("allocate rwlocsk %d", (int)_rwlocksize);

        for (size_t i=0; i<_rwlocksize; ++i) {
            BSL_RWLOCK_INIT(_rwlocks+i);
        }
        return 0;
    }
    /**
     * @brief 重新创建phashtable
     */
    int recreate(size_t bitems, size_t rwlsize) {
        _tmpsize = 0;
        _tmplist = NULL;
        _flag = PHASH_NORMAL;

        _lockalloc.create();
        //_nodealloc.create();
        _setalloc.create();

        if (alloc_rwlocks(bitems, rwlsize) != 0) {
            goto err;
        }

        return 0;
  err:
        if (_rwlocks) {
            for (size_t i=0; i<_rwlocksize; ++i) {
                BSL_RWLOCK_DESTROY(_rwlocks+i);
            }
            _lockalloc.deallocate(_rwlocks, _rwlocksize);
        }
        _rwlocks = NULL;
        _rwlocksize = 0;
        _lockalloc.destroy();
        //_nodealloc.destroy();
        _setalloc.destroy();
        return -1;
    }

    void destruct_node(node_pointer node, size_t key) {
        BSL_RWLOCK_T &__lock = _rwlocks[key % _rwlocksize];
        BSL_RWLOCK_WRLOCK(&__lock);
        bsl::bsl_destruct(&(_base._sp_alloc.getp(node)->val));
        _base._sp_alloc.deallocate(node, 1);
        BSL_RWLOCK_UNLOCK(&__lock);
    }
  private:
    /**
     * @brief 拷贝构造函数
     */
    bsl_phashtable(const _Self&);           //禁用
    /**
     * @brief 赋值运算符
     */
    _Self & operator = (const _Self &);     //禁用
 
  public:
    /**
     * @brief 构造函数
     * 无异常
     * 需调create
     */
    bsl_phashtable() {
        _reset();
    }
    /**
     * @brief 带桶大小的构造函数
     *        如果构造失败，将销毁对象 
     * 
     * 抛异常
     * 不需调create
     *
     * @param   [in] bitems     : size_t hash桶个数
     * @param   [in] rwlsize    : size_t 读写锁个数
     * @param   [in] hf         : const _HashFun& hash函数
     * @param   [in] eq         : const _Equl& 比较函数
     * @return
     * @retval
     * @see
     * @author liaoshangbin
     * @data 2010/08/30 13:42:53
     **/
    bsl_phashtable(size_t bitems, size_t rwlsize = 0,
                   const _HashFun &hf = _HashFun(), 
                   const _Equl &eq = _Equl()) {
        _reset();
        if (create(bitems,rwlsize,hf,eq) != 0) {
            destroy();
            throw BadAllocException()<<BSL_EARG
                                     <<"create error when create phashtable with bitems "<<bitems;
        }
    }
	
    ~bsl_phashtable() {
        destroy();
    }

    /**
     * @brief 起始迭代器
     */
    iterator begin() {
        return _base.begin();
    }

    /**
     * @brief 起始只读迭代器
     */
    const_iterator begin() const {
        return _base.begin();
    }
    /**
     * @brief 末尾迭代器
     */
    iterator end() {
        return _base.end();
    }
    /**
     * @brief 末尾只读迭代器
     */
    const_iterator end() const {
        return _base.end();
    }
    /**
     * @brief phash表中元素的个数
     */
    size_t size() const {
        return _tmpsize;
    }

    /**
     * @brief 检查目前phash表的状态
     *
     * @return  int 
     * 		PHASH_NORMAL 表示目前处于正常状态
     * 		PHASH_START_CHECKPOINT 表示目前开始CHECKPOINTER
     * 		PHASH_END_CHECKPOINT 表示checkpoint结束，开始修正数据
     * @retval   
     * @see 
     * @note 
     * @author xiaowei
     * @date 2008/08/18 17:17:26
     **/
    int dumpstatus() const {
        return _flag;
    }

    /**
     * @brief 创建一张线程安全的hash表
     *
     * @param [in/out] bitems   : size_t	hash桶个数
     * @param [in/out] lsize   : size_t		读写锁个数
     * @param [in/out] hf   : const _HashFun&	hash函数
     * @param [in/out] eq   : const _Equl&		比较函数
     * @return  int 返回 0表示创建成功，其他失败
     * @retval  线程不安全
     * @see 
     * @author xiaowei
     * @date 2008/08/12 21:41:08
     **/
    int create(size_t bitems, size_t rwlsize = 0,
               const _HashFun &hf = _HashFun(), 
               const _Equl &eq = _Equl()) {
        //判断溢出
        if (bitems >= size_t(-1) / sizeof(void *)) {
            __BSL_ERROR("too large bitems, overflower");
            return -1;
        }

        //判断hash桶数
        if(0 == bitems){
            __BSL_ERROR("bitems == 0");
        }

        if (destroy() != 0) {
            __BSL_ERROR("destroy error when create hash bitems[%lu]", (unsigned long)bitems);
            return -1;
        }
        if (recreate(bitems, rwlsize) != 0) {
            __BSL_ERROR("recreate error when create hash bitems[%lu], rwlsize[%lu]", 
                        (unsigned long)bitems, (unsigned long)rwlsize);
            return -1;
        }

        if (pthread_mutex_init(&_cplock, NULL) != 0) {
            int ret = destroy();
            __BSL_ERROR("init cplock err, destroy data[%d]", ret);
            return -1;
        }
        if (pthread_mutex_init(&_uplock, NULL) != 0) {
            int ret = destroy();
            __BSL_ERROR("init uplock err, destroy data[%d]", ret);
            pthread_mutex_destroy(&_cplock);
            return -1;
        }

        if (pthread_mutex_init(&_hslock, NULL) != 0) {
            int ret = destroy();
            __BSL_ERROR("init uplock err, destroy data[%d]", ret);
            pthread_mutex_destroy(&_cplock);
            pthread_mutex_destroy(&_uplock);
            return -1;
        }

        _mlockcreat = true;

        _hashset = NULL;
        return _base.create(bitems, hf, eq);
    }

    /* *
     * @brief 判断phashtable是否已create
     * @author zhujianwei
     * @date 2010/12/13
     * */
    bool is_created() const{
        return _base.is_created();
    }

    /**
     * @brief 销毁hash表
     *
     * @return  int 返回0表示删除成功，其他表示删除失败
     * 			失败的情况是容器正在dump数据
     * @retval  线程不安全
     * @see 
     * @author xiaowei
     * @date 2008/08/12 21:16:39
     **/
    int destroy() {
        if (_flag != PHASH_NORMAL) {
            return -1;
        }
        if (_rwlocks) {
            for (size_t i=0; i<_rwlocksize; ++i) {
                BSL_RWLOCK_DESTROY(_rwlocks + i);
            }
            //__BSL_DEBUG("release rwlocks[%ld]", _rwlocksize);
            _lockalloc.deallocate(_rwlocks, _rwlocksize);
        }
        _rwlocks = NULL;
        _rwlocksize = 0;

        if (_tmplist) {
            //_nodealloc.deallocate(_tmplist, _base._bitems);
            _base._np_alloc.deallocate(_tmplist, _base._bitems);
        }
        _tmplist = NULL;

        if (_hashset != NULL) {
            _hashset->destroy();
            bsl::bsl_destruct(_hashset);
            _setalloc.deallocate(_hashset, 1);
            _hashset = NULL;
        }

        _lockalloc.destroy();
        //_nodealloc.destroy();
        _setalloc.destroy();
        if (_mlockcreat) {
            pthread_mutex_destroy(&_cplock);
            pthread_mutex_destroy(&_uplock);
            pthread_mutex_destroy(&_hslock);
            _mlockcreat = false;
        }
        return _base.destroy();
    }

	
    /**
     * @brief 清空数据
     *
     * @return  int 0表示成功,其他表示失败
     * @retval  只有在非dump状态才能清空数据
     * 			如果容器处于checkpoint状态，clear返回-1
     * @see 
     * @author xiaowei
     * @date 2008/08/12 22:59:36
     **/
    int clear() {
        if (!_mlockcreat) {//没有初始化锁
            return 0;
        }
        mutexlock_monitor q(&_uplock);
        if (_flag != PHASH_NORMAL) {
            return -1;
        }
        for (size_t i=0; i<_base._bitems; ++i) {
            node_pointer nd = 0;
            while (_base._bucket[i]) {
                nd = _base._bucket[i];
                _base._bucket[i] = _base._sp_alloc.getp(_base._bucket[i])->next;
                destruct_node(nd, i);
            }
        }
        _base._size = 0;
        _tmpsize = 0;
        return 0;
    }

    /**
     * @brief 当前时刻创建内存dump
     *
     * @return  int 0表示成功创建
     * @retval   
     * @see 
     * @author xiaowei
     * @date 2008/08/12 23:00:17
     **/
    int make_checkpoint() {
        //先锁cp 再锁up防止死锁
        //mutexlock_monitor l(&_cplock);
        //mutexlock_monitor q(&_uplock);
        if (pthread_mutex_trylock(&_cplock) != 0) {
            return -1;
        }
        if (pthread_mutex_lock(&_uplock) != 0) {
            pthread_mutex_unlock(&_cplock);
            return -1;
        }
        int ret = _make_checkpoint();
        pthread_mutex_unlock(&_uplock);
        pthread_mutex_unlock(&_cplock);

        return ret;
    }

    /**
     * @brief make_checkpoint掉完之后和end_checkpoint被调用之前
     * 	make_checkpoint调用瞬间的内存将被锁住，你可以对这个内存在另外的线程里面
     * 	做遍历读操作.
     *
     * @return  int 
     * @retval   
     * @see 
     * @note 
     * @author xiaowei
     * @date 2008/08/18 17:37:46
     **/
    int end_checkpoint() {
        mutexlock_monitor l(&_cplock);
        return _end_checkpoint();
    }

    /**
     * @brief 序列化方法，外面申明一个archive实体 ar
     * 		调用ar.write()进行序列化到指定位置
     * 		这里实现的是如何将这个类打包成流
     *
     * @param [in/out] ar   : _Archive&	序列化句柄
     * @return  int 成功返回0，失败返回 -1
     * @retval   注意！！！！想序列化一个线程安全的hash表
     * 				必须调用make_checkpoint，这样他才知道序列化哪些内存
     * 				因为多读多写下，hash表的状态是变化的,但是序列化内存是你调用
     * 				make_checkpoint瞬间的内存映射，在 serialization 函数返回之前，
     * 				所有的更新操作，都不会被序列化
     *
     * 				如果你没有调用checkpoint，那么 serialization 函数调用的时候，会
     * 				显著帮你调用一个，dump当前内存
     *
     * 				线程安全
     * @see 
     * @note 
     * @author xiaowei
     * @date 2008/08/13 14:21:21
     **/
    template <class _Archive>
    int serialization(_Archive & ar) {
        mutexlock_monitor l(&_cplock);
        int ret = -1;
        __BSL_DEBUG("this = 0x%lx", (long)(this));
        //锁没被获取，这样需要调用make_checkpoint获取锁 
        {
            mutexlock_monitor q(&_uplock);
            if (_flag == PHASH_NORMAL) {
                ret = _make_checkpoint();
                if (ret != 0) {
                    return -1;
                }
            }
            if (_flag != PHASH_START_CHECKPOINT) {
                return -1;
            }
        }
        {
            if (_base.serialization(ar) != 0) {
                _end_checkpoint();
                return -1;
            }
        }

        __BSL_DEBUG("_flag = %d", _flag);
        //checkpoint完毕之后的收尾工作
        return _end_checkpoint();
    }

    /**
     * @brief 从序列化流中，重建数据结构
     *
     * @param [in/out] ar   : _Archive&
     * @return  template <class _Archive> 	int 成功返回0，失败返回-1
     * @retval   不可重入的算法
     * @see 
     * @note 
     * @author xiaowei
     * @date 2008/08/13 14:24:50
     **/
    template <class _Archive>
    int deserialization(_Archive & ar) {
        size_t bitems = 0;
        if (bsl::deserialization(ar, bitems) != 0) {
            __BSL_ERROR("deserialization bitems error");
            return -1;
        }
        __BSL_DEBUG("bitems = %ld", (long)bitems);

        //已经被初始化过
        if (_base._bucket) {
            if (clear() != 0) {
                __BSL_ERROR("clear phashtable error");
                return -1;
            }
        } else {	//没有初始化过
            if (create(bitems) != 0) {
                __BSL_ERROR("create hashtable error");
                return -1;
            }
        }

        size_t __size = 0;
        if (bsl::deserialization(ar, __size) != 0) {
            __BSL_ERROR("deserialization size error");
            return -1;
        }
        value_type val;
        for (size_t i=0; i<__size; ++i) {
            if (bsl::deserialization(ar, val) != 0) {
                __BSL_ERROR("deserialization val at[%ld] error", (long)i);
                return -1;
            }
            if (set(_base._getkey(val), val) == -1) {
                __BSL_ERROR("deserialization set at[%ld] error", (long)i);
                return -1;
            }
        }

        return 0;
    }

    /**
     * @brief 根据指定key获取value值
     *
     * @param [in/out] k   : const key_type&	指定的查找key
     * @param [in/out] val   : value_type*	获取的value值的存储地址
     * 				如果value不为空，将*val赋值查找的值
     * @return  int 
     *				返回 HASH_EXIST		表示hash值存在
     *				返回 HASH_NOEXIST	表示hash值不存在
     * @retval   
     * @see 
     * @author xiaowei
     * @date 2008/08/12 21:17:52
     **/
    int get(const key_type &k, value_type *val = 0) const {
        if(0 == _base._bitems){
            return HASH_NOEXIST;
        }
        size_t key = _base._hashfun(k) % _base._bitems;
        BSL_RWLOCK_T &__lock = _rwlocks[ key % _rwlocksize];
        BSL_RWLOCK_RDLOCK(&__lock);
        value_type *ptr = _get(key, k);
        int ret = HASH_NOEXIST;
        if (ptr) {
            ret = HASH_EXIST;
            if (val) {
                *val = *ptr;
            }
        }
        BSL_RWLOCK_UNLOCK(&__lock);
        return ret;
    }
    
    /**
     * @brief 根据指定key获取value值
     */
    int get(const key_type &k, value_type *val = 0) {
        if(0 == _base._bitems){
            return HASH_NOEXIST;
        }
        size_t key = _base._hashfun(k) % _base._bitems;
        BSL_RWLOCK_T &__lock = _rwlocks[ key % _rwlocksize];
        BSL_RWLOCK_RDLOCK(&__lock);
        value_type *ptr = _get(key, k);
        int ret = HASH_NOEXIST;
        if (ptr) {
            ret = HASH_EXIST;
            if (val) {
                *val = *ptr;
            }
        }
        BSL_RWLOCK_UNLOCK(&__lock);
        return ret;
    }
    
    /**
     * @brief 根据指定key获取value值，应用回调函数，返回原值
     * 若不需要原值，传入NULL指针
     *  _ValOpFunc可以是函数或functor，参数为(value_type*, void*)
     */
    template <typename _ValOpFunc>
    int get(const key_type &k, value_type *old_val, _ValOpFunc val_op_func, void *args = 0){
        if(0 == _base._bitems){
            return HASH_NOEXIST;
        }
        size_t key = _base._hashfun(k) % _base._bitems;
        BSL_RWLOCK_T &__lock = _rwlocks[ key % _rwlocksize];
        BSL_RWLOCK_WRLOCK(&__lock);
        value_type *ptr = _get(key, k);
        int ret = HASH_NOEXIST;
        if (ptr) {
            ret = HASH_EXIST;
            if (old_val) {
                *old_val = *ptr;
            }
            val_op_func(ptr, args);
        }
        BSL_RWLOCK_UNLOCK(&__lock);
        return ret;
    }

    /**
     * @brief 根据指定key获取valua pair中的second值
     *
     * @param [in/out] k   : const key_type&	指定的查找key
     * @param [in/out] val   : _Second*	获取的second值的存储地址
     * @return  int 
     *				返回 HASH_EXIST		表示hash值存在
     *				返回 HASH_NOEXIST	表示hash值不存在
     * @retval   
     * @see 
     * @author xiaowei
     * @date 2008/08/12 21:17:52
     **/
    template <typename _Second>
    int get_map(const key_type &k, _Second *val = 0) const {
        if(0 == _base._bitems){
            return HASH_NOEXIST;
        }
        size_t key = _base._hashfun(k) % _base._bitems;
        BSL_RWLOCK_T &__lock = _rwlocks[key % _rwlocksize];
        BSL_RWLOCK_RDLOCK(&__lock);
        value_type *ptr = _get(key, k);
        int ret = HASH_NOEXIST;
        if (ptr) {
            ret = HASH_EXIST;
            if (val) {
                *val = ptr->second;
            }
        }
        BSL_RWLOCK_UNLOCK(&__lock);
        return ret;
    }
    
    /**
     * @brief 根据指定key获取valua pair中的second值
     **/
    template <typename _Second>
    int get_map(const key_type &k, _Second *val = 0) {
        if(0 == _base._bitems){
            return HASH_NOEXIST;
        }
        size_t key = _base._hashfun(k) % _base._bitems;
        BSL_RWLOCK_T &__lock = _rwlocks[key % _rwlocksize];
        BSL_RWLOCK_RDLOCK(&__lock);
        value_type *ptr = _get(key, k);
        int ret = HASH_NOEXIST;
        if (ptr) {
            ret = HASH_EXIST;
            if (val) {
                *val = ptr->second;
            }
        }
        BSL_RWLOCK_UNLOCK(&__lock);
        return ret;
    }
    
    /**
     * @brief 根据指定key获取value pair中的second值，应用回调函数，返回原值
     * 若不需要原值，传入NULL指针
     * _ValOpFunc可以是函数指针，也可以是functor，参数类型为(_Second*, void*)
     **/
    template <typename _Second, typename _ValOpFunc>
    int get_map(const key_type &k, _Second *old_val, _ValOpFunc val_op_func, void *args = 0) {
        if(0 == _base._bitems){
            return HASH_NOEXIST;
        }
        size_t key = _base._hashfun(k) % _base._bitems;
        BSL_RWLOCK_T &__lock = _rwlocks[key % _rwlocksize];
        BSL_RWLOCK_WRLOCK(&__lock);
        value_type *ptr = _get(key, k);
        int ret = HASH_NOEXIST;
        if (ptr) {
            ret = HASH_EXIST;
            if (old_val) {
                *old_val = ptr->second;
            }
            val_op_func(&(ptr->second), args);
        }
        BSL_RWLOCK_UNLOCK(&__lock);
        return ret;
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
        if(0 == _base._bitems){
            return -1;
        }
        size_t key = _base._hashfun(k) % _base._bitems;
        mutexlock_monitor q(&_uplock);
        int ret = _set(key, k, val, flag);
        return ret;
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
        if(0 == _base._bitems){
            return HASH_NOEXIST;
        }
        size_t key = _base._hashfun(k) % _base._bitems;
        mutexlock_monitor q(&_uplock);
        int ret = _erase(key, k);
        return ret;
    }

    /**
     * @brief 附值是先清空数据，后附值，所以如果附值失败，会将数据清空
     * 	@附值过程中，原来的老数据已经被删除，可能检索不出来
     *
     * @param [in/out] __begin   : _Iterator
     * @param [in/out] __end   : _Iterator
     * @return  int 
     * 		成功返回 0
     * 		如果容器没有调用create，将返回－1失败
     * 		如果容器clean失败，将返回-1
     * 		如果容器处于checkpoint状态，将返回－1失败
     * @retval   
     * @see 
     * @note 
     * @author xiaowei
     * @date 2008/08/13 22:03:43
     **/
    template <class _Iterator>
    int assign(_Iterator __begin, _Iterator __end) {
        if (clear() != 0) {
            __BSL_ERROR("inside assign clear error");
            return -1;
        }
        if (_base._bitems <= 0) {
            __BSL_ERROR("didn't create before assign");
            return -1;
        }
        //__BSL_DEBUG("start to phashtable assign");
        mutexlock_monitor q(&_uplock);
        //__BSL_DEBUG("inside lock");
        if (_flag == PHASH_NORMAL) {
            for (_Iterator iter = __begin; iter != __end; ++iter) {
                if ( _base.set(_base._hashfun(_base._getkey(*iter)), 
                               _base._getkey(*iter), *iter) == -1 ) {
                    __BSL_ERROR("set error in phashtable assign");
                    return -1;
                }
                ++ _tmpsize;
            }
            return 0;
        }
        return -1;
    }

  public:
    value_type * _get(size_t key, const key_type &k) const {
        //正常情况下
        if (_flag == PHASH_NORMAL) {
            return _base.find(key, k);
        } else {
            if(_tmplist!=NULL){
                mutexlock_monitor __lock(&_hslock);
                if(_tmplist!=NULL){
                    //在新链上查是否有数据
                    node_pointer lst = _tmplist[key];
                    while (lst) {
                        if (_base._equl(_base._getkey(_base._sp_alloc.getp(lst)->val), k)) {
                            return & _base._sp_alloc.getp(lst)->val;
                        }
                        lst = _base._sp_alloc.getp(lst)->next;
                    }
                }
            }
            //新链上没有查到数据
            //老链上查找数据
            value_type *val = _base.find(key, k);
            if(_hashset!=NULL){
                mutexlock_monitor __lock(&_hslock);
                //查看是否标志删除
                if (_hashset!=NULL && _hashset->get(val) == HASH_EXIST) {
                    return NULL;
                }
            }
            return val;
        }
    }

    
    value_type * _get(size_t key, const key_type &k) {
        //正常情况下
        if (_flag == PHASH_NORMAL) {
            return _base.find(key, k);
        } else {
            if(_tmplist!=NULL){
                mutexlock_monitor __lock(&_hslock);
                if(_tmplist!=NULL){
                    //在新链上查是否有数据
                    node_pointer lst = _tmplist[key];
                    while (lst) {
                        if (_base._equl(_base._getkey(_base._sp_alloc.getp(lst)->val), k)) {
                            return & _base._sp_alloc.getp(lst)->val;
                        }
                        lst = _base._sp_alloc.getp(lst)->next;
                    }
                }
            }
            //新链上没有查到数据
            //老链上查找数据
            value_type *val = _base.find(key, k);
            if(_hashset!=NULL){
                mutexlock_monitor __lock(&_hslock);
                //查看是否标志删除
                if (_hashset!=NULL && _hashset->get(val) == HASH_EXIST) {
                    return NULL;
                }
            }
            return val;
        }
    }

    int _set(size_t key, const key_type &k, const value_type &val, int flag) {
        int ret = 0;
        switch (_flag) {
            case PHASH_START_CHECKPOINT:
                ret = _start_cp_set(key, k, val, flag);
                break;
            case PHASH_END_CHECKPOINT:
                ret = _end_cp_set(key, k, val, flag);
                break;
            default:
                ret = _normal_set(key, k, val, flag, _base._bucket);
                break;
        }
        if (ret == HASH_INSERT_SEC) {
            ++ _tmpsize;
        }
        return ret;
    }

    int _end_cp_set(size_t key, const key_type &k, const value_type &val, int flag) {
#if 0
        value_type *ptr = _base.find(key, k);
        if (ptr) {	//底层bucket中存在结点
            //查看结点是否被删除
            if (_hashset->get(ptr) != HASH_EXIST) {
                //结点不存在
                return _normal_set(key, k, val, flag, _base._bucket);
            }
        }
        return _normal_set(key, k, val, flag, _tmplist);
#endif
        value_type *ptr = _base.find(key, k);
        if (ptr) {	//底层bucket中存在结点
            //查看结点是否被删除, 且队列也是空的
            if (_hashset->get(ptr) != HASH_EXIST) {
                //结点没有被删除过
                if (_tmplist[key] == 0) {
                    return _normal_set(key, k, val, flag, _base._bucket);
                } else {
                    if (flag == 0) {
                        return HASH_EXIST;
                    } else {
                        _normal_erase(key, k, _base._bucket);
                        _normal_set(key, k, val, flag, _tmplist);
                        return HASH_OVERWRITE;
                    }
                }
            } else {
                return _normal_set(key, k, val, flag, _tmplist);
            }
        } else {
            if (_tmplist[key] == 0) {
                return _normal_set(key, k, val, flag, _base._bucket);
            } else {
                return _normal_set(key, k, val, flag, _tmplist);
            }
        }
        return 0;
    }
    int _start_cp_set(size_t key, const key_type &k, const value_type &val, int flag) {
        int tmp = 0;
        value_type *ptr = _base.find(key, k);
        if (ptr) {	//覆盖
            if (flag) {
                //标志删除
                if (_hashset->set(ptr) == HASH_INSERT_SEC) {
                    tmp = 1;	//表示存在这个元素，且有效,现在标志删除
                }
            } else {
                //如果不需要替换，查看是否被标志删除过
                if (_hashset->get(ptr) != HASH_EXIST) {
                    //没有被标志删除过，直接返回
                    return HASH_EXIST;
                }
            }
        }
        //需要在新链上修改数据
        int ret = _normal_set(key, k, val, flag, _tmplist);
        if (ret != -1 && tmp == 1) {
            return HASH_OVERWRITE;
        }
        return ret;
    }

    int _normal_set(size_t key, const key_type &k, const value_type &val, 
                    int flag, node_pointer * __bucket) {
        node_pointer lst = __bucket[key];
        node_pointer front = 0;
        int ret = -1;
        while (lst) {
            if (_base._equl(_base._getkey(_base._sp_alloc.getp(lst)->val), k)) {
                if (flag) {
                    ret = HASH_OVERWRITE;
                    break;
                }
                return HASH_EXIST;
            }
            front = lst;
            lst = _base._sp_alloc.getp(lst)->next;
        }
        node_pointer node = _base._sp_alloc.allocate(1);
        if (0 == node) {
            return -1;
        }

        bsl::bsl_construct(&(_base._sp_alloc.getp(node)->val), val);
        
        //insertion bug fixed.
        if (ret == HASH_OVERWRITE) {
            _base._sp_alloc.getp(node)->next = _base._sp_alloc.getp(lst)->next;  
            if (front) {
                _base._sp_alloc.getp(front)->next = node;
            }else{
                __bucket[key] = node;
            }
            destruct_node(lst, key);
            return ret;
        } else {
            _base._sp_alloc.getp(node)->next = __bucket[key];
            __bucket[key] = node;
        }

        return HASH_INSERT_SEC;
    }

    int _erase(size_t key, const key_type &k) {
        int ret = 0;
        switch (_flag) {
            case PHASH_START_CHECKPOINT:
                ret = _start_cp_erase(key, k);
                break;
            case PHASH_END_CHECKPOINT:
                ret = _end_cp_erase(key, k);
                break;
            default:
                ret = _normal_erase(key, k, _base._bucket);
                break;
        }
        if (ret == HASH_EXIST) {
            -- _tmpsize;
        }
        return ret;
    }

    int _start_cp_erase(size_t key, const key_type &k) {
        value_type *ptr = _base.find(key, k);
        if (ptr != NULL) {
            if (_hashset->set(ptr) == HASH_INSERT_SEC) {
                //存在，标志删除
                return HASH_EXIST;
            }
        }
        return _normal_erase(key, k, _tmplist);
    }

    int _end_cp_erase(size_t key, const key_type &k) {
        int ret = _normal_erase(key, k, _tmplist);
        if (ret == HASH_NOEXIST) {
            value_type *ptr = _base.find(key, k);
            if (ptr == NULL || _hashset->get(ptr) == HASH_EXIST) {
                return HASH_NOEXIST;
            }
            return _normal_erase(key, k, _base._bucket);
        }
        return ret;
    }

    int _normal_erase(size_t key, 
                      const key_type &k, 
                      node_pointer *__bucket, 
                      deleteVec *delvec = NULL
                      ) {
        node_pointer lst = __bucket[key];
        node_pointer front = 0;
        while (lst) {
            if (_base._equl(_base._getkey(_base._sp_alloc.getp(lst)->val), k)) {
                if (front == 0) {
                    __bucket[key] = _base._sp_alloc.getp(lst)->next;
                } else {
                    _base._sp_alloc.getp(front)->next = _base._sp_alloc.getp(lst)->next;
                }
                if (delvec == NULL) {
                    destruct_node(lst, key);
                } else {
                    deleteNode delnode;
                    delnode.lst = lst;
                    delnode.key = key;
                    try {
                        delvec->push_back(delnode);
                    } catch (...) {
                        destruct_node(lst, key);
                    }
                }
                return HASH_EXIST;
            }
            front = lst;
            lst = _base._sp_alloc.getp(lst)->next;
        }
        return HASH_NOEXIST;
    }

  private:
    void _reset(){
        _rwlocks = NULL;
        _rwlocksize = 0;
        _tmpsize = 0;
        _tmplist = NULL;
        _flag = PHASH_NORMAL;
        _hashset = NULL;
        _mlockcreat = false;
    }

    int _make_checkpoint() {
        if (_flag != PHASH_NORMAL) {
            return -1;
        }
        if (_tmplist == NULL) {
            //_tmplist = _nodealloc.allocate(_base._bitems);
            _tmplist = _base._np_alloc.allocate(_base._bitems);
            if (_tmplist == NULL) {
                return -1;
            }
            memset(_tmplist, 0, sizeof(node_pointer ) * _base._bitems);
        }
        if (_hashset == NULL) {
            _hashset = _setalloc.allocate(1);
            bsl::bsl_construct(_hashset);
            if (_hashset == NULL) {
                return -1;
            }
            if (_hashset->create(__bsl_next_prime(1<<15)) != 0) {
                bsl::bsl_destruct(_hashset);
                _setalloc.deallocate(_hashset, 1);
                _hashset = NULL;
                return -1;
            }
        }
        _flag = PHASH_START_CHECKPOINT;
        _base._size = _tmpsize;
        __BSL_DEBUG("make checkpoint end %lu %lu", (long)_base._size, (long)_tmpsize);
        return 0;
    }

    int _end_checkpoint() {
        //__BSL_DEBUG("start to end checkpoint");
        if (_flag != PHASH_START_CHECKPOINT) {
            //__BSL_ERROR("dumpstatus error flag[%d]", _flag);
            return -1;
        }

        AUTO_TIMER("end checkpoint");
        {
            mutexlock_monitor l(&_uplock);
            _flag = PHASH_END_CHECKPOINT;
        }
        {
            {
                __BSL_DEBUG("lock _hashset->lock");
                mutexlock_monitor t(&_hashset->_lock);
                __BSL_DEBUG("in lock _hashset->lock items %lu", (long)_hashset->_ht._bitems);
                for (size_t i=0; i<_hashset->_ht._bitems; ++i) {
                    mutexlock_monitor l(&_uplock);
                    BSL_RWLOCK_T &__lock = _hashset->get_rwlock(i);
                    BSL_RWLOCK_WRLOCK(&__lock);
                    deleteVec delvec;
                    typename _Set::hash_type::node_pointer  nd = 0;
                    while (_hashset->_ht._bucket[i]) {
                        nd = _hashset->_ht._bucket[i];
                        {
                            const key_type & key = 
                                    _base._getkey(*(_hashset->_ht._sp_alloc.getp(nd)->val));
                            _normal_erase(_base._hashfun(key) % _base._bitems, 
                                          key, 
                                          _base._bucket, 
                                          &delvec
                                          );
                        }
                        //考虑get的时候 检索可能出的问题
                        _hashset->_ht._bucket[i] = 
                                _hashset->_ht._sp_alloc.getp(_hashset->_ht._bucket[i])->next;
                        bsl::bsl_destruct(&(_hashset->_ht._sp_alloc.getp(nd)->val));
                        _hashset->_ht._sp_alloc.deallocate(nd, 1);
                    }
                    BSL_RWLOCK_UNLOCK(&__lock);
                    for (typename deleteVec::iterator delIter = delvec.begin();
                         delIter != delvec.end(); ++delIter) {
                        destruct_node(delIter->lst, delIter->key);
                    }
                }
                __BSL_DEBUG("walk _ht._bitems");
            }
            __BSL_DEBUG("base bitems %lu", (long)_base._bitems);
            for (size_t i=0; i<_base._bitems; ++i) {
                //__BSL_DEBUG("start in uplock");
                mutexlock_monitor l(&_uplock);
                //__BSL_DEBUG("inside uplock");
                if (0) {
                    node_pointer nd = _tmplist[i];
                    while (nd) {
                        const key_type & key = _base._getkey(_base._sp_alloc.getp(nd)->val);
                        if (_normal_erase(i, key, _base._bucket) == HASH_EXIST) {
                            -- _tmpsize;
                            __BSL_DEBUG("has same id %d", (int)i);
                        }
                        nd = _base._sp_alloc.getp(nd)->next;
                    }
                }
                //__BSL_DEBUG("inside uplock");
                if (_tmplist[i] != 0) {
                    node_pointer lst = _base._bucket[i];
                    node_pointer front = 0;
                    while (lst) {
                        front = lst;
                        lst = _base._sp_alloc.getp(lst)->next;
                    }
                    if (front) {
                        _base._sp_alloc.getp(front)->next = _tmplist[i];
                    } else {
                        _base._bucket[i] = _tmplist[i];
                    }
                    _tmplist[i] = 0;

                }
                
            }
            __BSL_DEBUG("walk _tmplist");
        }

        {
            _Set * __hashset = _hashset;
            node_pointer *__tmplist = _tmplist;
            mutexlock_monitor t(&_uplock);
            //将_flag放在前面,可以使_get的时候少走一步
            _flag = PHASH_NORMAL;
            {
            mutexlock_monitor __lock(&_hslock);
            _hashset = NULL;
            _tmplist = NULL;
            }
            __hashset->destroy();
            bsl::bsl_destruct(__hashset);
            _setalloc.deallocate(__hashset, 1);
            //同理,不必判断_tmplist是否为NULL
            _base._np_alloc.deallocate(__tmplist, _base._bitems);
        }
        
        
        return 0;
    }
};

}
#endif  //__BSL_PHASHTABLE_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
