////====================================================================
//
// df_2d_ary.h - Pyramid / DFS / df-lib
//
// Copyright (C) 2008 Baidu.com, Inc.
//
// Created on 2008-01-05 by YANG Zhenkun (yangzhenkun@baidu.com)
//
// -------------------------------------------------------------------
//
// Description
//
//    declaration and implementation of dfs_bt2d_ary_t (2-dimension array) template
//
// -------------------------------------------------------------------
//
// Change Log
//
//    updated on 2008-07-28 by YANG Zhenkun (yangzhenkun@baidu.com)
//
////====================================================================

#ifndef __DF_2D_ARY_INCLUDE_H_
#define __DF_2D_ARY_INCLUDE_H_

//������Ϣ��ӡ�������
//#ifndef DF_BT_PRINT_DEBUG_INFO
//#define DF_BT_PRINT_DEBUG_INFO
//#endif

# if defined __x86_64__
# else
#  error "MUST COMPILED ON x86_64"
# endif

#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <new>

/* #include "ul_def.h" */
/* #include "ul_log.h" */
//#include "df_common.h"
#include "df_log.h"
#include "df_misc.h"

//#ifndef DF_BT_PRINT_DEBUG_INFO
//#define DF_BT_PRINT_DEBUG_INFO
//#endif


static const uint64_t UNDEF_INDEX = MAX_U_INT64;
static const uint64_t NULL_INDEX = ((uint64_t)0ULL);
static const uint64_t UNDEF_ID = UNDEF_INDEX;
static const uint64_t NULL_ID = NULL_INDEX;



//pvoid: pointer to void
#ifndef _pvoid_defined
typedef void *pvoid;
#define _pvoid_defined
#endif

//pchar: pointer to char
#ifndef _pchar_defined
typedef char *pchar;
#define _pchar_defined
#endif

//pcchar: pointer to const char
#ifndef _pcchar_defined
typedef const char *pcchar;
#define _pcchar_defined
#endif



/* //���ܣ���¼һ���ڲ�������롣 */
/* //���أ�ǰ�εĴ�����롣 */
/* int dfs_bt_set_internal_err( */
/*     const int err, */
/*     const int lineno = -1, */
/*     const char * funcname = NULL, */
/*     const char * filename = NULL); */
/* //���ܣ���¼һ����ͨ������롣 */
/* //���أ�ǰ�εĴ�����롣 */
/* int dfs_bt_set_normal_err( */
/*     const int err, */
/*     const int lineno = -1, */
/*     const char * funcname = NULL, */
/*     const char * filename = NULL); */
/* ////���أ����Ĵ�����롣 */
/* //int dfs_bt_get_last_error(void); */


/* #define DF_BT_SET_INTERNAL_ERR(err) dfs_bt_set_internal_err(err, __LINE__, __FUNCTION__, __FILE__) */
/* #define DF_BT_SET_NORMAL_ERR(err) dfs_bt_set_normal_err(err, __LINE__, __FUNCTION__, __FILE__) */
#define DF_BT_SET_INTERNAL_ERR(err) 0
#define DF_BT_SET_NORMAL_ERR(err) 0

/* #ifdef  DF_TEST_BTREE */
/* extern volatile uint64_t s_dfs_bt_debug_trap_counter; */
/* uint64_t dfs_bt_debug_trap(void); */
/* #define DFS_BT_DEBUG_TRAP dfs_bt_debug_trap() */
/* #else */
/* #define DFS_BT_DEBUG_TRAP */
/* #endif */
#define DFS_BT_DEBUG_TRAP


#define ERRINFO_BT_NUM_ENCODE_BUF           "error encode buf/data"
#define ERRINFO_BT_NUM_DECODE_BUF           "error decode buf/data"
#define ERRINFO_BT_STORE_POS                "error store pos"
#define ERRINFO_BT_STORE_BUF                "error store buf"
#define ERRINFO_BT_STORE_TYPE               "error store type"
#define ERRINFO_BT_LOAD_POS                 "error load pos"
#define ERRINFO_BT_LOAD_BUF                 "error load buf"
#define ERRINFO_BT_LOAD_MAJOR_TAG           "error load major tag"
#define ERRINFO_BT_LOAD_MINOR_TAG           "error load minor tag"
#define ERRINFO_BT_LOAD_HEAD_VER            "error load head version"
#define ERRINFO_BT_LOAD_HEAD_SIZE           "error load head size"
#define ERRINFO_BT_LOAD_KEY_NODE_NUM        "error load key/node num"
#define ERRINFO_BT_LOAD_MAX_ID              "get_next_allocate_id() > 1 && pcdu->get_id() >= get_next_allocate_id()"
#define ERRINFO_BT_NOMEM                    "%s == NULL"
#define ERRINFO_BT_DEL                      ""
#define ERRINFO_BT_NULL_BUF_POINTER         ""
#define ERRINFO_BT_OBJ_NULL_POINTER         ""
#define ERRINFO_BT_NOT_NULL_BUF_POINTER	 ""
#define ERRINFO_BT_NOT_OBJ_NULL_POINTER     ""
#define ERRINFO_BT_2D_BASE_FREE             ""
#define ERRINFO_BT_2D_BASE_IN_USE           ""
#define ERRINFO_BT_BUF_SIZE                 "insuffient buffer size or null buffer"
#define ERRINFO_BT_BUF_POS                  ""
#define ERRINFO_BT_DIFF_POINTER             ""
#define ERRINFO_BT_DIFF_INDEX               ""
#define ERRINFO_BT_DIFF_ID                  ""
#define ERRINFO_BT_INVALID_ID               ""
#define ERRINFO_BT_INVALID_INDEX            ""
#define ERRINFO_BT_REF_COUNTER              ""
#define ERRINFO_BT_DIFF_NUM_IN_BT_2D_ARY ""
#define ERRINFO_BT_OBJ_ACQUIRE              ""
#define ERRINFO_BT_KEY_SIZE                 ""
#define ERRINFO_BT_KEY_MARK                 ""
#define ERRINFO_BT_KEY_POS                  ""
#define ERRINFO_BT_KEY_NOT_EXIST            "error not existed key"
#define ERRINFO_BT_KEY_INVALID              "error invalid key"
#define ERRINFO_BT_NODE_INDEX               ""
#define ERRINFO_BT_ROOT_NODE_INDEX          ""
#define ERRINFO_BT_MID_NODE_INDEX           ""
#define ERRINFO_BT_LEAF_NODE_INDEX          ""
#define ERRINFO_BT_MID_NODE_ACQUIRE         ""
#define ERRINFO_BT_LEAF_NODE_ACQUIRE        ""
#define ERRINFO_BT_NODE_NULL_POINTER        ""
#define ERRINFO_BT_MID_NODE_NULL_POINTER    ""
#define ERRINFO_BT_LEAF_NODE_NULL_POINTER   ""
#define ERRINFO_BT_RESERVED_MID_NODE        ""
#define ERRINFO_BT_RESERVED_LEAF_NODE       ""
#define ERRINFO_BT_HETEROGENEOUS_MID_NODE   ""
#define ERRINFO_BT_NODE_POS                 ""
#define ERRINFO_BT_NODE_NUM                 ""
#define ERRINFO_BT_NODE_NOTFULL_SPLIT       ""
#define ERRINFO_BT_KEY_INDEX                ""
#define ERRINFO_BT_KEY_ACQUIRE              ""
#define ERRINFO_BT_KEY_NULL_POINTER         ""
#define ERRINFO_BT_SUBKEY_POS               ""
#define ERRINFO_BT_SUBKEY_NUM               ""
#define ERRINFO_BT_SUBKEY_ORDER             ""
#define ERRINFO_BT_SUBKEY_OVERFLOW          ""
#define ERRINFO_BT_SUBKEY_UNDERFLOW         ""
#define ERRINFO_BT_SUBKEY_INDEX             ""
#define ERRINFO_BT_SUBNODE_INDEX            ""
#define ERRINFO_BT_CKP_STATE                "error ckp state"
#define ERRINFO_BT_CKP_ROOT                 "UNDEF_INDEX != ckp_root_index|| ckp_mutation_counter > 0"
#define ERRINFO_BT_CKP_SERIAL_READ_WRITE    "checkpoint is not allowed during serial_read_write"
#define ERRINFO_BT_CKP_CANCELLED            ""
#define ERRINFO_BT_CKP_FAIL                 ""
#define ERRINFO_BT_DRILL_OVERFLOW           ""
#define ERRINFO_BT_DRILL_UNDERFLOW          ""
#define ERRINFO_BT_DRILL_POS                ""
#define ERRINFO_BT_DRILL_INS_POS            ""
#define ERRINFO_BT_DRILL_NODE_INDEX         ""
#define ERRINFO_BT_DRILL_KEY_INDEX          ""
#define ERRINFO_BT_DRILL_NODE_NULL_POINTER  ""
#define ERRINFO_BT_DRILL_KEY_NULL_POINTER   ""
#define ERRINFO_BT_DRILL_DEL_NODE_POS       ""
#define ERRINFO_BT_DRILL_DEL_NODE_INDEX     ""
#define ERRINFO_BT_MUTATE_TYPE              ""
#define ERRINFO_BT_NULL_ID                  ""
#define ERRINFO_BT_NOTNULL_ID               ""
#define ERRINFO_BT_UNDEF_NAME_ID            ""
#define ERRINFO_BT_NULL_PREFIX_ID           ""
#define ERRINFO_BT_NOTNULL_PREFIX_ID        ""
#define ERRINFO_BT_UNDEF_PREFIX_ID          ""
#define ERRINFO_BT_UNDEF_SUFFIX_ID          ""
#define ERRINFO_BT_UNDEF_NAME_INDEX         ""
#define ERRINFO_BT_NONE_EMPTY_PREFIX_BTREE  ""
#define ERRINFO_BT_NONE_EMPTY_SUFFIX_BTREE  ""
#define ERRINFO_BT_NONE_EMPTY_BTREE         ""
#define ERRINFO_BT_NONE_EMPTY_BUF           ""
#define ERRINFO_BT_T_LEAK                   ""
#define ERRINFO_BT_DIFF_FP_ID_BTREE         ""
#define ERRINFO_BT_WRITE_FILE               ""
#define ERRINFO_BT_READ_FILE                ""
#define ERRINFO_BT_FILE_POS                 ""
#define ERRINFO_BT_TIMEOUT                  ""
#define ERRINFO_BT_MUTATION_COUNTER         ""
#define ERRINFO_BT_OFFSET                   ""
#define ERRINFO_BT_SET_GC_INFO              ""
#define ERRINFO_BT_OBJ_FREE_IN_USE_STATE    ""
#define ERRINFO_BT_COW_ONCOW_RETRY          ""
#define ERRINFO_BT_COW_NUM                  ""
#define ERRINFO_BT_COW_MUTATION_COUNTER     ""
#define ERRINFO_BT_LOCK_RE_RELEASE          "error re-released btree mutate lock"
#define ERRINFO_BT_LOCK_HOLD_VERIFY         "error btree mutate locke verify failed"
#define ERRINFO_BT_WRITE_ONLY               "error btree is write only"



#define BT_DF_WRITE_LOG_BYTES(log_level, info, buf, data_len) \
{ \
    int64_t _j = 0; \
    const int64_t _n = (int64_t)data_len; \
    if (NULL != (info)) \
    { \
        DF_WRITE_LOG_US(log_level, "%s,len=%ld", info, _n); \
    } \
    if (NULL != (buf) && _n > 0) \
    { \
        while (_j < _n) \
        { \
            DF_WRITE_LOG_US(log_level, \
                    "%.2x%.2x%.2x%.2x %.2x%.2x%.2x%.2x", \
                    (buf)[_j],   (buf)[_j+1], \
                    (buf)[_j+2], (buf)[_j+3], \
                    (buf)[_j+4], (buf)[_j+5], \
                    (buf)[_j+6], (buf)[_j+7]); \
            _j += 8; \
        } \
    } \
} \
 
#define BT_DF_WRITE_LOG_INT32S(log_level, info, buf, data_len) \
{ \
    int64_t _j = 0; \
    const int64_t _n = (int64_t)data_len; \
    if (NULL != (info)) \
    { \
        DF_WRITE_LOG_US(log_level, "%s,len=%ld", info, _n); \
    } \
    if (NULL != (buf) && _n > 0) \
    { \
        while (_j < _n) \
        { \
            DF_WRITE_LOG_US(log_level, \
                    "0x%.4x 0x%.4x 0x%.4x 0x%.4x", \
                    (buf)[_j],   (buf)[_j+1], \
                    (buf)[_j+2], (buf)[_j+3]); \
            j += 4; \
        } \
    } \
} \
 
#define BT_DF_WRITE_LOG_INT64S(log_level, info, buf, data_len) \
{ \
    int64_t _j = 0; \
    const int64_t _n = (int64_t)data_len; \
    if (NULL != (info)) \
    { \
        DF_WRITE_LOG_US(log_level, "%s,len=%ld", info, _n); \
    } \
    if (NULL != (buf) && _n > 0) \
    { \
        while (_j < _n) \
        { \
            DF_WRITE_LOG_US(log_level, \
                    "0x%lx 0x%lx", \
                    (buf)[_j],   (buf)[_j+1]); \
            _j += 2; \
        } \
    } \
} \
 
//internal errno
class dfs_bt_const_t {
public:
    enum DFS_BT_ERRNO_MASK {
        ERRNO_BT_INTERNAL_ERR_FLAG  = 0x40000000,
        ERRNO_BT_ERR_NO_MASK        = 0x0000ffff,
    };
    enum DFS_BT_ERR_SYS {
        ERRNO_BT_BASE_SYS       =   0, //(ERRNO_BT_ERR_NO_MASK+1)*2  ,
    };
    enum DFS_BT_ERR_NO {
        ERRNO_BT_TEST_ERR                   = 0x1111 ,
        ERRNO_BT_BUILD_IN_MIN_NO            = 0x2000 ,
        ERRNO_BT_NUM_ENCODE_BUF             = ERRNO_BT_BUILD_IN_MIN_NO,
        ERRNO_BT_NUM_DECODE_BUF                 ,
        ERRNO_BT_STORE_POS                      ,
        ERRNO_BT_STORE_BUF                      ,
        ERRNO_BT_STORE_TYPE                     ,
        ERRNO_BT_LOAD_POS                       ,
        ERRNO_BT_LOAD_BUF                       ,
        ERRNO_BT_LOAD_MAJOR_TAG                 ,
        ERRNO_BT_LOAD_MINOR_TAG                 ,
        ERRNO_BT_LOAD_HEAD_VER                  ,
        ERRNO_BT_LOAD_HEAD_SIZE                 ,
        ERRNO_BT_LOAD_KEY_NODE_NUM              ,
        ERRNO_BT_LOAD_MAX_ID                    ,
        ERRNO_BT_NOMEM                          ,
        ERRNO_BT_DEL                            ,
        ERRNO_BT_NULL_BUF_POINTER               ,
        ERRNO_BT_OBJ_NULL_POINTER               ,
        ERRNO_BT_NOT_NULL_BUF_POINTER	        ,
        ERRNO_BT_NOT_OBJ_NULL_POINTER           ,
        ERRNO_BT_2D_BASE_FREE                   ,
        ERRNO_BT_2D_BASE_IN_USE                 ,
        ERRNO_BT_BUF_SIZE                       ,
        ERRNO_BT_BUF_POS                        ,
        ERRNO_BT_DIFF_POINTER                   ,
        ERRNO_BT_DIFF_INDEX                     ,
        ERRNO_BT_DIFF_ID                        ,
        ERRNO_BT_INVALID_ID                     ,
        ERRNO_BT_INVALID_INDEX                  ,
        ERRNO_BT_REF_COUNTER                    ,
        ERRNO_BT_DIFF_NUM_IN_BT_2D_ARY          ,   //diff nums in btree and 2d_ary
        ERRNO_BT_OBJ_ACQUIRE                    ,
        ERRNO_BT_KEY_SIZE                       ,
        ERRNO_BT_KEY_MARK                       ,
        ERRNO_BT_KEY_POS                        ,
        ERRNO_BT_KEY_NOT_EXIST                  ,
        ERRNO_BT_KEY_INVALID                    ,
        ERRNO_BT_NODE_INDEX                     ,
        ERRNO_BT_ROOT_NODE_INDEX                ,
        ERRNO_BT_MID_NODE_INDEX                 ,
        ERRNO_BT_LEAF_NODE_INDEX                ,
        ERRNO_BT_MID_NODE_ACQUIRE               ,
        ERRNO_BT_LEAF_NODE_ACQUIRE              ,
        ERRNO_BT_NODE_NULL_POINTER              ,
        ERRNO_BT_MID_NODE_NULL_POINTER          ,
        ERRNO_BT_LEAF_NODE_NULL_POINTER         ,
        ERRNO_BT_RESERVED_MID_NODE              ,
        ERRNO_BT_RESERVED_LEAF_NODE             ,
        ERRNO_BT_HETEROGENEOUS_MID_NODE         ,
        ERRNO_BT_NODE_POS                       ,
        ERRNO_BT_NODE_NUM                       ,
        ERRNO_BT_NODE_NOTFULL_SPLIT             ,
        ERRNO_BT_KEY_INDEX                      ,
        ERRNO_BT_KEY_ACQUIRE                    ,
        ERRNO_BT_KEY_NULL_POINTER               ,
        ERRNO_BT_SUBKEY_POS                     ,
        ERRNO_BT_SUBKEY_NUM                     ,
        ERRNO_BT_SUBKEY_ORDER                   ,
        ERRNO_BT_SUBKEY_OVERFLOW                ,
        ERRNO_BT_SUBKEY_UNDERFLOW               ,
        ERRNO_BT_SUBKEY_INDEX                   ,
        ERRNO_BT_SUBNODE_INDEX                  ,
        ERRNO_BT_CKP_STATE                      ,
        ERRNO_BT_CKP_ROOT                       ,   //_ckp_root_info�����쳣
        ERRNO_BT_CKP_SERIAL_READ_WRITE          ,   //���ж�дʱ���ֹ����checkpoint
        ERRNO_BT_CKP_CANCELLED                  ,
        ERRNO_BT_CKP_FAIL                       ,
        ERRNO_BT_DRILL_OVERFLOW                 ,
        ERRNO_BT_DRILL_UNDERFLOW                ,
        ERRNO_BT_DRILL_POS                      ,
        ERRNO_BT_DRILL_INS_POS                  ,
        ERRNO_BT_DRILL_NODE_INDEX               ,
        ERRNO_BT_DRILL_KEY_INDEX                ,
        ERRNO_BT_DRILL_NODE_NULL_POINTER        ,
        ERRNO_BT_DRILL_KEY_NULL_POINTER         ,
        ERRNO_BT_DRILL_DEL_NODE_POS             ,
        ERRNO_BT_DRILL_DEL_NODE_INDEX           ,
        ERRNO_BT_MUTATE_TYPE                    ,
        ERRNO_BT_NULL_ID                        ,
        ERRNO_BT_NOTNULL_ID                     ,
        ERRNO_BT_UNDEF_NAME_ID                  ,
        ERRNO_BT_NULL_PREFIX_ID                 ,
        ERRNO_BT_NOTNULL_PREFIX_ID              ,
        ERRNO_BT_UNDEF_PREFIX_ID                ,
        ERRNO_BT_UNDEF_SUFFIX_ID                ,
        ERRNO_BT_UNDEF_NAME_INDEX               ,
        ERRNO_BT_NONE_EMPTY_PREFIX_BTREE        ,
        ERRNO_BT_NONE_EMPTY_SUFFIX_BTREE        ,
        ERRNO_BT_NONE_EMPTY_BTREE               ,
        ERRNO_BT_NONE_EMPTY_BUF                 ,
        ERRNO_BT_T_LEAK                         ,
        ERRNO_BT_DIFF_SF_ID_BTREE               ,
        ERRNO_BT_WRITE_FILE                     ,
        ERRNO_BT_READ_FILE                      ,
        ERRNO_BT_FILE_POS                       ,
        ERRNO_BT_TIMEOUT                        ,
        ERRNO_BT_MUTATION_COUNTER               ,
        ERRNO_BT_OFFSET                         ,
        ERRNO_BT_SET_GC_INFO                    ,
        ERRNO_BT_OBJ_FREE_IN_USE_STATE          ,
        ERRNO_BT_COW_ONCOW_RETRY                ,
        ERRNO_BT_COW_NUM                        ,
        ERRNO_BT_COW_MUTATION_COUNTER           ,
        ERRNO_BT_LOCK_RE_RELEASE                ,
        ERRNO_BT_LOCK_HOLD_VERIFY               ,
        ERRNO_BT_WRITE_ONLY                     ,   //B��Ϊֻдģʽ
        ERRNO_BT_BUILD_IN_MAX_NO                ,   //must be the last one
    };
    enum DFS_BT_CONST {
        DEF_ROW_SIZE            = 8192              ,   //��ά������
        DEF_ROW_ARY_SIZE        = 256               ,   //��ά������
        DEF_KEY_BUF_SIZE        = 0x20000           ,   //128KB char
        MAX_BT_DEPTH            = 64                ,
        UNDEF_POS               = MAX_U_INT32       ,
        DEF_FANOUT              = 17                ,   //B�����ȳ�: dfs_bt_obj_t(2*8-byte), dfs_btree_node_base_t((2+(DEF_FANOUT-1))*8-byte)
        MIN_FANOUT              = 5                 ,
        MAX_FANOUT              = 4090              ,
        MICROSECONDS_PER_SLEEP  = 20                ,
        MAX_2D_ARY_HOLD_REALLOC_NUM     = 16        ,   //�ݴ����������Ķ�ά�������������(ÿ������һ��)
        MAX_T_STORE_SIZE                = 0x80000   ,   //512KB����������T�Ĵ洢�ߴ�����
        MAX_BT_ROOT_ONCOW_RETRY_NUM   = 16384*1024  ,   //���������ڵ����Դ���(microseconds), about 256s
        MAX_BT_RC_ROOT_NUM   = 1024     ,   //һ��B��ͬһʱ�䱻��д���ܸ���
        MAX_BT_INSTANCE_NUM  = 4        ,   //����btree����ʵ���еĸ�����һ��Ϊ1��fp_btree��Ϊ4�� ns_btreeΪ2
        DEF_BT_INSTANCE_POS     = 0     ,   //����btree����һϵ��ʵ���е���ţ�һ��Ϊ0��fp_btree��Ϊ0~3

        BT_FANOUT = DEF_FANOUT              ,
        BT_LEAF_ROW_SIZE = DEF_ROW_SIZE/16  ,
        BT_MID_ROW_SIZE  = DEF_ROW_SIZE/64  ,
    };
    static bool is_internal_err(const int err) {
        return ((err & ERRNO_BT_INTERNAL_ERR_FLAG) != 0);
    };
    static const uint64_t UNDEF_OFFSET = MAX_U_INT64;
    static const uint64_t UNDEF_REF_COUNTER = MAX_U_INT64;
    static const uint64_t MAX_ARY_INDEX = (1ULL<<(64-16-1))-3;    //at most 2**47 = 128T indexes.
public:
    //uint64_t: encode & decode
    static const uint64_t ENCODE_1B_MAX = 128UL-1;
    static const uint8_t ENCODE_1B_BASE = 0;
    static const uint64_t ENCODE_2B_MAX = 128UL*128UL-1;
    static const uint8_t ENCODE_2B_BASE = 0x80;
    static const uint64_t ENCODE_3B_MAX = 128UL*128UL*128UL-1;
    static const uint8_t ENCODE_3B_BASE = 0x80+0x40;
    static const uint64_t ENCODE_4B_MAX = 128UL*128UL*128UL*128UL-1;
    static const uint8_t ENCODE_4B_BASE = 0x80+0x40+0x20;
    static const uint64_t ENCODE_5B_MAX = 128UL*128UL*128UL*128UL*128UL-1;
    static const uint8_t ENCODE_5B_BASE = 0x80+0x40+0x20+0x10;
    static const uint64_t ENCODE_6B_MAX = 128UL*128UL*128UL*128UL*128UL*128UL-1;
    static const uint8_t ENCODE_6B_BASE = 0x80+0x40+0x20+0x10+0x08;
    static const uint64_t ENCODE_7B_MAX = 128UL*128UL*128UL*128UL*128UL*128UL*128UL-1;
    static const uint8_t ENCODE_7B_BASE = 0x80+0x40+0x20+0x10+0x08+0x04;
    static const uint8_t ENCODE_8B_BASE = 0x80+0x40+0x20+0x10+0x08+0x04+0x02;
    //at least 9 bytes space
    static int encode_num(
        const uint64_t data,
        char * buf,
        const uint64_t buf_size,
        uint64_t & data_pos) {
        int log_level = DF_UL_LOG_NONE;
        int err = 0;
        int j = 0;
        unsigned char base = 0;

        if (NULL == buf || (data_pos+9) > buf_size) {
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_NUM_ENCODE_BUF);
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, ERRINFO_BT_NUM_ENCODE_BUF);
        } else {
            if (data <= ENCODE_1B_MAX) {
                j = (1-1)*8;
                base = ENCODE_1B_BASE;
            } else if (data <= ENCODE_2B_MAX) {
                j = (2-1)*8;
                base = ENCODE_2B_BASE;
            } else if (data <= ENCODE_3B_MAX) {
                j = (3-1)*8;
                base = ENCODE_3B_BASE;
            } else if (data <= ENCODE_4B_MAX) {
                j = (4-1)*8;
                base = ENCODE_4B_BASE;
            } else if (data <= ENCODE_5B_MAX) {
                j = (5-1)*8;
                base = ENCODE_5B_BASE;
            } else if (data <= ENCODE_6B_MAX) {
                j = (6-1)*8;
                base = ENCODE_6B_BASE;
            } else if (data <= ENCODE_7B_MAX) {
                j = (7-1)*8;
                base = ENCODE_7B_BASE;
            } else {
                buf[data_pos++] = (unsigned char)ENCODE_8B_BASE;
                j = (8-1)*8;
                base = 0;
            }
            buf[data_pos++] = (unsigned char)base|(unsigned char)(data>>j);
            while(j > 0) {
                j -= 8;
                buf[data_pos++] = (unsigned char)(data>>j);
            }
        }
        return err;
    };
    //at least 9 bytes data
    static int decode_num(
        uint64_t & data,
        const char * buf,
        const uint64_t data_len,
        uint64_t & data_pos) {
        int log_level = DF_UL_LOG_NONE;
        int err = 0;
        int j = 0;
        unsigned char base = 0;

        if (NULL == buf || (data_pos+9) > data_len) {
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_NUM_DECODE_BUF);
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, ERRINFO_BT_NUM_DECODE_BUF);
        } else {
            data = 0;
            base = buf[data_pos++];
            if (base < ENCODE_2B_BASE) {
                j = (1-1);
                data = base - ENCODE_1B_BASE;
            } else if (base < ENCODE_3B_BASE) {
                j = (2-1);
                data = base - ENCODE_2B_BASE;
            } else if (base < ENCODE_4B_BASE) {
                j = (3-1);
                data = base - ENCODE_3B_BASE;
            } else if (base < ENCODE_5B_BASE) {
                j = (4-1);
                data = base - ENCODE_4B_BASE;
            } else if (base < ENCODE_6B_BASE) {
                j = (5-1);
                data = base - ENCODE_5B_BASE;
            } else if (base < ENCODE_7B_BASE) {
                j = (6-1);
                data = base - ENCODE_6B_BASE;
            } else if (base < ENCODE_8B_BASE) {
                j = (7-1);
                data = base - ENCODE_7B_BASE;
            } else {
                j = (8-1);
                data = buf[data_pos++];
            }
            while(j > 0) {
                --j;
                data = (data<<8)|((unsigned char)buf[data_pos++]);
            }
        }
        return err;
    };

};

template<typename T>
class dfs_gc_du_t : public T {
private:
    typedef T *PT;
    typedef const T *PCT;
public:
    dfs_gc_du_t() : T() {
    };
    const dfs_gc_du_t & operator=(const dfs_gc_du_t & src) {
        if (&src != this) {
            *((T *)this) = (const T &)src;
        }
        return *this;
    };
    const dfs_gc_du_t & operator=(const T & tobj) {
        if (&tobj != this) {
            *((T *)this) = tobj;
        }
        return *this;
    };
    dfs_gc_du_t(const dfs_gc_du_t & src) : T((const T &)src) {
        //*this = src;
    };
    dfs_gc_du_t(const T & tobj) {
        if ((T *)this != &tobj) {
            *((T *)this) = tobj;
        }
    };
    ~dfs_gc_du_t() {
    };
    void init(void) { //��ʼ������
        T::init();
    };
public:
    //����ʱ�����Ĳ���
    inline int action_while_added(void * /*pgc_info*/,
                                  const uint64_t /*t_index*/) {
        return 0;
    };
    //ɾ��ǰ�����Ĳ���
    inline int action_before_gc(void * /*pgc_info*/,
                                const uint64_t /*t_index*/) {
        return 0;
    };
};



//dfs_2d_base_t����¼һ�������Ƿ�ʹ�ã������ʹ���У����¼����Ӧ�ü���(���2**60-3)��
//  ��������ɶ������¼��ָ����һ�����ɶ����index(���2**60-3)
//  ���ü�������һ�������index����ͬʱ��ʾ
//  �ڵ�������ͷ�ģʽ�£���Ϊ���ü���������Ϊ�����������̰߳�ȫ��(�������̼߳Ӽ�Ԫ��)
//�������Ϊ��ά����dfs_bt2d_ary_t���������͵Ļ��࣬Ҳ�����������ɻ��ն���Ļ���
class dfs_2d_base_t : public dfs_bt_const_t {
private:
    //highest bit: 1 for free, 0 for in_use
    //lowest 60 bits: for ref_counter or ~next_index
    volatile uint64_t _ref_counter_next_index;
private:
    enum cconst_private {
        FREE_FLAG_BIT   = 63,    //this unit is in use
        PAY_LOAD_BITS   = 60,
    };
    //��Ч�غ�...
    const static uint64_t PAY_LOAD_MASK = ((((uint64_t)0x1)<<PAY_LOAD_BITS)-1);
    const static uint64_t FREE_FLAG = ((uint64_t)0x1)<<FREE_FLAG_BIT;
    const static uint64_t MAX_REF_COUNTER = PAY_LOAD_MASK-3;
public:
    static uint64_t extract_next_index(const uint64_t raw_data) {
        return ((raw_data & PAY_LOAD_MASK) == 0) ? UNDEF_INDEX : (~raw_data & PAY_LOAD_MASK);
    };
    static uint64_t fill_next_index(const uint64_t raw_data, const uint64_t next_index) {
        return (raw_data & ~PAY_LOAD_MASK) | (~next_index & PAY_LOAD_MASK);
    };
public:
    dfs_2d_base_t() : _ref_counter_next_index(0) {
    };
    const dfs_2d_base_t & operator=(const dfs_2d_base_t & src) {
        return *this;
    };
    dfs_2d_base_t(const dfs_2d_base_t & src) : _ref_counter_next_index(0) {
        *this = src;
    };
    ~dfs_2d_base_t() {
    };
    void init(void) { //��ʼ������
        _ref_counter_next_index &= PAY_LOAD_MASK;
    };
    uint64_t get_raw_data(void) const {
        return _ref_counter_next_index;
    };
public:
    bool is_in_use(void) const {
        return ((_ref_counter_next_index&FREE_FLAG) == 0);
    };
    void set_in_use(void) {
        df_atomic_exchange(&_ref_counter_next_index, 0);
    };
    bool is_free(void) const {
        return ((_ref_counter_next_index&FREE_FLAG) != 0);
    };
    void set_free(void) {
        //also set the next as UNDEF_INDEX
        df_atomic_exchange(&_ref_counter_next_index, FREE_FLAG);
    };
    //
    //��next_item(��indexΪnext_index)����free_list��
    //thisָ��next_item��next_itemָ��ԭ��thisָ��
    int add_to_list(const uint64_t next_index, dfs_2d_base_t & next_item) {
        uint64_t cv = 0;
        const uint64_t nv = fill_next_index(_ref_counter_next_index, next_index);
        int log_level = DF_UL_LOG_NONE;
        int err = 0;
        //������һ��free index...
        if (is_in_use()) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "set_next_index() to an in_use obj");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_FREE_IN_USE_STATE);
        } else {
            do {
                cv = _ref_counter_next_index;
                next_item._ref_counter_next_index = cv;
            } while(df_atomic_compare_exchange(&_ref_counter_next_index, nv, cv) != cv);
        }

        return err;
    };
    //
    //��thisָ���next���߲���thisָ��third_index
    int remove_from_list(const uint64_t cv, const uint64_t third_index, bool & is_removed) {
        const uint64_t nv = fill_next_index(_ref_counter_next_index, third_index);
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        is_removed = false;
        if (is_in_use()) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "set_next_index() to an in_use obj");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_FREE_IN_USE_STATE);
        } else {
            if(df_atomic_compare_exchange(&_ref_counter_next_index, nv, cv) == cv) {
                is_removed = true;
            }
        }

        return err;
    };
    uint64_t get_next_index(void) const {
        uint64_t next_index = UNDEF_INDEX;

        if (is_free() && (_ref_counter_next_index & PAY_LOAD_MASK) != 0) {
            next_index = (~_ref_counter_next_index) & PAY_LOAD_MASK;
        }

        return next_index;
    };
public:
    void clear_ref_counter(void) {
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        if (is_free()) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "clear_ref_counter() to a free obj");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_FREE_IN_USE_STATE);
        } else {
            df_atomic_exchange(&_ref_counter_next_index, 0);
        }

        return;
    };
    //Return: 0 for success, other values for error
    int inc_ref_counter(uint64_t & ref_counter) {
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        if ((ref_counter = df_atomic_inc(&_ref_counter_next_index)) >= MAX_REF_COUNTER) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_ref_counter >= MAX_REF_COUNTER");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_REF_COUNTER);
            ref_counter = UNDEF_REF_COUNTER;
        }

        return err;
    };
    //if (ref_counter != 0)��+1������ʲôҲ����
    //Return: 0 for success, other values for error
    int inc_ref_counter_if_not_zero(uint64_t & ref_counter) {
        uint64_t pre_v = _ref_counter_next_index;   //����v��ԭʼֵ��cv
        uint64_t org_v = pre_v;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        ref_counter = 0;
        // ���ԭʼֵref_counterΪ0�����߶�����free�ģ��򷵻�
        while (org_v != 0 && (org_v & FREE_FLAG) == 0) {
            pre_v = df_atomic_compare_exchange(&_ref_counter_next_index, org_v+1, org_v);
            if (pre_v == org_v) {
                //���*pvԭʼֵ��*pv��ȣ���û�������̶߳�*p���в������ɹ����+1����
                ++org_v;
                break;
            } else {
                //�������߳��Ѿ���*pv���˲�������org_v����Ϊ��ǰ������߳��޸ĺ��ֵ������
                org_v = pre_v;
            }
        }

        if ((org_v & FREE_FLAG) == 0) {
            if ((ref_counter = org_v) >= MAX_REF_COUNTER) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "ref_counter >= MAX_REF_COUNTER");
                err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_REF_COUNTER);
                ref_counter = 0;
            }
        }

        return err;
    };
    //Return: 0 for success, other values for error
    int dec_ref_counter(uint64_t & ref_counter) {
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        if ((ref_counter = df_atomic_dec(&_ref_counter_next_index)) >= MAX_REF_COUNTER) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_ref_counter >= MAX_REF_COUNTER");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_REF_COUNTER);
            ref_counter = UNDEF_REF_COUNTER;
        }

        return err;
    };
    uint64_t get_ref_counter(void) const {
        uint64_t ref_counter = _ref_counter_next_index;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        if (ref_counter >= MAX_REF_COUNTER) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_ref_counter >= MAX_REF_COUNTER");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_REF_COUNTER);
            ref_counter = UNDEF_REF_COUNTER;
        }

        return ref_counter;
    };
};

//��ά����dfs_bt2d_ary_t���������͵Ļ���
template<typename T>
class dfs_2d_du_t : public dfs_2d_base_t, public T {
private:
    typedef T *PT;
    typedef const T *PCT;
public:
    dfs_2d_du_t() : dfs_2d_base_t(), T() {
    };
    const dfs_2d_du_t & operator=(const dfs_2d_du_t<T> & src) {
        if (&src != this) {
            *((dfs_2d_base_t *)this) = (const dfs_2d_base_t &)src;
            *((T *)this) = (const T &)src;
        }
        return *this;
    };
    dfs_2d_du_t(const dfs_2d_du_t<T> & src) : dfs_2d_base_t((const dfs_2d_base_t &)src), T((const T &)src) {
        //*this = src;
    };
    dfs_2d_du_t(const T & tobj) {
        if ((T *)this != &tobj) {
            init();
            *((T *)this) = tobj;
        }
    };
    ~dfs_2d_du_t() {
    };
    void init(void) { //��ʼ������
        dfs_2d_base_t::init();
        T::init();
    };
};



class dfs_u_int64_t : public dfs_bt_const_t {
private:
    uint64_t _data;
public:
    dfs_u_int64_t(const uint64_t data = 0) {
        _data = data;
    };
    dfs_u_int64_t(const dfs_u_int64_t & src) {
        *this = src;
    };
    ~dfs_u_int64_t() {
    };
    const dfs_u_int64_t & operator=(const dfs_u_int64_t & src) {
        if (&src != this) {
            _data = src._data;
        }
        return *this;
    };
    void init(const uint64_t data = 0) { //��ʼ������
        _data = data;
    };
public:
    void set_data(const uint64_t data) {
        _data = data;
    };
    uint64_t get_data(void) const {
        return _data;
    };
    void set_index(const uint64_t index) {
        _data = index;
    };
    uint64_t get_index(void) const {
        return _data;
    };
public:
    bool operator==(const dfs_u_int64_t & src) const {
        return (_data == src._data);
    };
    bool operator!=(const dfs_u_int64_t & src) const {
        return (_data != src._data);
    };
    bool operator>=(const dfs_u_int64_t & src) const {
        return (_data >= src._data);
    };
    bool operator<=(const dfs_u_int64_t & src) const {
        return (_data <= src._data);
    };
    bool operator>(const dfs_u_int64_t & src) const {
        return (_data > src._data);
    };
    bool operator<(const dfs_u_int64_t & src) const {
        return (_data < src._data);
    };
public:
    //�û���������T��Ҫ֧�����½ӿڣ�
    //  ���ܣ���ø�ʵ��store�����ݳ���(�ֽ�)����ֵ���ܳ���MAX_T_STORE_SIZE
    static uint64_t get_store_size(void) {
        return sizeof(uint64_t);
    };
    //
    //  ���ܣ��洢��ʵ����buf
    //  ���룺data_posΪ�洢���ݵ���ʼλ��
    //  �����data_posΪ�洢T���µ���ʼλ��
    //  ���أ�for success, other values for error (���绺����ʣ��ռ䲻��)
    int store(char * buf, const uint64_t buf_size, uint64_t & data_pos) const {
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        if ((buf_size-data_pos) < sizeof(uint64_t)) {
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_STORE_POS);
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, ERRINFO_BT_STORE_POS);
        } else if (NULL == buf) {
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_STORE_BUF);
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, ERRINFO_BT_STORE_BUF);
        } else {
            *((uint64_t *)(buf+data_pos)) = get_data();
            data_pos += sizeof(uint64_t);
        }

        return err;
    };
    //
    //  ���ܣ���buf��װ�ص���ʵ��
    //  ���룺data_posΪװ��Tǰ������������λ��
    //  �����data_posΪװ��T�󻺳���������λ��
    //  ���أ�for success, other values for error(���绺���������ݲ���)
    int load(const char * buf, const uint64_t data_len, uint64_t & data_pos) {
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        if ((data_len-data_pos) < sizeof(uint64_t)) {
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_LOAD_POS);
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, ERRINFO_BT_LOAD_POS);
        } else if (NULL == buf) {
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_LOAD_BUF);
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, ERRINFO_BT_LOAD_BUF);
        } else {
            set_data(*((uint64_t *)(buf+data_pos)));
            data_pos += sizeof(uint64_t);
        }

        return err;
    };
};


class dfs_id_t : public dfs_bt_const_t {
private:
    uint64_t _id;
public:
    dfs_id_t() {
        _id = UNDEF_ID;
    };
    dfs_id_t(const uint64_t id) {
        _id = id;
    };
    dfs_id_t(const dfs_id_t & src) {
        *this = src;
    };
    ~dfs_id_t() {
    };
    const dfs_id_t & operator=(const dfs_id_t & src) {
        if (&src != this) {
            _id = src._id;
        }
        return *this;
    };
    void init(const uint64_t id = UNDEF_ID) { //��ʼ������
        _id = id;
    };
public:
    void set_id(const uint64_t id) {
        _id = id;
    };
    uint64_t get_id(void) const {
        return _id;
    };
public:
    bool operator==(const dfs_id_t & src) const {
        return (_id == src._id);
    };
    bool operator!=(const dfs_id_t & src) const {
        return (_id != src._id);
    };
    bool operator>=(const dfs_id_t & src) const {
        return (_id >= src._id);
    };
    bool operator<=(const dfs_id_t & src) const {
        return (_id <= src._id);
    };
    bool operator>(const dfs_id_t & src) const {
        return (_id > src._id);
    };
    bool operator<(const dfs_id_t & src) const {
        return (_id < src._id);
    };
public:
    bool operator==(const uint64_t & id) const {
        return (_id == id);
    };
    bool operator!=(const uint64_t & id) const {
        return (_id != id);
    };
    bool operator>=(const uint64_t & id) const {
        return (_id >= id);
    };
    bool operator<=(const uint64_t & id) const {
        return (_id <= id);
    };
    bool operator>(const uint64_t & id) const {
        return (_id > id);
    };
    bool operator<(const uint64_t & id) const {
        return (_id < id);
    };
public:
    //�û���������T��Ҫ֧�����½ӿڣ�
    //  ���ܣ���ø�ʵ��store�����ݳ���(�ֽ�)����ֵ���ܳ���MAX_T_STORE_SIZE
    static uint64_t get_store_size(void) {
        return sizeof(uint64_t);
    };
    //
    //  ���ܣ��洢��ʵ����buf
    //  ���룺data_posΪ�洢���ݵ���ʼλ��
    //  �����data_posΪ�洢T���µ���ʼλ��
    //  ���أ�for success, other values for error (���绺����ʣ��ռ䲻��)
    int store(char * buf, const uint64_t buf_size, uint64_t & data_pos) const {
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        if ((buf_size-data_pos) < sizeof(uint64_t)) {
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_STORE_POS);
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, ERRINFO_BT_STORE_POS);
        } else if (NULL == buf) {
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_STORE_BUF);
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, ERRINFO_BT_STORE_BUF);
        } else {
            *((uint64_t *)(buf+data_pos)) = get_id();
            data_pos += sizeof(uint64_t);
        }

        return err;
    };
    //
    //  ���ܣ���buf��װ�ص���ʵ��
    //  ���룺data_posΪװ��Tǰ������������λ��
    //  �����data_posΪװ��T�󻺳���������λ��
    //  ���أ�for success, other values for error(���绺���������ݲ���)
    int load(const char * buf, const uint64_t data_len, uint64_t & data_pos) {
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        if ((data_len-data_pos) < sizeof(uint64_t)) {
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_LOAD_POS);
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, ERRINFO_BT_LOAD_POS);
        } else if (NULL == buf) {
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_LOAD_BUF);
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, ERRINFO_BT_LOAD_BUF);
        } else {
            set_id(*((uint64_t *)(buf+data_pos)));
            data_pos += sizeof(uint64_t);
        }

        return err;
    };
};


class dfs_id_data_t : public dfs_id_t {
private:
    uint64_t _data;
public:
    dfs_id_data_t() : dfs_id_t() {
        _data = MAX_U_INT64;
    };
    dfs_id_data_t(const dfs_id_data_t & src) : dfs_id_t((const dfs_id_t &)src), _data(src._data) {
        ;
    };
    ~dfs_id_data_t() {
    };
    const dfs_id_data_t & operator=(const dfs_id_data_t & src) {
        if (&src != this) {
            *((dfs_id_t *)this) = (const dfs_id_t &)src;
            _data = src._data;
        }
        return *this;
    };
    void init() { //��ʼ������
        dfs_id_t::init();
        _data = MAX_U_INT64;
    };
public:
    void set_data(const uint64_t data) {
        _data = data;
    };
    uint64_t get_data(void) const {
        return _data;
    };
public:
    //�û���������T��Ҫ֧�����½ӿڣ�
    //  ���ܣ���ø�ʵ��store�����ݳ���(�ֽ�)����ֵ���ܳ���MAX_T_STORE_SIZE
    static uint64_t get_store_size(void) {
        return (dfs_id_t::get_store_size()+sizeof(uint64_t));
    };
    //
    //  ���ܣ��洢��ʵ����buf
    //  ���룺data_posΪ�洢���ݵ���ʼλ��
    //  �����data_posΪ�洢T���µ���ʼλ��
    //  ���أ�for success, other values for error (���绺����ʣ��ռ䲻��)
    int store(char * buf, const uint64_t buf_size, uint64_t & data_pos) const {
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        if ((err = dfs_id_t::store(buf, buf_size, data_pos)) == 0) {
            if ((buf_size-data_pos) < sizeof(uint64_t)) {
                err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_STORE_POS);
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, ERRINFO_BT_STORE_POS);
            } else if (NULL == buf) {
                err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_STORE_BUF);
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, ERRINFO_BT_STORE_BUF);
            } else {
                *((uint64_t *)(buf+data_pos)) = get_data();
                data_pos += sizeof(uint64_t);
            }
        }

        return err;
    };
    //
    //  ���ܣ���buf��װ�ص���ʵ��
    //  ���룺data_posΪװ��Tǰ������������λ��
    //  �����data_posΪװ��T�󻺳���������λ��
    //  ���أ�for success, other values for error(���绺���������ݲ���)
    int load(const char * buf, const uint64_t data_len, uint64_t & data_pos) {
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        if ((err = dfs_id_t::load(buf, data_len, data_pos)) == 0) {
            if ((data_len-data_pos) < sizeof(uint64_t)) {
                err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_LOAD_POS);
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, ERRINFO_BT_LOAD_POS);
            } else if (NULL == buf) {
                err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_LOAD_BUF);
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, ERRINFO_BT_LOAD_BUF);
            } else {
                set_data(*((uint64_t *)(buf+data_pos)));
                data_pos += sizeof(uint64_t);
            }
        }

        return err;
    };
};



//  ���ܣ�����ͨ����һ���ṩ�����±��������ʣ������Զ���չ�����С��
//        ��֧�ֶ��߳�SingleWriteMultipleRead���ʡ�
//  T: �����࣬��Ҫʵ�����³�Ա������
//      const T & operator=(const T & src);
//      const T & operator==(const T & src);
template<typename T, uint32_t ROW_SIZE = dfs_bt_const_t::DEF_ROW_SIZE> //���鳣��ROW_SIZE��2�ķ���
class dfs_s_ary_t : virtual public dfs_bt_const_t {
private:
    typedef T *PT;
    typedef const T *PCT;
private:
    enum cconst {
        MIN_ROW_ARY_SIZE    = DEF_ROW_ARY_SIZE ,
        //MIN_ROW_SIZE        = 256   ,
    };
private:
    dfs_init_t _init_state;
    T ** _t_row_ary;
    uint64_t _t_row_ary_size;
    uint64_t _t_index_for_next_row;
    uint64_t _max_kept_key_index;
    uint64_t _kept_key_num;
    const T _init_t_value;
public:
    dfs_s_ary_t(const T & init_t_value) : _init_t_value(init_t_value) {
        _t_row_ary = NULL;

        init();
    };

    virtual ~dfs_s_ary_t() {
        if (NULL != _t_row_ary) {
            for (uint64_t j = 0; j < _t_row_ary_size; ++j) {
                if (NULL != _t_row_ary[j]) {
                    delete[] _t_row_ary[j];
                    _t_row_ary[j] = NULL;
                }
            }
            delete[] _t_row_ary;
            _t_row_ary = NULL;
        }
    };
    bool is_valid_t_index(const uint64_t t_index) const {
        return (t_index < _t_index_for_next_row);
    };
    int init(void) {
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        if (_init_state.is_not_init_yet()) {
            _t_row_ary_size = MIN_ROW_ARY_SIZE;
            _t_index_for_next_row = 0;
            _max_kept_key_index = 0;
            _kept_key_num = 0;

            if (NULL != _t_row_ary) {
                err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_NOT_NULL_BUF_POINTER);
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "NULL != _t_row_ary");
            } else if ((_t_row_ary = new(std::nothrow) T*[_t_row_ary_size]) == NULL) {
                err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_NOMEM);
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, ERRINFO_BT_NOMEM, "_t_row_ary");
            } else {
                for (uint64_t j = 0; j < _t_row_ary_size; ++j) {
                    _t_row_ary[j] = NULL;
                }
            }

            if (DF_UL_LOG_NONE != log_level) {
                DF_WRITE_LOG_US(log_level,
                                "_t_row_ary=0x%p, _t_row_ary_size=%ld",
                                _t_row_ary, _t_row_ary_size);
            }

            _init_state.set_init_result(err);
        } else {
            err = _init_state.get_init_result();
        }

        return err;
    };
private:
    //����row_ary
    //return: 0 for success, other values for error
    int _enlarge_row_ary(void) {
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        if ((_t_index_for_next_row/ROW_SIZE) >= _t_row_ary_size) {
            //��ǰ_t_row_ary��������
            //typedef T *PT;
            uint64_t j = 0;
            uint64_t new_t_row_ary_size = _t_row_ary_size * 2;
            uint64_t old_t_row_ary_size = 0;
            T ** new_t_row_ary = new(std::nothrow) T*[new_t_row_ary_size];
            T ** old_t_row_ary = NULL;

            if (NULL == new_t_row_ary) {
                err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_NOMEM);
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, ERRINFO_BT_NOMEM, "new_t_row_ary");
            } else {
                j = 0;
                for (; j < _t_row_ary_size; ++j) {
                    //df_atomic_exchange_pointer((volatile pvoid *)&(new_t_row_ary[j]), (pvoid)_t_row_ary[j]);
                    new_t_row_ary[j] = _t_row_ary[j];
                }
                for (; j < new_t_row_ary_size; ++j) {
                    new_t_row_ary[j] = NULL;
                }
                old_t_row_ary = (T **)df_atomic_exchange_pointer(
                                    (volatile pvoid *)&(_t_row_ary),
                                    (pvoid)new_t_row_ary);
                old_t_row_ary_size = df_atomic_exchange(
                                         (volatile uint64_t *)&_t_row_ary_size,
                                         new_t_row_ary_size);
                delete [] old_t_row_ary;
                old_t_row_ary = NULL;
            }
        }

        return err;
    };
    //����һ������
    //return: 0 for success, other values for error
    int _alloc_new_row(void) {
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        if ((_t_index_for_next_row/ROW_SIZE) >= _t_row_ary_size) {
            //��ǰ_t_row_ary��������
            err = _enlarge_row_ary();
        }

        if (0 == err) {
            if ((_t_index_for_next_row/ROW_SIZE) < _t_row_ary_size) {
                if (NULL == _t_row_ary[_t_index_for_next_row/ROW_SIZE]) {
                    if ((_t_row_ary[_t_index_for_next_row/ROW_SIZE] =
                                new(std::nothrow) T[ROW_SIZE]) == NULL) {
                        err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_NOMEM);
                        log_level = DF_UL_LOG_FATAL;
                        DF_WRITE_LOG_US(
                            log_level,
                            ERRINFO_BT_NOMEM,
                            "_t_row_ary[_t_index_for_next_row/ROW_SIZE]");
                    }
                }
                if (NULL != _t_row_ary[_t_index_for_next_row/ROW_SIZE]) {
                    T * head_t = NULL;
                    uint64_t j = 0;

                    head_t = _t_row_ary[_t_index_for_next_row/ROW_SIZE];
                    for (j = 0; j < ROW_SIZE; ++j) {
                        *head_t = _init_t_value;
                        ++head_t;
                    }
                    _t_index_for_next_row += ROW_SIZE;
                }
            } else {
                err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_BUF_SIZE);
            }
        }

        return err;
    };
private:
    //return: 0 for success, other values for error
    int _get_t(const uint64_t t_index, PT & pt) const {
        int err = 0;

        pt = NULL;
        if (is_valid_t_index(t_index)) {
            pt = &(((_t_row_ary[t_index/ROW_SIZE])[t_index%ROW_SIZE]));
        } else {
            err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
            pt = NULL;
        }

        return err;
    };
public:
    //return: 0 for success, other values for error
    int get_t(const uint64_t orig_t_index, T & tobj) const {
        const uint64_t true_t_index = orig_t_index;
        T * pt = NULL;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        tobj = _init_t_value;
        if (UNDEF_INDEX == orig_t_index || true_t_index > _max_kept_key_index) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "UNDEF_INDEX == orig_t_index || true_t_index > _max_kept_key_index");
            DF_WRITE_LOG_US(log_level,
                            "orig_t_index=0x%lx, true_t_index=0x%lx, _max_kept_key_index=0x%lx",
                            orig_t_index, true_t_index, _max_kept_key_index);
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_INVALID_INDEX);
            //err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
        } else if ((err = _get_t(true_t_index, pt)) == 0) {
            tobj = *pt;
        }

        return err;
    };
    int put_t(const uint64_t orig_t_index, const T & tobj) {
        const uint64_t true_t_index = orig_t_index;
        T * pt = NULL;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        if (UNDEF_INDEX == orig_t_index || true_t_index > MAX_ARY_INDEX) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "UNDEF_INDEX == orig_t_index || true_t_index > MAX_ARY_INDEX");
            DF_WRITE_LOG_US(log_level,
                            "orig_t_index=0x%lx, true_t_index=0x%lx",
                            orig_t_index, true_t_index);
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_INVALID_INDEX);
            //err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_INVALID_INDEX);
        } else {
            while (true_t_index >= _t_index_for_next_row) {
                if ((err = _alloc_new_row()) != 0) {
                    break;
                }
            }
            if (0 == err) {
                if ((err = _get_t(true_t_index, pt)) != 0) {
                    err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_INVALID_INDEX);
                } else if (_init_t_value != *pt) {
                    //�����Ϊ��ʼֵ
                    err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_INVALID_INDEX);
                } else {
                    *pt = tobj;
                }
            }

            if (true_t_index > _max_kept_key_index) {
                _max_kept_key_index = true_t_index;
            }
            ++_kept_key_num;
        }

        return err;
    };
public:
    //clear all data and set to tobj
    int clear(void) {
        T * head_t = NULL;
        uint64_t j = 0;
        uint32_t k = 0;
        int err = 0;

        for (j = 0; j < _t_row_ary_size; ++j) {
            if ((head_t = _t_row_ary[j]) != NULL) {
                for (k = 0; k < ROW_SIZE; ++k) {
                    *head_t = _init_t_value;
                    ++head_t;
                }
            }
        }

        _t_index_for_next_row = 0;
        _max_kept_key_index = 0;
        _kept_key_num = 0;

        return err;
    };
    uint64_t get_max_kept_key_index(void) const {
        return _max_kept_key_index;
    };
    uint64_t get_kept_key_num(void) const {
        return _kept_key_num;
    };
    uint64_t get_mem_size(void) const {
        return (_t_row_ary_size*sizeof(T *)+_t_index_for_next_row*sizeof(T));
    };
};

class dfs_rc_set_base_t;
class dfs_rc_indexer_t;
//reference counter setter..
class dfs_rc_set_base_t : virtual public dfs_bt_const_t {
    friend class dfs_rc_indexer_t;
public:
    dfs_rc_set_base_t() {
        ;
    };
    virtual ~dfs_rc_set_base_t() {
        ;
    };
protected:
    //Return��0 for success, other values for error
    virtual int _vir_inc_t_ref_counter(const uint64_t index, uint64_t & ref_counter) const = 0;
    virtual int _vir_dec_t_ref_counter(const uint64_t index, uint64_t & ref_counter) const = 0;
};

//reference count indexer..
class dfs_rc_indexer_t : public dfs_bt_const_t {
    friend class dfs_rc_indexer_ext_t;
protected:
    dfs_rc_set_base_t * _prc_set;
    uint64_t _index;
    uint64_t _user_def;   //�û����壬�������
public:
    dfs_rc_indexer_t() :
        _prc_set(NULL), _index(UNDEF_INDEX), _user_def(0) {
    };
    ~dfs_rc_indexer_t() {
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        if ((err = _clear_indexer()) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_clear_indexer() returns 0x%x", err);
            DF_WRITE_LOG_US(log_level, "_index=%ld", _index);
        }
        _prc_set = NULL;
        _index = UNDEF_INDEX;
    };
    //���ƶ������������ü���
    const dfs_rc_indexer_t & operator=(const dfs_rc_indexer_t & src) {
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        if (&src != this) {
            //ռ����ôһ��index..
            if ((err = this->_set_indexer(&src)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_set_indexer() returns 0x%x", err);
                DF_WRITE_LOG_US(log_level, "_index=%ld, src._index=%ld", _index, src._index);
            }
            this->_user_def = src._user_def;
        }

        return *this;
    };
    //���������������Ӧ�����ü���
    dfs_rc_indexer_t(const dfs_rc_indexer_t & src) :
        _prc_set(NULL), _index(UNDEF_INDEX), _user_def(0) {
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        if (&src != this) {
            _user_def = src._user_def;
            if ((err = this->_set_indexer(&src)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_set_indexer() returns 0x%x", err);
                DF_WRITE_LOG_US(log_level, "_index=%ld, src._index=%ld", _index, src._index);
            }
        }

        return;
    };
    int init(void) {
        int log_level = DF_UL_LOG_NONE;
        int err = 0;
        //�����UNDEF INDEX�Ļ�.
        if ((err = _clear_indexer()) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_clear_index() returns 0x%x", err);
            DF_WRITE_LOG_US(log_level, "_index=%ld", _index);
        }
        //_prc_set = NULL;
        _index = UNDEF_INDEX;

        return err;
    };
public:
    bool is_valid(void) const {
        return (UNDEF_INDEX != _index);
    };
    void set_user_def(const uint64_t user_def) {
        _user_def = user_def;
    };
    uint64_t get_user_def(void) const {
        return _user_def;
    };
private:
    int _set_indexer(const dfs_rc_indexer_t * psrc) {
        uint64_t ref_counter = 0;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        //�൱�ڿ���ת��...
        //����psrc��index������,Ȼ���ͷű����index����...
        if (NULL != psrc && UNDEF_INDEX != psrc->_index) {
            if ((err = psrc->_prc_set->_vir_inc_t_ref_counter(psrc->_index, ref_counter)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_vir_inc_t_ref_counter() returns 0x%x", err);
            } else if (ref_counter <= 1) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "ref_counter <= 1");
                err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_REF_COUNTER);
            }
        }

        if (UNDEF_INDEX != this->_index) {
            if ((err = this->_prc_set->_vir_dec_t_ref_counter(this->_index, ref_counter)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_vir_dec_t_ref_counter() returns 0x%x", err);
            }
        }

        if (NULL != psrc) {
            this->_prc_set = psrc->_prc_set;
            this->_index = psrc->_index;
            this->_user_def = psrc->_user_def;
        } else {
            this->_index = UNDEF_INDEX;
        }

        if (DF_UL_LOG_NONE != log_level) {
            DF_WRITE_LOG_US(log_level, "_index=%ld, psrc->_index=%ld, ref_counter=%ld",
                            _index,
                            (NULL != psrc && UNDEF_INDEX != psrc->_index) ? psrc->_index : UNDEF_INDEX,
                            ref_counter);
        }

        return err;
    };
    int _clear_indexer(void) {
        uint64_t ref_counter = 0;
        //const uint64_t old_index = this->_index;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        if (UNDEF_INDEX != this->_index) {
            if ((err = this->_prc_set->_vir_dec_t_ref_counter(this->_index, ref_counter)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_vir_dec_t_ref_counter() returns 0x%x", err);
            }
        }

        this->_index = UNDEF_INDEX;

        if (DF_UL_LOG_NONE != log_level) {
            //DF_WRITE_LOG_US(log_level, "old_index=%ld, ref_counter=%ld",
            //old_index, ref_counter);
            DF_WRITE_LOG_US(log_level, "old_index=%ld, ref_counter=%ld",
                            this->_index, ref_counter);
        }

        return err;
    };
};

class dfs_rc_indexer_ext_t : public dfs_rc_indexer_t {
public:
    dfs_rc_indexer_ext_t(const dfs_rc_indexer_t & rc_indexer) {
        _prc_set = rc_indexer._prc_set;
        _index = rc_indexer._index;
        _user_def = rc_indexer._user_def;
    };
    dfs_rc_indexer_ext_t(dfs_rc_set_base_t * prc_set, const uint64_t index) {
        _prc_set = prc_set;
        _index = index;
        _user_def = 0;
    };
    ~dfs_rc_indexer_ext_t() {
        set_indexer(NULL, UNDEF_INDEX);
        _user_def = 0;
    };
    void init(void) {
        set_indexer(NULL, UNDEF_INDEX);
        _user_def = 0;
    };
    void set_indexer(dfs_rc_set_base_t * prc_set, const uint64_t index) {
        _prc_set = prc_set;
        _index = index;
    };
    void set_indexer(const uint64_t index) {
        _index = index;
    };
    dfs_rc_set_base_t * get_rc_set(void) const {
        return _prc_set;
    };
    uint64_t get_index(void) const {
        return _index;
    };
};


//recycle set������Ԫ��ѭ��ʹ��:
//ֱ�ӽ���freee_head�Ĺ���...
template<typename T, uint32_t SET_SIZE>
class dfs_rc_set_t : public dfs_rc_set_base_t {
private:
    typedef T *PT;
    typedef const T *PCT;
    typedef dfs_2d_du_t<T> DU;
    typedef dfs_2d_du_t<T> *PDU;
    typedef const dfs_2d_du_t<T> *PCDU;
private:
    dfs_init_t _init_state;
    dfs_2d_base_t _free_head;
    volatile uint64_t _free_counter;
    volatile uint64_t _cur_index;   //��ǰ��
    void * _pgc_info;   //for garbage collect purpose
    DU * _rc_ary;
#ifdef  DF_BT_STATISTIC_INFO
    //statistic information purpose
    volatile uint64_t _obj_del_counter;
    volatile uint64_t _obj_add_counter;
    volatile uint64_t _obj_ref_inc_counter;
    volatile uint64_t _obj_ref_dec_counter;
#endif
public:
    dfs_rc_set_t() {
        _free_head.init();
        _free_head.set_free();
        _free_counter = 0;
        _cur_index = UNDEF_INDEX;
        _pgc_info = NULL;
        _rc_ary = NULL;

#ifdef  DF_BT_STATISTIC_INFO
        _obj_del_counter = 0;
        _obj_add_counter = 0;
        _obj_ref_inc_counter = 0;
        _obj_ref_dec_counter = 0;
#endif

        //init();
        return;
    };
    virtual ~dfs_rc_set_t() {
        int log_level = DF_UL_LOG_NONE;
        int err = 0;
        uint64_t ref_counter = UNDEF_REF_COUNTER;

        if (UNDEF_INDEX != _cur_index) {
            //#ifdef DF_BT_PRINT_DEBUG_INFO
            //printf("~dfs_rc_set_t: line: <%d>, func: <%s>, file: <%s>\n",
            //        __LINE__, __FUNCTION__, __FILE__);
            //#endif

            if ((err = _dec_t_ref_counter(_cur_index, ref_counter)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_dec_t_ref_counter() returns 0x%x", err);
                DF_WRITE_LOG_US(log_level, "_cur_index=%ld", _cur_index);
            }
            _cur_index = UNDEF_INDEX;

            //add by zhangyan04@baidu.com
            //fixed it to avoid the valgrind warning..
            delete [] _rc_ary;

            //#ifdef DF_BT_PRINT_DEBUG_INFO
            //printf("~dfs_rc_set_t: line: <%d>, func: <%s>, file: <%s>\n",
            //        __LINE__, __FUNCTION__, __FILE__);
            //#endif
        }
    };
    int init(void * pgc_info);
private:
    const dfs_rc_set_t & operator=(const dfs_rc_set_t & src);
    dfs_rc_set_t(const dfs_rc_set_t & src);
public:
    pvoid get_gc_info(void) const {
        return _pgc_info;
    };
    //int set_gc_info(void * pgc_info)
    //{
    //    int log_level = DF_UL_LOG_NONE;
    //    int err = 0;
    //
    //    if (NULL != _pgc_info)
    //    {
    //        log_level = DF_UL_LOG_FATAL;
    //        DF_WRITE_LOG_US(log_level, "NULL != _pgc_info");
    //        err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_SET_GC_INFO);
    //    }
    //    else
    //    {
    //        _pgc_info = pgc_info;
    //    }
    //
    //    return err;
    //};
private:
#ifdef  DF_BT_STATISTIC_INFO
    //statistic information purpose
    //called whenever an obj is deleted
    uint64_t _inc_obj_del_counter(void) {
        return df_atomic_inc(&_obj_del_counter);
    };
    //
    //called whenever a new obj is added
    uint64_t _inc_obj_add_counter(void) {
        return df_atomic_inc(&_obj_add_counter);
    };
    //
    //called whenever an obj's ref_counter is incremented
    uint64_t _inc_obj_ref_inc_counter(void) {
        return df_atomic_inc(&_obj_ref_inc_counter);
    };
    //
    //called whenever an obj's ref_counter is decremented
    uint64_t _inc_obj_ref_dec_counter(void) {
        return df_atomic_inc(&_obj_ref_dec_counter);
    };
#endif
private:
    //��д�̶߳����ܵ���
    int _push_free(const uint64_t freed_index) {
        PDU pdu = NULL;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        if ((err = _get_exist_du(freed_index, pdu)) != 0 || NULL == pdu) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_exist_du() returns 0x%x", err);
            err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
        } else {
            //����Ϊfree..
            pdu->dfs_2d_base_t::set_free();
            if ((err = _free_head.add_to_list(freed_index, *((dfs_2d_base_t *)pdu))) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_free_head.add_to_list(%ld) returns 0x%x",
                                freed_index, err);
            } else {
                df_atomic_inc(&_free_counter);
            }
        }

        return err;
    };
    //ֻ��д�̵߳��ã����ֻ��һ��ʵ��
    int _pop_free(uint64_t & ret_index) {
        PDU pdu = NULL;
        uint64_t raw_data = 0;
        bool is_removed = false;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        raw_data = _free_head.get_raw_data();
        ret_index = dfs_2d_base_t::extract_next_index(raw_data);
        while (UNDEF_INDEX != ret_index) {
            is_removed = false;
            if ((err = _get_exist_du(ret_index, pdu)) != 0 || NULL == pdu) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_get_exist_du() returns 0x%x", err);
                err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
                ret_index = UNDEF_INDEX;
                break;
            }
            //���raw_data�Լ����pdu�Լ�pdu->get_next_index()����һ��ԭ�Ӳ�����
            //����ж��_pop_free�̣߳����ڻ��raw_data��pdu->get_next_index()����
            //�����仯(��ʹraw_data��ret_indexû�б仯)����˸��㷨�Զ��_pop_free()�̲߳�����ȫ��
            else if ((err = _free_head.remove_from_list(
                                raw_data,
                                pdu->dfs_2d_base_t::get_next_index(),
                                is_removed)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_free_head.remove_from_list() returns 0x%x", err);
                ret_index = UNDEF_INDEX;
                break;
            } else if (is_removed) {
                pdu->set_in_use();
                df_atomic_dec(&_free_counter);
                break;
            }
            raw_data = _free_head.dfs_2d_base_t::get_raw_data();
            ret_index = dfs_2d_base_t::extract_next_index(raw_data);
        }

        return err;
    };

private:
    //Return: 0 for success, other values for error
    int _release_t(const uint64_t t_index) {
        PDU pdu = NULL;
        uint64_t ref_counter = UNDEF_REF_COUNTER;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

#ifdef  DF_BT_STATISTIC_INFO
        _inc_obj_del_counter();
#endif

        if (!_is_valid_t_index(t_index)) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "!is_valid_t_index(t_index)");
            err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
        } else if ((err = _get_exist_du(t_index, pdu)) != 0 || NULL == pdu) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_exist_du() returns 0x%x", err);
            err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
        } else if ((ref_counter = pdu->dfs_2d_base_t::get_ref_counter()) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "ref_counter != 0");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_REF_COUNTER);
        } else if ((err = pdu->action_before_gc(get_gc_info(), t_index)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "pdu->action_before_gc(get_gc_info()...) returns 0x%x", err);
        } else {
            //Added by yangzhenkun per DTS in order to release the resource pointed by T on 20090717
            pdu->T::init();

            //
            if ((err = _push_free(t_index)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_push_free() returns 0x%x", err);
            }
        }

        if (DF_UL_LOG_NONE != log_level) {
            DF_WRITE_LOG_US(log_level,
                            "t_index=0x%lx, ref_counter=%ld, pdu=0x%p",
                            t_index, ref_counter, pdu);
        }

        return err;
    };
private:
    inline BAIDU_INLINE int _get_exist_du(const uint64_t t_index, PDU & pdu) const {
        const uint64_t true_t_index = t_index;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        pdu = NULL;
        if (true_t_index < SET_SIZE) {
            pdu = _rc_ary + true_t_index;
        } else {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "true_t_index=%ld > SET_SIZE=%d",
                            true_t_index, SET_SIZE);
            err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
        }

        if (DF_UL_LOG_NONE != log_level) {
            DF_WRITE_LOG_US(log_level,
                            "t_index=0x%lx, true_t_index=0x%lx",
                            t_index, true_t_index);
        }

        return err;
    };
    //int _get_may_not_exist_du(const uint64_t t_index, PDU & pdu) const
    //{
    //    const uint64_t true_t_index = t_index;
    //    int log_level = DF_UL_LOG_NONE;
    //    int err = 0;
    //
    //    pdu = NULL;
    //    if (_is_valid_t_index(t_index))
    //    {
    //        pdu = _rc_ary + true_t_index;
    //    }
    //    else
    //    {
    //        //err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
    //        pdu = NULL;
    //    }
    //
    //    if (DF_UL_LOG_NONE != log_level)
    //    {
    //        DF_WRITE_LOG_US(log_level,
    //                "t_index=0x%lx, true_t_index=0x%lx, pdu=0x%p",
    //                t_index, true_t_index, pdu);
    //    }
    //
    //    return err;
    //};
public:
    ////Return: 0 for success, other values for error
    //int get_exist_t_unit(const uint64_t t_index, PT & pt) const
    //{
    //    PDU pdu = NULL;
    //    int log_level = DF_UL_LOG_NONE;
    //    int err = 0;
    //
    //    pt = NULL;
    //    if ((err = _get_exist_du(t_index, pdu)) != 0 || NULL == pdu)
    //    {
    //        log_level = DF_UL_LOG_FATAL;
    //        DF_WRITE_LOG_US(log_level, "_get_exist_du() returns 0x%x", err);
    //        DF_WRITE_LOG_US(log_level,
    //                "t_index=0x%lx, pt=0x%p, pdu=0x%p",
    //                t_index, pt, pdu);
    //        err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
    //    }
    //    else
    //    {
    //        pt = ((T *)pdu);
    //    }
    //
    //    return err;
    //};
    //int get_may_not_exist_t_unit(const uint64_t t_index, PT & pt) const
    //{
    //    PDU pdu = NULL;
    //    int log_level = DF_UL_LOG_NONE;
    //    int err = 0;
    //
    //    pt = NULL;
    //    if ((err = _get_may_not_exist_du(t_index, pdu)) != 0)
    //    {
    //        log_level = DF_UL_LOG_FATAL;
    //        DF_WRITE_LOG_US(log_level, "_get_may_not_exist_du() returns 0x%x", err);
    //        DF_WRITE_LOG_US(log_level,
    //                "t_index=0x%lx, pt=0x%p, pdu=0x%p",
    //                t_index, pt, pdu);
    //    }
    //    else if (NULL != pdu)
    //    {
    //        pt = ((T *)pdu);
    //    }
    //
    //    return err;
    //};

public:
    //���룺tobj���¶����ֵ
    //������¶����indexer
    int acquire_t(const T & tobj, dfs_rc_indexer_t & rc_indexer) {
        uint64_t ref_counter = 0;
        uint64_t t_index = UNDEF_INDEX;
        PDU pdu = NULL;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        t_index = UNDEF_INDEX;

#ifdef  DF_BT_STATISTIC_INFO
        _inc_obj_add_counter();
#endif

        if ((err = _pop_free(t_index)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_pop_free() returns 0x%x", err);
        } else if (UNDEF_INDEX == t_index) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "fail to acquire t_obj");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_ACQUIRE);
            t_index = UNDEF_INDEX;
        } else {
            if ((err = _get_exist_du(t_index, pdu)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_get_exist_du() returns 0x%x", err);
                t_index = UNDEF_INDEX;
            } else if (NULL == pdu) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "NULL == pdu");
                err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
                t_index = UNDEF_INDEX;
            } else {
                pdu->init();
                *((T *)pdu) = tobj;
                if ((err = pdu->action_while_added(get_gc_info(), t_index)) != 0) {
                    log_level = DF_UL_LOG_FATAL;
                    DF_WRITE_LOG_US(log_level, "pdu->action_while_added() returns 0x%x", err);
                } else if ((err = pdu->dfs_2d_base_t::inc_ref_counter(ref_counter)) != 0) {
                    log_level = DF_UL_LOG_FATAL;
                    DF_WRITE_LOG_US(log_level, "pdu->dfs_2d_base_t::inc_ref_counter() returns 0x%x", err);
                    DF_WRITE_LOG_US(log_level, "t_index=0x%lx, pdu=0x%p", t_index, pdu);
                } else {
                    ((dfs_rc_indexer_ext_t *)&rc_indexer)->set_indexer(
                        (dfs_rc_set_base_t *)this,
                        t_index);
                }
            }
        }

        if (DF_UL_LOG_NONE != log_level) {
            DF_WRITE_LOG_US(log_level, "t_index=0x%lx", t_index);
        }

        return err;
    };

protected:
    //Return��0 for success, other values for error
    int _inc_t_ref_counter(const uint64_t t_index, uint64_t & ref_counter) {
        PDU pdu = NULL;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

#ifdef  DF_BT_STATISTIC_INFO
        _inc_obj_ref_inc_counter();
#endif

        ref_counter = UNDEF_REF_COUNTER;
        if ((err = _get_exist_du(t_index, pdu)) != 0 || NULL == pdu) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_exist_du() returns 0x%x", err);
            DF_WRITE_LOG_US(log_level, "t_index=0x%lx, pdu=0x%p", t_index, pdu);
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = pdu->dfs_2d_base_t::inc_ref_counter(ref_counter)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "pdu->dfs_2d_base_t::inc_ref_counter() returns 0x%x", err);
            DF_WRITE_LOG_US(log_level, "t_index=0x%lx, pdu=0x%p", t_index, pdu);
        }

        return err;
    };
    //if (ref_counter != 0)��+1������ʲôҲ����
    //Return: 0 for success, other values for error
    int _inc_t_ref_counter_if_not_zero(const uint64_t t_index, uint64_t & ref_counter) {
        PDU pdu = NULL;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        ref_counter = 0;
        if ((err = _get_exist_du(t_index, pdu)) != 0 || NULL == pdu) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_exist_du() returns 0x%x", err);
            DF_WRITE_LOG_US(log_level, "t_index=0x%lx, pdu=0x%p", t_index, pdu);
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = pdu->dfs_2d_base_t::inc_ref_counter_if_not_zero(ref_counter)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level,
                            "pdu->dfs_2d_base_t::inc_ref_counter_if_not_zero() returns 0x%x",
                            err);
            DF_WRITE_LOG_US(log_level, "t_index=0x%lx, pdu=0x%p", t_index, pdu);
        }
#ifdef  DF_BT_STATISTIC_INFO
        else if (0 != ref_counter && UNDEF_REF_COUNTER != ref_counter) {
            _inc_obj_ref_inc_counter();
        }
#endif

        return err;
    };
    //Return��0 for success, other values for error
    int _dec_t_ref_counter(const uint64_t t_index, uint64_t & ref_counter) {
        PDU pdu = NULL;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

#ifdef  DF_BT_STATISTIC_INFO
        _inc_obj_ref_dec_counter();
#endif

        ref_counter = UNDEF_REF_COUNTER;
        if ((err = _get_exist_du(t_index, pdu)) != 0 || NULL == pdu) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_exist_du() returns 0x%x", err);
            DF_WRITE_LOG_US(log_level, "t_index=0x%lx, pdu=0x%p", t_index, pdu);
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = pdu->dfs_2d_base_t::dec_ref_counter(ref_counter)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "pdu->dfs_2d_base_t::dec_ref_counter() returns 0x%x", err);
            DF_WRITE_LOG_US(log_level, "t_index=0x%lx, pdu=0x%p", t_index, pdu);
        } else if (0 == ref_counter && (err = _release_t(t_index)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_release_t() returns 0x%x", err);
            DF_WRITE_LOG_US(log_level, "t_index=0x%lx, pdu=0x%p", t_index, pdu);
        }

        return err;
    };
    //return: 0 for success, other values for error
    int _get_t_ref_counter(const uint64_t t_index, uint64_t & ref_counter) const {
        PDU pdu = NULL;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        ref_counter = UNDEF_REF_COUNTER;
        if ((err = _get_exist_du(t_index, pdu)) != 0 || NULL == pdu) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_exist_du() returns 0x%x", err);
            DF_WRITE_LOG_US(log_level, "t_index=0x%lx, pdu=0x%p", t_index, pdu);
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((ref_counter = pdu->dfs_2d_base_t::get_ref_counter()) == UNDEF_REF_COUNTER) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "ref_counter == UNDEF_REF_COUNTER");
            DF_WRITE_LOG_US(log_level, "t_index=0x%lx, pdu=0x%p", t_index, pdu);
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_REF_COUNTER);
        }

        return err;
    };
public:
    //Ҫ��֤�����new_index����UNDEF_INDEX����ô�����ü����������0��
    int update_cur_index(const dfs_rc_indexer_t & new_rc_indexer, dfs_rc_indexer_t & old_rc_indexer) {
        //const dfs_rc_indexer_ext_t * pnew_rc_indexer_ext = (const dfs_rc_indexer_ext_t *)&new_rc_indexer;
        //uint64_t ref_counter = 0;
        uint64_t new_cur_index = ((const dfs_rc_indexer_ext_t *)&new_rc_indexer)->get_index();
        uint64_t old_cur_index = UNDEF_INDEX;
        //PDU pdu = NULL;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        old_rc_indexer = new_rc_indexer;
        if (!_is_valid_t_index(new_cur_index)) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "new_cur_index=%ld", new_cur_index);
            err = DF_BT_SET_INTERNAL_ERR(this->ERRNO_BT_INVALID_INDEX);
        } else if (((const dfs_rc_indexer_ext_t *)&new_rc_indexer)->get_rc_set()
                   != (dfs_rc_set_base_t *)this) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "new_rc_indexer.get_rc_set() != this");
            DF_WRITE_LOG_US(log_level,
                            "new_cur_index=%ld",
                            new_cur_index);
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_DIFF_POINTER);
        }
        //else if ((err = _inc_t_ref_counter(new_cur_index, ref_counter)) != 0)
        //{
        //    log_level = DF_UL_LOG_FATAL;
        //    DF_WRITE_LOG_US(log_level, "_inc_ref_counter() returns 0x%x", err);
        //    DF_WRITE_LOG_US(log_level,
        //            "new_cur_index=%ld",
        //            new_cur_index);
        //}
        else {
            old_cur_index = df_atomic_exchange(&_cur_index, new_cur_index);
            //�˴�����Ҫ����������ü����ļӼ���ǰ���old_rc_indexer = new_rc_indexer
            //����new_cur_index�����ü���+1�����µ�set_index(old_cur_index)
            //�����յ���old_cur_index�����ü���-1
            ((dfs_rc_indexer_ext_t *)&old_rc_indexer)->set_indexer(old_cur_index);
            //if ((err = _dec_t_ref_counter(old_cur_index, ref_counter)) != 0)
            //{
            //    log_level = DF_UL_LOG_FATAL;
            //    DF_WRITE_LOG_US(log_level, "_dec_ref_counter() returns 0x%x", err);
            //    DF_WRITE_LOG_US(log_level, "old_cur_index=%ld", old_cur_index);
            //}
        }

        return err;
    };
    //˵������Ҫ��֤��õ���û�б�����
    inline BAIDU_INLINE int retrieve_cur_index(dfs_rc_indexer_t & rc_indexer) const {
        //dfs_rc_indexer_ext_t * pnew_rc_index_ext = (dfs_rc_indexer_ext_t *)&rc_indexer;
        PDU pdu = NULL;
        uint64_t ref_counter = 0;
        uint64_t t_index = _cur_index;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        rc_indexer.init();
        while (UNDEF_INDEX != t_index) {
            if ((err = _get_exist_du(t_index, pdu)) != 0 || NULL == pdu) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_get_exist_du() returns 0x%x", err);
                DF_WRITE_LOG_US(log_level, "t_index=0x%lx, pdu=0x%p", t_index, pdu);
                err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
                break;
            } else if ((err = pdu->dfs_2d_base_t::inc_ref_counter_if_not_zero(ref_counter)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level,
                                "pdu->dfs_2d_base_t::inc_ref_counter_if_not_zero() returns 0x%x",
                                err);
                DF_WRITE_LOG_US(log_level, "t_index=0x%lx, pdu=0x%p", t_index, pdu);
                break;
            } else if (0 != ref_counter) {
#ifdef  DF_BT_STATISTIC_INFO
                _inc_obj_ref_inc_counter();
#endif

                //ǰ�����_inc_t_ref_counter_if_not_zero()ʱ�Ѿ����������ü���
                ((dfs_rc_indexer_ext_t *)&rc_indexer)->set_indexer(
                    (dfs_rc_set_base_t *)this,
                    t_index);

                break;
            }
            //���_cur_index�ĵ�ǰֵ�Ա�����
            t_index = _cur_index;
        }

        return err;
    };
protected:
    bool _is_valid_t_index(const uint64_t t_index) const {
        return (t_index < SET_SIZE);
    };
    //int valid_t_index_verify(const uint64_t t_index) const
    //{
    //    uint64_t true_t_index = t_index;
    //    int log_level = DF_UL_LOG_NONE;
    //    int err = 0;
    //
    //    if (true_t_index >= SET_SIZE)
    //    {
    //        log_level = DF_UL_LOG_FATAL;
    //        DF_WRITE_LOG_US(log_level, "true_t_index >= SET_SIZE");
    //        DF_WRITE_LOG_US(log_level,
    //                "t_index=0x%lx, true_t_index=0x%lx, SET_SIZE=%d",
    //                t_index, true_t_index, _t_index_for_next_row);
    //        err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_INVALID_INDEX);
    //    }
    //
    //    return err;
    //};

    //return: 0 for success, other values for error
    int _get_t_unit(const uint64_t t_index, PCT & pct) const {
        PDU pdu = NULL;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        pct = NULL;
        if ((err = _get_exist_du(t_index, pdu)) != 0 || NULL == pdu) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_exist_du() returns 0x%x", err);
            DF_WRITE_LOG_US(log_level, "t_index=0x%lx, pdu=0x%p", t_index, pdu);
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else {
            pct = ((T *)pdu);
        }

        return err;
    };
    int _get_t_unit(const dfs_rc_indexer_t & rc_indexer, PCT & pct) const {
        PDU pdu = NULL;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        pct = NULL;
        if ((err = _get_exist_du(
                       ((const dfs_rc_indexer_ext_t *)&rc_indexer)->get_index(),
                       pdu)) != 0
                || NULL == pdu) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_exist_du() returns 0x%x", err);
            DF_WRITE_LOG_US(log_level, "t_index=0x%lx, pdu=0x%p",
                            ((const dfs_rc_indexer_ext_t *)&rc_indexer)->get_index(),
                            pdu);
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else {
            pct = ((T *)pdu);
        }

        return err;
    };
public:
    int get_t_unit(const dfs_rc_indexer_t & rc_indexer, PCT & pct) const {
        return _get_t_unit(rc_indexer, pct);
    };
    uint64_t get_in_use_num(void) const {
        return (SET_SIZE-_free_counter);
    };
    uint64_t get_free_counter(void) const {
        return _free_counter;
    };
    //bool is_valid_indexer(const uint64_t t_index) const
    //{
    //    return (t_index < SET_SIZE);
    //};
    bool is_valid_in_use_indexer(const dfs_rc_indexer_t & rc_indexer) const {
        PDU pdu = NULL;
        const dfs_rc_set_base_t * prc_set =
            ((const dfs_rc_indexer_ext_t *)&rc_indexer)->get_rc_set();
        const uint64_t t_index = ((const dfs_rc_indexer_ext_t *)&rc_indexer)->get_index();
        int log_level = DF_UL_LOG_NONE;
        int err = 0;
        bool bret = false;

        if(NULL == prc_set || UNDEF_INDEX == t_index) {
            bret = false;
        } else if((dfs_rc_set_base_t *)this != prc_set) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "rc_indexer._prc_set != (dfs_rc_set_base_t *)this");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_INVALID_INDEX);
        } else if (t_index >= SET_SIZE) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "t_index=%ld > SET_SIZE=%d", t_index, SET_SIZE);
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_INVALID_INDEX);
        } else {
            if ((err = _get_exist_du(t_index,pdu)) != 0 || NULL == pdu) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_get_exist_du() returns 0x%x", err);
                DF_WRITE_LOG_US(log_level, "t_index=0x%lx, pdu=0x%p",t_index, pdu);
                err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
            } else {
                bret = pdu->dfs_2d_base_t::is_in_use();
            }
        }

        return bret;
    };
protected:
    //Return��0 for success, other values for error
    virtual int _vir_inc_t_ref_counter(const uint64_t index, uint64_t & ref_counter) const {
        return ((dfs_rc_set_t<T, SET_SIZE> *)this)->_inc_t_ref_counter(index, ref_counter);
    };
    virtual int _vir_dec_t_ref_counter(const uint64_t index, uint64_t & ref_counter) const {
        return ((dfs_rc_set_t<T, SET_SIZE> *)this)->_dec_t_ref_counter(index, ref_counter);
    };
};


template<typename T, uint32_t SET_SIZE>
int dfs_rc_set_t<T, SET_SIZE>::init(void * pgc_info) {
    int log_level = DF_UL_LOG_NONE;
    int err = 0;

    if (_init_state.is_not_init_yet()) {
        uint64_t ref_counter = 0;
        uint64_t t_index = UNDEF_INDEX;
        int64_t j = 0;
        int64_t n = (int64_t)SET_SIZE;
        DU * pdu = NULL;


        _pgc_info = pgc_info;

        if ((_rc_ary = new(std::nothrow) DU[SET_SIZE]) == NULL) {
            err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_NOMEM);
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, ERRINFO_BT_NOMEM, "_prc_root_set");
        } else {
            for (j = 0; j < n; ++j) {
                if ((err = _get_exist_du(j, pdu)) != 0 || NULL == pdu) {
                    log_level = DF_UL_LOG_FATAL;
                    DF_WRITE_LOG_US(log_level, "_get_exist_du() returns 0x%x", err);
                    err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
                } else {
                    pdu->init();
                    pdu->set_free();

                    if ((err = _push_free(j)) != 0) {
                        log_level = DF_UL_LOG_FATAL;
                        DF_WRITE_LOG_US(log_level, "_push_free() returns 0x%x", err);
                        break;
                    }
                }
            }
        }

        if (0 == err) {
            if ((err = _pop_free(t_index)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_pop_free() returns 0x%x", err);
            } else if ((err = _inc_t_ref_counter(t_index, ref_counter)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_inc_ref_counter() returns 0x%x", err);
                DF_WRITE_LOG_US(log_level, "t_index=%ld", t_index);
            } else if (1 != ref_counter) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "1 != ref_counter(=%ld)", ref_counter);
                err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_REF_COUNTER);
            }
            _cur_index = t_index;
        }

        _init_state.set_init_result(err);
    } else {
        err = _init_state.get_init_result();
    }

    return err;
};



//����ģ���࣬�ⲿ����һά���飬��uint64_t��Ϊ�±���ʣ�����Ԫ��������T��
//dfs_bt2d_ary_t<typename T, uint32_t ROW_SIZE>
//  ģ�������T���û������ݵ�Ԫ����
//            ROW_SIZE�������������гߴ磬����Ϊ2�ķ���
//�ر�˵������Ҫʹ���ⲿ������������֤��������޸��ǵ�һ��(��ͬһʱ��ֻ����һ���߳��޸�����)��
//          ����public�ӿڽ��ṩ��ֻ����������protected�ӿ��ṩ�޸Ĳ�����
//
//�ڲ��Ƕ�ά���飬ÿ��ROW_SIZE��Ԫ�ء�
//���������
//  T: ������(�û����ݵ�Ԫ)����Ҫ�ṩ���½ӿڣ�
//    T();
//    T(const T & src);
//    const T & operator=(const T & src);
//
//    //  ���ܣ���T���г�ʼ��
//    void init(void);
//
//�ⲿ�ӿ�(public)��
//  ����public�ӿڽ��ṩ��ֻ����������protected�ӿ��ṩ�޸Ĳ���
//  ���캯����ȱʡ���캯��;
//  int acquire_t(const T * ptobjv, uint64_t & t_index, T ** pptobj);
//      ���ܣ�����һ���µĶ��󡣳ɹ�ʱ��pt��ָ�����ʼ��(pt�ǿ�ʱ)�����init()���г�ʼ��(ptΪ��)��
//            �ɹ�ʱ�������״̬����Ϊin_use�����ü���Ϊ0��
//      ���룺ptobjv����ָ���ֵ������ʼ���¶��󣬿���ΪNULL
//      ������¶����index�Լ�ָ���¶����ָ�룬UNDEF_INDEX��ʾû�����ɶ������޷������¶���
//  int get_t_unit(const uint64_t t_index, const T * pct) const
//      ���ܣ����һ�������ָ�룬�������index�Ƿ����߶�����ʹ���У��򷵻�NULL
//      ���룺�����INDEX
//      ���أ������ָ�룬�������index�Ƿ����߶�����ʹ�����򷵻�NULL
//  bool is_valid_t_index(const uint64_t t_index) const
//      ���ܣ��ж�һ��INDEX�Ƿ�Ϸ�(ʹ���л����ɵ�INDEX���ǺϷ���)
//      ���룺�����INDEX
//      ���أ�true��ʾ�Ϸ�INDEX��false��ʾ�Ƿ���INDEX
//      ˵�����¶����ǳ��������
//  uint64_t get_size(void)
//      ���ܣ���ǰ����Ĵ�С(��Ԫ����)
//      ���أ�����0��ʾ��δ���κζ���
//�ⲿ�ӿ�(protected)��
//  int inc_t_ref_counter(const uint64_t t_index, uint64_t & ref_counter)
//      ���ܣ���������ü�����1
//      ���룺�����INDEX
//      ���أ�0 for success, other values for error
//  int dec_t_ref_counter(const uint64_t t_index, uint64_t & ref_counter)
//      ���ܣ���������ü�����1
//      ���룺�����INDEX
//      ���أ�0 for success, other values for error
//  int get_t_ref_counter(const uint64_t t_index, uint64_t & ref_counter) const;
//      ���ܣ���ö�������ü���
//      ���룺�����INDEX
//      �������������ü�����UNDEF_REF_COUNTER��ʾ�Ƿ���INDEX���߶�������ü����쳣
//      ���أ�0 for success, other values for error
//  int get_exist_t_unit(const uint64_t t_index, PT & pt)
//  int get_may_not_exist_t_unit(const uint64_t t_index, PT & pt)
//      ���ܣ����һ�������ָ�룬�������index�Ƿ��򷵻�NULL�����򷵻ظö���ָ��
//      ���룺�����INDEX
//      ���أ�0 for success, other values for error
//private:
//  int _release_t(const uint64_t t_index)
//      ���ܣ��ͷ�һ������
//      ���룺Ҫ�ͷŶ����INDEX
//      ���أ�0 for success, other values for error

//���Ǳ�����һ��Ԫ��
template<typename T, uint32_t ROW_SIZE>   //, //���鳣��ROW_SIZE��2�ķ���
class dfs_bt2d_ary_t : virtual public dfs_bt_const_t {
private:
    typedef T *PT;
    typedef const T *PCT;
    typedef dfs_2d_du_t<T> DU;
    typedef dfs_2d_du_t<T> *PDU;
    typedef const dfs_2d_du_t<T> *PCDU;
private:
    enum cconst {
        MIN_ROW_ARY_SIZE = 256,
        MIN_ROW_SIZE     = 256   ,
    };
private:
    dfs_init_t _init_state;
    PDU * _t_row_ary;
    uint64_t _t_row_ary_size;
    uint64_t _t_index_for_next_alloc; //������ROW�ķ���λ��
    uint64_t _t_index_for_next_row;
    PDU * _realloc_t_row_ary[MAX_2D_ARY_HOLD_REALLOC_NUM];
    uint64_t _realloc_counter;
    dfs_2d_base_t _free_head;
    volatile uint64_t _free_counter;
    pvoid _pgc_info;   //for garbage collect purpose
#ifdef  DF_BT_STATISTIC_INFO
    //statistic information purpose
    volatile uint64_t _obj_del_counter;
    volatile uint64_t _obj_add_counter;
    volatile uint64_t _obj_ref_inc_counter;
    volatile uint64_t _obj_ref_dec_counter;
#endif
public:
    dfs_bt2d_ary_t() {
        _t_row_ary_size = MIN_ROW_ARY_SIZE;
        _t_row_ary = NULL;
        _t_index_for_next_alloc = 0;
        _t_index_for_next_row = 0;

        _free_head.set_free();
        _free_counter = 0;

        for (uint32_t j = 0; j < df_len_of_ary(_realloc_t_row_ary); ++j) {
            _realloc_t_row_ary[j] = NULL;
        }
        _realloc_counter = 0;
        _pgc_info = NULL;

#ifdef  DF_BT_STATISTIC_INFO
        _obj_del_counter = 0;
        _obj_add_counter = 0;
        _obj_ref_inc_counter = 0;
        _obj_ref_dec_counter = 0;
#endif

        init();
    };
    virtual ~ dfs_bt2d_ary_t() {
        if (NULL != _t_row_ary) {
            for (uint64_t j = 0; j < _t_row_ary_size; ++j) {
                if (NULL != _t_row_ary[j]) {
                    delete[] _t_row_ary[j];
                    _t_row_ary[j] = NULL;
                }
            }
            delete[] _t_row_ary;
            _t_row_ary = NULL;
        }

        for (uint32_t j = 0; j < df_len_of_ary(_realloc_t_row_ary); ++j) {
            delete [] _realloc_t_row_ary[j];
        }

        _init_state.set_destructed();

        return;
    };
    int init(void) {
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        if (_init_state.is_not_init_yet()) {
            if (NULL != _t_row_ary) {
                err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_NOT_NULL_BUF_POINTER);
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "NULL != _t_row_ary");
            } else if ((_t_row_ary = new(std::nothrow) PDU[_t_row_ary_size]) == NULL) {
                err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_NOMEM);
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, ERRINFO_BT_NOMEM, "_t_row_ary");
            } else {
                for (uint64_t j = 0; j < _t_row_ary_size; ++j) {
                    _t_row_ary[j] = NULL;
                }
                if ((err = _alloc_new_row()) != 0) {
                    log_level = DF_UL_LOG_FATAL;
                    DF_WRITE_LOG_US(log_level, "_alloc_new_row() returns 0x%x", err);
                } else {
                    //���Ǳ�����һ��Ԫ��
                    ++_t_index_for_next_alloc;
                }
            }

            if (DF_UL_LOG_NONE != log_level) {
                DF_WRITE_LOG_US(log_level,
                                "_t_row_ary=0x%p, _t_row_ary_size=%ld",
                                _t_row_ary, _t_row_ary_size);
            }

            _init_state.set_init_result(err);
        } else {
            err = _init_state.get_init_result();
        }

        return err;
    };
public:
    pvoid get_gc_info(void) const {
        return _pgc_info;
    };
    int set_gc_info(void * pgc_info) {
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        if (NULL != _pgc_info) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "NULL != _pgc_info");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_SET_GC_INFO);
        } else {
            _pgc_info = pgc_info;
        }

        return err;
    };
private:
#ifdef  DF_BT_STATISTIC_INFO
    //statistic information purpose
    //called whenever an obj is deleted
    uint64_t _inc_obj_del_counter(void) {
        return df_atomic_inc(&_obj_del_counter);
    };
    //
    //called whenever a new obj is added
    uint64_t _inc_obj_add_counter(void) {
        return df_atomic_inc(&_obj_add_counter);
    };
    //
    //called whenever an obj's ref_counter is incremented
    uint64_t _inc_obj_ref_inc_counter(void) {
        return df_atomic_inc(&_obj_ref_inc_counter);
    };
    //
    //called whenever an obj's ref_counter is decremented
    uint64_t _inc_obj_ref_dec_counter(void) {
        return df_atomic_inc(&_obj_ref_dec_counter);
    };
#endif
public:
    void clear_statistic_info(void) {
#ifdef  DF_BT_STATISTIC_INFO
        df_atomic_exchange(&_obj_del_counter, 0);
        df_atomic_exchange(&_obj_add_counter, 0);
        df_atomic_exchange(&_obj_ref_inc_counter, 0);
        df_atomic_exchange(&_obj_ref_dec_counter, 0);
#endif
        return;
    };
    void log_statistic_info(
        const int log_level,
        const char * filename,
        const int lineno,
        const char * funcname,
        const char * btreename,
        const char * bt2dname) const {
        DF_WRITE_LOG_US(log_level, "%s,%d,%s", filename, lineno, funcname);
        DF_WRITE_LOG_US(log_level, "%s, %s", btreename, bt2dname);
#ifdef  DF_BT_STATISTIC_INFO
        DF_WRITE_LOG_US(log_level, "_obj_del_counter=%ld", _obj_del_counter);
        DF_WRITE_LOG_US(log_level, "_obj_add_counter=%ld", _obj_add_counter);
        DF_WRITE_LOG_US(log_level, "_obj_ref_inc_counter=%ld", _obj_ref_inc_counter);
        DF_WRITE_LOG_US(log_level, "_obj_ref_dec_counter=%ld", _obj_ref_dec_counter);
#endif
        return;
    };
    void log_debug_info(
        const int log_level,
        const char * filename,
        const int lineno,
        const char * funcname,
        const char * btreename,
        const char * bt2dname) const {
        DF_WRITE_LOG_US(log_level, "%s,%d,%s", filename, lineno, funcname);
        DF_WRITE_LOG_US(log_level, "%s, %s", btreename, bt2dname);
        DF_WRITE_LOG_US(log_level, "_t_row_ary_size=%ld", _t_row_ary_size);
        DF_WRITE_LOG_US(log_level, "_t_index_for_next_alloc=%ld", _t_index_for_next_alloc);
        DF_WRITE_LOG_US(log_level, "_t_index_for_next_row=%ld", _t_index_for_next_row);
        DF_WRITE_LOG_US(log_level, "_realloc_counter=%ld", _realloc_counter);
        DF_WRITE_LOG_US(log_level, "_free_head.get_next_index()=%ld", _free_head.get_next_index());
        DF_WRITE_LOG_US(log_level, "_free_counter=%ld", _free_counter);
    };

private:
    //����row_ary
    //return: 0 for success, other values for error
    int _enlarge_row_ary(void) {
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        if ((_t_index_for_next_row/ROW_SIZE) >= _t_row_ary_size) {
            //��ǰ_t_row_ary��������
            //typedef T *PT;
            uint64_t j = 0;
            uint64_t new_t_row_ary_size = _t_row_ary_size * 2;
            uint64_t old_t_row_ary_size = 0;
            PDU * new_t_row_ary = new(std::nothrow) PDU[new_t_row_ary_size];
            PDU * old_t_row_ary = NULL;

            if (NULL == new_t_row_ary) {
                err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_NOMEM);
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, ERRINFO_BT_NOMEM, "new_t_row_ary");
            } else {
                j = 0;
                for (; j < _t_row_ary_size; ++j) {
                    new_t_row_ary[j] = _t_row_ary[j];
                }
                for (; j < new_t_row_ary_size; ++j) {
                    new_t_row_ary[j] = NULL;
                }
                old_t_row_ary = (dfs_2d_du_t<T> **)df_atomic_exchange_pointer(
                                    (volatile pvoid *)&(_t_row_ary),
                                    (pvoid)new_t_row_ary);
                old_t_row_ary_size = df_atomic_exchange(
                                         (volatile uint64_t *)&_t_row_ary_size,
                                         new_t_row_ary_size);
                //�Ѿ�������뵽������������ʱ���ͷţ���Ϊ�������߳̿������ڷ��ʡ�
                delete [] _realloc_t_row_ary[_realloc_counter];
                _realloc_t_row_ary[_realloc_counter] = old_t_row_ary;
                _realloc_counter = (_realloc_counter+1)%df_len_of_ary(_realloc_t_row_ary);
            }
        }

        return err;
    };
    //����һ������
    //return: 0 for success, other values for error
    int _alloc_new_row(void) {
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        if ((_t_index_for_next_row/ROW_SIZE) >= _t_row_ary_size) {
            //��ǰ_t_row_ary��������
            err = _enlarge_row_ary();
        }

        if (0 == err) {
            if ((_t_index_for_next_row/ROW_SIZE) < _t_row_ary_size) {
                if (NULL == _t_row_ary[_t_index_for_next_row/ROW_SIZE]) {
                    if ((_t_row_ary[_t_index_for_next_row/ROW_SIZE] =
                                new(std::nothrow) DU[ROW_SIZE]) == NULL) {
                        err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_NOMEM);
                        log_level = DF_UL_LOG_FATAL;
                        DF_WRITE_LOG_US(
                            log_level,
                            ERRINFO_BT_NOMEM,
                            "_t_row_ary[_t_index_for_next_row/ROW_SIZE]");
                    }
                }
                if (NULL != _t_row_ary[_t_index_for_next_row/ROW_SIZE]) {
                    _t_index_for_next_alloc = _t_index_for_next_row;
                    _t_index_for_next_row += ROW_SIZE;
                }
            } else {
                err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_BUF_SIZE);
            }
        }

        return err;
    };
private:
    //��д�̶߳����ܵ���
    int _push_free(const uint64_t freed_index) {
        PDU pdu = NULL;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        //if ((err = _t_free_locker.lock(0)) != 0)
        //{
        //    log_level = DF_UL_LOG_FATAL;
        //    DF_WRITE_LOG_US(log_level, "_t_free_locker.lock() returns 0x%x", err);
        //}
        //else
        //{
        if ((err = _get_exist_du(freed_index, pdu)) != 0 || NULL == pdu) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_exist_du() returns 0x%x", err);
            err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
        } else {
            pdu->dfs_2d_base_t::set_free();
            if ((err = _free_head.add_to_list(freed_index, *((dfs_2d_base_t *)pdu))) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_free_head.add_to_list() returns 0x%x", err);
            } else {
                df_atomic_inc(&_free_counter);
            }
        }

        //    _t_free_locker.unlock();
        //}

        return err;
    };
    //ֻ��д�̵߳��ã����ֻ��һ��ʵ��
    int _pop_free(uint64_t & ret_index) {
        PDU pdu = NULL;
        uint64_t raw_data = 0;
        bool is_removed = false;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        //if ((err = _t_free_locker.lock(0)) != 0)
        //{
        //    log_level = DF_UL_LOG_FATAL;
        //    DF_WRITE_LOG_US(log_level, "_t_free_locker.lock() returns 0x%x", err);
        //}
        //else
        //{
        raw_data = _free_head.get_raw_data();
        ret_index = dfs_2d_base_t::extract_next_index(raw_data);
        while (UNDEF_INDEX != ret_index) {
            is_removed = false;
            if ((err = _get_exist_du(ret_index, pdu)) != 0 || NULL == pdu) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_get_exist_du() returns 0x%x", err);
                err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
                ret_index = UNDEF_INDEX;
                break;
            }
            //���raw_data�Լ����pdu�Լ�pdu->get_next_index()����һ��ԭ�Ӳ�����
            //����ж��_pop_free�̣߳����ڻ��raw_data��pdu->get_next_index()����
            //�����仯(��ʹraw_data��ret_indexû�б仯)����˸��㷨�Զ��_pop_free()�̲߳�����ȫ��
            else if ((err = _free_head.remove_from_list(
                                raw_data,
                                pdu->dfs_2d_base_t::get_next_index(),
                                is_removed)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_free_head.remove_from_list() returns 0x%x", err);
                ret_index = UNDEF_INDEX;
                break;
            } else if (is_removed) {
                pdu->set_in_use();
                df_atomic_dec(&_free_counter);
                break;
            }
            raw_data = _free_head.dfs_2d_base_t::get_raw_data();
            ret_index = dfs_2d_base_t::extract_next_index(raw_data);
        }

        //    _t_free_locker.unlock();
        //}

        return err;
    };

private:
    //Return: 0 for success, other values for error
    int _release_t(const uint64_t t_index) {
        PDU pdu = NULL;
        uint64_t ref_counter = UNDEF_REF_COUNTER;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

#ifdef  DF_BT_STATISTIC_INFO
        _inc_obj_del_counter();
#endif

        if (!is_valid_t_index(t_index)) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "!is_valid_t_index(t_index)");
            err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
        } else if ((err = _get_exist_du(t_index, pdu)) != 0 || NULL == pdu) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_exist_du() returns 0x%x", err);
            err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
        } else if ((ref_counter = pdu->dfs_2d_base_t::get_ref_counter()) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "ref_counter != 0");
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_REF_COUNTER);
        } else if ((err = pdu->action_before_gc(get_gc_info(), t_index)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "pdu->action_before_gc(get_gc_info()...) returns 0x%x", err);
        } else {
            //Added by yangzhenkun per DTS in order to release the resource pointed by T on 20090717
            pdu->T::init();
            if ((err = _push_free(t_index)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_push_free() returns 0x%x", err);
            }
        }

        if (DF_UL_LOG_NONE != log_level) {
            DF_WRITE_LOG_US(log_level,
                            "t_index=0x%lx, ref_counter=%ld, pdu=0x%p",
                            t_index, ref_counter, pdu);
        }

        return err;
    };
private:
    //Return: 0 for success(even if t_index is invalid), other values for error
    //Notice: may return 0 even if pdu is NULL
    //int _get_du(const uint64_t t_index, PDU & pdu) const
    //{
    //    const uint64_t true_t_index = t_index;
    //    int log_level = DF_UL_LOG_NONE;
    //    int err = 0;

    //    pdu = NULL;
    //    if (is_valid_t_index(t_index))
    //    {
    //        pdu = &(((_t_row_ary[true_t_index/ROW_SIZE])[true_t_index%ROW_SIZE]));
    //    }
    //    else
    //    {
    //        //err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
    //        pdu = NULL;
    //    }

    //    return err;
    //};
    inline BAIDU_INLINE int _get_exist_du(const uint64_t t_index, PDU & pdu) const {
        const uint64_t true_t_index = t_index;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        pdu = NULL;
        if (is_valid_t_index(t_index)) {
            pdu = &(((_t_row_ary[true_t_index/ROW_SIZE])[true_t_index%ROW_SIZE]));
        } else {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "!is_valid_t_index(t_index)");
            pdu = NULL;
            err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
        }

        if (DF_UL_LOG_NONE != log_level) {
            DF_WRITE_LOG_US(log_level,
                            "t_index=0x%lx, true_t_index=0x%lx, pdu=0x%p",
                            t_index, true_t_index, pdu);
        }

        return err;
    };
    int _get_may_not_exist_du(const uint64_t t_index, PDU & pdu) const {
        const uint64_t true_t_index = t_index;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        pdu = NULL;
        if (is_valid_t_index(t_index)) {
            pdu = &(((_t_row_ary[true_t_index/ROW_SIZE])[true_t_index%ROW_SIZE]));
        } else {
            //err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
            pdu = NULL;
        }

        if (DF_UL_LOG_NONE != log_level) {
            DF_WRITE_LOG_US(log_level,
                            "t_index=0x%lx, true_t_index=0x%lx, pdu=0x%p",
                            t_index, true_t_index, pdu);
        }

        return err;
    };
    ////Return: 0 for success(even if t_index is invalid), other values for error
    ////Notice: may return 0 even if pdu is NULL
    //int _get_du_ignore_recycle_counter(const uint64_t t_index, PDU & pdu) const
    //{
    //    const uint64_t true_t_index = t_index;
    //    //int log_level = DF_UL_LOG_NONE;
    //    int err = 0;
    //
    //    pdu = NULL;
    //    if (is_valid_t_index(t_index))
    //    {
    //        pdu = &(((_t_row_ary[true_t_index/ROW_SIZE])[true_t_index%ROW_SIZE]));
    //    }
    //    else
    //    {
    //        //err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
    //        pdu = NULL;
    //    }
    //
    //    return err;
    //};
public:
    //Return: 0 for success, other values for error
    inline BAIDU_INLINE int get_exist_t_unit(const uint64_t t_index, PT & pt) const {
        PDU pdu = NULL;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        pt = NULL;
        if ((err = _get_exist_du(t_index, pdu)) != 0 || NULL == pdu) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_exist_du() returns 0x%x", err);
            DF_WRITE_LOG_US(log_level,
                            "t_index=0x%lx, pt=0x%p, pdu=0x%p",
                            t_index, pt, pdu);
            err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else {
            pt = ((T *)pdu);
        }

        return err;
    };
    int get_may_not_exist_t_unit(const uint64_t t_index, PT & pt) const {
        PDU pdu = NULL;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        pt = NULL;
        if ((err = _get_may_not_exist_du(t_index, pdu)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_may_not_exist_du() returns 0x%x", err);
            DF_WRITE_LOG_US(log_level,
                            "t_index=0x%lx, pt=0x%p, pdu=0x%p",
                            t_index, pt, pdu);
        } else if (NULL != pdu) {
            pt = ((T *)pdu);
        }

        return err;
    };

public:
    //      ���룺ptobjv����ָ���ֵ������ʼ���¶��󣬿���ΪNULL
    //      ������¶����index�Լ�ָ���¶����ָ�룬UNDEF_INDEX��ʾû�����ɶ������޷������¶���
    int acquire_t(const T * pctobjv, uint64_t & t_index, T ** pptobj) {
        dfs_u_int64_t idata;
        PDU pdu = NULL;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        t_index = UNDEF_INDEX;

#ifdef  DF_BT_STATISTIC_INFO
        _inc_obj_add_counter();
#endif

        if ((err = _pop_free(t_index)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_pop_free() returns 0x%x", err);
        } else {
            if (UNDEF_INDEX == t_index) {
                if (_t_index_for_next_alloc >= _t_index_for_next_row) {
                    if ((err = _alloc_new_row()) != 0) {
                        log_level = DF_UL_LOG_FATAL;
                        DF_WRITE_LOG_US(log_level, "_alloc_new_row() returns 0x%x", err);
                    }
                }
                if (0 == err) {
                    //new obj
                    if (_t_index_for_next_alloc < _t_index_for_next_row) {
                        t_index = _t_index_for_next_alloc++;
                    } else {
                        log_level = DF_UL_LOG_FATAL;
                        DF_WRITE_LOG_US(log_level, "fail to acquire t_obj");
                        err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_ACQUIRE);
                        t_index = UNDEF_INDEX;
                    }
                }
            }

            if (0 == err && UNDEF_INDEX != t_index) {
                if ((err = _get_exist_du(t_index, pdu)) != 0) {
                    log_level = DF_UL_LOG_FATAL;
                    DF_WRITE_LOG_US(log_level, "_get_exist_du() returns 0x%x", err);
                    t_index = UNDEF_INDEX;
                } else if (NULL == pdu) {
                    log_level = DF_UL_LOG_FATAL;
                    DF_WRITE_LOG_US(log_level, "NULL == pdu");
                    err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
                    t_index = UNDEF_INDEX;
                } else {
                    pdu->init();
                }
            }
        }

        if (0 == err) {
            if (NULL != pctobjv) {
                *((T *)pdu) = *pctobjv;
            }
            if (NULL != pptobj) {
                *pptobj = (T *)pdu;
            }
            if ((err = pdu->action_while_added(get_gc_info(), t_index)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "pdu->action_while_added() returns 0x%x", err);
            }
        } else {
            t_index = UNDEF_INDEX;
            if (NULL != pptobj) {
                *pptobj = NULL;
            }
        }

        if (DF_UL_LOG_NONE != log_level) {
            DF_WRITE_LOG_US(log_level,
                            "_t_index_for_next_alloc=0x%lx, _t_index_for_next_row=0x%lx",
                            _t_index_for_next_alloc, _t_index_for_next_row);
            DF_WRITE_LOG_US(log_level, "t_index=0x%lx", t_index);
        }

        return err;
    };
    uint64_t acquire_t(const T * pctobj) {
        uint64_t t_index = UNDEF_INDEX;
        int err = 0;

        if ((err = acquire_t(pctobj, t_index, NULL)) != 0) {
            DF_WRITE_LOG_US(DF_UL_LOG_FATAL, "acquire_t() returns 0x%x, t_index=0x%lx", err, t_index);
            t_index = UNDEF_INDEX;
        }

        return t_index;
    };

public:
    //Return��0 for success, other values for error
    int inc_t_ref_counter(const uint64_t t_index, uint64_t & ref_counter) {
        PDU pdu = NULL;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

#ifdef  DF_BT_STATISTIC_INFO
        _inc_obj_ref_inc_counter();
#endif

        ref_counter = UNDEF_REF_COUNTER;
        if ((err = _get_exist_du(t_index, pdu)) != 0 || NULL == pdu) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_exist_du() returns 0x%x", err);
            DF_WRITE_LOG_US(log_level, "t_index=0x%lx, pdu=0x%p", t_index, pdu);
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = pdu->dfs_2d_base_t::inc_ref_counter(ref_counter)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "pdu->dfs_2d_base_t::inc_ref_counter() returns 0x%x", err);
            DF_WRITE_LOG_US(log_level, "t_index=0x%lx, pdu=0x%p", t_index, pdu);
        }

        return err;
    };
    //if (ref_counter != 0)��+1������ʲôҲ����
    //Return: 0 for success, other values for error
    int inc_t_ref_counter_if_not_zero(const uint64_t t_index, uint64_t & ref_counter) {
        PDU pdu = NULL;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        ref_counter = 0;
        if ((err = _get_exist_du(t_index, pdu)) != 0 || NULL == pdu) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_exist_du() returns 0x%x", err);
            DF_WRITE_LOG_US(log_level, "t_index=0x%lx, pdu=0x%p", t_index, pdu);
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = pdu->dfs_2d_base_t::inc_ref_counter_if_not_zero(ref_counter)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level,
                            "pdu->dfs_2d_base_t::inc_ref_counter_if_not_zero() returns 0x%x",
                            err);
            DF_WRITE_LOG_US(log_level, "t_index=0x%lx, pdu=0x%p", t_index, pdu);
        }
#ifdef  DF_BT_STATISTIC_INFO
        else if (0 != ref_counter && UNDEF_REF_COUNTER != ref_counter) {
            _inc_obj_ref_inc_counter();
        }
#endif

        return err;
    };
    //Return��0 for success, other values for error
    int dec_t_ref_counter(const uint64_t t_index, uint64_t & ref_counter) {
        PDU pdu = NULL;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

#ifdef  DF_BT_STATISTIC_INFO
        _inc_obj_ref_dec_counter();
#endif

        ref_counter = UNDEF_REF_COUNTER;
        if ((err = _get_exist_du(t_index, pdu)) != 0 || NULL == pdu) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_exist_du() returns 0x%x", err);
            DF_WRITE_LOG_US(log_level, "t_index=0x%lx, pdu=0x%p", t_index, pdu);
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((err = pdu->dfs_2d_base_t::dec_ref_counter(ref_counter)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "pdu->dfs_2d_base_t::dec_ref_counter() returns 0x%x", err);
            DF_WRITE_LOG_US(log_level, "t_index=0x%lx, pdu=0x%p", t_index, pdu);
        }
        //�������ü������ڴ��ͷ�..
        else if (0 == ref_counter && (err = _release_t(t_index)) != 0) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_release_t() returns 0x%x", err);
            DF_WRITE_LOG_US(log_level, "t_index=0x%lx, pdu=0x%p", t_index, pdu);
        }

        return err;
    };


    //return: 0 for success, other values for error
    int get_t_ref_counter(const uint64_t t_index, uint64_t & ref_counter) const {
        PDU pdu = NULL;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        ref_counter = UNDEF_REF_COUNTER;
        if ((err = _get_exist_du(t_index, pdu)) != 0 || NULL == pdu) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_exist_du() returns 0x%x", err);
            DF_WRITE_LOG_US(log_level, "t_index=0x%lx, pdu=0x%p", t_index, pdu);
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else if ((ref_counter = pdu->dfs_2d_base_t::get_ref_counter()) == UNDEF_REF_COUNTER) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "ref_counter == UNDEF_REF_COUNTER");
            DF_WRITE_LOG_US(log_level, "t_index=0x%lx, pdu=0x%p", t_index, pdu);
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_REF_COUNTER);
        }

        return err;
    };


public:
    inline BAIDU_INLINE bool is_valid_t_index(const uint64_t t_index) const {
        return (t_index < _t_index_for_next_row);
    };
    int valid_t_index_verify(const uint64_t t_index) const {
        uint64_t true_t_index = t_index;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        if (true_t_index >= _t_index_for_next_row) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "true_t_index >= _t_index_for_next_row");
            DF_WRITE_LOG_US(log_level,
                            "t_index=0x%lx, true_t_index=0x%lx, _t_index_for_next_row=0x%lx",
                            t_index, true_t_index, _t_index_for_next_row);
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_INVALID_INDEX);
        }

        return err;
    };

    //return: 0 for success, other values for error
    int get_t_unit(const uint64_t t_index, PCT & pct) const {
        PDU pdu = NULL;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        pct = NULL;
        if ((err = _get_exist_du(t_index, pdu)) != 0 || NULL == pdu) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_exist_du() returns 0x%x", err);
            DF_WRITE_LOG_US(log_level, "t_index=0x%lx, pdu=0x%p", t_index, pdu);
            err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
        } else {
            pct = ((T *)pdu);
        }

        return err;
    };

public:
    //��ǰ����Ĵ�С(��Ԫ����)
    uint64_t get_size(void) const {
        return _t_index_for_next_row;
    };
    //  return: 0 for success, other values for error
    int get_in_use_num(uint64_t & in_use_num) const {
        in_use_num = _t_index_for_next_alloc-_free_counter;
        return 0;
    };
    //
    //  return: 0 for success, other values for error
    int get_total_ref_counter(uint64_t & total_ref_counter) const {
        PDU pdu = NULL;
        uint64_t ref_counter = 0;
        uint64_t j = 0;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        total_ref_counter = 0;
        //excluding the first element as we keep it reserved.
        for (j = 1; j < _t_index_for_next_alloc; ++j) {
            if ((err = _get_may_not_exist_du(j, pdu)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_get_du_ignore_recycle_counter() returns 0x%x", err);
                break;
            } else if (NULL == pdu) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "NULL == pdu");
                err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_OBJ_NULL_POINTER);
                break;
            } else if (pdu->is_in_use()) {
                if ((ref_counter = pdu->dfs_2d_base_t::get_ref_counter()) == UNDEF_REF_COUNTER) {
                    log_level = DF_UL_LOG_FATAL;
                    DF_WRITE_LOG_US(log_level,
                                    "pdu->dfs_2d_base_t::get_ref_counter() == UNDEF_REF_COUNTER");
                    err = DF_BT_SET_INTERNAL_ERR(ERRNO_BT_REF_COUNTER);
                    break;
                } else {
                    total_ref_counter += ref_counter;
                }
            }
        }

        if (DF_UL_LOG_NONE != log_level) {
            DF_WRITE_LOG_US(log_level,
                            "j=0x%lx, _t_index_for_next_alloc=0x%lx",
                            j, _t_index_for_next_alloc);
        }

        return err;
    };
    uint64_t get_mem_size(void) const {
        return (_t_row_ary_size*sizeof(PDU)+_t_index_for_next_row*sizeof(DU));
    };
};


//    //������������ʽ��һ���ṹ�����޸����ݺ͸�������
//    //�޸�����ʱ��_mutation_counter��1���޸���ɺ��_mutation_counter���Ƶ�_syn_mutation_counter
//    //��������ʱ�ȸ���_syn_mutation_counter��������ɺ��ٸ���_mutation_counter��
//    //����������ڱ��޸ģ���_mutation_counter��_syn_mutation_counter�����
//class dfs_mutation_sycnizer_t
//{
//private:
//    //�޸�_mutation_counter��1���޸����ʱ����_syn_mutation_counterΪ_mutation_counter
//    //����ʱ���෴˳���ȸ���_syn_mutation_counter����_mutation_counter
//    volatile uint64_t _mutation_counter;
//    volatile uint64_t _syn_mutation_counter;
//public:
//    dfs_mutation_sycnizer_t()
//    {
//        init();
//    };
//    ~dfs_mutation_sycnizer_t()
//    {
//    };
//protected:
//    //�Ѹ�ֵ��������Ϊprotected��Ϊ�˱�������ֱ�Ӹ�btree�ĸ��ڵ�_root_info��ֵ��
//    //��Ϊ����_root_info��Ҫ�ر�Ĳ���(start_mutating()=>����root_node_index=>end_mutating())
//    inline const dfs_mutation_sycnizer_t & operator=(const dfs_mutation_sycnizer_t & src)
//    {
//        if (&src != this)
//        {
//            //�޸�˳����_mutation_counter�����ʱ����_syn_mutation_counterΪ_mutation_counter
//            //����˳�����޸�˳�������෴
//            _syn_mutation_counter   = src._syn_mutation_counter ;
//            _mutation_counter       = src._mutation_counter     ;
//        }
//        return *this;
//    };
//public:
//    inline void set_value(const dfs_mutation_sycnizer_t & src)
//    {
//        *this = src;
//    };
//public:
//    dfs_mutation_sycnizer_t(const dfs_mutation_sycnizer_t & src) :
//                    _mutation_counter(src._mutation_counter),
//                    _syn_mutation_counter(src._syn_mutation_counter)
//    {
//    };
//    void init(void)
//    {
//        _mutation_counter = 0;
//        _syn_mutation_counter = 0;
//        return;
//    };
//public:
//    int set_mutation_counter(const uint64_t mutation_counter)
//    {
//        int log_level = DF_UL_LOG_NONE;
//        int err = 0;
//
//        if (mutation_counter < _mutation_counter)
//        {
//            log_level = DF_UL_LOG_FATAL;
//            DF_WRITE_LOG_US(log_level, "mutation_counter < _mutation_counter");
//            DF_WRITE_LOG_US(log_level,
//                    "mutation_counter=%lu, _mutation_counter=%lu",
//                    mutation_counter, _mutation_counter);
//            err = DF_BT_SET_INTERNAL_ERR(dfs_bt_const_t::ERRNO_BT_MUTATION_COUNTER);
//        }
//        else
//        {
//            df_atomic_exchange(&_mutation_counter, mutation_counter);
//        }
//
//        return err;
//    };
//    void start_mutating(void)
//    {
//        df_atomic_inc(&_mutation_counter);
//        return;
//    };
//    void end_mutating(void)
//    {
//        df_atomic_exchange(&_syn_mutation_counter, _mutation_counter);
//        return;
//    };
//    void start_copying(const dfs_mutation_sycnizer_t & src)
//    {
//        df_atomic_exchange(&_syn_mutation_counter, src._syn_mutation_counter);
//        return;
//    };
//    void end_copying(const dfs_mutation_sycnizer_t & src)
//    {
//        df_atomic_exchange(&_mutation_counter, src._mutation_counter);
//        return;
//    };
//    //�����Ƿ�����(vs.�����޸���)
//    bool intergrity_check(void) const
//    {
//        return (_mutation_counter == _syn_mutation_counter);
//    };
//    uint64_t get_mutation_counter(void) const
//    {
//        return _mutation_counter;
//    };
//    uint64_t get_syn_mutation_counter(void) const
//    {
//        return _syn_mutation_counter;
//    };
//};




#endif //__DF_2D_ARY_INCLUDE_H_

