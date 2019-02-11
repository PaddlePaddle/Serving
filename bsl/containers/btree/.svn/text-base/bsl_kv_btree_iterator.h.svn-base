/* -*- c++ -*-
   copy[write] by dirlt(dirtysalt1987@gmail.com) */
#ifndef _BSL_KV_BTREE_ITERATOR_H_
#define _BSL_KV_BTREE_ITERATOR_H_

//如果需要添加头文件
//统一在bsl_kv_btree.h里面

//============================================================
//============================================================
//============================================================
//这个是一个基类..
//提供遍历接口,这样iterator就只需要写一次了..:).
//纯粹就是一个接口...
//但是build btree和search btree所需要的接口是不同的.
template<typename DU>
class _bsl_kv_btree_build_iterator_base {
public:
    virtual bool get_smallest(DU &du)const=0;
    virtual bool get_largest(DU &du)const=0;
    virtual bool get_smaller(const DU &pdu,DU &du)const=0;
    virtual bool get_larger(const DU &pdu,DU &du)const=0;
    virtual ~_bsl_kv_btree_build_iterator_base() {
    }
};

//看了baonh的deque代码之后,发现这个地方实际上只需要定义一次
//iterator就ok了...如果需要reverse_iterator的话,直接使用std::reverse_iterator包装
//但是这里我不确定使用std::reverse_iterator自己的类里面需要包含哪些类..
//所以还是自己写了一个reverse_iterator.
template<typename DU,
         typename KRef,
         typename KPtr,
         typename VRef,
         typename VPtr,
         unsigned int ROW_SIZE,
         typename BTreeAdapter>
class _bsl_kv_btree_build_iterator {
public:
    //相关的迭代器..
    typedef typename DU::key_type key_type;
    typedef typename DU::value_type value_type;
    typedef key_type * kpointer;
    typedef key_type & kreference;
    typedef value_type * pointer;
    typedef value_type & reference;
    typedef const key_type * ckpointer;
    typedef const key_type & ckreference;
    typedef const value_type * cpointer;
    typedef const value_type & creference;
    typedef _bsl_kv_btree_build_iterator<DU,
            kreference,kpointer,
            reference,pointer,
            ROW_SIZE,BTreeAdapter> iterator;
    typedef _bsl_kv_btree_build_iterator<DU,
            ckreference,ckpointer,
            creference,cpointer,
            ROW_SIZE,BTreeAdapter> const_iterator;
    //自己是self_type..
    typedef _bsl_kv_btree_build_iterator self_type;
    //操纵的btree.
    typedef _bsl_kv_btree_build_iterator_base<DU> kv_btree_type;

private:
    friend class _bsl_kv_btree_build<DU,ROW_SIZE,BTreeAdapter>;
    const kv_btree_type *_btree;
    //data.
    DU _du;
    //做一个_end的标记位...
    bool _end;

public:
    _bsl_kv_btree_build_iterator()
        :_btree(NULL),_du(),_end(true) {
        //nothing.
    }
    _bsl_kv_btree_build_iterator(const kv_btree_type *btree,
                                 const DU &du,
                                 bool end=false):_btree(btree),_du(du),_end(end) {
    }
    _bsl_kv_btree_build_iterator(const iterator & iter):
        _btree(iter._btree),_du(iter._du),_end(iter._end) {
    }
    _bsl_kv_btree_build_iterator(const const_iterator & iter):
        _btree(iter._btree),_du(iter._du),_end(iter._end) {
    }
    //如果用户在这里已修改iterator的话
    //是一个安全的操作,不会反映到btree
    //但是却影响后面的查询.
    VRef operator *() {
        return _du._value;
    }
    VPtr operator->() {
        return &(_du._value);
    }
    //相对于其他容器的iterator有这么一个接口就是能够获得key.
    KPtr ptr_key() {
        return &(_du._key);
    }
    KRef ref_key() {
        return (_du._key);
    }
    bool operator == (const self_type & iter) const {
        //如果都true的话,那么==
        if(_end && iter._end) {
            return true;
        } else if(_end || iter._end) {
            return false;
        }
        //必须是指向同一个btree并且du内容相同..
        return (_btree == iter._btree) &&
               (_du == iter._du);
    }
    bool operator != (const self_type & iter)const {
        return  !(*this == iter);
    }
    self_type & operator++() {
        DU _tmp;
        bool next=_btree->get_larger(_du,_tmp);
        if(!next) {
            //设置true..
            //不修改任何内容...
            _end=true;
        } else {
            _du=_tmp;
        }
        return *this;
    }
    self_type operator++(int) {
        self_type iter=*this;
        ++*this;
        return iter;
    }
    self_type & operator--() {
        DU _tmp;
        bool prev=_btree->get_smaller(_du,_tmp);
        if(prev) {
            _du=_tmp;
        }
        return *this;
    }
    self_type operator--(int) {
        self_type iter=*this;
        --*this;
        return iter;
    }
};

template<typename DU,
         typename KRef,
         typename KPtr,
         typename VRef,
         typename VPtr,
         unsigned int ROW_SIZE,
         typename BTreeAdapter>
class _bsl_kv_btree_build_reverse_iterator {
public:
    //相关的迭代器..
    typedef typename DU::key_type key_type;
    typedef typename DU::value_type value_type;
    typedef key_type * kpointer;
    typedef key_type & kreference;
    typedef value_type * pointer;
    typedef value_type & reference;
    typedef const key_type * ckpointer;
    typedef const key_type & ckreference;
    typedef const value_type * cpointer;
    typedef const value_type & creference;
    typedef _bsl_kv_btree_build_reverse_iterator<DU,
            kreference,kpointer,
            reference,pointer,
            ROW_SIZE,BTreeAdapter> reverse_iterator;
    typedef _bsl_kv_btree_build_reverse_iterator<DU,
            ckreference,ckpointer,
            creference,cpointer,
            ROW_SIZE,BTreeAdapter> const_reverse_iterator;
    //自己是self_type
    typedef _bsl_kv_btree_build_reverse_iterator self_type;
    //自己操纵的btree
    typedef _bsl_kv_btree_build_iterator_base<DU> kv_btree_type;

private:
    friend class _bsl_kv_btree_build<DU,ROW_SIZE,BTreeAdapter>;
    const kv_btree_type *_btree;
    //data.
    DU _du;
    //做一个_end的标记位...
    bool _end;

public:
    _bsl_kv_btree_build_reverse_iterator()
        :_btree(NULL),_du(),_end(true) {
        //nothing.
    }
    _bsl_kv_btree_build_reverse_iterator(const kv_btree_type *btree,
                                         const DU &du,
                                         bool end=false):
        _btree(btree),_du(du),_end(end) {
    }
    _bsl_kv_btree_build_reverse_iterator(const reverse_iterator & iter):
        _btree(iter._btree),_du(iter._du),_end(iter._end) {
    }
    _bsl_kv_btree_build_reverse_iterator(const const_reverse_iterator & iter):
        _btree(iter._btree),_du(iter._du),_end(iter._end) {
    }
    //如果用户在这里已修改iterator的话
    //是一个安全的操作,不会反映到btree
    //但是却影响后面的查询
    VRef operator *() {
        return _du._value;
    }
    VPtr operator->() {
        return &(_du._value);
    }
    //相对于其他容器的iterator有这么一个接口就是能够获得key.
    KPtr ptr_key() {
        return &(_du._key);
    }
    KRef ref_key() {
        return (_du._key);
    }
    bool operator == (const self_type & iter) const {
        //如果都true的话,那么==
        if(_end && iter._end) {
            return true;
        } else if(_end || iter._end) {
            return false;
        }
        //必须是指向同一个btree并且du内容相同..
        return (_btree == iter._btree) &&
               (_du == iter._du);
    }
    bool operator != (const self_type & iter)const {
        return  !(*this == iter);
    }
    self_type & operator++() {
        DU _tmp;
        bool next=_btree->get_smaller(_du,_tmp);
        if(!next) {
            //设置true..
            //不修改任何内容...
            _end=true;
        } else {
            _du=_tmp;
        }
        return *this;
    }
    self_type operator++(int) {
        self_type iter=*this;
        ++*this;
        return iter;
    }
    self_type & operator--() {
        DU _tmp;
        bool prev=_btree->get_larger(_du,_tmp);
        if(prev) {
            _du=_tmp;
        }
        return *this;
    }
    self_type operator--(int) {
        self_type iter=*this;
        --*this;
        return iter;
    }
};

//下面这个部分开始就是search btree的iterator..
//============================================================
//============================================================
//============================================================
template<typename DU>
class _bsl_kv_btree_search_iterator_base {
public:
    virtual bool get_smallest(DU **du)const=0;
    virtual bool get_largest(DU **du)const=0;
    virtual bool get_smaller(const DU &pdu,DU **du,
                             dfs_btree_drill_t *pdrill_info)const=0;
    virtual bool get_larger(const DU &pdu,DU **du,
                            dfs_btree_drill_t *pdrill_info)const=0;
    virtual ~_bsl_kv_btree_search_iterator_base() {
    }
};
template<typename DU,
         typename KRef,
         typename KPtr,
         typename VRef,
         typename VPtr,
         unsigned int ROW_SIZE,
         typename BTreeAdapter>
class _bsl_kv_btree_search_iterator {
public:
    //相关的迭代器..
    typedef typename DU::key_type key_type;
    typedef typename DU::value_type value_type;
    typedef key_type * kpointer;
    typedef key_type & kreference;
    typedef value_type * pointer;
    typedef value_type & reference;
    typedef const key_type * ckpointer;
    typedef const key_type & ckreference;
    typedef const value_type * cpointer;
    typedef const value_type & creference;
    typedef _bsl_kv_btree_search_iterator<DU,
            kreference,kpointer,
            reference,pointer,
            ROW_SIZE,BTreeAdapter>
            iterator;
    typedef _bsl_kv_btree_search_iterator<DU,
            ckreference,ckpointer,
            creference,cpointer,
            ROW_SIZE,BTreeAdapter>
            const_iterator;
    //自己是self_type
    typedef _bsl_kv_btree_search_iterator self_type;
    //操纵的btree.
    typedef _bsl_kv_btree_search_iterator_base<DU> kv_btree_type;

private:
    friend class _bsl_kv_btree_search<DU,ROW_SIZE,BTreeAdapter>;
    const kv_btree_type *_btree;
    //data.
    //虽然这里du可以是const*的,但是我们为了提供
    //iterator和const iterator的话,那么只好在这里只使用非const *
    //但是用户必须牢记不要修改du引出的值.
    DU *_du;
    //drill info
    dfs_btree_drill_t _drill;
    //做一个_end的标记位...
    bool _end;
public:
    _bsl_kv_btree_search_iterator()
        :_btree(NULL),_du(NULL),_drill(),_end(true) {
        //nothing.
    }
    _bsl_kv_btree_search_iterator(const kv_btree_type *btree,
                                  DU *du,
                                  bool end=false)
        :_btree(btree),_du(du),_drill(),_end(end) {
    }
    _bsl_kv_btree_search_iterator(const iterator & iter):
        _btree(iter._btree),_du(iter._du),_end(iter._end) {
        _drill=iter._drill;
    }
    _bsl_kv_btree_search_iterator(const const_iterator & iter):
        _btree(iter._btree),_du(iter._du),_end(iter._end) {
        _drill=iter._drill;
    }
    //用户不能够修改这个值..否则会影响到btree内容...:(..
    reference operator *() {
        return _du->_value;
    }
    pointer operator->() {
        return &(_du->_value);
    }
    //相对于其他容器的iterator有这么一个接口就是能够获得key.
    KPtr ptr_key() {
        return &(_du->_key);
    }
    KRef ref_key() {
        return _du->_key;
    }
    bool operator == (const self_type & iter) const {
        //如果都true的话,那么==
        if(_end && iter._end) {
            return true;
        } else if(_end || iter._end) {
            return false;
        }
        //必须du相同并且指向同一个btree.
        return (_btree == iter._btree) &&
               (*_du == *iter._du);
    }
    bool operator != (const self_type & iter)const {
        return  !(*this == iter);
    }
    self_type & operator++() {
        DU *_tmp;
        bool next=_btree->get_larger(*_du,&_tmp,&_drill);
        if(!next) {
            //设置true..
            //不修改任何内容...
            _end=true;
        } else {
            _du=_tmp;
        }
        return *this;
    }
    self_type operator++(int) {
        self_type iter=*this;
        ++*this;
        return iter;
    }
    self_type & operator--() {
        DU *_tmp;
        bool prev=_btree->get_smaller(*_du,&_tmp,&_drill);
        if(prev) {
            _du=_tmp;
        }
        return *this;
    }
    self_type operator--(int) {
        self_type iter=*this;
        --*this;
        return iter;
    }
};

template<typename DU,
         typename KRef,
         typename KPtr,
         typename VRef,
         typename VPtr,
         unsigned int ROW_SIZE,
         typename BTreeAdapter>
class _bsl_kv_btree_search_reverse_iterator {
public:
    //相关的迭代器..
    typedef typename DU::key_type key_type;
    typedef typename DU::value_type value_type;
    typedef key_type * kpointer;
    typedef key_type & kreference;
    typedef value_type * pointer;
    typedef value_type & reference;
    typedef const key_type * ckpointer;
    typedef const key_type & ckreference;
    typedef const value_type * cpointer;
    typedef const value_type & creference;
    typedef _bsl_kv_btree_search_reverse_iterator<DU,
            kreference,kpointer,
            reference,pointer,
            ROW_SIZE,BTreeAdapter>
            reverse_iterator;
    typedef _bsl_kv_btree_search_reverse_iterator<DU,
            ckreference,ckpointer,
            creference,cpointer,
            ROW_SIZE,BTreeAdapter>
            const_reverse_iterator;
    //自己是self_type
    typedef _bsl_kv_btree_search_reverse_iterator self_type;
    //操纵的btree.
    typedef _bsl_kv_btree_search_iterator_base<DU> kv_btree_type;

private:
    friend class _bsl_kv_btree_search<DU,ROW_SIZE,BTreeAdapter>;
    const kv_btree_type *_btree;
    //data.
    DU *_du;
    //drill info
    dfs_btree_drill_t _drill;
    //做一个_end的标记位...
    bool _end;

public:
    _bsl_kv_btree_search_reverse_iterator()
        :_btree(NULL),_du(NULL),_drill(),_end(true) {
        //nothing.
    }
    _bsl_kv_btree_search_reverse_iterator(const kv_btree_type *btree,
                                          DU *du,
                                          bool end=false):
        _btree(btree),_du(du),_drill(),_end(end) {
    }
    _bsl_kv_btree_search_reverse_iterator(const reverse_iterator & iter):
        _btree(iter._btree),_du(iter._du),_end(iter._end) {
        _drill=iter._drill;
    }
    _bsl_kv_btree_search_reverse_iterator(const const_reverse_iterator & iter):
        _btree(iter._btree),_du(iter._du),_end(iter._end) {
        _drill=iter._drill;
    }
    //用户不允许修改这个值,不然会影响到btree的内容...
    reference operator *() {
        return _du->_value;
    }
    pointer operator->() {
        return &(_du->_value);
    }
    //相对于其他容器的iterator有这么一个接口就是能够获得key.
    KPtr ptr_key() {
        return &(_du->_key);
    }
    KRef ref_key() {
        return _du->_key;
    }
    bool operator == (const self_type & iter) const {
        //如果都true的话,那么==
        if(_end && iter._end) {
            return true;
        } else if(_end || iter._end) {
            return false;
        }
        //必须du相同并且指向同一个btree.
        return (_btree == iter._btree) &&
               (*_du == *iter._du);
    }
    bool operator != (const self_type & iter)const {
        return  !(*this == iter);
    }
    self_type & operator++() {
        DU *_tmp;
        bool next=_btree->get_smaller(*_du,&_tmp,&_drill);
        if(!next) {
            //设置true..
            //不修改任何内容...
            _end=true;
        } else {
            _du=_tmp;
        }
        return *this;
    }
    self_type operator++(int) {
        self_type iter=*this;
        ++*this;
        return iter;
    }
    self_type & operator--() {
        DU *_tmp;
        bool prev=_btree->get_larger(*_du,&_tmp,_drill);
        if(prev) {
            _du=_tmp;
        }
        return *this;
    }
    self_type operator--(int) {
        self_type iter=*this;
        --*this;
        return iter;
    }
};

#endif
