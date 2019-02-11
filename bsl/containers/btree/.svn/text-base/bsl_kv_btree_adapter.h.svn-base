/* -*- c++ -*-
   copy[write] by dirlt(dirtysalt1987@gmail.com) */
#ifndef _BSL_KV_BTREE_ADAPTER_H_
#define _BSL_KV_BTREE_ADAPTER_H_

//如果需要依赖头文件,那么在bsl_kv_btree.h里面添加..
//xz_btree_adapter..
//内部的操作都叫做unify_xxxx..
template<typename DU,
         unsigned int ROW_SIZE>
class _bsl_kv_btree_xz_adapter {
public:
    typedef typename DU::key_type K;

private:
    //这个真正持有一个对象...
    dfs_xz_btree_t<DU,K,ROW_SIZE>_btree;

public:
    //这个遍历接口是为了build btree准备的..
    bool unify_get_smallest(DU &du,
                            const dfs_rc_indexer_t *prc_indexer=NULL)const {
        return _btree.get_smallest(du,prc_indexer);
    }
    bool unify_get_largest(DU &du,
                           const dfs_rc_indexer_t *prc_indexer=NULL)const {
        return _btree.get_largest(du,prc_indexer);
    }
    bool unify_get_smaller(const DU &pdu,DU &du,
                           const dfs_rc_indexer_t *prc_indexer=NULL)const {
        //使用key而不用data unit来查询.
        return _btree.get_smaller(pdu,du,NULL,prc_indexer);
    }
    bool unify_get_larger(const DU &pdu,DU &du,
                          const dfs_rc_indexer_t *prc_indexer=NULL)const {
        //使用key而不用data unit来查询.
        return _btree.get_larger(pdu,du,NULL,prc_indexer);
    }

    //这个是为了search btree设计的..
    //search btree可以只掌握指针..
    //而build btree却需要掌握值.
    bool unify_get_smallest(DU **du,
                            const dfs_rc_indexer_t *prc_indexer)const {
        return _btree.zy_get_smallest_p(du,prc_indexer);
    }
    bool unify_get_largest(DU **du,
                           const dfs_rc_indexer_t *prc_indexer)const {
        return _btree.zy_get_largest_p(du,prc_indexer);
    }
    //可以使用桶保存信息来加快查询...
    bool unify_get_smaller(const DU &p,DU **du,
                           dfs_btree_drill_t *pdrill_info,
                           const dfs_rc_indexer_t *prc_indexer)const {
        //使用data unit来进行查询..
        return _btree.zy_get_smaller_p(p,du,pdrill_info,prc_indexer);
    }
    bool unify_get_larger(const DU &p,DU **du,
                          dfs_btree_drill_t *pdrill_info,
                          const dfs_rc_indexer_t *prc_indexer)const {
        //使用data unit来进行查询..
        return _btree.zy_get_larger_p(p,du,pdrill_info,prc_indexer);
    }

    //其他接口...
    int unify_insert(const K &/*k*/,
                     const DU &du) {
        return _btree.zy_insert(du);
    }
    int unify_insert_update(const K &/*k*/,
                            const DU &du) {
        return _btree.zy_insert_update(du);
    }
    bool unify_search(const K &k,DU *du,
                      const dfs_rc_indexer_t *prc_indexer=NULL)const {
        return _btree.zy_search(k,*du,prc_indexer);
    }
    int unify_get_total_num(uint64_t &key,uint64_t &leaf,uint64_t &mid,
                            const dfs_rc_indexer_t *prc_indexer=NULL)const {
        //这里需要走两套逻辑,但是前端接口都是一样的...
        if(prc_indexer==NULL) {
            return _btree.get_total_num(key,leaf,mid);
        } else {
            return _btree.zy_get_total_num(prc_indexer,key,leaf,mid);
        }
    }
    bool unify_del(const K &k) {
        return _btree.del(k,NULL);
    }
    int unify_set_checkpoint_state(bool is_cancel) {
        return _btree.set_checkpoint_state(is_cancel,0);
    }
    int unify_snapshot(dfs_rc_indexer_t &rc_indexer) {
        return _btree.snapshot(rc_indexer);
    }
    int unify_store_checkpoint(const dfs_write_proc_t write_proc,
                               void * file_info,
                               const dfs_rc_indexer_t *prc_indexer=NULL) {
        return _btree.store_checkpointing(write_proc,file_info,NULL,prc_indexer);
    }
    int unify_load_checkpoint(const dfs_read_proc_t read_proc,
                              void * file_info) {
        return _btree.load_checkpointing(read_proc,file_info,NULL);
    }
    int unify_clear() {
        return _btree.clear();
    }
    int unify_set_batch_mutate_mode(bool batch_mode) {
        return _btree.set_batch_mutate_mode(batch_mode);
    }
    bool unify_get_batch_mutate_mode()const {
        return _btree.get_batch_mutate_mode();
    }
};

#endif
