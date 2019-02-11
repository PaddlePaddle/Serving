/* -*- c++ -*-
   copy[write] by dirlt(dirtysalt1987@gmail.com) */
#ifndef _BSL_KV_BTREE_ITERATOR_H_
#define _BSL_KV_BTREE_ITERATOR_H_

//�����Ҫ���ͷ�ļ�
//ͳһ��bsl_kv_btree.h����

//============================================================
//============================================================
//============================================================
//�����һ������..
//�ṩ�����ӿ�,����iterator��ֻ��Ҫдһ����..:).
//�������һ���ӿ�...
//����build btree��search btree����Ҫ�Ľӿ��ǲ�ͬ��.
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

//����baonh��deque����֮��,��������ط�ʵ����ֻ��Ҫ����һ��
//iterator��ok��...�����Ҫreverse_iterator�Ļ�,ֱ��ʹ��std::reverse_iterator��װ
//���������Ҳ�ȷ��ʹ��std::reverse_iterator�Լ�����������Ҫ������Щ��..
//���Ի����Լ�д��һ��reverse_iterator.
template<typename DU,
         typename KRef,
         typename KPtr,
         typename VRef,
         typename VPtr,
         unsigned int ROW_SIZE,
         typename BTreeAdapter>
class _bsl_kv_btree_build_iterator {
public:
    //��صĵ�����..
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
    //�Լ���self_type..
    typedef _bsl_kv_btree_build_iterator self_type;
    //���ݵ�btree.
    typedef _bsl_kv_btree_build_iterator_base<DU> kv_btree_type;

private:
    friend class _bsl_kv_btree_build<DU,ROW_SIZE,BTreeAdapter>;
    const kv_btree_type *_btree;
    //data.
    DU _du;
    //��һ��_end�ı��λ...
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
    //����û����������޸�iterator�Ļ�
    //��һ����ȫ�Ĳ���,���ᷴӳ��btree
    //����ȴӰ�����Ĳ�ѯ.
    VRef operator *() {
        return _du._value;
    }
    VPtr operator->() {
        return &(_du._value);
    }
    //���������������iterator����ôһ���ӿھ����ܹ����key.
    KPtr ptr_key() {
        return &(_du._key);
    }
    KRef ref_key() {
        return (_du._key);
    }
    bool operator == (const self_type & iter) const {
        //�����true�Ļ�,��ô==
        if(_end && iter._end) {
            return true;
        } else if(_end || iter._end) {
            return false;
        }
        //������ָ��ͬһ��btree����du������ͬ..
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
            //����true..
            //���޸��κ�����...
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
    //��صĵ�����..
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
    //�Լ���self_type
    typedef _bsl_kv_btree_build_reverse_iterator self_type;
    //�Լ����ݵ�btree
    typedef _bsl_kv_btree_build_iterator_base<DU> kv_btree_type;

private:
    friend class _bsl_kv_btree_build<DU,ROW_SIZE,BTreeAdapter>;
    const kv_btree_type *_btree;
    //data.
    DU _du;
    //��һ��_end�ı��λ...
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
    //����û����������޸�iterator�Ļ�
    //��һ����ȫ�Ĳ���,���ᷴӳ��btree
    //����ȴӰ�����Ĳ�ѯ
    VRef operator *() {
        return _du._value;
    }
    VPtr operator->() {
        return &(_du._value);
    }
    //���������������iterator����ôһ���ӿھ����ܹ����key.
    KPtr ptr_key() {
        return &(_du._key);
    }
    KRef ref_key() {
        return (_du._key);
    }
    bool operator == (const self_type & iter) const {
        //�����true�Ļ�,��ô==
        if(_end && iter._end) {
            return true;
        } else if(_end || iter._end) {
            return false;
        }
        //������ָ��ͬһ��btree����du������ͬ..
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
            //����true..
            //���޸��κ�����...
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

//����������ֿ�ʼ����search btree��iterator..
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
    //��صĵ�����..
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
    //�Լ���self_type
    typedef _bsl_kv_btree_search_iterator self_type;
    //���ݵ�btree.
    typedef _bsl_kv_btree_search_iterator_base<DU> kv_btree_type;

private:
    friend class _bsl_kv_btree_search<DU,ROW_SIZE,BTreeAdapter>;
    const kv_btree_type *_btree;
    //data.
    //��Ȼ����du������const*��,��������Ϊ���ṩ
    //iterator��const iterator�Ļ�,��ôֻ��������ֻʹ�÷�const *
    //�����û������μǲ�Ҫ�޸�du������ֵ.
    DU *_du;
    //drill info
    dfs_btree_drill_t _drill;
    //��һ��_end�ı��λ...
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
    //�û����ܹ��޸����ֵ..�����Ӱ�쵽btree����...:(..
    reference operator *() {
        return _du->_value;
    }
    pointer operator->() {
        return &(_du->_value);
    }
    //���������������iterator����ôһ���ӿھ����ܹ����key.
    KPtr ptr_key() {
        return &(_du->_key);
    }
    KRef ref_key() {
        return _du->_key;
    }
    bool operator == (const self_type & iter) const {
        //�����true�Ļ�,��ô==
        if(_end && iter._end) {
            return true;
        } else if(_end || iter._end) {
            return false;
        }
        //����du��ͬ����ָ��ͬһ��btree.
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
            //����true..
            //���޸��κ�����...
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
    //��صĵ�����..
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
    //�Լ���self_type
    typedef _bsl_kv_btree_search_reverse_iterator self_type;
    //���ݵ�btree.
    typedef _bsl_kv_btree_search_iterator_base<DU> kv_btree_type;

private:
    friend class _bsl_kv_btree_search<DU,ROW_SIZE,BTreeAdapter>;
    const kv_btree_type *_btree;
    //data.
    DU *_du;
    //drill info
    dfs_btree_drill_t _drill;
    //��һ��_end�ı��λ...
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
    //�û��������޸����ֵ,��Ȼ��Ӱ�쵽btree������...
    reference operator *() {
        return _du->_value;
    }
    pointer operator->() {
        return &(_du->_value);
    }
    //���������������iterator����ôһ���ӿھ����ܹ����key.
    KPtr ptr_key() {
        return &(_du->_key);
    }
    KRef ref_key() {
        return _du->_key;
    }
    bool operator == (const self_type & iter) const {
        //�����true�Ļ�,��ô==
        if(_end && iter._end) {
            return true;
        } else if(_end || iter._end) {
            return false;
        }
        //����du��ͬ����ָ��ͬһ��btree.
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
            //����true..
            //���޸��κ�����...
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
