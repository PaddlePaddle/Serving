////===================================================================
//
// df_xz_btree.h Pyramid / DFS / df-lib
//
// Copyright (C) 2008 Baidu.com, Inc.
//
// Created on 2008-01-05 by YANG Zhenkun (yangzhenkun@baidu.com)
//
// -------------------------------------------------------------------
//
// Description
//
//    declaration and implementation of dfs_xz_btree_t (btree) template
//
// -------------------------------------------------------------------
//
// Change Log
//
//    updated on 2008-07-28 by YANG Zhenkun (yangzhenkun@baidu.com)
//
////====================================================================


#ifndef __DFS_XZ_BTREE_INCLUDE_H_
#define __DFS_XZ_BTREE_INCLUDE_H_

//调试信息打印输出开关
//#ifndef DF_BT_PRINT_DEBUG_INFO
//#define DF_BT_PRINT_DEBUG_INFO
//#endif

//#include "ul_def.h"
//#include "df_common.h"
#include "df_btree.h"

//#define TEST_RWLOCK_TIME

class dfs_xz_btree_fhead_ext_t {
public:
    uint64_t total_leaf_node_num;
    uint64_t total_mid_node_num;
    uint64_t total_key_num;
    uint64_t reserve31[25];
};

class dfs_xz_btree_fhead_t : public dfs_btree_fhead_base_t, public dfs_xz_btree_fhead_ext_t {
};



//
//模板类：dfs_xz_btree_t<typename T, uint32_t ROW_SIZE, uint32_t FANOUT>
//dfs_xz_btree_t是一棵以K为key的B树
//模板参数：
//  T： 基本数据单元
//  K： 关键字类型
//  ROW_SIZE：基本数据单元二维数组的ROW_SIZE，最好是的方幂。
//  FANOUT：B树的扇出尺寸，每个节点最多(FANOUT-1)个key和FANOUT个子节点。
//
//
//


template<typename T,typename K, uint32_t ROW_SIZE>
class _dfs_xz_btree_t :
    virtual public dfs_bt_const_t,
    //virtual private dfs_bt2d_ary_t<T,ROW_SIZE>,
    public dfs_btree_t {
protected:
    enum cconst_protected {
        XZ_BTREE_VER = 0x00000002, //Hihg-16-bit: main ver;
        XZ_BTREE_TAG = (((int)'x')<<24) | (((int)'z')<<16) | (((int)'b')<<8) | (((int)'t')), //"xzbt"
    };
private:
    //typedef T *PT;
    typedef const T *PCT;
    typedef T DU;
    typedef DU *PDU;
    typedef const DU *PCDU;
    typedef K *PK;
    typedef const K *PCK;
protected:
    typedef dfs_bt2d_ary_t<DU, ROW_SIZE> t_ary_t;
    typedef void * pvoid;
private:
    dfs_init_t _init_state;
    t_ary_t & _t_2d_ary;
public:
    _dfs_xz_btree_t(t_ary_t & t_2d_ary) :
        dfs_btree_t(),
        _t_2d_ary(t_2d_ary) {
        //init();
    }
    virtual ~_dfs_xz_btree_t() {
        _init_state.set_destructed();
    }
    int init(const uint32_t bt_instance_pos, dfs_bt_root_t & new_wr_bt_root);
private:
    //Disable operator=() and copy constructor
    const _dfs_xz_btree_t & operator=(const _dfs_xz_btree_t & src);
    _dfs_xz_btree_t(const _dfs_xz_btree_t & src);
    //const _dfs_xz_btree_t & operator=(const _dfs_xz_btree_t & src)
    //{
    //    return *this;
    //};
    //_dfs_xz_btree_t(const _dfs_xz_btree_t & src)
    //{
    //};
private:
    int _xz_store_fhead(
        dfs_btree_store_info_t & store_info,
        int64_t & file_head_offset,
        dfs_xz_btree_fhead_t & xz_btree_fhead) const;
    int _xz_update_stored_fhead(
        dfs_btree_store_info_t & store_info,
        const int64_t file_head_offset,
        dfs_xz_btree_fhead_t & xz_btree_fhead) const;
    int _xz_load_fhead(
        dfs_btree_load_info_t & load_info,
        int64_t & file_head_offset,
        dfs_xz_btree_fhead_t & xz_btree_fhead);
protected:
    inline int _xz_inc_root_ref_counter(const dfs_bt_root_t & bt_root) const {
        return _bt_inc_root_ref_counter(bt_root);
    };
    inline int _xz_dec_root_ref_counter(const dfs_bt_root_t & bt_root) const {
        return _bt_dec_root_ref_counter(bt_root);
    };
    void _xz_set_cancel_checkpointing(bool is_cancel) {
        _bt_set_cancel_checkpointing(is_cancel);
        return ;
    };
    void _xz_update_mutation_counter(const bool is_batch_mode) {
        _bt_update_mutation_counter(is_batch_mode);
        return ;
    };
    void _xz_update_max_cow_mutation_counter(void) const {
        _bt_update_max_cow_mutation_counter();
        return ;
    };
    //  功能：把已经设置的checkpoint存储到文件中
    //  参数：file_info：文件的控制信息，例如文件句柄，调用store或load时文件数据的位置等。
    //  返回：0 for success, other values for error
    int _xz_store_checkpointing(
        const dfs_bt_root_t & bt_root,
        dfs_btree_store_info_t & store_info) const;
    //
    //  功能：把先前存储在文件的checkpoint加载到内存中
    //  参数：file_info：文件的控制信息，例如文件句柄，调用store或load时文件数据的位置等。
    //  返回：0 for success, other values for error
    int _xz_load_checkpointing(
        dfs_bt_root_t & new_bt_root,
        dfs_btree_load_info_t & load_info);
protected:
    //  功能：获得B树最小元素
    //  返回: 成功(即使树空)返回0, 其他值错误
    int _xz_get_smallest(
        const dfs_bt_root_t & bt_root,
        PCDU & pcdu) const;
    //
    //  功能：获得B树最大元素
    //  返回: 成功(即使树空)返回0, 其他值错误
    int _xz_get_largest(
        const dfs_bt_root_t & bt_root,
        PCDU & pcdu) const;
    //
    //  功能：获得刚好比输入项srct小的元素
    //  输入：srct不需要在B树中存在
    //  输出：如果较小元素存在，其值复制到tobj，drill_info指向搜索key的结果。
    //  返回：0 for no error, other values for error
    int _xz_get_smaller(
        const dfs_bt_root_t & bt_root,
        const K & key,
        dfs_btree_drill_t & drill_info,
        PCDU & pcdu) const;
    int _xz_get_smaller(
        const dfs_bt_root_t & bt_root,
        const DU & srcdu,
        dfs_btree_drill_t & drill_info,
        PCDU & pcdu) const;
    //  功能：获得刚好比输入的srct大的元素
    //  输入：srct不需要在B树中存在
    //  输出：如果较大元素存在，其值复制到tobj，drill_info指向搜索srct的结果。
    //  返回：0 for no error, other values for error
    int _xz_get_larger(
        const dfs_bt_root_t & bt_root,
        const K & key,
        dfs_btree_drill_t & drill_info,
        PCDU & pcdu) const;
    int _xz_get_larger(
        const dfs_bt_root_t & bt_root,
        const DU & srcdu,
        dfs_btree_drill_t & drill_info,
        PCDU & pcdu) const;
protected:
    //  功能：把项t插入或更新到B树中。
    //       insert：tobj包含的关键字在B树中不存在，则加入B树并返回成功，否则返回失败。
    //       update：tobj包含的关键字在B树存在就更新其值为tobj并返回成功，否则返回失败。
    //       insert_update：先执行insert，如果该项存在就再执行update。
    //  输入：项tobj
    //  输出：更新后的树的根节点
    //        如果原来项存在且old_pt非空，则其值复制到*old_pt。
    //  return: 0 for success, other values for error
    int _xz_insert(
        const dfs_bt_root_t & old_wr_bt_root,
        dfs_bt_root_t & new_wr_bt_root,
        const T & tobj,
        T * old_pt,
        bool & is_old_exist);
    int _xz_update(
        const dfs_bt_root_t & old_wr_bt_root,
        dfs_bt_root_t & new_wr_bt_root,
        const T & tobj,
        T * old_pt,
        bool & is_old_exist);
    int _xz_insert_update(
        const dfs_bt_root_t & old_wr_bt_root,
        dfs_bt_root_t & new_wr_bt_root,
        const T & tobj,
        T * old_pt,
        bool & is_old_exist);
    //  功能：如果一个项存在，则删除；否则，不做任何事情。
    //  输出：更新后的树的根节点
    //        如果原来项存在，则pcdu为指向其的指针，否则是NULL。
    //  return: 0 for success, other values for error
    int _xz_del(
        const dfs_bt_root_t & old_wr_bt_root,
        dfs_bt_root_t & new_wr_bt_root,
        const K & key,
        T * old_pt,
        bool & is_old_exist);
    int _xz_del(
        const dfs_bt_root_t & old_wr_bt_root,
        dfs_bt_root_t & new_wr_bt_root,
        const T & tobj,
        T * old_pt,
        bool & is_old_exist);
protected:
    //return: 0 for success, other values for error
    //May return 0 even if obj_index is invalid and *ppdu is set to NULL.
    inline int _get_exist_obj_for_mutate(const uint64_t obj_index, PDU & pdu) const;
    //return: 0 for success, other values for error
    //Notice: the obj must exist.
    inline BAIDU_INLINE int _get_exist_obj(const uint64_t obj_index, PCDU & pcdu) const ;
    //return: 0 for success, other values for error
    //May return 0 even if obj_index is invalid and *ppcdu is set to NULL.
    inline int _get_may_not_exist_obj(const uint64_t obj_index, PCDU & pcdu) const;
protected:
    //return: 0 for success, other values for error
    //以tobj为基础生成一个新的T对象
    inline int _add_obj(const DU & duobj, uint64_t & obj_index);
//protected:
//    //告诉obj做必要的工作，在fp_btree中，当对象删除时，用来减少字符串的引用计数。
//    inline int _del_obj(const uint64_t obj_index);
private:
    //return: 0 for success, other values for error
    //cmp: -1, 0, 1 for less than, equal, great than
    inline BAIDU_INLINE int _compare_obj_obj(const DU & duobj1, const DU & duobj2, int & cmp) const;
    inline BAIDU_INLINE int _compare_obj_key(const DU & duobj1, const K & key2, int & cmp) const;
    inline BAIDU_INLINE int _inc_obj_ref_counter(const uint64_t obj_index, uint64_t & ref_counter);
    inline BAIDU_INLINE int _dec_obj_ref_counter(const uint64_t obj_index, uint64_t & ref_counter);
protected:
    ////  return: 0 for success, other values for error
    //virtual int vir_get_total_obj_ref_counter_in_ary(uint64_t & total_ref_counter) const;
    //  return: 0 for success, other values for error
    virtual int vir_get_total_in_use_obj_num_in_ary(uint64_t & total_in_use_obj_num) const;
    //  Return: 0 for success, other values for error
    virtual int vir_obj_store_size(
        const uint64_t obj_index,
        uint64_t & store_size,
        dfs_btree_store_info_t & store_info) const;
    //  return: 0 for success, other values for error
    virtual int vir_store_obj(
        const uint64_t obj_index,
        dfs_btree_store_info_t & store_info) const;
    //  return: 0 for success, other values for error
    virtual int vir_load_obj(
        uint64_t & obj_index,
        dfs_btree_load_info_t & load_info);
    ////  return: 0 for success, other values for error
    //virtual int vir_get_obj_ref_counter(const uint64_t obj_index, uint64_t & ref_counter) const;
    virtual int vir_inc_obj_ref_counter(const uint64_t obj_index);
    virtual int vir_dec_obj_ref_counter(const uint64_t obj_index);
    virtual int vir_inc_obj_ary_ref_counter(const uint64_t * obj_index_ary, const int32_t obj_num);
    virtual int vir_dec_obj_ary_ref_counter(const uint64_t * obj_index_ary, const int32_t obj_num);
protected:
    //
    //  return: 0 for success, other values for error
    //  cmp: -1, 0, 1 for less than, equal, great than
    //virtual int vir_compare_index_index(const uint64_t obj1_index, const uint64_t obj2_index, int & cmp) const;
    virtual int vir_compare_index_obj(
        const dfs_bt_root_t & bt_root,
        const uint64_t obj1_index,
        const void * pobj2,
        int & cmp) const;
    virtual int vir_compare_index_key(
        const dfs_bt_root_t & bt_root,
        const uint64_t obj1_index,
        const void * pkey2,
        int & cmp) const;

    virtual int vir_search_ary_by_index(
        const dfs_bt_root_t & bt_root,
        const uint64_t * obj_index_ary,
        const int32_t obj_num,
        const uint64_t obj2_index,
        int & ins_pos,
        uint64_t & obj_index) const;
    virtual int vir_search_ary_by_obj(
        const dfs_bt_root_t & bt_root,
        const uint64_t * obj_index_ary,
        const int32_t obj_num,
        const void * pobj2,
        int & ins_pos,
        uint64_t & obj_index) const;
    virtual int vir_search_ary_by_key(
        const dfs_bt_root_t & bt_root,
        const uint64_t * obj_index_ary,
        const int32_t obj_num,
        const void * pkey2,
        int & ins_pos,
        uint64_t & obj_index) const;
    virtual int vir_verify_index_ary(
        const dfs_bt_root_t & bt_root,
        const uint64_t * obj_index_ary,
        const int32_t obj_num,
        bool & is_inc_order) const;
public:
    //  return: 0 for success, other values for error
    int xz_get_in_use_num_in_2d_ary(
        uint64_t & in_use_obj_num,
        uint64_t & in_use_leaf_node_num,
        uint64_t & in_use_mid_node_num) const;
protected:
    void _xz_clear_statistic_info(void) {
        _t_2d_ary.clear_statistic_info();
        this->_bt_clear_statistic_info();
    };
    void _xz_log_statistic_info(
        const int org_log_level,
        const char * filename,
        const int lineno,
        const char * funcname) const {
        _t_2d_ary.log_statistic_info(org_log_level, filename, lineno, funcname, "xz", "t_2d_ary");
        this->_bt_log_statistic_info(org_log_level, filename, lineno, funcname, "xz");
    };
    void _xz_log_debug_info(
        const int org_log_level,
        const char * filename,
        const int lineno,
        const char * funcname) const {
        _t_2d_ary.log_debug_info(org_log_level, filename, lineno, funcname, "xz", "t_2d_ary");
        this->_bt_log_debug_info(org_log_level, filename, lineno, funcname, "xz");
    };
    uint64_t _xz_get_mem_size(void) const {
        return this->_bt_get_mem_size()+_get_bt2d_ary_mem_size();
    };
    uint64_t _get_bt2d_ary_mem_size() const {
        return _t_2d_ary.get_mem_size();
    };
};


template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::init(
    const uint32_t bt_instance_pos,
    dfs_bt_root_t & new_wr_bt_root) {
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    if (_init_state.is_not_init_yet()) {
        if ((err = dfs_btree_t::init(bt_instance_pos, new_wr_bt_root)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "dfs_btree_t::init() returns 0x%x", err);
        }

        _init_state.set_init_result(err);
    } else {
        err = _init_state.get_init_result();
    }

    return err;
};

//return: 0 for success, other values for error
//May return 0 even if obj_index is invalid and *ppdu is set to NULL.
template<typename T, typename K, uint32_t ROW_SIZE>
inline int _dfs_xz_btree_t<T, K, ROW_SIZE>::_get_exist_obj_for_mutate(
    const uint64_t obj_index,
    PDU & pdu) const {
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    if ((err = _t_2d_ary.get_exist_t_unit(obj_index, pdu)) != 0 || NULL == pdu) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_t_2d_ary.get_exist_t_unit() returns 0x%x", err);
        DF_WRITE_LOG_US(log_level, "obj_index=0x%lx, pdu=0x%p", obj_index, pdu);
        err = DF_BT_SET_INTERNAL_ERR(this->ERRNO_BT_INVALID_INDEX);
    }

    return err;
};
//return: 0 for success, other values for error
//Notice: the obj must exist.
template<typename T, typename K, uint32_t ROW_SIZE>
inline BAIDU_INLINE int _dfs_xz_btree_t<T, K, ROW_SIZE>::_get_exist_obj(
    const uint64_t obj_index,
    PCDU & pcdu) const {
    PDU pdu = NULL;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    pcdu = NULL;
    if ((err = _t_2d_ary.get_exist_t_unit(obj_index, pdu)) != 0 || NULL == pdu) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_t_2d_ary.get_exist_t_unit() returns 0x%x", err);
        DF_WRITE_LOG_US(log_level, "obj_index=0x%lx, pdu=0x%p", obj_index, pdu);
        err = DF_BT_SET_INTERNAL_ERR(this->ERRNO_BT_INVALID_INDEX);
    } else {
        pcdu = pdu;
    }

    return err;
};
//return: 0 for success, other values for error
//May return 0 even if obj_index is invalid and *ppcdu is set to NULL.
template<typename T, typename K, uint32_t ROW_SIZE>
inline int _dfs_xz_btree_t<T, K, ROW_SIZE>::_get_may_not_exist_obj(
    const uint64_t obj_index,
    PCDU & pcdu) const {
    PDU pdu = NULL;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    pcdu = NULL;
    if ((err = _t_2d_ary.get_may_not_exist_t_unit(obj_index, pdu)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_t_2d_ary.get_may_not_exist_t_unit() returns 0x%x", err);
        DF_WRITE_LOG_US(log_level, "obj_index=0x%lx, pdu=0x%p", obj_index, pdu);
    } else {
        pcdu = pdu;
    }

    return err;
};

//return: 0 for success, other values for error
//以tobj为基础生成一个新的T对象
template<typename T, typename K, uint32_t ROW_SIZE>
inline int _dfs_xz_btree_t<T, K, ROW_SIZE>::_add_obj(
    const DU & duobj,
    uint64_t & obj_index) {
    PDU pdu = NULL;
    int err = 0;

    if ((err = _t_2d_ary.acquire_t(&duobj, obj_index, &pdu)) != 0) {
        DF_WRITE_LOG_US(DF_UL_LOG_FATAL, "_t_2d_ary.acquire_t() returns 0x%x", err);
    } else if (UNDEF_INDEX == obj_index || NULL == pdu) {
        DF_WRITE_LOG_US(DF_UL_LOG_FATAL,
                        "_t_2d_ary.acquire_t() succeeds, but obj_index=0x%lx, pt=NULL",
                        obj_index);
        err = DF_BT_SET_INTERNAL_ERR(this->ERRNO_BT_OBJ_ACQUIRE);
    } else {
        //if ((err = vir_add_index(key_index)) != 0)
        //{
        //    key_index = UNDEF_INDEX;
        //}
    }

    return err;
};

//template<typename T, typename K, uint32_t ROW_SIZE>
//inline int _dfs_xz_btree_t<T, K, ROW_SIZE>::_del_obj(const uint64_t obj_index)
//{
//    PDU pdu = NULL;
//    int log_level = DF_UL_LOG_NONE;
//    int err = 0;
//
//    if ((err = this->_get_exist_obj_for_mutate(obj_index, pdu)) != 0 || NULL == pdu)
//    {
//        log_level = DF_UL_LOG_FATAL;
//        DF_WRITE_LOG_US(log_level, "_get_exist_obj_for_mutate() returns 0x%x", err);
//        err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
//    }
//    else if ((err = pdu->gc_obj(this->bt_get_extra_info())) != 0)
//    {
//        log_level = DF_UL_LOG_FATAL;
//        DF_WRITE_LOG_US(log_level, "pcdu->gc_obj(this->bt_get_extra_info()) returns 0x%x", err);
//    }
//
//    return err;
//};


//return: 0 for success, other values for error
//cmp: -1, 0, 1 for less than, equal, great than
template<typename T, typename K, uint32_t ROW_SIZE>
inline int _dfs_xz_btree_t<T, K, ROW_SIZE>::_compare_obj_obj(
    const DU & duobj1,
    const DU & duobj2,
    int & cmp) const {
    cmp = 1;
    if (duobj1 == duobj2) {
        cmp = 0;
    } else if (duobj1 < duobj2) {
        cmp = -1;
    } else {
        cmp = 1;
    }

    return 0;
};
//return: 0 for success, other values for error
//cmp: -1, 0, 1 for less than, equal, great than
template<typename T, typename K, uint32_t ROW_SIZE>
inline BAIDU_INLINE int _dfs_xz_btree_t<T, K, ROW_SIZE>::_compare_obj_key(
    const DU & duobj1,
    const K & key2,
    int & cmp) const {
    if (duobj1 == key2) {
        cmp = 0;
    } else if (duobj1 < key2) {
        cmp = -1;
    } else {
        cmp = 1;
    }

    return 0;
};

template<typename T, typename K, uint32_t ROW_SIZE>
inline int _dfs_xz_btree_t<T, K, ROW_SIZE>::_inc_obj_ref_counter(
    const uint64_t obj_index,
    uint64_t & ref_counter) {
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    //_bt_inc_key_ref_inc_counter();
    ref_counter = UNDEF_REF_COUNTER;
    if ((err = _t_2d_ary.inc_t_ref_counter(obj_index, ref_counter)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_t_2d_ary.inc_t_ref_counter() returns 0x%x", err);
        //err = DF_BT_SET_INTERNAL_ERR(this->ERRNO_BT_REF_COUNTER);
    } else if (0 == ref_counter || UNDEF_REF_COUNTER == ref_counter) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "ref_counter=%ld", ref_counter);
        err = DF_BT_SET_INTERNAL_ERR(this->ERRNO_BT_REF_COUNTER);
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "obj_index=0x%lx, ref_counter=%ld",
                        obj_index, ref_counter);
    }

    return err;
};
template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::_dec_obj_ref_counter(
    const uint64_t obj_index,
    uint64_t & ref_counter) {
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    //DFS_BT_DEBUG_TRAP;
    //_bt_inc_key_ref_dec_counter();
    ref_counter = UNDEF_REF_COUNTER;

    if ((err = _t_2d_ary.dec_t_ref_counter(obj_index, ref_counter)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_t_2d_ary.dec_t_ref_counter() returns 0x%x", err);
        //err = DF_BT_SET_INTERNAL_ERR(this->ERRNO_BT_REF_COUNTER);
    } else if (UNDEF_REF_COUNTER == ref_counter) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "ref_counter=%ld", ref_counter);
        err = DF_BT_SET_INTERNAL_ERR(this->ERRNO_BT_REF_COUNTER);
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "obj_index=0x%lx, ref_counter=%ld",
                        obj_index, ref_counter);
    }

    return err;
};

////  return: 0 for success, other values for error
//virtual int vir_get_total_obj_ref_counter_in_ary(uint64_t & total_ref_counter) const
//{
//    int err = 0;

//    if ((err = _t_2d_ary.get_total_ref_counter(total_ref_counter)) != 0)
//    {
//        DF_WRITE_LOG_US(DF_UL_LOG_FATAL,
//                "_t_2d_ary.get_total_ref_counter() returns 0x%x, total_ref_counter=%ld",
//                err, total_ref_counter);
//    }
//    else
//    {
//        --total_ref_counter;    //deduct the ref_counter of [0]
//    }

//    return err;
//};
//  return: 0 for success, other values for error
template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::vir_get_total_in_use_obj_num_in_ary(
    uint64_t & total_in_use_obj_num) const {
    int err = 0;

    if ((err = _t_2d_ary.get_in_use_num(total_in_use_obj_num)) != 0) {
        DF_WRITE_LOG_US(DF_UL_LOG_FATAL,
                        "_t_2d_ary.get_in_use_num() returns 0x%x, total_in_use_obj_num=%ld",
                        err, total_in_use_obj_num);
    }

    return err;
};
//  Return: 0 for success, other values for error
template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::vir_obj_store_size(
    const uint64_t obj_index,
    uint64_t & store_size,
    dfs_btree_store_info_t & store_info) const {
    PCDU pcdu = NULL;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    store_size = 0;
    if(store_info.get_store_t_type() == dfs_btree_store_info_t::STORE_T_INDEX) {
        store_size = sizeof(uint64_t);
    } else if ((err = _get_exist_obj(obj_index, pcdu)) != 0 || NULL == pcdu) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_get_exist_obj() returns 0x%x", err);
        err = DF_BT_SET_INTERNAL_ERR(this->ERRNO_BT_OBJ_NULL_POINTER);
    } else {
        store_size = pcdu->get_store_size();
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "obj_index=0x%lx, store_size=%ld",
                        obj_index, store_size);
    }

    return err;
};
//  return: 0 for success, other values for error
template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::vir_store_obj(
    const uint64_t obj_index,
    dfs_btree_store_info_t & store_info) const {
    PCDU pcdu = NULL;
    uint64_t key_seq = 0;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    if(store_info.get_store_t_type() == dfs_btree_store_info_t::STORE_T_INDEX) {
        if ((err = store_info.get_kept_obj_seq(obj_index, key_seq)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "store_info.get_kept_obj_seq() returns 0x%x", err);
        } else if ((err = store_info.store_data(key_seq)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "store_info.store_data() returns 0x%x", err);
        }
    } else if ((err = _get_exist_obj(obj_index, pcdu)) != 0 || NULL == pcdu) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_get_exist_obj() returns 0x%x", err);
        err = DF_BT_SET_INTERNAL_ERR(this->ERRNO_BT_OBJ_NULL_POINTER);
    } else if ((err = pcdu->store(
                          store_info.get_buf(),
                          store_info.get_buf_size(),
                          store_info.get_data_pos())) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "pcdu->store() returns 0x%x", err);
    } else if (store_info.get_store_t_type() == dfs_btree_store_info_t::STORE_T_VALUE_KEEP_T_INDEX) {
        if ((err = store_info.keep_obj_index(obj_index)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "store_info.keep_obj_index() returns 0x%x", err);
        }
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "key_seq=%lu, obj_index=0x%lx",
                        key_seq, obj_index);
        DF_WRITE_LOG_US(log_level,
                        "store_buf_size=%ld/data_pos=%ld",
                        store_info.get_buf_size(), store_info.get_data_pos());
    }

    return err;
};
//  return: 0 for success, other values for error
template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::vir_load_obj(
    uint64_t & obj_index,
    dfs_btree_load_info_t & load_info) {
    DU duobj;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    obj_index = UNDEF_INDEX;
    duobj.init();
    if(load_info.get_store_t_type() == dfs_btree_load_info_t::STORE_T_INDEX) {
        if ((err = load_info.load_data(obj_index)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "load_info.load_data() returns 0x%x", err);
        }
    } else if ((err = duobj.load(
                          load_info.get_buf(),
                          load_info.get_data_len(),
                          load_info.get_data_pos())) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "duobj.load() returns 0x%x", err);
    } else if ((err = this->_add_obj(duobj, obj_index)) != 0) { //以tobj为基础生成一个新的T对象
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_add_obj() returns 0x%x", err);
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "obj_index=0x%lx, load_buf_data_len/data_pos=%ld/%ld",
                        obj_index, load_info.get_data_len(), load_info.get_data_pos());
    }

    return err;
};
////  return: 0 for success, other values for error
//virtual int vir_get_obj_ref_counter(const uint64_t obj_index, uint64_t & ref_counter) const
//{
//    int log_level = DF_UL_LOG_NONE;
//    int err = 0;
//
//    ref_counter = UNDEF_REF_COUNTER;
//    if ((err = _t_2d_ary.get_t_ref_counter(obj_index, ref_counter)) != 0)
//    {
//        log_level = DF_UL_LOG_FATAL;
//        DF_WRITE_LOG_US(log_level, "_t_2d_ary.get_t_ref_counter() returns 0x%x", err);
//        //err = DF_BT_SET_INTERNAL_ERR(this->ERRNO_BT_REF_COUNTER);
//    }
//    else if (UNDEF_REF_COUNTER == ref_counter)
//    {
//        log_level = DF_UL_LOG_FATAL;
//        DF_WRITE_LOG_US(log_level, "ref_counter=%ld", ref_counter);
//        err = DF_BT_SET_INTERNAL_ERR(this->ERRNO_BT_REF_COUNTER);
//    }

//    if (DF_UL_LOG_NONE != log_level)
//    {
//        DF_WRITE_LOG_US(log_level,
//                "obj_index=0x%lx, ref_counter=%ld",
//                obj_index, ref_counter);
//    }

//    return err;
//};
template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::vir_inc_obj_ref_counter(
    const uint64_t obj_index) {
    uint64_t ref_counter = UNDEF_REF_COUNTER;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    if ((err = _inc_obj_ref_counter(obj_index, ref_counter)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_inc_obj_ref_counter() returns 0x%x", err);
        DF_WRITE_LOG_US(log_level, "obj_index=0x%lx, ref_counter=%ld", obj_index, ref_counter);
        //err = DF_BT_SET_INTERNAL_ERR(this->ERRNO_BT_REF_COUNTER);
    }

    return err;
};
template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::vir_dec_obj_ref_counter(
    const uint64_t obj_index) {
    uint64_t ref_counter = UNDEF_REF_COUNTER;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    //DFS_BT_DEBUG_TRAP;
    if ((err = _dec_obj_ref_counter(obj_index, ref_counter)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_dec_obj_ref_counter() returns 0x%x", err);
        DF_WRITE_LOG_US(log_level, "obj_index=0x%lx, ref_counter=%ld", obj_index, ref_counter);
        //err = DF_BT_SET_INTERNAL_ERR(this->ERRNO_BT_REF_COUNTER);
    }

    return err;
};

template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::vir_inc_obj_ary_ref_counter(
    const uint64_t * obj_index_ary,
    const int32_t obj_num) {
    uint64_t ref_counter = UNDEF_REF_COUNTER;
    int32_t pos = 0;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    if (NULL == obj_index_ary) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "null obj_index_ary parameter");
        err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
    } else {
        for (pos = 0; pos < obj_num; ++pos) {
            ref_counter = UNDEF_REF_COUNTER;
            if ((err = _inc_obj_ref_counter(obj_index_ary[pos], ref_counter)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_inc_obj_ref_counter() returns 0x%x", err);
                DF_WRITE_LOG_US(log_level, "obj_index_ary[pos]=0x%lx, ref_counter=%ld",
                                obj_index_ary[pos], ref_counter);
                //err = DF_BT_SET_INTERNAL_ERR(this->ERRNO_BT_REF_COUNTER);
                break;
            }
        }
    }

    //不能在此处报告obj_index_ary[pos](因为obj_index_ary可能为NULL)
    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "obj_num=%d, pos=%d",
                        obj_num, pos);
    }

    return err;
};
template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::vir_dec_obj_ary_ref_counter(
    const uint64_t * obj_index_ary,
    const int32_t obj_num) {
    uint64_t ref_counter = UNDEF_REF_COUNTER;
    int32_t pos = 0;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    DFS_BT_DEBUG_TRAP;
    if (NULL == obj_index_ary) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "null obj_index_ary parameter");
        err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
    } else {
        for (pos = 0; pos < obj_num; ++pos) {
            ref_counter = UNDEF_REF_COUNTER;
            if ((err = _dec_obj_ref_counter(obj_index_ary[pos], ref_counter)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_dec_obj_ref_counter() returns 0x%x", err);
                DF_WRITE_LOG_US(log_level, "obj_index_ary[pos]=0x%lx, ref_counter=%ld",
                                obj_index_ary[pos], ref_counter);
                //err = DF_BT_SET_INTERNAL_ERR(this->ERRNO_BT_REF_COUNTER);
                break;
            }
        }
    }

    //不能在此处报告obj_index_ary[pos](因为obj_index_ary可能为NULL)
    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "obj_num=%d, pos=%d",
                        obj_num, pos);
    }

    return err;
};

//
//  return: 0 for success, other values for error
//  cmp: -1, 0, 1 for less than, equal, great than
//virtual int vir_compare_index_index(const uint64_t obj1_index, const uint64_t obj2_index, int & cmp) const
//{
//    PCDU pcdu1 = NULL;
//    PCDU pcdu2 = NULL;
//    int log_level = DF_UL_LOG_NONE;
//    int err = 0;

//    cmp = 1;
//    if (obj1_index == obj2_index)
//    {
//        cmp = 0;
//    }
//    else if ((err = _get_exist_obj(obj1_index, pcdu1)) != 0 || NULL == pcdu1)
//    {
//        log_level = DF_UL_LOG_FATAL;
//        DF_WRITE_LOG_US(log_level, "_get_exist_obj(obj1_index...) returns 0x%x", err);
//        err = DF_BT_SET_INTERNAL_ERR(this->ERRNO_BT_OBJ_NULL_POINTER);
//    }
//    else if ((err = _get_exist_obj(obj2_index, pcdu2)) != 0 || NULL == pcdu2)
//    {
//        log_level = DF_UL_LOG_FATAL;
//        DF_WRITE_LOG_US(log_level, "_get_exist_obj(obj2_index...) returns 0x%x", err);
//        err = DF_BT_SET_INTERNAL_ERR(this->ERRNO_BT_OBJ_NULL_POINTER);
//    }
//    else if ((err = _compare_obj_obj(*pcdu1, *pcdu2, cmp)) != 0)
//    {
//        log_level = DF_UL_LOG_FATAL;
//        DF_WRITE_LOG_US(log_level, "_compare_obj_obj() returns 0x%x", err);
//    }

//    if (DF_UL_LOG_NONE != log_level)
//    {
//        DF_WRITE_LOG_US(log_level,
//                "obj1_index=0x%lx, obj2_index=0x%lx, cmp=%d",
//                obj1_index, obj2_index, cmp);
//    }

//    return err;
//};
template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::vir_compare_index_obj(
    const dfs_bt_root_t & /*bt_root*/,
    const uint64_t obj1_index,
    const void * pobj2,
    int & cmp) const {
    PCDU pcdu1 = NULL;
    PCDU pcdu2 = (PCDU)pobj2;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    cmp = 1;
    if ((err = _get_exist_obj(obj1_index, pcdu1)) != 0 || NULL == pcdu1) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_get_exist_obj(obj1_index...) returns 0x%x", err);
        err = DF_BT_SET_INTERNAL_ERR(this->ERRNO_BT_OBJ_NULL_POINTER);
    } else if (NULL == pcdu2) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "NULL == pcdu2");
        err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
    } else if ((err = _compare_obj_obj(*pcdu1, *pcdu2, cmp)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_compare_obj_obj() returns 0x%x", err);
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "obj1_index=0x%lx, pobj2=0x%p, cmp=%d",
                        obj1_index, pobj2, cmp);
    }

    return err;
};
template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::vir_compare_index_key(
    const dfs_bt_root_t & /*bt_root*/,
    const uint64_t obj1_index,
    const void * pkey2,
    int & cmp) const {
    PCDU pcdu1 = NULL;
    PCK pckey2 = (const K *)pkey2;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    cmp = 1;
    if ((err = _get_exist_obj(obj1_index, pcdu1)) != 0 || NULL == pcdu1) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_get_exist_obj(obj1_index...) returns 0x%x", err);
        err = DF_BT_SET_INTERNAL_ERR(this->ERRNO_BT_OBJ_NULL_POINTER);
    } else if (NULL == pckey2) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "NULL == pckey2");
        err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_KEY_NULL_POINTER);
    } else if ((err = _compare_obj_key(*pcdu1, *pckey2, cmp)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_compare_obj_key() returns 0x%x", err);
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "obj1_index=0x%lx, pkey2=0x%p, cmp=%d",
                        obj1_index, pkey2, cmp);
    }

    return err;
};


template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::vir_search_ary_by_index(
    const dfs_bt_root_t & /*bt_root*/,
    const uint64_t * obj_index_ary,
    const int32_t obj_num,
    const uint64_t obj2_index,
    int & ins_pos,
    uint64_t & obj_index) const {
    PCDU pcdu1 = NULL;
    PCDU pcdu2 = NULL;

    int cmp = 1;
    int32_t pos1 = 0;
    int32_t pos2 = 0;
    int32_t mid = 0;

    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    ins_pos = 0;
    obj_index = UNDEF_INDEX;
    if ((err = _get_exist_obj(obj2_index, pcdu2)) != 0 || NULL == pcdu2) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_get_exist_obj(obj2_index...) returns 0x%x", err);
        err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
    } else if (NULL == obj_index_ary) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "NULL == obj_index_ary");
        err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
    } else {
        pos1 = 0;
        pos2 = obj_num-1;
        while (pos1 <= pos2) {
            mid = (pos1+pos2)/2;
            if ((err = _get_exist_obj(obj_index_ary[mid], pcdu1)) != 0 || NULL == pcdu1) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_get_exist_obj(obj_index_ary[mid]...) returns 0x%x", err);
                err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
                break;
            } else if ((err = _compare_obj_obj(*pcdu1, *pcdu2, cmp)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_compare_obj_obj() returns 0x%x", err);
                break;
            } else if (0 == cmp) {
                //找到了
                pos1 = mid;
                break;
            } else if (cmp > 0) {
                pos2 = mid-1;
            } else { //if (cmp < 0)
                pos1 = mid+1;
            }
        }
        ins_pos = pos1;
        if (0 == err && 0 == cmp) {
            if (pos1 >= obj_num || pos1 < 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "cmp == 0 && (pos1 >= obj_num || pos1 < 0)");
                err = DF_BT_SET_INTERNAL_ERR(this->ERRNO_BT_SUBKEY_OVERFLOW);
            } else {
                obj_index = obj_index_ary[pos1];
            }
        }
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "obj2_index=0x%lx, ins_pos=%d, obj_index=0x%lx",
                        obj2_index, ins_pos, obj_index);
        DF_WRITE_LOG_US(log_level,
                        "pos1=%d, pos2=%d, mid=%d",
                        pos1, pos2, mid);
        if (NULL != obj_index_ary && obj_num > 0) {
            BT_DF_WRITE_LOG_INT64S(log_level, "obj_index_ary", obj_index_ary, obj_num);
        }
    }

    return err;
};
template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::vir_search_ary_by_obj(
    const dfs_bt_root_t & /*bt_root*/,
    const uint64_t * obj_index_ary,
    const int32_t obj_num,
    const void * pobj2,
    int & ins_pos,
    uint64_t & obj_index) const {
    PCDU pcdu1 = NULL;
    PCDU pcdu2 = (PCDU)pobj2;

    int cmp = 1;
    int32_t pos1 = 0;
    int32_t pos2 = 0;
    int32_t mid = 0;

    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    ins_pos = 0;
    obj_index = UNDEF_INDEX;
    if (NULL == pcdu2) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "NULL == pcdu2");
        err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
    } else if (NULL == obj_index_ary) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "NULL == obj_index_ary");
        err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
    } else {
        pos1 = 0;
        pos2 = obj_num-1;
        while (pos1 <= pos2) {
            mid = (pos1+pos2)/2;
            if ((err = _get_exist_obj(obj_index_ary[mid], pcdu1)) != 0 || NULL == pcdu1) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_get_exist_obj(obj_index_ary[mid]...) returns 0x%x", err);
                err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
                break;
            } else if ((err = _compare_obj_obj(*pcdu1, *pcdu2, cmp)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_compare_obj_obj() returns 0x%x", err);
                break;
            } else if (0 == cmp) {
                //找到了
                pos1 = mid;
                break;
            } else if (cmp > 0) {
                pos2 = mid-1;
            } else { //if (cmp < 0)
                pos1 = mid+1;
            }
        }
        ins_pos = pos1;
        if (0 == err && 0 == cmp) {
            if (pos1 >= obj_num || pos1 < 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "cmp == 0 && (pos1 >= obj_num || pos1 < 0)");
                err = DF_BT_SET_INTERNAL_ERR(this->ERRNO_BT_SUBKEY_OVERFLOW);
            } else {
                obj_index = obj_index_ary[pos1];
            }
        }
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "pobj2=0x%p, ins_pos=%d, obj_index=0x%lx",
                        pobj2, ins_pos, obj_index);
        DF_WRITE_LOG_US(log_level,
                        "pos1=%d, pos2=%d, mid=%d",
                        pos1, pos2, mid);
        if (NULL != obj_index_ary && obj_num > 0) {
            BT_DF_WRITE_LOG_INT64S(log_level, "obj_index_ary", obj_index_ary, obj_num);
        }
    }

    return err;
};
template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::vir_search_ary_by_key(
    const dfs_bt_root_t & /*bt_root*/,
    const uint64_t * obj_index_ary,
    const int32_t obj_num,
    const void * pkey2,
    int & ins_pos,
    uint64_t & obj_index) const {
    PCDU pcdu1 = NULL;
    PCK pckey2 = (const K *)pkey2;

    int cmp = 1;
    int32_t pos1 = 0;
    int32_t pos2 = 0;
    int32_t mid = 0;

    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    ins_pos = 0;
    obj_index = UNDEF_INDEX;
    if (NULL == pckey2) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "NULL == pckey2");
        err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_KEY_NULL_POINTER);
    } else if (NULL == obj_index_ary) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "NULL == obj_index_ary");
        err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
    } else {
        pos1 = 0;
        pos2 = obj_num-1;
        while (pos1 <= pos2) {
            mid = (pos1+pos2)/2;
            if (BAIDU_UNLIKELY((err = _get_exist_obj(obj_index_ary[mid], pcdu1)) != 0 || NULL == pcdu1)) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_get_exist_obj(obj_index_ary[mid]...) returns 0x%x", err);
                err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
                break;
            } else if (BAIDU_UNLIKELY((err = _compare_obj_key(*pcdu1, *pckey2, cmp)) != 0)) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_compare_obj_key() returns 0x%x", err);
                break;
            } else if (0 == cmp) {
                //找到了
                pos1 = mid;
                break;
            } else if (cmp > 0) {
                pos2 = mid-1;
            } else { //if (cmp < 0)
                pos1 = mid+1;
            }
        }
        ins_pos = pos1;
        if (0 == err && 0 == cmp) {
            if (BAIDU_UNLIKELY(pos1 >= obj_num || pos1 < 0)) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "cmp == 0 && (pos1 >= obj_num || pos1 < 0)");
                err = DF_BT_SET_INTERNAL_ERR(this->ERRNO_BT_SUBKEY_OVERFLOW);
            } else {
                obj_index = obj_index_ary[pos1];
            }
        }
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "pkey2=0x%p, ins_pos=%d, obj_index=0x%lx",
                        pkey2, ins_pos, obj_index);
        DF_WRITE_LOG_US(log_level,
                        "pos1=%d, pos2=%d, mid=%d",
                        pos1, pos2, mid);
        if (NULL != obj_index_ary && obj_num > 0) {
            BT_DF_WRITE_LOG_INT64S(log_level, "obj_index_ary", obj_index_ary, obj_num);
        }
    }

    return err;
};

template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::vir_verify_index_ary(
    const dfs_bt_root_t & /*bt_root*/,
    const uint64_t * obj_index_ary,
    const int32_t obj_num,
    bool & is_inc_order) const {
    PCDU pcdu1 = NULL;
    PCDU pcdu2 = NULL;
    int32_t j = 0;
    int32_t cmp = 0;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    is_inc_order = false;

    if (NULL == obj_index_ary) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "NULL == obj_index_ary");
        err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
    } else if (obj_num <= 1) {
        is_inc_order = true;
    } else {
        j = 0;
        if ((err = _get_exist_obj(obj_index_ary[j], pcdu2)) != 0 || NULL == pcdu2) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_exist_obj(obj_index_ary[j]...) returns 0x%x", err);
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else {
            is_inc_order = true;
            ++j;
            while (j < obj_num) {
                pcdu1 = pcdu2;
                if ((err = _get_exist_obj(obj_index_ary[j], pcdu2)) != 0 || NULL == pcdu2) {
                    log_level = DF_UL_LOG_FATAL;
                    DF_WRITE_LOG_US(log_level, "_get_exist_obj(obj_index_ary[j]...) returns 0x%x", err);
                    err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
                    is_inc_order = false;
                    break;
                } else if ((err = _compare_obj_obj(*pcdu1, *pcdu2, cmp)) != 0) {
                    log_level = DF_UL_LOG_FATAL;
                    DF_WRITE_LOG_US(log_level, "_compare_obj_obj() returns 0x%x", err);
                    is_inc_order = false;
                    break;
                } else if (cmp >= 0) {
                    is_inc_order = false;
                    break;
                }
                ++j;
            }
        }
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "j=%d, cmp=%d, is_inc_order=%d",
                        j, cmp, is_inc_order);
        if (NULL != obj_index_ary && obj_num > 0) {
            BT_DF_WRITE_LOG_INT64S(log_level, "obj_index_ary", obj_index_ary, obj_num);
        }
    }

    return err;
};


//  return: 0 for success, other values for error
template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::xz_get_in_use_num_in_2d_ary(
    uint64_t & in_use_obj_num,
    uint64_t & in_use_leaf_node_num,
    uint64_t & in_use_mid_node_num) const {
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    in_use_obj_num = 0;
    in_use_leaf_node_num = 0;
    in_use_mid_node_num = 0;

    if ((err = _t_2d_ary.get_in_use_num(in_use_obj_num)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_t_2d_ary.get_in_use_num() returns 0x%x", err);
    } else if ((err = this->bt_get_in_use_node_num_in_2d_ary(
                          in_use_leaf_node_num,
                          in_use_mid_node_num)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "bt_get_in_use_node_num_in_2d_ary() returns 0x%x", err);
    }

    return err;
};


//  功能：获得B树最小元素并填充drill_info
//  输出：若存在(B树非空)，则pct指向它且key_index为其值，否则pct为NULL且key_index为UNDEF_INDEX
//  返回: 成功(即使树空)返回0, 其他值错误
template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::_xz_get_smallest(
    const dfs_bt_root_t & bt_root,
    PCDU & pcdu) const {
    dfs_btree_drill_t drill_info;
    uint64_t obj_index = UNDEF_INDEX;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    pcdu = NULL;

    if ((err = this->_get_smallest(bt_root, drill_info, obj_index)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_get_smallest() returns 0x%x", err);
    } else if (UNDEF_INDEX != obj_index) { //smallest obj existed
        if ((err = _get_exist_obj(obj_index, pcdu)) != 0 || NULL == pcdu) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_exist_obj(obj_index...) returns 0x%x", err);
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        }
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "obj_index=0x%lx, drill_info.get_drill_ptr()=%d",
                        obj_index, drill_info.get_drill_ptr());
    }

    return err;
};

//
//  功能：获得B树最大元素并填充drill_info
//  输出：若存在(B树非空)，则pct指向它且key_index为其值，否则pct为NULL且key_index为UNDEF_INDEX
//  返回: 成功(即使树空)返回0, 其他值错误
template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::_xz_get_largest(
    const dfs_bt_root_t & bt_root,
    PCDU & pcdu) const {
    dfs_btree_drill_t drill_info;
    uint64_t obj_index = UNDEF_INDEX;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    pcdu = NULL;

    if ((err = this->_get_largest(bt_root, drill_info, obj_index)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_get_largest() returns 0x%x", err);
    } else if (UNDEF_INDEX != obj_index) { //smallest obj existed
        if ((err = _get_exist_obj(obj_index, pcdu)) != 0 || NULL == pcdu) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_exist_obj(obj_index...) returns 0x%x", err);
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        }
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "obj_index=0x%lx, drill_info.get_drill_ptr()=%d",
                        obj_index, drill_info.get_drill_ptr());
    }

    return err;
};

//
//  功能：获得刚好比输入项srct小的元素
//  输入：srct不需要在B树中存在
//  输出：如果较小元素存在，pct指向它，否则pct为NULL，drill_info指向搜索srct的结果。
//  返回：0 for no error, other values for error
template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::_xz_get_smaller(
    const dfs_bt_root_t & bt_root,
    const K & key,
    dfs_btree_drill_t & drill_info,
    PCDU & pcdu) const {
    uint64_t obj_index = UNDEF_INDEX;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    pcdu = NULL;

    if ((err = this->_get_smaller_by_key(bt_root, (const void *)&key, drill_info, obj_index)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_get_smaller_by_key() returns 0x%x", err);
    }
    //smaller obj existed
    else if (UNDEF_INDEX != obj_index) {
        if ((err = _get_exist_obj(obj_index, pcdu)) != 0 || NULL == pcdu) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_exist_obj(obj_index...) returns 0x%x", err);
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        }
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "&key=0x%p, obj_index=0x%lx",
                        &key, obj_index);
    }

    return err;
};
template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::_xz_get_smaller(
    const dfs_bt_root_t & bt_root,
    const T & srct,
    dfs_btree_drill_t & drill_info,
    PCDU & pcdu) const {
    uint64_t obj_index = UNDEF_INDEX;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    pcdu = NULL;

    if ((err = this->_get_smaller_by_obj(bt_root, (const void *)&srct, drill_info, obj_index)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_get_smaller_by_key() returns 0x%x", err);
    }
    //smaller obj existed
    else if (UNDEF_INDEX != obj_index) {
        if ((err = _get_exist_obj(obj_index, pcdu)) != 0 || NULL == pcdu) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_exist_obj(obj_index...) returns 0x%x", err);
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        }
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "&srct=0x%p, obj_index=0x%lx",
                        &srct, obj_index);
    }

    return err;
};

//  功能：获得刚好比输入的srct大的元素
//  输入：srct不需要在B树中存在
//  输出：如果较大元素存在，pct指向它，否则pct为NULL，drill_info指向搜索srct的结果。
//  返回：0 for no error, other values for error
template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::_xz_get_larger(
    const dfs_bt_root_t & bt_root,
    const K & key,
    dfs_btree_drill_t & drill_info,PCDU & pcdu) const {
    uint64_t obj_index = UNDEF_INDEX;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    pcdu = NULL;

    if ((err = this->_get_larger_by_key(bt_root, (const void *)&key, drill_info, obj_index)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_get_larger_by_key() returns 0x%x", err);
    }
    //larger obj existed
    else if (UNDEF_INDEX != obj_index) {
        if ((err = _get_exist_obj(obj_index, pcdu)) != 0 || NULL == pcdu) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_exist_obj(obj_index...) returns 0x%x", err);
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        }
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "&key=0x%p, obj_index=0x%lx",
                        &key, obj_index);
    }

    return err;
};
template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::_xz_get_larger(
    const dfs_bt_root_t & bt_root,
    const T & srct,
    dfs_btree_drill_t & drill_info,
    PCDU & pcdu) const {
    uint64_t obj_index = UNDEF_INDEX;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    pcdu = NULL;

    if ((err = this->_get_larger_by_obj(bt_root, (const void *)&srct, drill_info, obj_index)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_get_larger_by_key() returns 0x%x", err);
    }
    //larger obj existed
    else if (UNDEF_INDEX != obj_index) {
        if ((err = _get_exist_obj(obj_index, pcdu)) != 0 || NULL == pcdu) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_exist_obj(obj_index...) returns 0x%x", err);
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        }
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "&srct=0x%p, obj_index=0x%lx",
                        &srct, obj_index);
    }

    return err;
};


//  功能：把项t插入或更新到B树中。
//       insert：如果T的id为UNDEF_ID则为其申请一个新ID。如果T的id在B树中不存在，则加入B树，否则不做操作并返回UNDEF_ID。
//       update：如果T的id在B树存在就更新其值为t，否则返回UNDEF_ID。
//       insert_update：先执行insert，如果该项存在就再执行update。
//  输入：项t
//  输出：更新后的树的根节点
//        如果原来项存在且old_pt非空，则其值复制到*old_pt。
//return: 0 for success, other values for error
template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::_xz_insert(
    const dfs_bt_root_t & old_wr_bt_root,
    dfs_bt_root_t & new_wr_bt_root,
    const T & tobj,
    T * old_pt,
    bool & is_old_exist) {
    DU new_duobj(tobj);
    PCDU pcdu = NULL;
    uint64_t old_obj_index = UNDEF_INDEX;
    uint64_t new_obj_index = UNDEF_INDEX;
    dfs_btree_drill_t drill_info;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    is_old_exist = false;

    if ((err = this->_search_by_obj(
                   old_wr_bt_root,
                   (const void *)&tobj,
                   drill_info,
                   old_obj_index)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_search_by_obj() returns 0x%x", err);
    }
    //存在
    else if (UNDEF_INDEX != old_obj_index) {
        if ((err = _get_exist_obj(old_obj_index, pcdu)) != 0 || NULL == pcdu) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_exist_obj(old_obj_index...) returns 0x%x", err);
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else {
            //复制旧对象：一定要在更新B树之前，因为之后就对象可能被释放并调用了init()
            if (NULL != old_pt) {
                *old_pt = *((const T *)pcdu);
            }
            is_old_exist = true;
        }
    }
    //不存在
    else { //if (UNDEF_INDEX == old_obj_index)
        if ((err = this->_add_obj(new_duobj, new_obj_index)) != 0) { //以tobj为基础生成一个新的T对象
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_add_obj() returns 0x%x", err);
        } else if ((err = this->_insert_to_btree(
                              old_wr_bt_root,
                              new_wr_bt_root,
                              new_obj_index,
                              drill_info)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_insert_to_btree() returns 0x%x", err);
        }
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "new_obj_index=0x%lx, old_obj_index=0x%lx",
                        new_obj_index, old_obj_index);
    }

    return err;
};

//  功能：把项t插入或更新到B树中。
//       insert：如果T的id为UNDEF_ID则为其申请一个新ID。如果T的id在B树中不存在，则加入B树，否则不做操作并返回UNDEF_ID。
//       update：如果T的id在B树存在就更新其值为t，否则返回UNDEF_ID。
//       insert_update：先执行insert，如果该项存在就再执行update。
//  输入：项t
//  输出：更新后的树的根节点
//        如果原来项存在且old_pt非空，则其值复制到*old_pt。
//return: 0 for success, other values for error
template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::_xz_update(
    const dfs_bt_root_t & old_wr_bt_root,
    dfs_bt_root_t & new_wr_bt_root,
    const T & tobj,
    T * old_pt,
    bool & is_old_exist) {
    DU new_duobj(tobj);
    PCDU pcdu = NULL;
    uint64_t old_obj_index = UNDEF_INDEX;
    uint64_t new_obj_index = UNDEF_INDEX;
    dfs_btree_drill_t drill_info;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    is_old_exist = false;

    if ((err = this->_search_by_obj(
                   old_wr_bt_root,
                   (const void *)&tobj,
                   drill_info,
                   old_obj_index)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_search_by_obj() returns 0x%x", err);
    } else if (UNDEF_INDEX != old_obj_index) {
        //exist
        if ((err = _get_exist_obj(old_obj_index, pcdu)) != 0 || NULL == pcdu) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_exist_obj(old_obj_index...) returns 0x%x", err);
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else {
            //复制旧对象：一定要在更新B树之前，因为之后就对象可能被释放并调用了init()
            if (NULL != old_pt) {
                *old_pt = *((const T *)pcdu);
            }
            is_old_exist = true;

            if ((err = this->_add_obj(new_duobj, new_obj_index)) != 0) { //以tobj为基础生成一个新的T对象
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_add_obj() returns 0x%x", err);
            } else if ((err =  this->_update_of_btree(
                                   old_wr_bt_root,
                                   new_wr_bt_root,
                                   new_obj_index,
                                   drill_info)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_update_of_btree() returns 0x%x", err);
            }
        }
    } else { //not existed
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "new_obj_index=0x%lx, old_obj_index=0x%lx",
                        new_obj_index, old_obj_index);
    }

    return err;
};

//  功能：把项t插入或更新到B树中。
//       insert：如果T的id为UNDEF_ID则为其申请一个新ID。如果T的id在B树中不存在，
//              则加入B树，否则不做操作并返回失败。
//       update：如果T的id在B树存在就更新其值为t，否则返回UNDEF_ID。
//       insert_update：先执行insert，如果该项存在就再执行update。
//  输入：项t
//  输出：更新后的树的根节点
//        如果原来项存在且old_pt非空，则其值复制到*old_pt。
//return: 0 for success, other values for error
template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::_xz_insert_update(
    const dfs_bt_root_t & old_wr_bt_root,
    dfs_bt_root_t & new_wr_bt_root,
    const T & tobj,
    T * old_pt,
    bool & is_old_exist) {
    DU new_duobj(tobj);
    PCDU pcdu = NULL;
    uint64_t old_obj_index = UNDEF_INDEX;
    uint64_t new_obj_index = UNDEF_INDEX;
    dfs_btree_drill_t drill_info;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    is_old_exist = false;

    if ((err = this->_search_by_obj(
                   old_wr_bt_root,
                   (const void *)&tobj,
                   drill_info,
                   old_obj_index)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_search_by_obj() returns 0x%x", err);
    } else if ((err = this->_add_obj(new_duobj, new_obj_index)) != 0) { //以tobj为基础生成一个新的T对象
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_add_obj() returns 0x%x", err);
    } else if (UNDEF_INDEX != old_obj_index) { //existed
        if ((err = _get_exist_obj(old_obj_index, pcdu)) != 0 || NULL == pcdu) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_exist_obj(old_obj_index...) returns 0x%x", err);
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else {
            //复制旧对象：一定要在更新B树之前，因为之后就对象可能被释放并调用了init()
            if (NULL != old_pt) {
                *old_pt = *((const T *)pcdu);
            }
            is_old_exist = true;

            if ((err =  this->_update_of_btree(
                            old_wr_bt_root,
                            new_wr_bt_root,
                            new_obj_index,
                            drill_info)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_update_of_btree() returns 0x%x", err);
            }
        }
    } else { //not exist
        if ((err = this->_insert_to_btree(
                       old_wr_bt_root,
                       new_wr_bt_root,
                       new_obj_index,
                       drill_info)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_insert_to_btree() returns 0x%x", err);
        }
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "new_obj_index=0x%lx, old_obj_index=0x%lx",
                        new_obj_index, old_obj_index);
    }

    return err;
};


//  功能：如果一个项存在，则删除；否则，不做任何事情。
//  输出：is_deleted: true如果项存在，false如果项不存在
//  输出：更新后的树的根节点
//        如果原来项存在，则pcdu为指向其的指针，否则是NULL。
//return: 0 for success, other values for error
template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::_xz_del(
    const dfs_bt_root_t & old_wr_bt_root,
    dfs_bt_root_t & new_wr_bt_root,
    const K & key,
    T * old_pt,
    bool & is_old_exist) {
    PCDU pcdu = NULL;
    uint64_t old_obj_index = UNDEF_INDEX;
    dfs_btree_drill_t drill_info;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    is_old_exist = false;

    if ((err = this->_search_by_key(
                   old_wr_bt_root,
                   (const void *)&key,
                   drill_info,
                   old_obj_index)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_search_by_key() returns 0x%x", err);
    } else if (UNDEF_INDEX != old_obj_index) {
        if ((err = _get_exist_obj(old_obj_index, pcdu)) != 0 || NULL == pcdu) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_exist_obj(old_obj_index...) returns 0x%x", err);
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else {
            //复制旧对象：一定要在更新B树之前，因为之后就对象可能被释放并调用了init()
            if (NULL != old_pt) {
                *old_pt = *((const T *)pcdu);
            }
            is_old_exist = true;

            if ((err = this->_del_from_btree(old_wr_bt_root, new_wr_bt_root, drill_info)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_del_from_btree() returns 0x%x", err);
            } else {
            }
        }
    }

    return err;
};
template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::_xz_del(
    const dfs_bt_root_t & old_wr_bt_root,
    dfs_bt_root_t & new_wr_bt_root,
    const T & tobj,
    T * old_pt,
    bool & is_old_exist) {
    PCDU pcdu = NULL;
    uint64_t old_obj_index = UNDEF_INDEX;
    dfs_btree_drill_t drill_info;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    is_old_exist = false;

    if ((err = this->_search_by_obj(
                   old_wr_bt_root,
                   (const void *)&tobj,
                   drill_info,
                   old_obj_index)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_search_by_obj() returns 0x%x", err);
    } else if (UNDEF_INDEX != old_obj_index) {
        if ((err = _get_exist_obj(old_obj_index, pcdu)) != 0 || NULL == pcdu) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_exist_obj(old_obj_index...) returns 0x%x", err);
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else {
            //复制旧对象：一定要在更新B树之前，因为之后就对象可能被释放并调用了init()
            if (NULL != old_pt) {
                *old_pt = *((const T *)pcdu);
            }
            is_old_exist = true;

            if ((err = this->_del_from_btree(old_wr_bt_root, new_wr_bt_root, drill_info)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_del_from_btree() returns 0x%x", err);
            } else {
            }
        }
    }

    return err;
};

//  功能：把已经设置的checkpoint存储到文件中
//  参数：file_info：文件的控制信息，例如文件句柄，调用store或load时文件数据的位置等。
//  返回：0 for success, other values for error
template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::_xz_store_checkpointing(
    const dfs_bt_root_t & bt_root,
    dfs_btree_store_info_t & store_info) const {
    //const dfs_sbt_root_t & sbt_root = _bt_get_sbt_root(bt_root);
    dfs_xz_btree_fhead_t xz_btree_fhead;
    dfs_btree_fhead_t btree_fhead;
    int64_t file_head_offset = 0;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    log_level = this->CKP_LOG_LEVEL;

    if ((err = _xz_store_fhead(store_info, file_head_offset, xz_btree_fhead)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_xz_store_fhead() returns 0x%x", err);
    } else if ((err = this->_bt_store_tree(bt_root, store_info, btree_fhead)) != 0) {
        log_level = (ERRNO_BT_CKP_CANCELLED == err) ? DF_UL_LOG_NOTICE : DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_bt_store_tree() returns 0x%x", err);
    } else if ((err = _xz_update_stored_fhead(store_info, file_head_offset, xz_btree_fhead)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_xz_update_stored_fhead() returns 0x%x", err);
    } else if ((err = store_info.flush_data()) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "store_info.flush_data() returns 0x%x", err);
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "xz_btree_fhead.total_data_len=%ld, file_head_offset=%ld",
                        xz_btree_fhead.total_data_len, file_head_offset);
    }

    return err;
};

//  功能：把先前存储在文件的checkpoint加载到内存中
//  参数：file_info：文件的控制信息，例如文件句柄，调用store或load时文件数据的位置等。
//  返回：0 for success, other values for error
template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::_xz_load_checkpointing(
    dfs_bt_root_t & new_bt_root,
    dfs_btree_load_info_t & load_info) {
    //dfs_sbt_root_t sbt_root;
    dfs_xz_btree_fhead_t xz_btree_fhead;
    dfs_btree_fhead_t btree_fhead;
    int64_t file_head_offset = 0;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    log_level = this->CKP_LOG_LEVEL;
    if ((err = _xz_load_fhead(load_info, file_head_offset, xz_btree_fhead)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_xz_load_fhead() returns 0x%x", err);
    } else if ((err = this->_bt_load_tree(
                          new_bt_root,
                          load_info,
                          btree_fhead)) != 0) {
        log_level = (ERRNO_BT_CKP_CANCELLED == err) ? DF_UL_LOG_NOTICE : DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_bt_store_tree() returns 0x%x", err);
    }
    //move the file to the correct position
    else if ((err = load_info.file_seek(file_head_offset+xz_btree_fhead.total_data_len)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "load_info.file_seek() returns 0x%x", err);
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "xz_btree_fhead.total_data_len=%ld, file_head_offset=%ld",
                        xz_btree_fhead.total_data_len, file_head_offset);
    }

    return err;
};


template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::_xz_store_fhead(
    dfs_btree_store_info_t & store_info,
    int64_t & file_head_offset,
    dfs_xz_btree_fhead_t & xz_btree_fhead) const {
    int err = 0;

    file_head_offset = store_info.get_cur_file_offset();
    //zhangyan04@baidu.com
    //memset(&xz_btree_fhead, 0, sizeof(xz_btree_fhead));
    bzero(&xz_btree_fhead,sizeof(xz_btree_fhead));

    xz_btree_fhead.size = sizeof(xz_btree_fhead);
    xz_btree_fhead.ver  = XZ_BTREE_VER;
    xz_btree_fhead.major_tag  = store_info.get_major_tag();
    xz_btree_fhead.minor_tag = XZ_BTREE_TAG;

    err = store_info.store_buf(&xz_btree_fhead, sizeof(xz_btree_fhead));

    return err;
};

template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::_xz_update_stored_fhead(
    dfs_btree_store_info_t & store_info,
    const int64_t file_head_offset,
    dfs_xz_btree_fhead_t & xz_btree_fhead) const {
    int err = 0;

    xz_btree_fhead.total_data_len       = store_info.get_cur_file_offset() - file_head_offset;
    xz_btree_fhead.total_leaf_node_num  = store_info.get_total_leaf_node_num()    ;
    xz_btree_fhead.total_mid_node_num   = store_info.get_total_mid_node_num()     ;
    xz_btree_fhead.total_key_num        = store_info.get_total_key_num()          ;
    err = store_info.rewrite_data(&xz_btree_fhead, sizeof(xz_btree_fhead), file_head_offset);

    return err;
};

template<typename T, typename K, uint32_t ROW_SIZE>
int _dfs_xz_btree_t<T, K, ROW_SIZE>::_xz_load_fhead(
    dfs_btree_load_info_t & load_info,
    int64_t & file_head_offset,
    dfs_xz_btree_fhead_t & xz_btree_fhead) {
    uint32_t unread_len = 0;
    const uint32_t unread_head = sizeof(dfs_xz_btree_fhead_t)-sizeof(dfs_btree_fhead_base_t);
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    file_head_offset = load_info.get_cur_file_offset();
    //zhangyan04@baidu.com
    //memset(&xz_btree_fhead, 0, sizeof(xz_btree_fhead));
    bzero(&xz_btree_fhead,sizeof(xz_btree_fhead));

    //此时最大长度不知道，暂时设置为基本头的长度
    load_info.set_max_file_offset(file_head_offset+sizeof(dfs_btree_fhead_base_t));
    //加载基本头信息
    if ((err = load_info.load_buf(
                   (void *)((dfs_btree_fhead_base_t *)&xz_btree_fhead),
                   sizeof(dfs_btree_fhead_base_t))) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "load_info.load_buf(dfs_btree_fhead_base_t) returns 0x%x", err);
    } else if (load_info.get_major_tag() != xz_btree_fhead.major_tag) {
        err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_LOAD_MAJOR_TAG);
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, ERRINFO_BT_LOAD_MAJOR_TAG);
    } else if (XZ_BTREE_TAG != xz_btree_fhead.minor_tag) {
        err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_LOAD_MINOR_TAG);
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, ERRINFO_BT_LOAD_MINOR_TAG);
    } else if (XZ_BTREE_VER < xz_btree_fhead.ver) {
        err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_LOAD_HEAD_VER);
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, ERRINFO_BT_LOAD_HEAD_VER);
    } else if (xz_btree_fhead.size < sizeof(dfs_btree_fhead_base_t)) {
        err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_LOAD_HEAD_SIZE);
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, ERRINFO_BT_LOAD_HEAD_SIZE);
    } else {
        unread_len = xz_btree_fhead.size-sizeof(dfs_btree_fhead_base_t);

        //设置准确的最大长度
        load_info.set_max_file_offset(file_head_offset+xz_btree_fhead.total_data_len);

        //加载扩展头信息：头信息尺寸可能不等于sizeof(xz_btree_fhead)
        if ((err = load_info.load_buf(
                       (void *)((dfs_xz_btree_fhead_ext_t *)&xz_btree_fhead),
                       MIN(unread_len, unread_head))) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(
                log_level,
                "load_info.load_buf(dfs_xz_btree_fhead_ext_t) returns 0x%x",
                err);
        } else if (unread_len > unread_head) {
            if ((err = load_info.skip_data_len(unread_len-unread_head)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(
                    log_level,
                    "load_info.skip_data_len(unread_len-unread_head) returns 0x%x",
                    err);
            }
        }
    }

    load_info.set_total_nums(
        xz_btree_fhead.total_leaf_node_num  ,
        xz_btree_fhead.total_mid_node_num   ,
        xz_btree_fhead.total_key_num);

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "xz_btree_fhead.total_data_len=%ld, file_head_offset=%ld",
                        xz_btree_fhead.total_data_len, file_head_offset);
        DF_WRITE_LOG_US(log_level,
                        "load_info.get_major_tag()=0x%x, xz_btree_fhead.major_tag=0x%x",
                        load_info.get_major_tag(), xz_btree_fhead.major_tag);
        DF_WRITE_LOG_US(log_level,
                        "XZ_BTREE_TAG=0x%x, xz_btree_fhead.minor_tag=0x%x",
                        XZ_BTREE_TAG, xz_btree_fhead.minor_tag);
        DF_WRITE_LOG_US(log_level,
                        "XZ_BTREE_VER=0x%x, xz_btree_fhead.ver=0x%x",
                        XZ_BTREE_VER, xz_btree_fhead.ver);
        DF_WRITE_LOG_US(log_level,
                        "unread_len=0x%x, unread_head=0x%x, xz_btree_fhead.size=0x%x",
                        unread_len, unread_head, xz_btree_fhead.size);
    }

    return err;
};


//xz_btree with public interfaces and lock mechanism
template<typename T, typename K, uint32_t ROW_SIZE = dfs_bt_const_t::DEF_ROW_SIZE>
class dfs_xz_btree_t :
    virtual public dfs_bt_const_t,
public dfs_btree_lock_t,
private dfs_bt2d_ary_t<dfs_gc_du_t<T>, ROW_SIZE>,
    //private dfs_rc_set_t<dfs_bt_root_t, dfs_bt_const_t::MAX_BT_RC_ROOT_NUM>,
    private _dfs_xz_btree_t<dfs_gc_du_t<T>, K, ROW_SIZE> {
private:
    enum cconst_private {
        DEF_BT_INSTANCE_POS  = 0,
    };
private:
    //typedef T *PT;
    //typedef const T *PCT;
    typedef dfs_gc_du_t<T> DU;
    typedef DU *PDU;
    typedef const DU *PCDU;
    typedef K *PK;
    typedef const K *PCK;
    typedef dfs_bt2d_ary_t<DU, ROW_SIZE> t_ary_t;
    typedef void * pvoid;
    typedef dfs_rc_set_t<dfs_bt_root_t, dfs_bt_const_t::MAX_BT_RC_ROOT_NUM>  rc_root_set_t;
private:
    dfs_init_t _init_state;
    rc_root_set_t _rc_root_set;
    dfs_rc_indexer_t _ckp_root_indexer;   //当前checkpoint根节点在rc_root_set_t中的位置
    volatile uint32_t _is_batch_mutate_mode;
    uint32_t _reserved;
    //for test only
#ifdef  TEST_RWLOCK_TIME
    pthread_rwlock_t _test_locker;
#endif //#ifdef  TEST_RWLOCK_TIME
public:
    dfs_xz_btree_t() :
        _dfs_xz_btree_t<DU, K, ROW_SIZE>(*((t_ary_t *)this)) {
        init();
    };
    virtual ~dfs_xz_btree_t() {
#ifdef  TEST_RWLOCK_TIME
        //for test only
        pthread_rwlock_destroy(&_test_locker);
#endif //#ifdef  TEST_RWLOCK_TIME
    };
    int init(void);
private:
    //Disable operator=() and copy constructor
    const dfs_xz_btree_t & operator=(const dfs_xz_btree_t & src);
    dfs_xz_btree_t(const dfs_xz_btree_t & src);
    //const dfs_xz_btree_t & operator=(const dfs_xz_btree_t & src)
    //{
    //    return *this;
    //};
    //dfs_xz_btree_t(const dfs_xz_btree_t & src) :
    //            _dfs_xz_btree_t<DU, K>()
    //{
    //};
private:
    //对当前的wr_root进行快照(需要更新_max_cow_mutation_counter)
    inline int _snapshot_wr_root_indexer(
        dfs_rc_indexer_t & rc_indexer) const;
    //获得当前wr_root，但不进行快照(也不更新_max_cow_mutation_counter)
    inline int _retrieve_wr_root_indexer(
        dfs_rc_indexer_t & rc_indexer,
        const dfs_bt_root_t *& pbt_root) const;
    //进行修改(insert/update...)后更新当前wr_root
    inline int _update_wr_root_indexer(const dfs_bt_root_t & bt_root);
    inline int _get_root(
        const dfs_rc_indexer_t & rc_indexer,
        const dfs_bt_root_t *& pbt_root) const;
protected:
    //增加/减少根节点引用计数
    virtual int vir_inc_root_ref_counter(const dfs_bt_root_t & bt_root) const;
    virtual int vir_dec_root_ref_counter(const dfs_bt_root_t & bt_root) const;
public:
    //  功能：搜索一个项，存在则返回true，否则返回false。对应项存在时其值被复制到tagt
    //        如果prc_indexer非空则指向一个根节点，操作在该根节点进行，否则在当前根节点进行；
    //        如果search()及其他读操作的dfs_rc_indexer_t非空，则修改操作适用batch mode
    bool search(const K & key, T & tagt, const dfs_rc_indexer_t * prc_indexer = NULL) const;
    bool search(const T & srct, T & tagt, const dfs_rc_indexer_t * prc_indexer = NULL) const;

    //  功能：is_batch_mutate_mode为true则允许batch mutation，seacrh等读操作时
    //        需要提供有效的dfs_rc_indexer_t；为false时禁止batch mutation，
    //        search等操作可以提供空的dfs_rc_indexer_t。
    //  说明：batch_mutate_mode为true时更新速度快，但search()等读操作的dfs_rc_indexer_t必须非空
    //  返回：0 for success, other values for error
    int set_batch_mutate_mode(
        const bool is_batch_mutate_mode,
        const dfs_btree_lock_hold_t * plock_hold = NULL);
    bool get_batch_mutate_mode(void) const;
    //
    //  功能：把项t插入或更新到B树中。
    //        insert：如果K在B树中不存在，则加入B树并返回成功，否则不做操作并返回失败。
    //        update：如果K在B树中存在，则更新之并返回成功，否则不做操作并返回失败。
    //        insert_update：如果项不存在则执行insert，否则执行update。
    //  输入：项t
    //        如果原来项存在且old_pt非空，则其值复制到*old_pt。
    //  返回：操作成功返回true，否则返回false
    bool insert(
        const T & tobj,
        T * old_pt = NULL,
        const dfs_btree_lock_hold_t * plock_hold = NULL);
    bool update(
        const T & tobj,
        T * old_pt = NULL,
        const dfs_btree_lock_hold_t * plock_hold = NULL);
    bool insert_update(
        const T & tobj,
        T * old_pt = NULL,
        const dfs_btree_lock_hold_t * plock_hold = NULL);
    //
    //  功能：如果一个项存在，则删除；否则，不做任何事情。
    //        如果原来项存在且old_pt非空，则其值复制到*old_pt。
    //  返回：true如果删除成功，false如果删除失败(例如项不存在)
    bool del(
        const K & key,
        T * old_pt = NULL,
        const dfs_btree_lock_hold_t * plock_hold = NULL);
    bool del(
        const T & tobj,
        T * old_pt = NULL,
        const dfs_btree_lock_hold_t * plock_hold = NULL);
    //  功能：清空整个树。
    //  返回：0 for success, other values for error
    int clear(const dfs_btree_lock_hold_t * plock_hold = NULL);
    //功能：验证整个树的顺序和每个节点分支个数
    //返回：返回0如果验证通过，返回非0(错误代码)如果验证失败
    int verify(const dfs_rc_indexer_t * prc_indexer = NULL) const;
public: //obsolete APIs
    int set_serial_read_write_state(
        const bool is_serial_read_write,
        bool & is_success,
        const dfs_btree_lock_hold_t * plock_hold = NULL) {
        return 0;
    };
    int get_serial_read_write_state(
        bool & is_serial_read_write,
        const dfs_btree_lock_hold_t * plock_hold = NULL) const {
        return 0;
    };
public:
    //  return: 0 for success, other values for error
    int get_total_num(
        uint64_t & total_key_num,
        uint64_t & total_leaf_node_num,
        uint64_t & total_mid_node_num) const;
    //  return: 0 for success, other values for error
    int get_in_use_num_in_2d_ary(
        uint64_t & in_use_key_num,
        uint64_t & in_use_leaf_node_num,
        uint64_t & in_use_mid_node_num) const;
    //  return: 0 for success, other values for error
    int empty_btree_check(
        const bool is_should_empty,
        const dfs_btree_lock_hold_t * plock_hold = NULL) const;
    //  return: 0 for success, other values for error
    //  Should be called in locked state and without checkpointing or other COW(load or store)
    int sanitary_check(const dfs_btree_lock_hold_t * plock_hold = NULL);
    void clear_statistic_info(const dfs_btree_lock_hold_t * plock_hold = NULL);
    void log_statistic_info(
        const int org_log_level,
        const char * filename,
        const int lineno,
        const char * funcname,
        const dfs_btree_lock_hold_t * plock_hold = NULL) const;
    void log_debug_info(
        const int org_log_level,
        const char * filename,
        const int lineno,
        const char * funcname,
        const dfs_btree_lock_hold_t * plock_hold = NULL) const;
    uint64_t get_mem_size(const dfs_btree_lock_hold_t * plock_hold = NULL) const;
public:
    //
    //  功能：获得B树最小元素
    //  输出：如果B树非空，则最小项的值被复制到tobj，否则tobj被初始化
    //  返回：最小元素存在则返回true，最小元素不存在(如B树为空)返回false。
    bool get_smallest(T & tobj, const dfs_rc_indexer_t * prc_indexer = NULL) const;
    //  功能：获得B树最大元素
    //  输出：如果B树非空，则最大项的值被复制到tobj，否则tobj被初始化
    //  返回：最大元素存在则返回true，最大元素不存在(如B树为空)返回false。
    bool get_largest(T & tobj, const dfs_rc_indexer_t * prc_indexer = NULL) const;
    //  功能：获得刚好比输入项srct小的元素
    //  输入：srct不需要在B树中存在
    //  输出：如果较小元素存在，则其值被复制到tagt，否则tagt被初始化
    //  返回：较小元素存在则返回true，较小元素不存在返回false。
    bool get_smaller(
        const K & key,
        T & tagt,
        dfs_btree_drill_t * pdrill_info = NULL,
        const dfs_rc_indexer_t * prc_indexer = NULL) const;
    bool get_smaller(
        const T & srct,
        T & tagt,
        dfs_btree_drill_t * pdrill_info = NULL,
        const dfs_rc_indexer_t * prc_indexer = NULL) const;
    //  功能：获得刚好比输入的srct大的元素
    //  输入：srct不需要在B树中存在
    //  输出：如果较大元素存在，则其值被复制到tagt，否则tagt被初始化
    //  返回：较大元素存在则返回true，较大元素不存在返回false。
    bool get_larger(
        const K & key,
        T & tagt,
        dfs_btree_drill_t * pdrill_info = NULL,
        const dfs_rc_indexer_t * prc_indexer = NULL) const;
    bool get_larger(
        const T & srct,
        T & tagt,
        dfs_btree_drill_t * pdrill_info = NULL,
        const dfs_rc_indexer_t * prc_indexer = NULL) const;
public:
    //功能：设置或取消checkpoint状态
    //输入：is_cancel：true取消checkpoint点及进行中的checkpoing操作，false设置一个新的checkpoint点
    //      参数max_wait_microseconds不再使用
    //建议：仅用此函数取消进行中的checkpoint，通过snapshot()获得一个checkpoint
    //返回：0: 设置或取消成功，取消不存在的checkpoint状态也返回0
    int set_checkpoint_state(
        const bool is_cancel,
        const int64_t max_wait_microseconds,
        const dfs_btree_lock_hold_t * plock_hold = NULL);
    int get_checkpoint_state(void) const {
        return this->_bt_is_cancel_checkpointing() ;
    };
public:
    //获得当前B树的一个快照，不再使用后尽快调用rc_indexer.init()以释放快照占用的内存
    int snapshot(
        dfs_rc_indexer_t & rc_indexer,
        const dfs_btree_lock_hold_t * plock_hold = NULL);
    //
    //  功能：把已经设置的checkpoint存储到文件中
    //  参数：file_info：文件的控制信息，例如文件句柄，调用store或load时文件数据的位置等。
    //  返回：0 for success, other values for error
    int store_checkpointing(
        const dfs_write_proc_t write_proc,
        void * file_info,
        const dfs_btree_lock_hold_t * plock_hold = NULL,
        const dfs_rc_indexer_t * prc_indexer = NULL);
    //
    //  功能：把先前存储在文件的checkpoint加载到内存中
    //  参数：file_info：文件的控制信息，例如文件句柄，调用store或load时文件数据的位置等。
    //  返回：0 for success, other values for error
    int load_checkpointing(
        const dfs_read_proc_t read_proc,
        void * file_info,
        const dfs_btree_lock_hold_t * plock_hold = NULL);

////////////////////////////////////////////////////////////
//zhangyan04@baidu.com
//新增接口..
//内部接口,请勿使用...
    bool zy_search(const K & key,
                   T & tagt,
                   const dfs_rc_indexer_t * prc_indexer) const;
    int zy_insert(const T & tobj,
                  T * old_pt = NULL,
                  const dfs_btree_lock_hold_t * plock_hold = NULL);
    int zy_insert_update(const T & tobj,
                         T * old_pt = NULL,
                         const dfs_btree_lock_hold_t * plock_hold = NULL);
    bool zy_get_smallest_p(T ** tobj,
                           const dfs_rc_indexer_t * prc_indexer) const;
    bool zy_get_largest_p(T ** tobj,
                          const dfs_rc_indexer_t * prc_indexer) const;
    bool zy_get_smaller_p(const T & srct,
                          T ** tagt,
                          dfs_btree_drill_t * pdrill_info,
                          const dfs_rc_indexer_t * prc_indexer) const;
    bool zy_get_larger_p(const T & srct,
                         T ** tagt,
                         dfs_btree_drill_t * pdrill_info,
                         const dfs_rc_indexer_t * prc_indexer) const;
    int zy_get_total_num(const dfs_rc_indexer_t *prc_indexer,
                         uint64_t & total_key_num,
                         uint64_t & total_leaf_node_num,
                         uint64_t & total_mod_node_num) const;
////////////////////////////////////////////////////////////
};


template<typename T, typename K, uint32_t ROW_SIZE>
int dfs_xz_btree_t<T, K, ROW_SIZE>::init(void) {
    //const dfs_bt_root_t * pold_wr_bt_root = NULL;
    dfs_bt_root_t new_wr_bt_root;
    //dfs_rc_indexer_t old_rc_indexer;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    if (_init_state.is_not_init_yet()) {
        _is_batch_mutate_mode = 0;
        _reserved = 0;

        if ((err = dfs_btree_lock_t::init()) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "dfs_btree_lock_t::init() returns 0x%x", err);
        } else if ((err = t_ary_t::init()) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "t_ary_t::init() returns 0x%x", err);
        } else if ((err = _rc_root_set.init((dfs_btree_t *)this)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_rc_root_set::init() returns 0x%x", err);
        } else if ((err = _dfs_xz_btree_t<DU, K, ROW_SIZE>::init(
                        DEF_BT_INSTANCE_POS,
                        new_wr_bt_root)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level,
                            "_dfs_xz_btree_t<T, K, ROW_SIZE>::init() returns 0x%x",
                            err);
        }
        //把rc_root_set_t的当前indexer设置为修改后的root
        else if((err = this->_update_wr_root_indexer(new_wr_bt_root)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_update_wr_root_indexer() returns 0x%x", err);
        }
#ifdef  TEST_RWLOCK_TIME
        //for test only
        else if ((err = pthread_rwlock_init(&_test_locker, NULL)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "pthread_rwlock_init(&_test_locker,NULL) returns 0x%x", err);
        }
#endif //#ifdef  TEST_RWLOCK_TIME

        _init_state.set_init_result(err);
    } else {
        err = _init_state.get_init_result();
    }

    return err;
};


//对当前的wr_root进行快照(需要更新_max_cow_mutation_counter)
template<typename T, typename K, uint32_t ROW_SIZE>
inline int dfs_xz_btree_t<T, K, ROW_SIZE>::_snapshot_wr_root_indexer(
    dfs_rc_indexer_t & rc_indexer) const {
    const dfs_bt_root_t * pbt_root = NULL;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    //更新_max_cow_mutation_counter
    this->_xz_update_max_cow_mutation_counter();

    if ((err = _rc_root_set.retrieve_cur_index(rc_indexer)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_rc_root_set.retrieve_cur_index() returns 0x%x", err);
    } else if ((err = _get_root(rc_indexer, pbt_root)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_get_root() returns 0x%x", err);
    } else if (NULL == pbt_root) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "NULL == pbt_root");
        err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
    } else {
        ;
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "rc_indexer._index=%lu, pbt_root=0x%p",
                        ((dfs_rc_indexer_ext_t *)&rc_indexer)->get_index(), pbt_root);
    }

    return err;
};

//获得当前wr_root，但不进行快照(也不更新_max_cow_mutation_counter)
template<typename T, typename K, uint32_t ROW_SIZE>
inline int dfs_xz_btree_t<T, K, ROW_SIZE>::_retrieve_wr_root_indexer(
    dfs_rc_indexer_t & rc_indexer,
    const dfs_bt_root_t *& pbt_root) const

{
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    if ((err = _rc_root_set.retrieve_cur_index(rc_indexer)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_rc_root_set.retrieve_cur_index() returns 0x%x", err);
    } else if ((err = _get_root(rc_indexer, pbt_root)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_get_root() returns 0x%x", err);
    } else {
        ;
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "rc_indexer._index=%lu, pbt_root=0x%p",
                        ((dfs_rc_indexer_ext_t *)&rc_indexer)->get_index(), pbt_root);
    }

    return err;
};


//进行修改(insert/update...)后更新当前wr_root
template<typename T, typename K, uint32_t ROW_SIZE>
inline int dfs_xz_btree_t<T, K, ROW_SIZE>::_update_wr_root_indexer(const dfs_bt_root_t & bt_root) {
    dfs_rc_indexer_t new_rc_indexer;
    dfs_rc_indexer_t old_rc_indexer;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    if ((err = _rc_root_set.acquire_t(bt_root, new_rc_indexer)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_rc_root_set.update_cur_index() returns 0x%x", err);
    } else if ((err = _rc_root_set.update_cur_index(new_rc_indexer, old_rc_indexer)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_rc_root_set.update_cur_index() returns 0x%x", err);
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level, "new_rc_indexer._index=%lu, old_rc_indexer._index=%lu",
                        ((const dfs_rc_indexer_ext_t *)&new_rc_indexer)->get_index(),
                        ((const dfs_rc_indexer_ext_t *)&old_rc_indexer)->get_index());
    }

    return err;
};

//根据indexer获得dfs_bt_root
template<typename T, typename K, uint32_t ROW_SIZE>
inline int dfs_xz_btree_t<T, K, ROW_SIZE>::_get_root(
    const dfs_rc_indexer_t & rc_indexer,
    const dfs_bt_root_t *& pbt_root) const {
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    if (!rc_indexer.is_valid()) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "!rc_indexer.is_valid()");
        err = DF_BT_SET_INTERNAL_ERR(this->ERRNO_BT_INVALID_INDEX);
    } else if ((err = _rc_root_set.get_t_unit(rc_indexer, pbt_root)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_rc_root_set.get_t_unit() returns 0x%x", err);
    } else if (NULL == pbt_root) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "NULL == pbt_root");
        err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "rc_indexer._index=%lu, pbt_root=0x%p",
                        ((dfs_rc_indexer_ext_t *)&rc_indexer)->get_index(), pbt_root);
    }

    return err;
};

//增加/减少根节点引用计数
template<typename T, typename K, uint32_t ROW_SIZE>
int dfs_xz_btree_t<T, K, ROW_SIZE>::vir_inc_root_ref_counter(
    const dfs_bt_root_t & bt_root) const {
    return this->_bt_inc_root_ref_counter(bt_root);
};
template<typename T, typename K, uint32_t ROW_SIZE>
int dfs_xz_btree_t<T, K, ROW_SIZE>::vir_dec_root_ref_counter(
    const dfs_bt_root_t & bt_root) const {
    return this->_bt_dec_root_ref_counter(bt_root);
};


//  功能：搜索一个项，存在则返回true，否则返回false。如果pt不为空，则存在时对应项的值被复制到*pt
//        prc_indexer非空则指向一个根节点，操作在该根节点进行，否则在当前根节点进行；
//        如果search()及其他读操作的dfs_rc_indexer_t非空，则修改操作适用batch mode
template<typename T, typename K, uint32_t ROW_SIZE>
bool dfs_xz_btree_t<T, K, ROW_SIZE>::search(
    const K & key,
    T & tagt,
    const dfs_rc_indexer_t * prc_indexer) const {
    dfs_rc_indexer_t rc_indexer;
    const dfs_bt_root_t * pbt_root = NULL;
    dfs_btree_drill_t drill_info;
    PCDU pcdu = NULL;
    uint64_t obj_index = NULL_INDEX;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

#ifdef  TEST_RWLOCK_TIME
    //for test only
    if ((err = pthread_rwlock_rdlock((pthread_rwlock_t *)&_test_locker)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "pthread_rdlock_wrlock(&_test_locker) returns 0x%x", err);
    }
#endif //#ifdef  TEST_RWLOCK_TIME


    tagt.init();

#ifdef  TEST_RWLOCK_TIME
    //for test only
    if ((err = pthread_rwlock_unlock((pthread_rwlock_t *)&_test_locker)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "pthread_rwlock_unlock(&_test_locker) returns 0x%x", err);
    }
#endif //#ifdef  TEST_RWLOCK_TIME

    if (NULL == prc_indexer || !(prc_indexer->is_valid())) {
        if (get_batch_mutate_mode()) {
            log_level = DF_UL_LOG_WARNING;
            DF_WRITE_LOG_US(log_level, "get_batch_mutate_mode() is true but NULL == prc_indexer");
            err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = _snapshot_wr_root_indexer(rc_indexer)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_snapshot_wr_root_indexer() returns 0x%x", err);
        } else {
            prc_indexer = &rc_indexer;
        }
    } else if(BAIDU_UNLIKELY(!_rc_root_set.is_valid_in_use_indexer(*prc_indexer))) {
        log_level = DF_UL_LOG_WARNING;
        DF_WRITE_LOG_US(log_level, "!_rc_root_set.is_valid_in_use_indexer(*prc_indexer)");
        err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
    }

    if (0 == err) {
        if ((err = _get_root(*prc_indexer, pbt_root)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_root() returns 0x%x", err);
        } else if (NULL == pbt_root) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "NULL == pbt_root");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        }
        if (BAIDU_UNLIKELY((err = this->_search_by_key(*pbt_root,
                                  (const void *)&key,
                                  drill_info,
                                  obj_index)) != 0)) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_search_by_key() returns 0x%x", err);
        }
        //object existed
        else if (UNDEF_INDEX != obj_index) {
            if ((err = _get_exist_obj(obj_index, pcdu)) != 0 || NULL == pcdu) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_get_exist_obj(obj_index...) returns 0x%x", err);
                err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
            } else {
                tagt = *((T *)pcdu);
            }
        }
        //object not existed
        else {
        }
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "obj_index=0x%lx, &key=0x%p, drill_info.get_drill_ptr()=%d",
                        obj_index, &key, drill_info.get_drill_ptr());
        DF_WRITE_LOG_US(log_level,
                        "prc_indexer=%p, prc_indexer->_index=%lu",
                        prc_indexer,
                        (NULL == prc_indexer) ?
                        UNDEF_INDEX : ((dfs_rc_indexer_ext_t *)prc_indexer)->get_index());
    }

    return (0 == err && UNDEF_INDEX != obj_index);
};

//这个纯粹是为了便于管理..:).和search是一样的..
template<typename T, typename K, uint32_t ROW_SIZE>
bool dfs_xz_btree_t<T, K, ROW_SIZE>::zy_search(
    const K & key,
    T & tagt,
    const dfs_rc_indexer_t * prc_indexer) const {
    dfs_rc_indexer_t rc_indexer;
    const dfs_bt_root_t * pbt_root = NULL;
    dfs_btree_drill_t drill_info;
    PCDU pcdu = NULL;
    uint64_t obj_index = NULL_INDEX;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

#ifdef  TEST_RWLOCK_TIME
    //for test only
    if ((err = pthread_rwlock_rdlock((pthread_rwlock_t *)&_test_locker)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "pthread_rdlock_wrlock(&_test_locker) returns 0x%x", err);
    }
#endif //#ifdef  TEST_RWLOCK_TIME


    tagt.init();

#ifdef  TEST_RWLOCK_TIME
    //for test only
    if ((err = pthread_rwlock_unlock((pthread_rwlock_t *)&_test_locker)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "pthread_rwlock_unlock(&_test_locker) returns 0x%x", err);
    }
#endif //#ifdef  TEST_RWLOCK_TIME

    if (NULL == prc_indexer || !(prc_indexer->is_valid())) {
        if (get_batch_mutate_mode()) {
            log_level = DF_UL_LOG_WARNING;
            DF_WRITE_LOG_US(log_level, "get_batch_mutate_mode() is true but NULL == prc_indexer");
            err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = _snapshot_wr_root_indexer(rc_indexer)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_snapshot_wr_root_indexer() returns 0x%x", err);
        } else {
            prc_indexer = &rc_indexer;
        }
    } else if(!_rc_root_set.is_valid_in_use_indexer(*prc_indexer)) {
        log_level = DF_UL_LOG_WARNING;
        DF_WRITE_LOG_US(log_level, "!_rc_root_set.is_valid_in_use_indexer(*prc_indexer)");
        err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
    }

    if (0 == err) {
        if ((err = _get_root(*prc_indexer, pbt_root)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_root() returns 0x%x", err);
        } else if (NULL == pbt_root) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "NULL == pbt_root");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        }
        if ((err = this->_search_by_key(
                       *pbt_root,
                       (const void *)&key,
                       drill_info,
                       obj_index)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_search_by_key() returns 0x%x", err);
        }
        //object existed
        else if (UNDEF_INDEX != obj_index) {
            if ((err = _get_exist_obj(obj_index, pcdu)) != 0 || NULL == pcdu) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_get_exist_obj(obj_index...) returns 0x%x", err);
                err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
            } else {
                tagt = *((T *)pcdu);
            }
        }
        //object not existed
        else {
        }
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "obj_index=0x%lx, &key=0x%p, drill_info.get_drill_ptr()=%d",
                        obj_index, &key, drill_info.get_drill_ptr());
        DF_WRITE_LOG_US(log_level,
                        "prc_indexer=%p, prc_indexer->_index=%lu",
                        prc_indexer,
                        (NULL == prc_indexer) ?
                        UNDEF_INDEX : ((dfs_rc_indexer_ext_t *)prc_indexer)->get_index());
    }

    return (0 == err && UNDEF_INDEX != obj_index);
};

template<typename T, typename K, uint32_t ROW_SIZE>
bool dfs_xz_btree_t<T, K, ROW_SIZE>::search(
    const T & srct,
    T & tagt,
    const dfs_rc_indexer_t * prc_indexer) const {
    dfs_rc_indexer_t rc_indexer;
    const dfs_bt_root_t * pbt_root = NULL;
    dfs_btree_drill_t drill_info;
    PCDU pcdu = NULL;
    DU duobj(srct);
    uint64_t obj_index = NULL_INDEX;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

#ifdef  TEST_RWLOCK_TIME
    //for test only
    if ((err = pthread_rwlock_rdlock((pthread_rwlock_t *)&_test_locker)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "pthread_rdlock_wrlock(&_test_locker) returns 0x%x", err);
    }
#endif //#ifdef  TEST_RWLOCK_TIME

    tagt.init();

#ifdef  TEST_RWLOCK_TIME
    //for test only
    if ((err = pthread_rwlock_unlock((pthread_rwlock_t *)&_test_locker)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "pthread_rwlock_unlock(&_test_locker) returns 0x%x", err);
    }
#endif //#ifdef  TEST_RWLOCK_TIME

    if (NULL == prc_indexer || !(prc_indexer->is_valid())) {
        if (get_batch_mutate_mode()) {
            log_level = DF_UL_LOG_WARNING;
            DF_WRITE_LOG_US(log_level, "get_batch_mutate_mode() is true but NULL == prc_indexer");
            err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = _snapshot_wr_root_indexer(rc_indexer)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_snapshot_wr_root_indexer() returns 0x%x", err);
        } else {
            prc_indexer = &rc_indexer;
        }
    } else if(!_rc_root_set.is_valid_in_use_indexer(*prc_indexer)) {
        log_level = DF_UL_LOG_WARNING;
        DF_WRITE_LOG_US(log_level, "!_rc_root_set.is_valid_in_use_indexer(*prc_indexer)");
        err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
    }

    if (0 == err) {
        if ((err = _get_root(*prc_indexer, pbt_root)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_root() returns 0x%x", err);
        } else if (NULL == pbt_root) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "NULL == pbt_root");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        }
        if ((err = this->_search_by_obj(
                       *pbt_root,
                       (const void *)&duobj,
                       drill_info,
                       obj_index)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_search_by_obj() returns 0x%x", err);
        }
        //object existed
        else if (UNDEF_INDEX != obj_index) {
            if ((err = _get_exist_obj(obj_index, pcdu)) != 0 || NULL == pcdu) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_get_exist_obj(obj_index...) returns 0x%x", err);
                err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
            } else {
                tagt = *((T *)pcdu);
            }
        }
        //object not existed
        else {
        }
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "obj_index=0x%lx, &srct=0x%p, drill_info.get_drill_ptr()=%d",
                        obj_index, &srct, drill_info.get_drill_ptr());
        DF_WRITE_LOG_US(log_level,
                        "prc_indexer=%p, prc_indexer->_index=%lu",
                        prc_indexer,
                        (NULL == prc_indexer) ?
                        UNDEF_INDEX : ((dfs_rc_indexer_ext_t *)prc_indexer)->get_index());
    }

    return (0 == err && UNDEF_INDEX != obj_index);
};


//  功能：is_batch_mutate_mode为true则允许batch mutation，seacrh等读操作时
//        需要提供有效的dfs_rc_indexer_t；为false时禁止batch mutation，
//        search等操作可以提供空的dfs_rc_indexer_t。
//  返回：0 for success, other values for error
template<typename T, typename K, uint32_t ROW_SIZE>
int dfs_xz_btree_t<T, K, ROW_SIZE>::set_batch_mutate_mode(
    const bool is_batch_mutate_mode,
    const dfs_btree_lock_hold_t * plock_hold) {
    dfs_btree_lock_hold_t lock_hold;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;
    int err1 = 0;

    if ((err = BT_MUTATE_LOCK_ACQUIRE(plock_hold, lock_hold, 0)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_ACQUIRE_ERR_INFO);
    } else {
        //如果是非batch mutate mode，则max_cow_mutation_counter需要更新到
        //当前mutation_counter以免读对象被原子操作修改。
        if (!is_batch_mutate_mode) {
            this->_xz_update_max_cow_mutation_counter();
        }

        df_atomic_exchange(&_is_batch_mutate_mode, (is_batch_mutate_mode) ? 1 : 0);

        if ((err1 = BT_MUTATE_LOCK_RELEASE(plock_hold, lock_hold)) != 0) {
            err = err1;
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_RELEASE_ERR_INFO);
        }
    }

    return err;
};

template<typename T, typename K, uint32_t ROW_SIZE>
bool dfs_xz_btree_t<T, K, ROW_SIZE>::get_batch_mutate_mode(void) const {
    return (0 != _is_batch_mutate_mode);
};


//  功能：把项tobj插入或更新到B树中。
//        insert：如果T的id为UNDEF_ID则为其申请一个新ID。
//                如果T的id在B树中不存在，则加入B树，否则不做操作并返回UNDEF_ID。
//        update：如果T的id在B树存在就更新其值为tobj，否则返回UNDEF_ID。
//        insert_update：先执行insert，如果该项存在就再执行update。
//        如果项存在且pt非空，则其原值被复制到*pt中
//  输入：项tobj
//  返回：项的id(操作成功)或UNDEF_ID
template<typename T, typename K, uint32_t ROW_SIZE>
bool dfs_xz_btree_t<T, K, ROW_SIZE>::insert(
    const T & tobj,
    T * old_pt,
    const dfs_btree_lock_hold_t * plock_hold) {
    const dfs_bt_root_t * pold_wr_bt_root = NULL;
    dfs_bt_root_t new_wr_bt_root;
    dfs_rc_indexer_t old_rc_indexer;
    DU new_duobj(tobj);
    DU old_duobj;
    bool is_old_exist = false;
    dfs_btree_lock_hold_t lock_hold;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;
    int err1 = 0;

    old_duobj.init();

    if ((err = BT_MUTATE_LOCK_ACQUIRE(plock_hold, lock_hold, 0)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_ACQUIRE_ERR_INFO);
    } else {
        //如果不是batch mode，则同时更新_max_cow_mutation_counter从而禁止修改时的指针原子替换
        this->_xz_update_mutation_counter(get_batch_mutate_mode());

        if ((err = _retrieve_wr_root_indexer(old_rc_indexer, pold_wr_bt_root)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_retrieve_wr_root_indexer() returns 0x%x", err);
        } else if (NULL == pold_wr_bt_root) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "NULL == pold_wr_bt_root");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else {
            new_wr_bt_root = *pold_wr_bt_root;
            if ((err = this->_xz_insert(
                           *pold_wr_bt_root,
                           new_wr_bt_root,
                           new_duobj,
                           &old_duobj,
                           is_old_exist)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_insert() returns 0x%x", err);
            }
            //原来项存在
            else if (is_old_exist) {
            }
            //原来项不存在
            else {
                //把rc_root_set_t的当前indexer设置为修改后的root
                if((err = this->_update_wr_root_indexer(new_wr_bt_root)) != 0) {
                    log_level = DF_UL_LOG_FATAL;
                    DF_WRITE_LOG_US(log_level, "_update_wr_root_indexer() returns 0x%x", err);
                }
            }
        }

        if ((err1 = BT_MUTATE_LOCK_RELEASE(plock_hold, lock_hold)) != 0) {
            err = err1;
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_RELEASE_ERR_INFO);
        }
    }

    if (NULL != old_pt) {
        *old_pt = *((T *)&old_duobj);
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "old_rc_indexer._index=%lu",
                        ((dfs_rc_indexer_ext_t *)&old_rc_indexer)->get_index());
        DF_WRITE_LOG_US(log_level, "batch_mutate_mode=%d", get_batch_mutate_mode());
    }


    return (0 == err && !is_old_exist);
};

//修改原来的insert接口..
//返回值使用int表示,bool已经不够了..
//我们需要知道
//1.内部错误,-1
//2.原来的值存在,0
//3.原来的值不存在,1
template<typename T, typename K, uint32_t ROW_SIZE>
int dfs_xz_btree_t<T, K, ROW_SIZE>::zy_insert(
    const T & tobj,
    T * old_pt,
    const dfs_btree_lock_hold_t * plock_hold) {
    const dfs_bt_root_t * pold_wr_bt_root = NULL;
    dfs_bt_root_t new_wr_bt_root;
    dfs_rc_indexer_t old_rc_indexer;
    DU new_duobj(tobj);
    DU old_duobj;
    bool is_old_exist = false;
    dfs_btree_lock_hold_t lock_hold;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;
    int err1 = 0;

    old_duobj.init();

    if ((err = BT_MUTATE_LOCK_ACQUIRE(plock_hold, lock_hold, 0)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_ACQUIRE_ERR_INFO);
    } else {
        //如果不是batch mode，则同时更新_max_cow_mutation_counter从而禁止修改时的指针原子替换
        this->_xz_update_mutation_counter(get_batch_mutate_mode());

        if ((err = _retrieve_wr_root_indexer(old_rc_indexer, pold_wr_bt_root)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_retrieve_wr_root_indexer() returns 0x%x", err);
        } else if (NULL == pold_wr_bt_root) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "NULL == pold_wr_bt_root");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else {
            new_wr_bt_root = *pold_wr_bt_root;
            if ((err = this->_xz_insert(
                           *pold_wr_bt_root,
                           new_wr_bt_root,
                           new_duobj,
                           &old_duobj,
                           is_old_exist)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_insert() returns 0x%x", err);
            }
            //原来项存在
            else if (is_old_exist) {
            }
            //原来项不存在
            else {
                //把rc_root_set_t的当前indexer设置为修改后的root
                if((err = this->_update_wr_root_indexer(new_wr_bt_root)) != 0) {
                    log_level = DF_UL_LOG_FATAL;
                    DF_WRITE_LOG_US(log_level, "_update_wr_root_indexer() returns 0x%x", err);
                }
            }
        }

        if ((err1 = BT_MUTATE_LOCK_RELEASE(plock_hold, lock_hold)) != 0) {
            err = err1;
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_RELEASE_ERR_INFO);
        }
    }

    if (NULL != old_pt) {
        *old_pt = *((T *)&old_duobj);
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "old_rc_indexer._index=%lu",
                        ((dfs_rc_indexer_ext_t *)&old_rc_indexer)->get_index());
        DF_WRITE_LOG_US(log_level, "batch_mutate_mode=%d", get_batch_mutate_mode());
    }

    //如果存在错误..
    if(err!=0) {
        return -1;
    }
    if(is_old_exist) {
        return 0;
    } else {
        return 1;
    }
    //return (0 == err && !is_old_exist);
};

template<typename T, typename K, uint32_t ROW_SIZE>
bool dfs_xz_btree_t<T, K, ROW_SIZE>::update(
    const T & tobj,
    T * old_pt,
    const dfs_btree_lock_hold_t * plock_hold) {
    const dfs_bt_root_t * pold_wr_bt_root = NULL;
    dfs_bt_root_t new_wr_bt_root;
    dfs_rc_indexer_t old_rc_indexer;
    DU new_duobj(tobj);
    DU old_duobj;
    bool is_old_exist = false;
    dfs_btree_lock_hold_t lock_hold;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;
    int err1 = 0;

    old_duobj.init();

    if ((err = BT_MUTATE_LOCK_ACQUIRE(plock_hold, lock_hold, 0)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_ACQUIRE_ERR_INFO);
    } else {
        //如果不是batch mode，则同时更新_max_cow_mutation_counter从而禁止修改时的指针原子替换
        this->_xz_update_mutation_counter(get_batch_mutate_mode());

        if ((err = _retrieve_wr_root_indexer(old_rc_indexer, pold_wr_bt_root)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_retrieve_wr_root_indexer() returns 0x%x", err);
        } else if (NULL == pold_wr_bt_root) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "NULL == pold_wr_bt_root");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else {
            new_wr_bt_root = *pold_wr_bt_root;
            if ((err = this->_xz_update(
                           *pold_wr_bt_root,
                           new_wr_bt_root,
                           new_duobj,
                           &old_duobj,
                           is_old_exist)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_update() returns 0x%x", err);
            }
            //原来项存在
            else if (is_old_exist) {
                //把rc_root_set_t的当前indexer设置为修改后的root
                if((err = this->_update_wr_root_indexer(new_wr_bt_root)) != 0) {
                    log_level = DF_UL_LOG_FATAL;
                    DF_WRITE_LOG_US(log_level, "_update_wr_root_indexer() returns 0x%x", err);
                }
            }
            //原来项不存在
            else {
                ;
            }
        }

        if ((err1 = BT_MUTATE_LOCK_RELEASE(plock_hold, lock_hold)) != 0) {
            err = err1;
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_RELEASE_ERR_INFO);
        }
    }

    if (NULL != old_pt) {
        *old_pt = *((T *)&old_duobj);
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "old_rc_indexer._index=%lu",
                        ((dfs_rc_indexer_ext_t *)&old_rc_indexer)->get_index());
        DF_WRITE_LOG_US(log_level, "get_batch_mutate_mode()=%d", get_batch_mutate_mode());
    }


    return (0 == err && is_old_exist);
};

template<typename T, typename K, uint32_t ROW_SIZE>
bool dfs_xz_btree_t<T, K, ROW_SIZE>::insert_update(
    const T & tobj,
    T * old_pt,
    const dfs_btree_lock_hold_t * plock_hold) {
    const dfs_bt_root_t * pold_wr_bt_root = NULL;
    dfs_bt_root_t new_wr_bt_root;
    dfs_rc_indexer_t old_rc_indexer;
    DU new_duobj(tobj);
    DU old_duobj;
    bool is_old_exist = false;
    dfs_btree_lock_hold_t lock_hold;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;
    int err1 = 0;

    old_duobj.init();

    if ((err = BT_MUTATE_LOCK_ACQUIRE(plock_hold, lock_hold, 0)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_ACQUIRE_ERR_INFO);
    } else {
        //如果不是batch mode，则同时更新_max_cow_mutation_counter从而禁止修改时的指针原子替换
        this->_xz_update_mutation_counter(get_batch_mutate_mode());

        if ((err = _retrieve_wr_root_indexer(old_rc_indexer, pold_wr_bt_root)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_retrieve_wr_root_indexer() returns 0x%x", err);
        } else if (NULL == pold_wr_bt_root) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "NULL == pold_wr_bt_root");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else {
            new_wr_bt_root = *pold_wr_bt_root;
            if ((err = this->_xz_insert_update(
                           *pold_wr_bt_root,
                           new_wr_bt_root,
                           new_duobj,
                           &old_duobj,
                           is_old_exist)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_update() returns 0x%x", err);
            } else if((err = this->_update_wr_root_indexer(new_wr_bt_root)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_update_wr_root_indexer() returns 0x%x", err);
            } else {
                ;
            }
        }

        if ((err1 = BT_MUTATE_LOCK_RELEASE(plock_hold, lock_hold)) != 0) {
            err = err1;
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_RELEASE_ERR_INFO);
        }
    }

    if (NULL != old_pt) {
        *old_pt = *((T *)&old_duobj);
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "old_rc_indexer._index=%lu",
                        ((dfs_rc_indexer_ext_t *)&old_rc_indexer)->get_index());
        DF_WRITE_LOG_US(log_level, "get_batch_mutate_mode()=%d", get_batch_mutate_mode());
    }


    return (0 == err);
};

//内部错误-1
//如果原来值存在,那么返回0
//如果原来值不存在,那么返回1
template<typename T, typename K, uint32_t ROW_SIZE>
int dfs_xz_btree_t<T, K, ROW_SIZE>::zy_insert_update(
    const T & tobj,
    T * old_pt,
    const dfs_btree_lock_hold_t * plock_hold) {
    const dfs_bt_root_t * pold_wr_bt_root = NULL;
    dfs_bt_root_t new_wr_bt_root;
    dfs_rc_indexer_t old_rc_indexer;
    DU new_duobj(tobj);
    DU old_duobj;
    bool is_old_exist = false;
    dfs_btree_lock_hold_t lock_hold;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;
    int err1 = 0;

    old_duobj.init();

    if ((err = BT_MUTATE_LOCK_ACQUIRE(plock_hold, lock_hold, 0)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_ACQUIRE_ERR_INFO);
    } else {
        //如果不是batch mode，则同时更新_max_cow_mutation_counter从而禁止修改时的指针原子替换
        this->_xz_update_mutation_counter(get_batch_mutate_mode());

        if ((err = _retrieve_wr_root_indexer(old_rc_indexer, pold_wr_bt_root)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_retrieve_wr_root_indexer() returns 0x%x", err);
        } else if (NULL == pold_wr_bt_root) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "NULL == pold_wr_bt_root");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else {
            new_wr_bt_root = *pold_wr_bt_root;
            if ((err = this->_xz_insert_update(
                           *pold_wr_bt_root,
                           new_wr_bt_root,
                           new_duobj,
                           &old_duobj,
                           is_old_exist)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_update() returns 0x%x", err);
            } else if((err = this->_update_wr_root_indexer(new_wr_bt_root)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_update_wr_root_indexer() returns 0x%x", err);
            } else {
                ;
            }
        }

        if ((err1 = BT_MUTATE_LOCK_RELEASE(plock_hold, lock_hold)) != 0) {
            err = err1;
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_RELEASE_ERR_INFO);
        }
    }

    if (NULL != old_pt) {
        *old_pt = *((T *)&old_duobj);
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "old_rc_indexer._index=%lu",
                        ((dfs_rc_indexer_ext_t *)&old_rc_indexer)->get_index());
        DF_WRITE_LOG_US(log_level, "get_batch_mutate_mode()=%d", get_batch_mutate_mode());
    }

    if(err!=0) {
        return -1;
    }
    if(is_old_exist) {
        return 0;
    } else {
        return 1;
    }
    //return (0 == err);
};

//  功能：如果一个项存在，则删除；否则，不做任何事情。
//        如果项存在且pt非空，则其原值被复制到*pt中
//  返回：true如果删除成功，false如果删除失败(例如项不存在)
template<typename T, typename K, uint32_t ROW_SIZE>
bool dfs_xz_btree_t<T, K, ROW_SIZE>::del(
    const K & key,
    T * old_pt,
    const dfs_btree_lock_hold_t * plock_hold) {
    const dfs_bt_root_t * pold_wr_bt_root = NULL;
    dfs_bt_root_t new_wr_bt_root;
    dfs_rc_indexer_t old_rc_indexer;
    DU old_duobj;
    bool is_old_exist = false;
    dfs_btree_lock_hold_t lock_hold;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;
    int err1 = 0;

    old_duobj.init();

    if ((err = BT_MUTATE_LOCK_ACQUIRE(plock_hold, lock_hold, 0)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_ACQUIRE_ERR_INFO);
    } else {
        //如果不是batch mode，则同时更新_max_cow_mutation_counter从而禁止修改时的指针原子替换
        this->_xz_update_mutation_counter(get_batch_mutate_mode());

        if ((err = _retrieve_wr_root_indexer(old_rc_indexer, pold_wr_bt_root)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_retrieve_wr_root_indexer() returns 0x%x", err);
        } else if (NULL == pold_wr_bt_root) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "NULL == pold_wr_bt_root");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else {
            new_wr_bt_root = *pold_wr_bt_root;
            if((err = this->_xz_del(
                          *pold_wr_bt_root,
                          new_wr_bt_root,
                          key,
                          &old_duobj,
                          is_old_exist)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_update() returns 0x%x", err);
            }
            //原来项存在
            else if (is_old_exist) {
                //把rc_root_set_t的当前indexer设置为修改后的root
                if((err = this->_update_wr_root_indexer(new_wr_bt_root)) != 0) {
                    log_level = DF_UL_LOG_FATAL;
                    DF_WRITE_LOG_US(log_level, "_update_wr_root_indexer() returns 0x%x", err);
                }
            }
            //原来项不存在
            else {
            }
        }

        if ((err1 = BT_MUTATE_LOCK_RELEASE(plock_hold, lock_hold)) != 0) {
            err = err1;
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_RELEASE_ERR_INFO);
        }
    }

    if (NULL != old_pt) {
        *old_pt = *((T *)&old_duobj);
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "old_rc_indexer._index=%lu",
                        ((dfs_rc_indexer_ext_t *)&old_rc_indexer)->get_index());
        DF_WRITE_LOG_US(log_level, "get_batch_mutate_mode()=%d", get_batch_mutate_mode());
    }


    return (0 == err && is_old_exist);
};
template<typename T, typename K, uint32_t ROW_SIZE>
bool dfs_xz_btree_t<T, K, ROW_SIZE>::del(
    const T & tobj,
    T * old_pt,
    const dfs_btree_lock_hold_t * plock_hold) {
    const dfs_bt_root_t * pold_wr_bt_root = NULL;
    dfs_bt_root_t new_wr_bt_root;
    dfs_rc_indexer_t old_rc_indexer;
    DU old_duobj;
    bool is_old_exist = false;
    dfs_btree_lock_hold_t lock_hold;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;
    int err1 = 0;

    old_duobj.init();

    if ((err = BT_MUTATE_LOCK_ACQUIRE(plock_hold, lock_hold, 0)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_ACQUIRE_ERR_INFO);
    } else {
        //如果不是batch mode，则同时更新_max_cow_mutation_counter从而禁止修改时的指针原子替换
        this->_xz_update_mutation_counter(get_batch_mutate_mode());

        if ((err = _retrieve_wr_root_indexer(old_rc_indexer, pold_wr_bt_root)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_retrieve_wr_root_indexer() returns 0x%x", err);
        } else if (NULL == pold_wr_bt_root) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "NULL == pold_wr_bt_root");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else {
            new_wr_bt_root = *pold_wr_bt_root;
            if ((err = this->_xz_del(
                           *pold_wr_bt_root,
                           new_wr_bt_root,
                           tobj,
                           &old_duobj,
                           is_old_exist)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_update() returns 0x%x", err);
            }
            //原来项存在
            else if (is_old_exist) {
                //把rc_root_set_t的当前indexer设置为修改后的root
                if((err = this->_update_wr_root_indexer(new_wr_bt_root)) != 0) {
                    log_level = DF_UL_LOG_FATAL;
                    DF_WRITE_LOG_US(log_level, "_update_wr_root_indexer() returns 0x%x", err);
                }
            }
            //原来项不存在
            else {
            }
        }

        if ((err1 = BT_MUTATE_LOCK_RELEASE(plock_hold, lock_hold)) != 0) {
            err = err1;
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_RELEASE_ERR_INFO);
        }
    }

    if (NULL != old_pt) {
        *old_pt = *((T *)&old_duobj);
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "old_rc_indexer._index=%lu",
                        ((dfs_rc_indexer_ext_t *)&old_rc_indexer)->get_index());
        DF_WRITE_LOG_US(log_level, "get_batch_mutate_mode()=%d", get_batch_mutate_mode());
    }


    return (0 == err && is_old_exist);
};

//  功能：清空整个树。
//  返回：0 for success, other values for error
template<typename T, typename K, uint32_t ROW_SIZE>
int dfs_xz_btree_t<T, K, ROW_SIZE>::clear(
    const dfs_btree_lock_hold_t * plock_hold) {
    const dfs_bt_root_t * pold_wr_bt_root = NULL;
    dfs_bt_root_t new_wr_bt_root;
    dfs_rc_indexer_t old_rc_indexer;
    dfs_btree_lock_hold_t lock_hold;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;
    int err1 = 0;

    if ((err = BT_MUTATE_LOCK_ACQUIRE(plock_hold, lock_hold, 0)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_ACQUIRE_ERR_INFO);
    } else {
        //如果不是batch mode，则同时更新_max_cow_mutation_counter从而禁止修改时的指针原子替换
        this->_xz_update_mutation_counter(false); //get_batch_mutate_mode()

        if ((err = _retrieve_wr_root_indexer(old_rc_indexer, pold_wr_bt_root)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_retrieve_wr_root_indexer() returns 0x%x", err);
        } else if (NULL == pold_wr_bt_root) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "NULL == pold_wr_bt_root");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else {
            //先复制根节点以便获得mutation counter, next_allocate_id等信息
            new_wr_bt_root = *pold_wr_bt_root;
            if ((err = this->_acquire_empty_root(new_wr_bt_root)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_acquire_empty_root() returns 0x%x", err);
            }
            //把rc_root_set_t的当前indexer设置为修改后的root
            else if((err = this->_update_wr_root_indexer(new_wr_bt_root)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_update_wr_root_indexer() returns 0x%x", err);
            }
        }

        if ((err1 = BT_MUTATE_LOCK_RELEASE(plock_hold, lock_hold)) != 0) {
            err = err1;
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_RELEASE_ERR_INFO);
        }
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "old_rc_indexer._index=%lu",
                        ((dfs_rc_indexer_ext_t *)&old_rc_indexer)->get_index());
    }


    return err;
};


//获得当前B树的一个快照
template<typename T, typename K, uint32_t ROW_SIZE>
int dfs_xz_btree_t<T, K, ROW_SIZE>::snapshot(
    dfs_rc_indexer_t & rc_indexer,
    const dfs_btree_lock_hold_t * plock_hold) {
    dfs_btree_lock_hold_t lock_hold;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;
    int err1 = 0;

    if ((err = BT_MUTATE_LOCK_ACQUIRE(plock_hold, lock_hold, 0)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_ACQUIRE_ERR_INFO);
    } else {
        //把max_cow_mutation_counter更新到当前mutation_counter以免对象被原子操作修改。
        this->_xz_update_max_cow_mutation_counter();

        if ((err = _snapshot_wr_root_indexer(rc_indexer)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_snapshot_wr_root_indexer() returns 0x%x", err);
        }

        if ((err1 = BT_MUTATE_LOCK_RELEASE(plock_hold, lock_hold)) != 0) {
            err = err1;
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_RELEASE_ERR_INFO);
        }
    }


    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "rc_indexer._index=%lu",
                        ((dfs_rc_indexer_ext_t *)&rc_indexer)->get_index());
    }


    return err;
};



//  return: 0 for success, other values for error
template<typename T, typename K, uint32_t ROW_SIZE>
int dfs_xz_btree_t<T, K, ROW_SIZE>::get_total_num(
    uint64_t & total_key_num,
    uint64_t & total_leaf_node_num,
    uint64_t & total_mid_node_num) const {
    const dfs_bt_root_t * pold_wr_bt_root = NULL;
    //dfs_bt_root_t * pnew_wr_bt_root = NULL;
    dfs_rc_indexer_t old_rc_indexer;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    if ((err = _retrieve_wr_root_indexer(old_rc_indexer, pold_wr_bt_root)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_retrieve_wr_root_indexer() returns 0x%x", err);
    } else if (NULL == pold_wr_bt_root) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "NULL == pold_wr_bt_root");
        err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
    } else {
        if ((err = this->bt_get_total_num(
                       *pold_wr_bt_root,
                       total_key_num,
                       total_leaf_node_num,
                       total_mid_node_num)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "bt_get_total_num() returns 0x%x", err);
        }
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "old_rc_indexer._index=%lu",
                        ((dfs_rc_indexer_ext_t *)&old_rc_indexer)->get_index());
    }


    return err;
};

//zhangyan04@baidu.com
//原始的get_total_num之能够获得当前write的root的内容
//但是有时候对于snapshot的btree也需要..
template<typename T, typename K, uint32_t ROW_SIZE>
int dfs_xz_btree_t<T, K, ROW_SIZE>::zy_get_total_num(
    const dfs_rc_indexer_t * prc_indexer,
    uint64_t & total_key_num,
    uint64_t & total_leaf_node_num,
    uint64_t & total_mid_node_num) const {
    const dfs_bt_root_t * pbt_root = NULL;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;
    //根据snapshot的index获得root,然后去统计..
    if((err=_get_root(*prc_indexer,pbt_root))!=0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "_get_root() returns 0x%x", err);
    } else if (NULL == pbt_root) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "NULL == pbt_root");
        err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
    } else {
        if ((err = this->bt_get_total_num(
                       *pbt_root,
                       total_key_num,
                       total_leaf_node_num,
                       total_mid_node_num)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "bt_get_total_num() returns 0x%x", err);
        }
    }
    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "prc_indexer->_index=%lu",
                        ((dfs_rc_indexer_ext_t *)prc_indexer)->get_index());
    }
    return err;
};

//  return: 0 for success, other values for error
template<typename T, typename K, uint32_t ROW_SIZE>
int dfs_xz_btree_t<T, K, ROW_SIZE>::get_in_use_num_in_2d_ary(
    uint64_t & in_use_key_num,
    uint64_t & in_use_leaf_node_num,
    uint64_t & in_use_mid_node_num) const {
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    if ((err = this->xz_get_in_use_num_in_2d_ary(
                   in_use_key_num,
                   in_use_leaf_node_num,
                   in_use_mid_node_num)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, "xz_get_in_use_num_in_2d_ary() returns 0x%x", err);
    }

    return err;
}
//  return: 0 for success, other values for error
template<typename T, typename K, uint32_t ROW_SIZE>
int dfs_xz_btree_t<T, K, ROW_SIZE>::empty_btree_check(
    const bool is_should_empty,
    const dfs_btree_lock_hold_t * plock_hold) const {
    dfs_rc_indexer_t rc_indexer;
    const dfs_bt_root_t * pbt_root = NULL;
    uint64_t key_num_in_btree = 0;
    uint64_t key_num_in_2d_ary = 0;
    uint64_t leaf_node_num_in_btree = 0;
    uint64_t leaf_node_num_in_2d_ary = 0;
    uint64_t mid_node_num_in_btree = 0;
    uint64_t mid_node_num_in_2d_ary = 0;
    dfs_btree_lock_hold_t lock_hold;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;
    int err1 = 0;

    if ((err = BT_MUTATE_LOCK_ACQUIRE(plock_hold, lock_hold, 0)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_ACQUIRE_ERR_INFO);
    } else {
        if ((err = _snapshot_wr_root_indexer(rc_indexer)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_snapshot_wr_root_indexer() returns 0x%x", err);
        } else if ((err = _get_root(rc_indexer, pbt_root)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_root() returns 0x%x", err);
        } else if (NULL == pbt_root) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "NULL == pbt_root");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = this->bt_get_total_num(
                              *pbt_root,
                              key_num_in_btree,
                              leaf_node_num_in_btree,
                              mid_node_num_in_btree)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "bt_get_total_num() returns 0x%x", err);
        } else if ((err = this->xz_get_in_use_num_in_2d_ary(
                              key_num_in_2d_ary,
                              leaf_node_num_in_2d_ary,
                              mid_node_num_in_2d_ary)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "xz_get_in_use_num_in_2d_ary() returns 0x%x", err);
        } else if (is_should_empty) {
            if (_rc_root_set.get_in_use_num() != 1) {
                DFS_BT_DEBUG_TRAP;

                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level,
                                "_rc_root_set.get_in_use_num() (=%ld) != 1",
                                _rc_root_set.get_in_use_num());
                DF_WRITE_LOG_US(log_level,
                                "_ckp_root_indexer=%ld",
                                ((dfs_rc_indexer_ext_t *)&_ckp_root_indexer)->get_index());
                err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_T_LEAK);
            } else if (0 != key_num_in_btree) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "0 != key_num_in_btree");
                err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_NONE_EMPTY_BTREE);
            } else if ((key_num_in_btree+1) != key_num_in_2d_ary) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "(key_num_in_btree+1) == key_num_in_2d_ary");
                err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_DIFF_NUM_IN_BT_2D_ARY);
            }
        }

        if ((err1 = BT_MUTATE_LOCK_RELEASE(plock_hold, lock_hold)) != 0) {
            err = err1;
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_RELEASE_ERR_INFO);
        }
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "is_should_empty=%d",
                        is_should_empty);
        DF_WRITE_LOG_US(log_level,
                        "key_num_in_btree=%ld, key_num_in_2d_ary=%ld",
                        key_num_in_btree, key_num_in_2d_ary);
        DF_WRITE_LOG_US(log_level,
                        "leaf_node_num_in_btree=%ld, leaf_node_num_in_2d_ary=%ld",
                        leaf_node_num_in_btree, leaf_node_num_in_2d_ary);
        DF_WRITE_LOG_US(log_level,
                        "mid_node_num_in_btree=%ld, mid_node_num_in_2d_ary=%ld",
                        mid_node_num_in_btree, mid_node_num_in_2d_ary);
    }

    return err;
};
//  return: 0 for success, other values for error
//  Should be called in locked state and without checkpointing or other COW(load or store)
template<typename T, typename K, uint32_t ROW_SIZE>
int dfs_xz_btree_t<T, K, ROW_SIZE>::sanitary_check(const dfs_btree_lock_hold_t * plock_hold) {
    dfs_rc_indexer_t rc_indexer;
    const dfs_bt_root_t * pbt_root = NULL;
    dfs_btree_lock_hold_t lock_hold;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;
    int err1 = 0;

    if ((err = BT_MUTATE_LOCK_ACQUIRE(plock_hold, lock_hold, 0)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_ACQUIRE_ERR_INFO);
    } else {
        if (_rc_root_set.get_in_use_num() != 1) {
            DFS_BT_DEBUG_TRAP;

            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level,
                            "_rc_root_set.get_in_use_num() (= %ld) != 1",
                            _rc_root_set.get_in_use_num());
            DF_WRITE_LOG_US(log_level,
                            "_ckp_root_indexer=%ld",
                            ((dfs_rc_indexer_ext_t *)&_ckp_root_indexer)->get_index());
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_T_LEAK);
        } else if ((err = _snapshot_wr_root_indexer(rc_indexer)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_snapshot_wr_root_indexer() returns 0x%x", err);
        } else if ((err = _get_root(rc_indexer, pbt_root)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_root() returns 0x%x", err);
        } else if (NULL == pbt_root) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "NULL == pbt_root");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = this->_bt_sanitary_check(*pbt_root)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_bt_sanitary_check() returns 0x%x", err);
        }

        if ((err1 = BT_MUTATE_LOCK_RELEASE(plock_hold, lock_hold)) != 0) {
            err = err1;
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_RELEASE_ERR_INFO);
        }
    }

    return err;
};
template<typename T, typename K, uint32_t ROW_SIZE>
void dfs_xz_btree_t<T, K, ROW_SIZE>::clear_statistic_info(const dfs_btree_lock_hold_t * plock_hold) {
    dfs_btree_lock_hold_t lock_hold;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;
    int err1 = 0;

    if ((err = BT_MUTATE_LOCK_ACQUIRE(plock_hold, lock_hold, 0)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_ACQUIRE_ERR_INFO);
    } else {
        this->_xz_clear_statistic_info();

        if ((err1 = BT_MUTATE_LOCK_RELEASE(plock_hold, lock_hold)) != 0) {
            err = err1;
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_RELEASE_ERR_INFO);
        }
    }

    return;
};
template<typename T, typename K, uint32_t ROW_SIZE>
void dfs_xz_btree_t<T, K, ROW_SIZE>::log_statistic_info(
    const int org_log_level,
    const char * filename,
    const int lineno,
    const char * funcname,
    const dfs_btree_lock_hold_t * plock_hold) const {
    uint64_t mem_size = 0;
    dfs_btree_lock_hold_t lock_hold;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;
    int err1 = 0;

    if ((err = BT_MUTATE_LOCK_ACQUIRE(plock_hold, lock_hold, 0)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_ACQUIRE_ERR_INFO);
    } else {
        this->_xz_log_statistic_info(org_log_level, filename, lineno, funcname);
        mem_size = this->_xz_get_mem_size();
        DF_WRITE_LOG_US(log_level,"xz_btree in-use memory=%luMB", mem_size/1024/1024);

        if ((err1 = BT_MUTATE_LOCK_RELEASE(plock_hold, lock_hold)) != 0) {
            err = err1;
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_RELEASE_ERR_INFO);
        }
    }

    return;
};
template<typename T, typename K, uint32_t ROW_SIZE>
void dfs_xz_btree_t<T, K, ROW_SIZE>::log_debug_info(
    const int org_log_level,
    const char * filename,
    const int lineno,
    const char * funcname,
    const dfs_btree_lock_hold_t * plock_hold) const {
    this->_xz_log_debug_info(org_log_level, filename, lineno, funcname);
};

template<typename T, typename K, uint32_t ROW_SIZE>
uint64_t dfs_xz_btree_t<T, K, ROW_SIZE>::get_mem_size(
    const dfs_btree_lock_hold_t * plock_hold) const {
    uint64_t mem_size = 0;
    dfs_btree_lock_hold_t lock_hold;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;
    int err1 = 0;

    if ((err = BT_MUTATE_LOCK_ACQUIRE(plock_hold, lock_hold, 0)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_ACQUIRE_ERR_INFO);
    } else {
        mem_size = this->_xz_get_mem_size();

        if ((err1 = BT_MUTATE_LOCK_RELEASE(plock_hold, lock_hold)) != 0) {
            err = err1;
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_RELEASE_ERR_INFO);
        }
    }

    return mem_size;
};



//
//  功能：获得B树最小元素
//  输出：如果B树非空，则最小项的值被复制到tobj，否则tobj被初始化
//  返回：最小元素存在则返回true，最小元素不存在(如B树为空)返回false。
template<typename T, typename K, uint32_t ROW_SIZE>
bool dfs_xz_btree_t<T, K, ROW_SIZE>::get_smallest(
    T & tobj,
    const dfs_rc_indexer_t * prc_indexer) const {
    dfs_rc_indexer_t rc_indexer;
    const dfs_bt_root_t * pbt_root = NULL;
    PCDU pcdu = NULL;
    bool is_exist = false;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    tobj.init();

    if (NULL == prc_indexer || !(prc_indexer->is_valid())) {
        if (get_batch_mutate_mode()) {
            log_level = DF_UL_LOG_WARNING;
            DF_WRITE_LOG_US(log_level, "get_batch_mutate_mode() is true but NULL == prc_indexer");
            err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = _snapshot_wr_root_indexer(rc_indexer)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_snapshot_wr_root_indexer() returns 0x%x", err);
        } else {
            prc_indexer = &rc_indexer;
        }
    } else if(!_rc_root_set.is_valid_in_use_indexer(*prc_indexer)) {
        log_level = DF_UL_LOG_WARNING;
        DF_WRITE_LOG_US(log_level, "!_rc_root_set.is_valid_in_use_indexer(*prc_indexer)");
        err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
    }

    if (0 == err) {
        if ((err = _get_root(*prc_indexer, pbt_root)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_root() returns 0x%x", err);
        } else if (NULL == pbt_root) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "NULL == pbt_root");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = _xz_get_smallest(*pbt_root, pcdu)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_xz_get_smallest() returns 0x%x", err);
        } else if (NULL != pcdu) {
            tobj = *((const T *)pcdu);
            is_exist = true;
        }
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "prc_indexer=%p, prc_indexer->_index=%lu",
                        prc_indexer,
                        (NULL == prc_indexer) ?
                        UNDEF_INDEX : ((dfs_rc_indexer_ext_t *)prc_indexer)->get_index());
    }


    return is_exist;
};

//返回T的指针..这个只是在snapshot的状态下面有效..
template<typename T, typename K, uint32_t ROW_SIZE>
bool dfs_xz_btree_t<T, K, ROW_SIZE>::zy_get_smallest_p(
    T ** tobj,
    const dfs_rc_indexer_t * prc_indexer) const {
    dfs_rc_indexer_t rc_indexer;
    const dfs_bt_root_t * pbt_root = NULL;
    PCDU pcdu = NULL;
    bool is_exist = false;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    //tobj.init();

    if (NULL == prc_indexer || !(prc_indexer->is_valid())) {
        if (get_batch_mutate_mode()) {
            log_level = DF_UL_LOG_WARNING;
            DF_WRITE_LOG_US(log_level, "get_batch_mutate_mode() is true but NULL == prc_indexer");
            err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = _snapshot_wr_root_indexer(rc_indexer)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_snapshot_wr_root_indexer() returns 0x%x", err);
        } else {
            prc_indexer = &rc_indexer;
        }
    } else if(!_rc_root_set.is_valid_in_use_indexer(*prc_indexer)) {
        log_level = DF_UL_LOG_WARNING;
        DF_WRITE_LOG_US(log_level, "!_rc_root_set.is_valid_in_use_indexer(*prc_indexer)");
        err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
    }

    if (0 == err) {
        if ((err = _get_root(*prc_indexer, pbt_root)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_root() returns 0x%x", err);
        } else if (NULL == pbt_root) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "NULL == pbt_root");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = _xz_get_smallest(*pbt_root, pcdu)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_xz_get_smallest() returns 0x%x", err);
        } else if (NULL != pcdu) {
            //tobj = *((const T *)pcdu);
            *tobj=(T*)pcdu;
            is_exist = true;
        }
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "prc_indexer=%p, prc_indexer->_index=%lu",
                        prc_indexer,
                        (NULL == prc_indexer) ?
                        UNDEF_INDEX : ((dfs_rc_indexer_ext_t *)prc_indexer)->get_index());
    }


    return is_exist;
};

//  功能：获得B树最大元素
//  输出：如果B树非空，则最大项的值被复制到tobj，否则tobj被初始化
//  返回：最大元素存在则返回true，最大元素不存在(如B树为空)返回false。
template<typename T, typename K, uint32_t ROW_SIZE>
bool dfs_xz_btree_t<T, K, ROW_SIZE>::get_largest(
    T & tobj,
    const dfs_rc_indexer_t * prc_indexer) const {
    dfs_rc_indexer_t rc_indexer;
    const dfs_bt_root_t * pbt_root = NULL;
    PCDU pcdu = NULL;
    bool is_exist = false;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    tobj.init();

    if (NULL == prc_indexer || !(prc_indexer->is_valid())) {
        if (get_batch_mutate_mode()) {
            log_level = DF_UL_LOG_WARNING;
            DF_WRITE_LOG_US(log_level, "get_batch_mutate_mode() is true but NULL == prc_indexer");
            err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = _snapshot_wr_root_indexer(rc_indexer)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_snapshot_wr_root_indexer() returns 0x%x", err);
        } else {
            prc_indexer = &rc_indexer;
        }
    } else if(!_rc_root_set.is_valid_in_use_indexer(*prc_indexer)) {
        log_level = DF_UL_LOG_WARNING;
        DF_WRITE_LOG_US(log_level, "!_rc_root_set.is_valid_in_use_indexer(*prc_indexer)");
        err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
    }

    if (0 == err) {
        if ((err = _get_root(*prc_indexer, pbt_root)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_root() returns 0x%x", err);
        } else if (NULL == pbt_root) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "NULL == pbt_root");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = _xz_get_largest(*pbt_root, pcdu)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_xz_get_largest() returns 0x%x", err);
        } else if (NULL != pcdu) {
            tobj = *((const T *)pcdu);
            is_exist = true;
        }
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "prc_indexer=%p, prc_indexer->_index=%lu",
                        prc_indexer,
                        (NULL == prc_indexer) ?
                        UNDEF_INDEX : ((dfs_rc_indexer_ext_t *)prc_indexer)->get_index());
    }


    return is_exist;
};

//返回tobj的指针,必须保证在snapshot的状态..
template<typename T, typename K, uint32_t ROW_SIZE>
bool dfs_xz_btree_t<T, K, ROW_SIZE>::zy_get_largest_p(
    T ** tobj,
    const dfs_rc_indexer_t * prc_indexer) const {
    dfs_rc_indexer_t rc_indexer;
    const dfs_bt_root_t * pbt_root = NULL;
    PCDU pcdu = NULL;
    bool is_exist = false;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    //tobj.init();

    if (NULL == prc_indexer || !(prc_indexer->is_valid())) {
        if (get_batch_mutate_mode()) {
            log_level = DF_UL_LOG_WARNING;
            DF_WRITE_LOG_US(log_level, "get_batch_mutate_mode() is true but NULL == prc_indexer");
            err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = _snapshot_wr_root_indexer(rc_indexer)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_snapshot_wr_root_indexer() returns 0x%x", err);
        } else {
            prc_indexer = &rc_indexer;
        }
    } else if(!_rc_root_set.is_valid_in_use_indexer(*prc_indexer)) {
        log_level = DF_UL_LOG_WARNING;
        DF_WRITE_LOG_US(log_level, "!_rc_root_set.is_valid_in_use_indexer(*prc_indexer)");
        err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
    }

    if (0 == err) {
        if ((err = _get_root(*prc_indexer, pbt_root)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_root() returns 0x%x", err);
        } else if (NULL == pbt_root) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "NULL == pbt_root");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = _xz_get_largest(*pbt_root, pcdu)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_xz_get_largest() returns 0x%x", err);
        } else if (NULL != pcdu) {
            *tobj=(T*)pcdu;
            //tobj = *((const T *)pcdu);
            is_exist = true;
        }
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "prc_indexer=%p, prc_indexer->_index=%lu",
                        prc_indexer,
                        (NULL == prc_indexer) ?
                        UNDEF_INDEX : ((dfs_rc_indexer_ext_t *)prc_indexer)->get_index());
    }


    return is_exist;
};

//  功能：获得刚好比输入项srct小的元素
//  输入：srct不需要在B树中存在
//  输出：如果较小元素存在，则其值被复制到tagt，否则tagt被初始化
//  返回：较小元素存在则返回true，较小元素不存在返回false。
template<typename T, typename K, uint32_t ROW_SIZE>
bool dfs_xz_btree_t<T, K, ROW_SIZE>::get_smaller(
    const K & key,
    T & tagt,
    dfs_btree_drill_t * pdrill_info,
    const dfs_rc_indexer_t * prc_indexer) const {
    dfs_rc_indexer_t rc_indexer;
    const dfs_bt_root_t * pbt_root = NULL;
    dfs_btree_drill_t bck_drill_info;
    dfs_btree_drill_t & drill_info  = (NULL == pdrill_info) ? bck_drill_info : *pdrill_info;
    PCDU pcdu = NULL;
    bool is_exist = false;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    tagt.init();

    if (NULL == prc_indexer || !(prc_indexer->is_valid())) {
        if (get_batch_mutate_mode()) {
            log_level = DF_UL_LOG_WARNING;
            DF_WRITE_LOG_US(log_level, "get_batch_mutate_mode() is true but NULL == prc_indexer");
            err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = _snapshot_wr_root_indexer(rc_indexer)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_snapshot_wr_root_indexer() returns 0x%x", err);
        } else {
            prc_indexer = &rc_indexer;
        }
    } else if(!_rc_root_set.is_valid_in_use_indexer(*prc_indexer)) {
        log_level = DF_UL_LOG_WARNING;
        DF_WRITE_LOG_US(log_level, "!_rc_root_set.is_valid_in_use_indexer(*prc_indexer)");
        err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
    }

    if (0 == err) {
        if ((err = _get_root(*prc_indexer, pbt_root)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_root() returns 0x%x", err);
        } else if (NULL == pbt_root) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "NULL == pbt_root");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = _xz_get_smaller(
                              *pbt_root,
                              key,
                              drill_info,
                              pcdu)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_xz_get_smaller() returns 0x%x", err);
        } else if (NULL != pcdu) {
            tagt = *((const T *)pcdu);
            is_exist = true;
        }
    }

    return is_exist;
};

template<typename T, typename K, uint32_t ROW_SIZE>
bool dfs_xz_btree_t<T, K, ROW_SIZE>::get_smaller(
    const T & srct,
    T & tagt,
    dfs_btree_drill_t * pdrill_info,
    const dfs_rc_indexer_t * prc_indexer) const {
    dfs_rc_indexer_t rc_indexer;
    const dfs_bt_root_t * pbt_root = NULL;
    dfs_btree_drill_t bck_drill_info;
    dfs_btree_drill_t & drill_info  = (NULL == pdrill_info) ? bck_drill_info : *pdrill_info;
    PCDU pcdu = NULL;
    DU duobj(srct);
    bool is_exist = false;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    tagt.init();

    if (NULL == prc_indexer || !(prc_indexer->is_valid())) {
        if (get_batch_mutate_mode()) {
            log_level = DF_UL_LOG_WARNING;
            DF_WRITE_LOG_US(log_level, "get_batch_mutate_mode() is true but NULL == prc_indexer");
            err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = _snapshot_wr_root_indexer(rc_indexer)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_snapshot_wr_root_indexer() returns 0x%x", err);
        } else {
            prc_indexer = &rc_indexer;
        }
    } else if(!_rc_root_set.is_valid_in_use_indexer(*prc_indexer)) {
        log_level = DF_UL_LOG_WARNING;
        DF_WRITE_LOG_US(log_level, "!_rc_root_set.is_valid_in_use_indexer(*prc_indexer)");
        err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
    }

    if (0 == err) {
        if ((err = _get_root(*prc_indexer, pbt_root)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_root() returns 0x%x", err);
        } else if (NULL == pbt_root) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "NULL == pbt_root");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = _xz_get_smaller(
                              *pbt_root,
                              duobj,
                              drill_info,
                              pcdu)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_xz_get_smaller() returns 0x%x", err);
        } else if (NULL != pcdu) {
            tagt = *((const T *)pcdu);
            is_exist = true;
        }
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "prc_indexer=%p, prc_indexer->_index=%lu",
                        prc_indexer,
                        (NULL == prc_indexer) ?
                        UNDEF_INDEX : ((dfs_rc_indexer_ext_t *)prc_indexer)->get_index());
    }


    return is_exist;
}

//返回target的指针,必须保证在snapshot状态下面..
template<typename T, typename K, uint32_t ROW_SIZE>
bool dfs_xz_btree_t<T, K, ROW_SIZE>::zy_get_smaller_p(
    const T & srct,
    T ** tagt,
    dfs_btree_drill_t * pdrill_info,
    const dfs_rc_indexer_t * prc_indexer) const {
    dfs_rc_indexer_t rc_indexer;
    const dfs_bt_root_t * pbt_root = NULL;
    dfs_btree_drill_t bck_drill_info;
    dfs_btree_drill_t & drill_info  = (NULL == pdrill_info) ? bck_drill_info : *pdrill_info;
    PCDU pcdu = NULL;
    DU duobj(srct);
    bool is_exist = false;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    //tagt.init();

    if (NULL == prc_indexer || !(prc_indexer->is_valid())) {
        if (get_batch_mutate_mode()) {
            log_level = DF_UL_LOG_WARNING;
            DF_WRITE_LOG_US(log_level, "get_batch_mutate_mode() is true but NULL == prc_indexer");
            err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = _snapshot_wr_root_indexer(rc_indexer)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_snapshot_wr_root_indexer() returns 0x%x", err);
        } else {
            prc_indexer = &rc_indexer;
        }
    } else if(!_rc_root_set.is_valid_in_use_indexer(*prc_indexer)) {
        log_level = DF_UL_LOG_WARNING;
        DF_WRITE_LOG_US(log_level, "!_rc_root_set.is_valid_in_use_indexer(*prc_indexer)");
        err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
    }

    if (0 == err) {
        if ((err = _get_root(*prc_indexer, pbt_root)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_root() returns 0x%x", err);
        } else if (NULL == pbt_root) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "NULL == pbt_root");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = _xz_get_smaller(
                              *pbt_root,
                              duobj,
                              drill_info,
                              pcdu)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_xz_get_smaller() returns 0x%x", err);
        } else if (NULL != pcdu) {
            //tagt = *((const T *)pcdu);
            *tagt=(T*)pcdu;
            is_exist = true;
        }
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "prc_indexer=%p, prc_indexer->_index=%lu",
                        prc_indexer,
                        (NULL == prc_indexer) ?
                        UNDEF_INDEX : ((dfs_rc_indexer_ext_t *)prc_indexer)->get_index());
    }


    return is_exist;
}


//  功能：获得刚好比输入的srct大的元素
//  输入：srct不需要在B树中存在
//  输出：如果较大元素存在，则其值被复制到tagt，否则tagt被初始化
//  返回：较大元素存在则返回true，较大元素不存在返回false。
template<typename T, typename K, uint32_t ROW_SIZE>
bool dfs_xz_btree_t<T, K, ROW_SIZE>::get_larger(
    const K & key,
    T & tagt,
    dfs_btree_drill_t * pdrill_info,
    const dfs_rc_indexer_t * prc_indexer) const {
    dfs_rc_indexer_t rc_indexer;
    const dfs_bt_root_t * pbt_root = NULL;
    dfs_btree_drill_t bck_drill_info;
    dfs_btree_drill_t & drill_info  = (NULL == pdrill_info) ? bck_drill_info : *pdrill_info;
    PCDU pcdu = NULL;
    bool is_exist = false;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    tagt.init();

    if (NULL == prc_indexer || !(prc_indexer->is_valid())) {
        if (get_batch_mutate_mode()) {
            log_level = DF_UL_LOG_WARNING;
            DF_WRITE_LOG_US(log_level, "get_batch_mutate_mode() is true but NULL == prc_indexer");
            err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = _snapshot_wr_root_indexer(rc_indexer)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_snapshot_wr_root_indexer() returns 0x%x", err);
        } else {
            prc_indexer = &rc_indexer;
        }
    } else if(!_rc_root_set.is_valid_in_use_indexer(*prc_indexer)) {
        log_level = DF_UL_LOG_WARNING;
        DF_WRITE_LOG_US(log_level, "!_rc_root_set.is_valid_in_use_indexer(*prc_indexer)");
        err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
    }

    if (0 == err) {
        if ((err = _get_root(*prc_indexer, pbt_root)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_root() returns 0x%x", err);
        } else if (NULL == pbt_root) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "NULL == pbt_root");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = _xz_get_larger(
                              *pbt_root,
                              key,
                              drill_info,
                              pcdu)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_xz_get_larger() returns 0x%x", err);
        } else if (NULL != pcdu) {
            tagt = *((const T *)pcdu);
            is_exist = true;
        }
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "prc_indexer=%p, prc_indexer->_index=%lu",
                        prc_indexer,
                        (NULL == prc_indexer) ?
                        UNDEF_INDEX : ((dfs_rc_indexer_ext_t *)prc_indexer)->get_index());
    }


    return is_exist;
};

template<typename T, typename K, uint32_t ROW_SIZE>
bool dfs_xz_btree_t<T, K, ROW_SIZE>::get_larger(
    const T & srct,
    T & tagt,
    dfs_btree_drill_t * pdrill_info,
    const dfs_rc_indexer_t * prc_indexer) const {
    dfs_rc_indexer_t rc_indexer;
    const dfs_bt_root_t * pbt_root = NULL;
    dfs_btree_drill_t bck_drill_info;
    dfs_btree_drill_t & drill_info  = (NULL == pdrill_info) ? bck_drill_info : *pdrill_info;
    PCDU pcdu = NULL;
    DU duobj(srct);
    bool is_exist = false;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    tagt.init();

    if (NULL == prc_indexer || !(prc_indexer->is_valid())) {
        if (get_batch_mutate_mode()) {
            log_level = DF_UL_LOG_WARNING;
            DF_WRITE_LOG_US(log_level, "get_batch_mutate_mode() is true but NULL == prc_indexer");
            err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = _snapshot_wr_root_indexer(rc_indexer)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_snapshot_wr_root_indexer() returns 0x%x", err);
        } else {
            prc_indexer = &rc_indexer;
        }
    } else if(!_rc_root_set.is_valid_in_use_indexer(*prc_indexer)) {
        log_level = DF_UL_LOG_WARNING;
        DF_WRITE_LOG_US(log_level, "!_rc_root_set.is_valid_in_use_indexer(*prc_indexer)");
        err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
    }

    if (0 == err) {
        if ((err = _get_root(*prc_indexer, pbt_root)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_root() returns 0x%x", err);
        } else if (NULL == pbt_root) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "NULL == pbt_root");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = _xz_get_larger(
                              *pbt_root,
                              duobj,
                              drill_info,
                              pcdu)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_xz_get_larger() returns 0x%x", err);
        } else if (NULL != pcdu) {
            tagt = *((const T *)pcdu);
            is_exist = true;
        }
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "prc_indexer=%p, prc_indexer->_index=%lu",
                        prc_indexer,
                        (NULL == prc_indexer) ?
                        UNDEF_INDEX : ((dfs_rc_indexer_ext_t *)prc_indexer)->get_index());
    }


    return is_exist;
};

//获得target的指针,必须保证在snapshot状态下面...
template<typename T, typename K, uint32_t ROW_SIZE>
bool dfs_xz_btree_t<T, K, ROW_SIZE>::zy_get_larger_p(
    const T & srct,
    T ** tagt,
    dfs_btree_drill_t * pdrill_info,
    const dfs_rc_indexer_t * prc_indexer) const {
    dfs_rc_indexer_t rc_indexer;
    const dfs_bt_root_t * pbt_root = NULL;
    dfs_btree_drill_t bck_drill_info;
    dfs_btree_drill_t & drill_info  = (NULL == pdrill_info) ? bck_drill_info : *pdrill_info;
    PCDU pcdu = NULL;
    DU duobj(srct);
    bool is_exist = false;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    //tagt.init();

    if (NULL == prc_indexer || !(prc_indexer->is_valid())) {
        if (get_batch_mutate_mode()) {
            log_level = DF_UL_LOG_WARNING;
            DF_WRITE_LOG_US(log_level, "get_batch_mutate_mode() is true but NULL == prc_indexer");
            err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = _snapshot_wr_root_indexer(rc_indexer)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_snapshot_wr_root_indexer() returns 0x%x", err);
        } else {
            prc_indexer = &rc_indexer;
        }
    } else if(!_rc_root_set.is_valid_in_use_indexer(*prc_indexer)) {
        log_level = DF_UL_LOG_WARNING;
        DF_WRITE_LOG_US(log_level, "!_rc_root_set.is_valid_in_use_indexer(*prc_indexer)");
        err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
    }

    if (0 == err) {
        if ((err = _get_root(*prc_indexer, pbt_root)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_root() returns 0x%x", err);
        } else if (NULL == pbt_root) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "NULL == pbt_root");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = _xz_get_larger(
                              *pbt_root,
                              duobj,
                              drill_info,
                              pcdu)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_xz_get_larger() returns 0x%x", err);
        } else if (NULL != pcdu) {
            //tagt = *((const T *)pcdu);
            *tagt=(T*)pcdu;
            is_exist = true;
        }
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "prc_indexer=%p, prc_indexer->_index=%lu",
                        prc_indexer,
                        (NULL == prc_indexer) ?
                        UNDEF_INDEX : ((dfs_rc_indexer_ext_t *)prc_indexer)->get_index());
    }


    return is_exist;
};



//功能：验证整个树的顺序和每个节点分支个数
//返回：返回0如果验证通过，返回非0(错误代码)如果验证失败
template<typename T, typename K, uint32_t ROW_SIZE>
int dfs_xz_btree_t<T, K, ROW_SIZE>::verify(const dfs_rc_indexer_t * prc_indexer) const {
    dfs_rc_indexer_t rc_indexer;
    const dfs_bt_root_t * pbt_root = NULL;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    if (NULL == prc_indexer || !(prc_indexer->is_valid())) {
        if (get_batch_mutate_mode()) {
            log_level = DF_UL_LOG_WARNING;
            DF_WRITE_LOG_US(log_level, "get_batch_mutate_mode() is true but NULL == prc_indexer");
            err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = _snapshot_wr_root_indexer(rc_indexer)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_snapshot_wr_root_indexer() returns 0x%x", err);
        } else {
            prc_indexer = &rc_indexer;
        }
    } else if(!_rc_root_set.is_valid_in_use_indexer(*prc_indexer)) {
        log_level = DF_UL_LOG_WARNING;
        DF_WRITE_LOG_US(log_level, "!_rc_root_set.is_valid_in_use_indexer(*prc_indexer)");
        err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
    }

    if (0 == err) {
        if ((err = _get_root(*prc_indexer, pbt_root)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_root() returns 0x%x", err);
        } else if (NULL == pbt_root) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "NULL == pbt_root");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = this->bt_verify_tree(*pbt_root)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_xz_verify() returns 0x%x", err);
        }
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "prc_indexer=%p, prc_indexer->_index=%lu",
                        prc_indexer,
                        (NULL == prc_indexer) ?
                        UNDEF_INDEX : ((dfs_rc_indexer_ext_t *)prc_indexer)->get_index());
    }


    return err;
};

//功能：设置或取消checkpoint状态
//输入：is_cancel：true取消checkpoint点及进行中的checkpoing操作，false设置一个新的checkpoint点
//      参数max_wait_microseconds不再使用
//建议：仅用此函数取消进行中的checkpoint，通过snapshot()获得一个checkpoint
//返回：0: 设置或取消成功，取消不存在的checkpoint状态也返回0
template<typename T, typename K, uint32_t ROW_SIZE>
int dfs_xz_btree_t<T, K, ROW_SIZE>::set_checkpoint_state(
    const bool is_cancel,
    const int64_t /*max_wait_microseconds*/,
    const dfs_btree_lock_hold_t * plock_hold) {
    dfs_btree_lock_hold_t lock_hold;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;
    int err1 = 0;

    //为了避免此时写线程修改btree
    //因此要锁住_mutate_locker()避免写线程工作。
    if ((err = BT_MUTATE_LOCK_ACQUIRE(plock_hold, lock_hold, 0)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_ACQUIRE_ERR_INFO);
    } else {
        if (is_cancel) {
            _ckp_root_indexer.init();
        } else {
            if ((err = _snapshot_wr_root_indexer(_ckp_root_indexer)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_snapshot_wr_root_indexer() returns 0x%x", err);
            }
        }
        //取消或设置checkpoint
        this->_bt_set_cancel_checkpointing(is_cancel);

        if ((err1 = BT_MUTATE_LOCK_RELEASE(plock_hold, lock_hold)) != 0) {
            err = err1;
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_RELEASE_ERR_INFO);
        }
    }

    return err;
};

//
//  功能：把已经设置的checkpoint存储到文件中
//  参数：file_info：文件的控制信息，例如文件句柄，调用store或load时文件数据的位置等。
//  返回：0 for success, other values for error
template<typename T, typename K, uint32_t ROW_SIZE>
int dfs_xz_btree_t<T, K, ROW_SIZE>::store_checkpointing(
    const dfs_write_proc_t write_proc,
    void * file_info,
    const dfs_btree_lock_hold_t * plock_hold,
    const dfs_rc_indexer_t * prc_indexer) {
    dfs_rc_indexer_t rc_indexer;
    const dfs_bt_root_t * pbt_root = NULL;
    dfs_btree_store_info_t store_info(write_proc, file_info, MAX_T_STORE_SIZE*4);
    dfs_btree_lock_hold_t lock_hold;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;
    int err1 = 0;

    log_level = this->CKP_LOG_LEVEL;

    //不论其他操作是否成功，_xz_ckp_store_begin和_xz_ckp_store_end都需要执行以清除ckp状态
    if ((err = BT_MUTATE_LOCK_ACQUIRE(plock_hold, lock_hold, 0)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_ACQUIRE_ERR_INFO);
    } else {
        //清除“取消store/load状态”
        this->_xz_set_cancel_checkpointing(false); //is_cancel;

        if ((err = store_info.init()) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "store_info.init() returns 0x%x", err);
        } else {
            store_info.set_major_tag(this->XZ_BTREE_TAG);
            store_info.set_store_t_value(); //store T's value
        }

        if (0 == err) {
            if (NULL == prc_indexer || !(prc_indexer->is_valid())) {
                rc_indexer = _ckp_root_indexer;
                prc_indexer = &rc_indexer;
            }

            if(!_rc_root_set.is_valid_in_use_indexer(*prc_indexer)) {
                log_level = DF_UL_LOG_WARNING;
                DF_WRITE_LOG_US(log_level, "!_rc_root_set.is_valid_in_use_indexer(*prc_indexer)");
                err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
            } else if ((err = _get_root(*prc_indexer, pbt_root)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_get_root() returns 0x%x", err);
            } else if (NULL == pbt_root) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "NULL == pbt_root");
                err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
            }
        }

        //只要能够得到锁，就清空_ckp_root_indexer
        _ckp_root_indexer.init();

        if ((err1 = BT_MUTATE_LOCK_RELEASE(plock_hold, lock_hold)) != 0) {
            err = err1;
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_RELEASE_ERR_INFO);
        }
    }

    if (0 == err) {
        if ((err = this->_xz_store_checkpointing(*pbt_root, store_info)) != 0) {
            log_level = (ERRNO_BT_CKP_CANCELLED == err) ? DF_UL_LOG_NOTICE : DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_xz_store_checkpointing() returns 0x%x", err);
        }
    }

    if (DF_UL_LOG_NONE != log_level) {
        DF_WRITE_LOG_US(log_level,
                        "prc_indexer=%p, prc_indexer->_index=%lu",
                        prc_indexer,
                        (NULL == prc_indexer) ?
                        UNDEF_INDEX : ((dfs_rc_indexer_ext_t *)prc_indexer)->get_index());
    }


    return err;
};

//
//  功能：把先前存储在文件的checkpoint加载到内存中
//  参数：file_info：文件的控制信息，例如文件句柄，调用store或load时文件数据的位置等。
//  返回：0 for success, other values for error
template<typename T, typename K, uint32_t ROW_SIZE>
int dfs_xz_btree_t<T, K, ROW_SIZE>::load_checkpointing(
    const dfs_read_proc_t read_proc,
    void * file_info,
    const dfs_btree_lock_hold_t * plock_hold) {
    const dfs_bt_root_t * pold_wr_bt_root = NULL;
    dfs_bt_root_t new_wr_bt_root;
    dfs_rc_indexer_t old_rc_indexer;
    dfs_btree_load_info_t load_info(read_proc, file_info, MAX_T_STORE_SIZE*4);
    dfs_btree_lock_hold_t lock_hold;
    int log_level = DF_UL_LOG_NONE;
    int err = 0;
    int err1 = 0;

    //Step 1. preparation
    //Step 2. load the btree
    //step 3. update the root

    log_level = this->CKP_LOG_LEVEL;

    //Step 1. preparation
    if ((err = BT_MUTATE_LOCK_ACQUIRE(plock_hold, lock_hold, 0)) != 0) {
        log_level = DF_UL_LOG_FATAL;
        DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_ACQUIRE_ERR_INFO);
    } else {
        //清除“取消store/load状态”
        this->_xz_set_cancel_checkpointing(false); //is_cancel;

        if ((err1 = load_info.init()) != 0) {
            err = err1;
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "load_info.init() returns 0x%x", err);
        } else {
            load_info.set_major_tag(this->XZ_BTREE_TAG);

            if ((err = _retrieve_wr_root_indexer(old_rc_indexer, pold_wr_bt_root)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_retrieve_wr_root_indexer() returns 0x%x", err);
            } else if (NULL == pold_wr_bt_root) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "NULL == pold_wr_bt_root");
                err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
            } else {
                //先复制根节点以便获得mutation counter, next_allocate_id等信息
                new_wr_bt_root = *pold_wr_bt_root;
            }
        }

        if ((err1 = BT_MUTATE_LOCK_RELEASE(plock_hold, lock_hold)) != 0) {
            err = err1;
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_RELEASE_ERR_INFO);
        }
    }

    //Step 2. load the btree
    if (0 == err) {
        if ((err1 = this->_xz_load_checkpointing(new_wr_bt_root, load_info)) != 0) {
            err = err1;
            log_level = (ERRNO_BT_CKP_CANCELLED == err) ? DF_UL_LOG_NOTICE : DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_xz_load_checkpointing() returns 0x%x", err);
        }

        if (0 != err) {
            //发生错误后要删除已经生成的对象
            //先+1引用计数，再-1引用计数，这导致对象回收
            if ((err1 = this->_xz_inc_root_ref_counter(new_wr_bt_root)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_xz_inc_root_ref_counter() returns 0x%x", err1);
            } else if ((err1 = this->_xz_dec_root_ref_counter(new_wr_bt_root)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_xz_dec_root_ref_counter() returns 0x%x", err1);
            }
        }
    }

    //step 3. update the root
    if (0 == err) {
        if ((err = BT_MUTATE_LOCK_ACQUIRE(plock_hold, lock_hold, 0)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_ACQUIRE_ERR_INFO);
        } else {
            //如果不是batch mode，则同时更新_max_cow_mutation_counter从而禁止修改时的指针原子替换
            this->_xz_update_mutation_counter(false); //get_batch_mutate_mode();

            if((err = this->_update_wr_root_indexer(new_wr_bt_root)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_update_wr_root_indexer() returns 0x%x", err);
            }

            //只要能够得到锁，就清空_ckp_root_indexer
            _ckp_root_indexer.init();

            if ((err1 = BT_MUTATE_LOCK_RELEASE(plock_hold, lock_hold)) != 0) {
                err = err1;
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, BT_MUTATE_LOCK_RELEASE_ERR_INFO);
            }
        }
    }

    return err;
};


#endif //__DFS_XZ_BTREE_INCLUDE_H_

