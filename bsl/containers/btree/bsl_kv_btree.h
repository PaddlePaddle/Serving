/* -*- c++ -*-
   copy[write] by dirlt(dirtysalt1987@gmail.com) */
#ifndef _BSL_KV_BTREE_H_
#define _BSL_KV_BTREE_H_

#if defined __x86_64

#define BSL_BTREE_WORKS
//ֻ����64λ������ܹ�����...

#include <functional>
#include "bsl/utils/bsl_construct.h"
//Ϊbsl::string�������ض���archive�ػ��汾.
#include "bsl/string.h"
#include "df_xz_btree.h"
#include <string>

namespace bsl {
//BTREE���󷵻�ֵ
enum {
    BTREE_UNKNOWN_ERROR=-1,
    BTREE_OK=0,
    //����ط����ó�Ϊ������ԭ��
    //��Ϊ�˺�bsl::hashmap��ֵ��ͬ.
    BTREE_EXIST=0xffff,
    BTREE_NOEXIST,
    BTREE_OVERWRITE,
    BTREE_INSERT_SUC,
};

//forward declaration.
template<typename DU,
         unsigned int ROW_SIZE,
         typename BTreeAdapter>
class _bsl_kv_btree_build;

template<typename DU,
         unsigned int ROW_SIZE,
         typename BTreeAdapter>
class _bsl_kv_btree_search;

//archive traits for bsl kv btree.
//���л��Ľӿ�..��ν������͵����л��ͷ����л�..
#include "bsl_kv_btree_archive_traits.h"

//kv btree data unit.
//���ݵ�Ԫ
#include "bsl_kv_btree_data_unit.h"

//kv btree adapter
//���ڽ����ṩ��xz adapter
#include "bsl_kv_btree_adapter.h"

//kv btree iterator
#include "bsl_kv_btree_iterator.h"

template<typename DU,
         unsigned int ROW_SIZE,
         typename BTreeAdapter>
class _bsl_kv_btree_build:public _bsl_kv_btree_build_iterator_base<DU> {
public:
    typedef typename DU::key_type key_type;
    typedef typename DU::value_type value_type;
    typedef _bsl_kv_btree_build_iterator<DU,
            key_type &,key_type *,
            value_type &,value_type *,
            ROW_SIZE,BTreeAdapter> iterator;
    typedef _bsl_kv_btree_build_iterator<DU,
            const key_type &,const key_type*,
            const value_type &,const value_type *,
            ROW_SIZE,BTreeAdapter> const_iterator;
    typedef _bsl_kv_btree_build_reverse_iterator<DU,
            key_type &,key_type *,
            value_type &,value_type *,
            ROW_SIZE,BTreeAdapter> reverse_iterator;
    typedef _bsl_kv_btree_build_reverse_iterator<DU,
            const key_type &,const key_type*,
            const value_type &,const value_type *,
            ROW_SIZE,BTreeAdapter> const_reverse_iterator;
    typedef BTreeAdapter btree_type;
    typedef _bsl_kv_btree_search<DU,ROW_SIZE,BTreeAdapter> kv_btree_search_type;

private:
    //���е����ݶ���
    btree_type _btree;
    //end of iterator...
    iterator _end_iter;
    const_iterator _cst_end_iter;
    reverse_iterator _rend_iter;
    const_reverse_iterator _cst_rend_iter;

public:
    _bsl_kv_btree_build() {
        _end_iter._end=true;
        _cst_end_iter._end=true;
        _rend_iter._end=true;
        _cst_rend_iter._end=true;
    }
    bool get_smallest(DU &du)const {
        return _btree.unify_get_smallest(du);
    }
    bool get_largest(DU &du)const {
        return _btree.unify_get_largest(du);
    }
    bool get_smaller(const DU &pdu,DU &du)const {
        return _btree.unify_get_smaller(pdu,du);
    }
    bool get_larger(const DU &pdu,DU &du)const {
        return _btree.unify_get_larger(pdu,du);
    }
    //insert (key,value) into btree
    //flag==0 ���ֵ���ڵĻ�ֱ�ӷ���,�����ֱ�Ӳ���
    //flag!=0 ���ֵ���ڵĻ���ô�ͻ�����滻,�����ֱ�Ӳ���
    //����ֵ
    //-1��ʾ�ڲ�����....
    //BTREE_OVERWRITE ��ʾ���Ǿɽڵ�ɹ�[flag!=0]
    //BTREE_INSERT_SUC ��ʾ�����½ڵ�ɹ�
    //BTREE_EXIST ��ʾ�ڵ����[flag=0]
    int set(const key_type &k,const value_type &v,int flag=0) {
        DU du(k,v);
        if(flag==0) {
            int ret=_btree.unify_insert(k,du);
            if(ret==0) {
                return BTREE_EXIST;
            } else if(ret==1) {
                return BTREE_INSERT_SUC;
            }
            return BTREE_UNKNOWN_ERROR;
        }
        int ret=_btree.unify_insert_update(k,du);
        if(ret==0) {
            return BTREE_OVERWRITE;
        } else if(ret==1) {
            return BTREE_INSERT_SUC;
        }
        return BTREE_UNKNOWN_ERROR;
    }
    //BTREE_EXIST �����
    //BTREE_NOEXIST �����..
    int get(const key_type &k,value_type *val=0) {
        if(_btree.unify_get_batch_mutate_mode()) {
            return BTREE_UNKNOWN_ERROR;
        }
        DU du;
        bool ok=_btree.unify_search(k,&du);
        if(ok) {
            if(val) {
                //��һ��copy..
                bsl_construct(val,du._value);
            }
            return BTREE_EXIST;
        }
        return BTREE_NOEXIST;
    }
    int get(const key_type &k,value_type *val=0)const {
        if(_btree.unify_get_batch_mutate_mode()) {
            return BTREE_UNKNOWN_ERROR;
        }
        DU du;
        bool ok=_btree.unify_search(k,&du);
        if(ok) {
            if(val) {
                //��һ��copy...
                bsl_construct(val,du._value);
            }
            return BTREE_EXIST;
        }
        return BTREE_NOEXIST;
    }
    //֧�ִ�ĳ��KV��ʼ����iterator.
    //���в�����begin���Ǵ�ĳ���㿪ʼ���й���iterator�ķ���.
    iterator begin() {
        DU du;
        bool ok=get_smallest(du);
        return iterator(this,du,!ok);
    }
    iterator begin(const key_type &k,const value_type &v) {
        DU du(k,v);
        return iterator(this,du,false);
    }
    const_iterator begin()const {
        DU du;
        bool ok=get_smallest(du);
        return const_iterator(this,du,!ok);
    }
    const_iterator begin(const key_type &k,const value_type &v) const {
        DU du(k,v);
        return const_iterator(this,du,false);
    }
    reverse_iterator rbegin() {
        DU du;
        bool ok=get_largest(du);
        return reverse_iterator(this,du,!ok);
    }
    reverse_iterator rbegin(const key_type &k,const value_type &v) {
        DU du(k,v);
        return reverse_iterator(this,du,false);
    }
    const_reverse_iterator rbegin()const {
        DU du;
        bool ok=get_largest(du);
        return const_reverse_iterator(this,du,!ok);
    }
    const_reverse_iterator rbegin(const key_type &k,const value_type &v)const {
        DU du(k,v);
        return const_reverse_iterator(this,du,false);
    }
    const iterator &end() {
        return _end_iter;
    }
    const const_iterator &end()const {
        return _cst_end_iter;
    }
    const reverse_iterator &rend() {
        return _rend_iter;
    }
    const const_reverse_iterator &rend()const {
        return _cst_rend_iter;
    }
    //�����ж��ٸ�key.
    size_t size() const {
        uint64_t key=0,leaf=0,mid=0;
        _btree.unify_get_total_num(key,leaf,mid);
        return key;
    }
    //ɾ��ĳ��Ԫ��...
    //BTREE_EXIST ���key����
    //BTREE_NOEXIST ���key������.
    int erase(const key_type &k) {
        bool ok=_btree.unify_del(k);
        if(ok) {
            return BTREE_EXIST;
        }
        return BTREE_NOEXIST;
    }
    //���,ʼ�շ���0.
    int clear() {
        int ret=_btree.unify_clear();
        if(ret==0) {
            return BTREE_OK;
        }
        return BTREE_UNKNOWN_ERROR;
    }
    //����checkpoint
    //��ʵû��̫����;,��Ϊ���Ѿ�����snapshot.
    //0�ɹ�,-1ʧ��...
    int make_checkpoint() {
        int ret=_btree.unify_set_checkpoint_state(false);
        if(ret==0) {
            return BTREE_OK;
        }
        return BTREE_UNKNOWN_ERROR;
    }
    int end_checkpoint() {
        int ret=_btree.unify_set_checkpoint_state(true);
        if(ret==0) {
            return BTREE_OK;
        }
        return BTREE_UNKNOWN_ERROR;
    }
    int snapshot(kv_btree_search_type & search) {
        int ret=_btree.unify_snapshot(search._rc_indexer);
        search._btree=&_btree;
        if(ret==0) {
            return BTREE_OK;
        }
        return BTREE_UNKNOWN_ERROR;
    }
    int store(int fd,long long offset) {
        dfs_file_rw_info_t info;
        info.fd=fd;
        info.orig_offset=offset;
        info.reserved=0;//nothing.
        int ret=_btree.unify_store_checkpoint(dfs_write_proc,&info);
        if(ret==0) {
            return BTREE_OK;
        }
        return BTREE_UNKNOWN_ERROR;
    }
    int load(int fd,long long offset) {
        dfs_file_rw_info_t info;
        info.fd=fd;
        info.orig_offset=offset;
        int ret=_btree.unify_load_checkpoint(dfs_read_proc,&info);
        if(ret==0) {
            return BTREE_OK;
        }
        return BTREE_UNKNOWN_ERROR;
    }
    int make_batch_mutate() {
        int ret=_btree.unify_set_batch_mutate_mode(true);
        if(ret==0) {
            return BTREE_OK;
        }
        return BTREE_UNKNOWN_ERROR;
    }
    int end_batch_mutate() {
        int ret=_btree.unify_set_batch_mutate_mode(false);
        if(ret==0) {
            return BTREE_OK;
        }
        return BTREE_UNKNOWN_ERROR;
    }
    //��������������ӿڽ�����Ϊ��bslʹ��ϰ�����.
    //�����϶��ɹ�.
    int create() {
        return 0;
    }
    //���ٵĻ�ֻ�����.�����κδ���.
    int destroy() {
        int ret=_btree.unify_clear();
        if(ret==0) {
            return BTREE_OK;
        }
        return BTREE_UNKNOWN_ERROR;
    }
};

template<typename DU,
         unsigned int ROW_SIZE,
         typename BTreeAdapter>
class _bsl_kv_btree_search:public _bsl_kv_btree_search_iterator_base<DU> {
public:
    typedef typename DU::key_type key_type;
    typedef typename DU::value_type value_type;
    typedef _bsl_kv_btree_search_iterator<DU,
            key_type &,key_type *,
            value_type &,value_type *,
            ROW_SIZE,BTreeAdapter> iterator;
    typedef _bsl_kv_btree_search_iterator<DU,
            const key_type &,const key_type *,
            const value_type &,const value_type *,
            ROW_SIZE,BTreeAdapter> const_iterator;
    typedef _bsl_kv_btree_search_reverse_iterator<DU,
            key_type &,key_type *,
            value_type &,value_type *,
            ROW_SIZE,BTreeAdapter> reverse_iterator;
    typedef _bsl_kv_btree_search_reverse_iterator<DU,
            const key_type &,const key_type *,
            const value_type &,const value_type *,
            ROW_SIZE,BTreeAdapter> const_reverse_iterator;
    typedef _bsl_kv_btree_build<DU,ROW_SIZE,BTreeAdapter> kv_btree_build_type;
    typedef BTreeAdapter btree_type;

private:
    friend class _bsl_kv_btree_build<DU,ROW_SIZE,BTreeAdapter>;
    //���е����ݶ���...
    btree_type *_btree;
    //rc indexer.
    dfs_rc_indexer_t _rc_indexer;
    //end iterator..
    iterator _end_iter;
    const_iterator _cst_end_iter;
    reverse_iterator _rend_iter;
    const_reverse_iterator _cst_rend_iter;

public:
    _bsl_kv_btree_search() {
        _end_iter._end=true;
        _cst_end_iter._end=true;
        _rend_iter._end=true;
        _cst_rend_iter._end=true;
    }
    bool get_smallest(DU **du)const {
        return _btree->unify_get_smallest(du,&_rc_indexer);
    }
    bool get_largest(DU **du)const {
        return _btree->unify_get_largest(du,&_rc_indexer);
    }
    bool get_smaller(const DU &pdu,DU **du,dfs_btree_drill_t *pdrill_info)const {
        return _btree->unify_get_smaller(pdu,du,pdrill_info,&_rc_indexer);
    }
    bool get_larger(const DU &pdu,DU **du,dfs_btree_drill_t *pdrill_info)const {
        return _btree->unify_get_larger(pdu,du,pdrill_info,&_rc_indexer);
    }
    void let_it_go() {
        //�ͷŻ��սڵ�..
        _rc_indexer.init();
    }
    ~_bsl_kv_btree_search() {
        let_it_go();
    }
    //BTREE_EXIST �����
    //BTREE_NOEXIST �����..
    int get(const key_type &k,value_type *val=0) {
        DU du;
        bool ok=_btree->unify_search(k,&du,&_rc_indexer);
        if(ok) {
            if(val) {
                bsl_construct(val,du._value);
            }
            return BTREE_EXIST;
        }
        return BTREE_NOEXIST;
    }
    int get(const key_type &k,value_type *val=0)const {
        DU du;
        bool ok=_btree->unify_search(k,&du,&_rc_indexer);
        if(ok) {
            if(val) {
                bsl_construct(val,du._value);
            }
            return BTREE_EXIST;
        }
        return BTREE_NOEXIST;
    }
    iterator begin() {
        DU *du;
        bool ok=get_smallest(&du);
        return iterator(this,du,!ok);
    }
    const_iterator begin()const {
        DU *du;
        bool ok=get_smallest(&du);
        return const_iterator(this,du,!ok);
    }
    reverse_iterator rbegin() {
        DU *du;
        bool ok=get_largest(&du);
        return reverse_iterator(this,du,!ok);
    }
    const_reverse_iterator rbegin()const {
        DU *du;
        bool ok=get_largest(&du);
        return const_reverse_iterator(this,du,!ok);
    }
    const iterator &end() {
        return _end_iter;
    }
    const const_iterator &end()const {
        return _cst_end_iter;
    }
    const reverse_iterator &rend() {
        return _rend_iter;
    }
    const const_reverse_iterator &rend()const {
        return _cst_rend_iter;
    }
    //�����ж��ٸ�key.
    size_t size() const {
        uint64_t key=0,leaf=0,mid=0;
        _btree->unify_get_total_num(key,leaf,mid,&_rc_indexer);
        return key;
    }
    int store(int fd,unsigned long long offset) {
        dfs_file_rw_info_t info;
        info.fd=fd;
        info.orig_offset=offset;
        int ret=_btree->unify_store_checkpoint(dfs_write_proc,&info,&_rc_indexer);
        if(ret==0) {
            return BTREE_OK;
        }
        return BTREE_UNKNOWN_ERROR;
    }
};

template<typename K,typename V,
         unsigned int ROW_SIZE=dfs_bt_const_t::DEF_ROW_SIZE,
         typename KARCHIVE=_btree_archive_traits<K>,
         typename VARCHIVE=_btree_archive_traits<V>,
         typename LT=std::less<K>,
         typename EQ=std::equal_to<K> >
class bsl_kv_btree_search:
    public _bsl_kv_btree_search< //data unit..
    _bsl_kv_btree_du<K,V,KARCHIVE,VARCHIVE,LT,EQ>,
    ROW_SIZE,
    //adapter...
        _bsl_kv_btree_xz_adapter <_bsl_kv_btree_du<K,V,KARCHIVE,VARCHIVE,LT,EQ>,ROW_SIZE > > {
};
template<typename K,typename V,
         unsigned int ROW_SIZE=dfs_bt_const_t::DEF_ROW_SIZE,
         typename KARCHIVE=_btree_archive_traits<K>,
         typename VARCHIVE=_btree_archive_traits<V>,
         typename LT=std::less<K>,
         typename EQ=std::equal_to<K> >
class bsl_kv_btree_build:
    public _bsl_kv_btree_build<//data unit..
    _bsl_kv_btree_du<K,V,KARCHIVE,VARCHIVE,LT,EQ>,
    ROW_SIZE ,
    //adapter..
        _bsl_kv_btree_xz_adapter <_bsl_kv_btree_du<K,V,KARCHIVE,VARCHIVE,LT,EQ>,ROW_SIZE > > {
};
}
#else
#undef BSL_BTREE_WORKS
#endif

#endif
