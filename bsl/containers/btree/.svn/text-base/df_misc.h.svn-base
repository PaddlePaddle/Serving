////====================================================================
//
// df_2d_ary.h - Pyramid / DFS / df-lib
//
// Copyright (C) 2008 Baidu.com, Inc.
//
// Created on 2008-02-01 by YANG Zhenkun (yangzhenkun@baidu.com)
//
// -------------------------------------------------------------------
//
// Description
//
//    declaration and implementation of some common functions and classes
//
// -------------------------------------------------------------------
//
// Change Log
//
//    updated on 2008-02-01 by YANG Zhenkun (yangzhenkun@baidu.com)
//
////====================================================================

#ifndef __DF_MISC_INCLUDE_H_
#define __DF_MISC_INCLUDE_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/param.h>
//#include <sys/types.h>
#include <pthread.h>
#include <errno.h>
#include <new>
//zhangyan04@baidu.com
#include "bsl_kv_btree_xmemcpy.h"

#include "df_def.h"
#include "df_atomic.h"
//#include "ul_def.h"
//#include "df_log.h"
//#include <ul_log.h>
//#include "df_common.h"

//using namespace std;

//Linux kernel 2.6的一个bug使得调用pthread_mutex_timedlock()可能出core
//因此把互斥的mutex锁换成了读写锁
//阳振坤(yangzhenkun@baidu.com) 20090311
#ifndef DF_NOT_USE_MUTEX_TIMEDLOCK
#define DF_NOT_USE_MUTEX_TIMEDLOCK
#endif

//Obtain the length of an array
//#define len_of_ary(ary) ((uint32_t)(sizeof(ary)/sizeof((ary)[0])))
#define df_len_of_ary(ary) (sizeof(ary)/sizeof((ary)[0]))


inline timeval df_microseconds_to_tv(const int64_t microseconds) {
    struct timeval tp;

    tp.tv_sec = microseconds / 1000000;
    tp.tv_usec = microseconds % 1000000;

    return tp;
}

inline timespec df_microseconds_to_ts(const int64_t microseconds) {
    struct timespec ts;

    ts.tv_sec = microseconds / 1000000;
    ts.tv_nsec = (microseconds % 1000000) * 1000;

    return ts;
}

inline int64_t df_tv_to_microseconds(const timeval & tp) {
    return (((int64_t) tp.tv_sec) * 1000000 + (int64_t) tp.tv_usec);
}

inline int64_t df_ts_to_microseconds(const timespec & ts) {
    return (((int64_t) ts.tv_sec) * 1000000 + (int64_t) ((ts.tv_nsec + 500) / 1000));
}

inline int64_t df_get_cur_microseconds_time(void) {
    struct timeval tp;

    gettimeofday(&tp, NULL);

    return df_tv_to_microseconds(tp);
}

inline void df_microseconds_sleep(const int64_t microseconds) {
    struct timespec ts = df_microseconds_to_ts(microseconds);

    nanosleep(&ts, NULL);
}

inline void df_make_timespec_with_interval(struct timespec& tsp, int64_t useconds) {
    struct timeval now;

    gettimeofday(&now, NULL);
    useconds += now.tv_usec;
    tsp.tv_sec = now.tv_sec;
    while (useconds >= 1000000) {
        tsp.tv_sec++;
        useconds -= 1000000;
    }
    tsp.tv_nsec = useconds * 1000;
}

# if defined __x86_64__

class dfs_init_t {
protected:
    enum cconst_private {
        STATE_INIT_SUCCEED      = 0     ,
        STATE_NOT_INIT_YET              ,
        STATE_INIT_FAIL                 ,
        STATE_DESTRUCTED                ,
    };
public:
    dfs_init_t() {
        _err = 0;
        set_not_init_yet();
    };
    ~dfs_init_t() {
        set_destructed();
    };
private:
    int _state;
    int _err;
public:
    bool is_not_init_yet(void) const {
        return (STATE_NOT_INIT_YET == _state);
    };
    bool is_init_fail(void) const {
        return (STATE_INIT_FAIL == _state);
    };
    bool is_init_succeed(void) const {
        return (STATE_INIT_SUCCEED == _state);
    };
    void set_not_init_yet(void) const {
        *((int *)&_state) = STATE_NOT_INIT_YET;
    };
    void set_init_result(const int err) const {
        if (is_not_init_yet()) {
            *((int *)&_err) = err;
            *((int *)&_state) = (0 == err) ? STATE_INIT_SUCCEED : STATE_INIT_FAIL;
        }
    };
    int get_init_result(void) const {
        return _err;
    };
    void set_destructed(void) const {
        *((int *)&_state) = STATE_DESTRUCTED;
    };
};

// a lightweight lock
class dfs_spinlock_t {
public:
    enum cconst_private {
        MICROSECONDS_PER_SLEEP  = 20    ,
    };
private:
    dfs_init_t _init_state;
    pthread_spinlock_t _spinlock;
public:
    inline dfs_spinlock_t() {
        init();
    }
    inline ~ dfs_spinlock_t() {
        if (_init_state.is_init_succeed()) {
            pthread_spin_destroy(&_spinlock);
            _init_state.set_destructed();
        }
    }
public:
    int init(void) const {
        int err = 0;

        if (_init_state.is_not_init_yet()) {
            err = pthread_spin_init((pthread_spinlock_t *)&_spinlock, PTHREAD_PROCESS_PRIVATE);
            _init_state.set_init_result(err);
        } else {
            err = _init_state.get_init_result();
        }

        return err;
    };
protected:
    //return: 0 for success, other values for error
    //      possible errors: EBUSY, EINVAL, EAGAIN, ENOMEM, EDEADLK, EPERM
    inline int _lock(const int64_t wait_microsecond = 0) const {
        int64_t wait_us = (wait_microsecond <= 0) ? (MAX_INT64/2) : wait_microsecond;
        int64_t time_us = 0;
        int err = 0;

        while (wait_us > 0) {
            time_us = df_get_cur_microseconds_time();
            if ((err = pthread_spin_trylock((pthread_spinlock_t *)&_spinlock)) == 0) {
                break;
            } else if (EBUSY == err) {
                df_microseconds_sleep(MICROSECONDS_PER_SLEEP);
                wait_us -= df_get_cur_microseconds_time() - time_us;
            } else {
                break;
            }
        }

        return err;
    }
    inline int _unlock(void) const {
        int err = 0;

        err = pthread_spin_unlock((pthread_spinlock_t *) &_spinlock);

        return err;
    }
public:
    //  功能：尝试在max_try_time微秒内锁住整棵B树，如果不能，返回失败。
    //        一旦B树被锁住，在解锁前无法进行任何修改操作，也不能verify其正确性(读操作可正常进行)。
    //        今后视需要实现锁住后在max_hold_time后自动解锁
    //  返回：返回0表示成功，返回非0(错误代码)表示失败
    int lock(const int64_t max_try_time) const {
        return _lock(max_try_time);
    };
    //  功能：解锁
    //  返回：返回0表示成功，返回非0(错误代码)表示失败
    int unlock(void) const {
        return _unlock();
    };
};


class dfs_wrlock_t {
private:
    enum cconst_private {
        MICROSECONDS_PER_R_SLEEP  = 20    ,
        MICROSECONDS_PER_W_SLEEP  = 20    ,
    };
    static const uint64_t W_FLAG = ((uint64_t)0x1)<<62;
    static const uint64_t R_MASK = W_FLAG-1;
private:
    dfs_init_t _init_state;
    pthread_rwlock_t _rwlock;
public:
    inline dfs_wrlock_t() {
        init();
    }
    inline ~ dfs_wrlock_t() {
        if (_init_state.is_init_succeed()) {
            pthread_rwlock_destroy(&_rwlock);
            _init_state.set_destructed();
        }
    }
public:
    int init(void) const {
        int err = 0;

        if (_init_state.is_not_init_yet()) {
            err = pthread_rwlock_init((pthread_rwlock_t *)&_rwlock, NULL);
            _init_state.set_init_result(err);
        } else {
            err = _init_state.get_init_result();
        }

        return err;
    };
protected:
    //return: 0 for success, other values for error
    //      possible errors: EBUSY, EINVAL, EAGAIN, ENOMEM, EDEADLK, EPERM
    inline int _r_lock(const int64_t wait_microsecond = 0) const {
        int64_t wait_us = (wait_microsecond <= 0) ? (MAX_INT64/2) : wait_microsecond;
        struct timespec ts = df_microseconds_to_ts(wait_us);
        int err = 0;

        err = pthread_rwlock_timedrdlock((pthread_rwlock_t *)&_rwlock, &ts);

        return err;
    };
    //return: 0 for success, other values for error
    //      possible errors: EBUSY, EINVAL, EAGAIN, ENOMEM, EDEADLK, EPERM
    inline int _w_lock(const int64_t wait_microsecond = 0) const {
        int64_t wait_us = (wait_microsecond <= 0) ? (MAX_INT64/2) : wait_microsecond;
        struct timespec ts = df_microseconds_to_ts(wait_us);
        int err = 0;

        err = pthread_rwlock_timedwrlock((pthread_rwlock_t *)&_rwlock, &ts);

        return err;
    };
    inline int _unlock(void) const {
        int err = 0;

        err = pthread_rwlock_unlock((pthread_rwlock_t *)&_rwlock);

        return err;
    };
public:
    //  功能：尝试在max_try_time微秒内锁住整棵B树，如果不能，返回失败。
    //        一旦B树被锁住，在解锁前无法进行任何修改操作，也不能verify其正确性(读操作可正常进行)。
    //        今后视需要实现锁住后在max_hold_time后自动解锁
    //  返回：返回0表示成功，返回非0(错误代码)表示失败
    int r_lock(const int64_t max_try_time) const {
        return _r_lock(max_try_time);
    };
    //  功能：解锁
    //  返回：返回0表示成功，返回非0(错误代码)表示失败
    int r_unlock(void) const {
        return _unlock();
    };
    int w_lock(const int64_t max_try_time) const {
        return _w_lock(max_try_time);
    };
    //  功能：解锁
    //  返回：返回0表示成功，返回非0(错误代码)表示失败
    int w_unlock(void) const {
        return _unlock();
    };
};


class dfs_mutex_lock_t; //锁本身
class dfs_mutex_hold_t; //当前上锁后的锁标识


class dfs_mutex_hold_t {
    friend class dfs_mutex_lock_t;
public:
    dfs_mutex_hold_t() {
        _init(NULL);
    };
    ~dfs_mutex_hold_t();
private:
    uint64_t _lock_uniq_cnt;
    const dfs_mutex_lock_t * _plock;
private:
    void _init(dfs_mutex_lock_t * plock) {
        _set_lock_uniq_cnt(0);
        _set_lock_ptr(plock);
    };
    void _set_lock_uniq_cnt(const uint64_t lock_uniq_cnt) {
        _lock_uniq_cnt = lock_uniq_cnt;
        return;
    };
    uint64_t _get_lock_uniq_cnt(void) const {
        return _lock_uniq_cnt;
    };
    void _set_lock_ptr(const dfs_mutex_lock_t * plock) {
        _plock = plock;
    };
    const dfs_mutex_lock_t * _get_lock_ptr(void) const {
        return _plock;
    };
};



class dfs_mutex_lock_t {
    friend class dfs_mutex_hold_t;
private:
protected:
    enum cconst_private {
        LOCK_UNIQ_CNT_ADD   = 2 ,   //_lock_uniq_cnt每次增加的值
        ON_MUTATE_MARK      = 1 ,   //当前正在修改的标记
    };
private:
    dfs_init_t _init_state;
    uint64_t _lock_uniq_cnt;
#ifdef DF_NOT_USE_MUTEX_TIMEDLOCK
    pthread_rwlock_t _rwlock;
#else
    pthread_mutex_t _lock;
#endif
public:
    inline dfs_mutex_lock_t() {
        //int log_level = DF_UL_LOG_NONE;
        int err = 0;

#ifdef DF_NOT_USE_MUTEX_TIMEDLOCK
        if ((err = pthread_rwlock_init((pthread_rwlock_t *)&_rwlock, NULL)) != 0)
#else
        if ((err = pthread_mutex_init(&_lock, NULL)) != 0)
#endif
        {
            //log_level = DF_UL_LOG_FATAL;
            //DF_WRITE_LOG_US(log_level, "pthread_mutex_init(,NULL) returns 0x%x", err);
        }

        _lock_uniq_cnt = LOCK_UNIQ_CNT_ADD;

        _init_state.set_init_result(err);
    }
    inline ~ dfs_mutex_lock_t() {
#ifdef DF_NOT_USE_MUTEX_TIMEDLOCK
        pthread_rwlock_destroy(&_rwlock);
#else
        pthread_mutex_destroy(&_lock);
#endif
        _init_state.set_destructed();
    }
public:
    int init(void) const {
        return 0;
    };
protected:
    inline int _acquire_lock(
        dfs_mutex_hold_t & lock_hold,
        const int64_t wait_microsecond = 0) const;
    inline int _release_lock(dfs_mutex_hold_t & lock_hold) const;
    //验证是否是当前的hold
    inline bool _verify_hold(const dfs_mutex_hold_t & lock_hold) const;
    //验证是当前的hold通过且设置ON_MUTATE_MARK，否则返回错误
    inline int _acquire_hold(const dfs_mutex_hold_t & lock_hold) const;
    //验证是当前的hold通过且清除ON_MUTATE_MARK，否则返回错误
    inline int _release_hold(const dfs_mutex_hold_t & lock_hold) const;
};


inline dfs_mutex_hold_t::~dfs_mutex_hold_t() {
    if (NULL != _plock) {
        _plock->_release_lock(*this);
        _plock = NULL;
    }
};


inline int dfs_mutex_lock_t::_acquire_lock(
    dfs_mutex_hold_t & lock_hold,
    const int64_t wait_microsecond) const {
    //int log_level = DF_UL_LOG_NONE;
    int err = 0;

    if (wait_microsecond <= 0 || wait_microsecond >=  MAX_INT64/4) {
#ifdef DF_NOT_USE_MUTEX_TIMEDLOCK
        if ((err = pthread_rwlock_wrlock((pthread_rwlock_t *)&_rwlock)) != 0)
#else
        if ((err = pthread_mutex_lock((pthread_mutex_t *)&_lock)) != 0)
#endif
        {
            //log_level = DF_UL_LOG_FATAL;
            //DF_WRITE_LOG_US(log_level, "pthread_mutex_lock() returns 0x%x", err);
        }
        //当前不应该正在修改中
        else if ((_lock_uniq_cnt & ON_MUTATE_MARK) != 0) {
            err = EINTR;
        } else {
            lock_hold._set_lock_uniq_cnt(_lock_uniq_cnt);
            lock_hold._set_lock_ptr(this);
        }
    } else {
        const int64_t wait_us = wait_microsecond+df_get_cur_microseconds_time();
        struct timespec ts = df_microseconds_to_ts(wait_us);

#ifdef DF_NOT_USE_MUTEX_TIMEDLOCK
        if ((err = pthread_rwlock_timedwrlock((pthread_rwlock_t *)&_rwlock, &ts)) != 0)
#else
        if ((err = pthread_mutex_timedlock((pthread_mutex_t *)&_lock, &ts)) != 0)
#endif
        {
            //log_level = DF_UL_LOG_FATAL;
            //DF_WRITE_LOG_US(log_level, "pthread_mutex_timedlock() returns 0x%x", err);
        }
        //当前不应该正在修改中
        else if ((_lock_uniq_cnt & ON_MUTATE_MARK) != 0) {
            err = EINTR;
        } else {
            lock_hold._set_lock_uniq_cnt(_lock_uniq_cnt);
            lock_hold._set_lock_ptr(this);
        }
    }

    return err;
};

inline int dfs_mutex_lock_t::_release_lock(dfs_mutex_hold_t & lock_hold) const {
    //int log_level = DF_UL_LOG_NONE;
    int err = 0;

    if (df_atomic_compare_exchange(
                (volatile uint64_t *)&_lock_uniq_cnt,
                lock_hold._get_lock_uniq_cnt()+LOCK_UNIQ_CNT_ADD,
                lock_hold._get_lock_uniq_cnt()) == lock_hold._get_lock_uniq_cnt()) {
#ifdef DF_NOT_USE_MUTEX_TIMEDLOCK
        if ((err = pthread_rwlock_unlock((pthread_rwlock_t *)&_rwlock)) != 0)
#else
        if ((err = pthread_mutex_unlock((pthread_mutex_t *)&_lock)) != 0)
#endif
        {
            //log_level = DF_UL_LOG_FATAL;
            //DF_WRITE_LOG_US(log_level, "pthread_mutex_unlock() returns 0x%x", err);
        } else {
            lock_hold._set_lock_uniq_cnt(0);
            lock_hold._set_lock_ptr(NULL);
        }
    } else {
        err = EPERM;
        //log_level = DF_UL_LOG_WARNING;
        //DF_WRITE_LOG_US(log_level, "lock_hold unmatch");
    }

    return err;
};

//验证是否是当前的hold
inline bool dfs_mutex_lock_t::_verify_hold(const dfs_mutex_hold_t & lock_hold) const {
    return (lock_hold._get_lock_uniq_cnt() == _lock_uniq_cnt);
};


//验证是当前的hold通过且设置ON_MUTATE_MARK，否则返回错误
inline int dfs_mutex_lock_t::_acquire_hold(const dfs_mutex_hold_t & lock_hold) const {
    int err = 0;
    //如果正在修改中...
    if ((lock_hold._get_lock_uniq_cnt() & ON_MUTATE_MARK) != 0) {
        err = EINVAL;
    }
    //原子修改
    //告诉已经mutate了..:)..
    else if (df_atomic_compare_exchange(
                 (uint64_t *)&_lock_uniq_cnt,
                 _lock_uniq_cnt | ON_MUTATE_MARK,
                 lock_hold._get_lock_uniq_cnt())
             != lock_hold._get_lock_uniq_cnt()) {
        err = EPERM;
    }

    return err;
};

//验证是当前的hold通过且清除ON_MUTATE_MARK，否则返回错误
inline int dfs_mutex_lock_t::_release_hold(const dfs_mutex_hold_t & lock_hold) const {
    int err = 0;

    if ((lock_hold._get_lock_uniq_cnt() & ON_MUTATE_MARK) != 0) {
        err = EINVAL;
    } else if (df_atomic_compare_exchange(
                   (uint64_t *)&_lock_uniq_cnt,
                   _lock_uniq_cnt & ~((uint64_t)ON_MUTATE_MARK),
                   lock_hold._get_lock_uniq_cnt() | ON_MUTATE_MARK)
               != (lock_hold._get_lock_uniq_cnt() | ON_MUTATE_MARK)) {
        err = EPERM;
    }

    return err;
};


//Function: if *pv is NOT equal to cv, then inc; otherwise, do nothing.
//return: current value of *pv
inline uint64_t df_atomic_inc_if_not_equal(volatile uint64_t * pv, const uint64_t cv) {
    uint64_t pre_v = *pv;   //保存v的原始值到cv
    uint64_t org_v = pre_v;

    while (cv != org_v) { // 如果原始值org_v为cv，什么都不做
        pre_v = df_atomic_compare_exchange(pv, org_v+1, org_v);
        if (pre_v == org_v) {
            //如果*pv原始值和*pv相等，则没有其他线程对*p进行操作，成功完成+1操作
            ++org_v;
            break;
        } else {
            //否则别的线程已经对*pv做了操作，把org_v更新为当前被别的线程修改后的值后重试
            org_v = pre_v;
        }
    }

    return org_v;
};
inline uint32_t df_atomic_inc_if_not_equal(volatile uint32_t * pv, const uint32_t cv) {
    uint32_t pre_v = *pv;   //保存v的原始值到cv
    uint32_t org_v = pre_v;

    while (cv != org_v) { // 如果原始值org_v为cv，什么都不做
        pre_v = df_atomic_compare_exchange(pv, org_v+1, org_v);
        if (pre_v == org_v) {
            //如果*pv原始值和*pv相等，则没有其他线程对*p进行操作，成功完成+1操作
            ++org_v;
            break;
        } else {
            //否则别的线程已经对*pv做了操作，把org_v更新为当前被别的线程修改后的值后重试
            org_v = pre_v;
        }
    }

    return org_v;
};
inline uint16_t df_atomic_inc_if_not_equal(volatile uint16_t * pv, const uint16_t cv) {
    uint16_t pre_v = *pv;   //保存v的原始值到cv
    uint16_t org_v = pre_v;

    while (cv != org_v) { // 如果原始值org_v为cv，什么都不做
        pre_v = df_atomic_compare_exchange(pv, org_v+1, org_v);
        if (pre_v == org_v) {
            //如果*pv原始值和*pv相等，则没有其他线程对*p进行操作，成功完成+1操作
            ++org_v;
            break;
        } else {
            //否则别的线程已经对*pv做了操作，把org_v更新为当前被别的线程修改后的值后重试
            org_v = pre_v;
        }
    }

    return org_v;
};
inline uint8_t df_atomic_inc_if_not_equal(volatile uint8_t * pv, const uint8_t cv) {
    uint8_t pre_v = *pv;   //保存v的原始值到cv
    uint8_t org_v = pre_v;

    while (cv != org_v) { // 如果原始值org_v为cv，什么都不做
        pre_v = df_atomic_compare_exchange(pv, org_v+1, org_v);
        if (pre_v == org_v) {
            //如果*pv原始值和*pv相等，则没有其他线程对*p进行操作，成功完成+1操作
            ++org_v;
            break;
        } else {
            //否则别的线程已经对*pv做了操作，把org_v更新为当前被别的线程修改后的值后重试
            org_v = pre_v;
        }
    }

    return org_v;
};


//Function: if nv > *pv, then set *pv to nv atomically; otherwise, do nothing.
//return: current value of *pv
inline uint64_t df_atomic_exchange_to_larger(volatile uint64_t * pv, const uint64_t nv) {
    uint64_t pre_v = *pv;   //保存v的原始值到cv
    uint64_t org_v = pre_v;

    while (nv > org_v) { // 如果原始值org_v >= nv，什么都不做
        pre_v = df_atomic_compare_exchange(pv, nv, org_v);
        if (pre_v == org_v) {
            //如果*pv原始值和*pv相等，则没有其他线程对*p进行操作，成功完成+1操作
            org_v = nv;
            break;
        } else {
            //否则别的线程已经对*pv做了操作，把org_v更新为当前被别的线程修改后的值后重试
            org_v = pre_v;
        }
    }

    return org_v;
};




//简单的缓冲区自动申请释放类
class dfs_buf_t {
protected:
    enum cconst {
        MIN_BUF_SIZE = 1024,
    };
public:
    dfs_buf_t(uint64_t buf_size = 0) {
        _buf = NULL;
        _buf_size = 0;
        if (buf_size > 0) {
            _buf_size = (buf_size + MIN_BUF_SIZE - 1) / MIN_BUF_SIZE * MIN_BUF_SIZE;
            _buf = new(std::nothrow) char[_buf_size];
        }
    }

    virtual ~ dfs_buf_t() {
        if (NULL != _buf) {
            delete[] _buf;
            _buf = NULL;
        }
        _buf_size = 0;
    }

    void * get_buf(void) const {
        return (void *) _buf;
    }

    uint64_t get_buf_size(void) const {
        return _buf_size;
    }

    //return: reallocated bufer.如果没有足够内存，则返回NULL，现有缓冲区不改变。
    void *realloc(const uint64_t new_buf_size) {
        const uint64_t adjusted_new_buf_size = ((new_buf_size + MIN_BUF_SIZE - 1) / MIN_BUF_SIZE)
                                               * MIN_BUF_SIZE;
        char *ret_buf = NULL;

        if (adjusted_new_buf_size != _buf_size) {
            if (adjusted_new_buf_size > 0) {
                ret_buf = new(std::nothrow) char[adjusted_new_buf_size];

                if (NULL != ret_buf) {
                    //memcpy(ret_buf, _buf, MIN(_buf_size, adjusted_new_buf_size));
                    ZY::xmemcpy(ret_buf, _buf, MIN(_buf_size, adjusted_new_buf_size));
                    delete[] _buf;
                    _buf = ret_buf;
                    _buf_size = adjusted_new_buf_size;
                }
            } else {
                delete[] _buf;
                _buf = NULL;
                _buf_size = 0;
                ret_buf = NULL;
            }
        } else {
            ret_buf = _buf;
        }
        return (void *) ret_buf;
    }

    void dealloc(void) {
        delete[] _buf;
        _buf = NULL;
        _buf_size = 0;
    }

protected:
    char *_buf;
    uint64_t _buf_size;
};

#endif

#endif //__DF_MISC_INCLUDE_H_

