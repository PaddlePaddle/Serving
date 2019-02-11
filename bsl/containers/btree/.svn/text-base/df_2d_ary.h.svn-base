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

//调试信息打印输出开关
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



/* //功能：记录一个内部错误代码。 */
/* //返回：前次的错误代码。 */
/* int dfs_bt_set_internal_err( */
/*     const int err, */
/*     const int lineno = -1, */
/*     const char * funcname = NULL, */
/*     const char * filename = NULL); */
/* //功能：记录一个普通错误代码。 */
/* //返回：前次的错误代码。 */
/* int dfs_bt_set_normal_err( */
/*     const int err, */
/*     const int lineno = -1, */
/*     const char * funcname = NULL, */
/*     const char * filename = NULL); */
/* ////返回：最后的错误代码。 */
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
        ERRNO_BT_CKP_ROOT                       ,   //_ckp_root_info内容异常
        ERRNO_BT_CKP_SERIAL_READ_WRITE          ,   //串行读写时候禁止设置checkpoint
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
        ERRNO_BT_WRITE_ONLY                     ,   //B树为只写模式
        ERRNO_BT_BUILD_IN_MAX_NO                ,   //must be the last one
    };
    enum DFS_BT_CONST {
        DEF_ROW_SIZE            = 8192              ,   //二维数组行
        DEF_ROW_ARY_SIZE        = 256               ,   //二维数组列
        DEF_KEY_BUF_SIZE        = 0x20000           ,   //128KB char
        MAX_BT_DEPTH            = 64                ,
        UNDEF_POS               = MAX_U_INT32       ,
        DEF_FANOUT              = 17                ,   //B树的扇出: dfs_bt_obj_t(2*8-byte), dfs_btree_node_base_t((2+(DEF_FANOUT-1))*8-byte)
        MIN_FANOUT              = 5                 ,
        MAX_FANOUT              = 4090              ,
        MICROSECONDS_PER_SLEEP  = 20                ,
        MAX_2D_ARY_HOLD_REALLOC_NUM     = 16        ,   //暂存的重新申请的二维数组的索引个数(每次扩大一倍)
        MAX_T_STORE_SIZE                = 0x80000   ,   //512KB，单个对象T的存储尺寸上限
        MAX_BT_ROOT_ONCOW_RETRY_NUM   = 16384*1024  ,   //锁定树根节点重试次数(microseconds), about 256s
        MAX_BT_RC_ROOT_NUM   = 1024     ,   //一个B树同一时间被读写的总个数
        MAX_BT_INSTANCE_NUM  = 4        ,   //基本btree树在实例中的个数，一般为1，fp_btree中为4， ns_btree为2
        DEF_BT_INSTANCE_POS     = 0     ,   //基本btree树在一系列实例中的序号，一般为0，fp_btree中为0~3

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
    void init(void) { //初始化对象
        T::init();
    };
public:
    //增加时所做的操作
    inline int action_while_added(void * /*pgc_info*/,
                                  const uint64_t /*t_index*/) {
        return 0;
    };
    //删除前所做的操作
    inline int action_before_gc(void * /*pgc_info*/,
                                const uint64_t /*t_index*/) {
        return 0;
    };
};



//dfs_2d_base_t：记录一个对象是否使用，如果在使用中，则记录了其应用计数(最大2**60-3)，
//  如果是自由对象，则记录其指向下一个自由对象的index(最大2**60-3)
//  引用计数与下一个对象的index不能同时表示
//  在单申请多释放模式下，作为引用计数还是作为单向链表都是线程安全的(无锁多线程加减元素)
//该类可作为二维数组dfs_bt2d_ary_t的数据类型的基类，也可用作其他可回收对象的基类
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
    //有效载荷...
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
    void init(void) { //初始化对象
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
    //把next_item(其index为next_index)加入free_list：
    //this指向next_item，next_item指向原来this指向
    int add_to_list(const uint64_t next_index, dfs_2d_base_t & next_item) {
        uint64_t cv = 0;
        const uint64_t nv = fill_next_index(_ref_counter_next_index, next_index);
        int log_level = DF_UL_LOG_NONE;
        int err = 0;
        //并不是一个free index...
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
    //把this指向的next拿走并让this指向third_index
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
    //if (ref_counter != 0)则+1，否则什么也不做
    //Return: 0 for success, other values for error
    int inc_ref_counter_if_not_zero(uint64_t & ref_counter) {
        uint64_t pre_v = _ref_counter_next_index;   //保存v的原始值到cv
        uint64_t org_v = pre_v;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        ref_counter = 0;
        // 如果原始值ref_counter为0，或者对象是free的，则返回
        while (org_v != 0 && (org_v & FREE_FLAG) == 0) {
            pre_v = df_atomic_compare_exchange(&_ref_counter_next_index, org_v+1, org_v);
            if (pre_v == org_v) {
                //如果*pv原始值和*pv相等，则没有其他线程对*p进行操作，成功完成+1操作
                ++org_v;
                break;
            } else {
                //否则别的线程已经对*pv做了操作，把org_v更新为当前被别的线程修改后的值后重试
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

//二维数组dfs_bt2d_ary_t的数据类型的基类
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
    void init(void) { //初始化对象
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
    void init(const uint64_t data = 0) { //初始化对象
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
    //用户数据类型T需要支持以下接口：
    //  功能：获得该实例store的数据长度(字节)，该值不能超过MAX_T_STORE_SIZE
    static uint64_t get_store_size(void) {
        return sizeof(uint64_t);
    };
    //
    //  功能：存储该实例到buf
    //  输入：data_pos为存储数据的起始位置
    //  输出：data_pos为存储T后新的起始位置
    //  返回：for success, other values for error (例如缓冲区剩余空间不足)
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
    //  功能：从buf中装载到该实例
    //  输入：data_pos为装载T前缓冲区的数据位置
    //  输出：data_pos为装载T后缓冲区的数据位置
    //  返回：for success, other values for error(例如缓冲区内数据不足)
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
    void init(const uint64_t id = UNDEF_ID) { //初始化对象
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
    //用户数据类型T需要支持以下接口：
    //  功能：获得该实例store的数据长度(字节)，该值不能超过MAX_T_STORE_SIZE
    static uint64_t get_store_size(void) {
        return sizeof(uint64_t);
    };
    //
    //  功能：存储该实例到buf
    //  输入：data_pos为存储数据的起始位置
    //  输出：data_pos为存储T后新的起始位置
    //  返回：for success, other values for error (例如缓冲区剩余空间不足)
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
    //  功能：从buf中装载到该实例
    //  输入：data_pos为装载T前缓冲区的数据位置
    //  输出：data_pos为装载T后缓冲区的数据位置
    //  返回：for success, other values for error(例如缓冲区内数据不足)
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
    void init() { //初始化对象
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
    //用户数据类型T需要支持以下接口：
    //  功能：获得该实例store的数据长度(字节)，该值不能超过MAX_T_STORE_SIZE
    static uint64_t get_store_size(void) {
        return (dfs_id_t::get_store_size()+sizeof(uint64_t));
    };
    //
    //  功能：存储该实例到buf
    //  输入：data_pos为存储数据的起始位置
    //  输出：data_pos为存储T后新的起始位置
    //  返回：for success, other values for error (例如缓冲区剩余空间不足)
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
    //  功能：从buf中装载到该实例
    //  输入：data_pos为装载T前缓冲区的数据位置
    //  输出：data_pos为装载T后缓冲区的数据位置
    //  返回：for success, other values for error(例如缓冲区内数据不足)
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



//  功能：象普通数组一样提供对象按下标的随机访问，可以自动扩展数组大小。
//        不支持多线程SingleWriteMultipleRead访问。
//  T: 对象类，需要实现以下成员函数：
//      const T & operator=(const T & src);
//      const T & operator==(const T & src);
template<typename T, uint32_t ROW_SIZE = dfs_bt_const_t::DEF_ROW_SIZE> //建议常数ROW_SIZE是2的方幂
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
    //扩大row_ary
    //return: 0 for success, other values for error
    int _enlarge_row_ary(void) {
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        if ((_t_index_for_next_row/ROW_SIZE) >= _t_row_ary_size) {
            //当前_t_row_ary数组已满
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
    //申请一个新列
    //return: 0 for success, other values for error
    int _alloc_new_row(void) {
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        if ((_t_index_for_next_row/ROW_SIZE) >= _t_row_ary_size) {
            //当前_t_row_ary数组已满
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
                    //项必须为初始值
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
    //Return：0 for success, other values for error
    virtual int _vir_inc_t_ref_counter(const uint64_t index, uint64_t & ref_counter) const = 0;
    virtual int _vir_dec_t_ref_counter(const uint64_t index, uint64_t & ref_counter) const = 0;
};

//reference count indexer..
class dfs_rc_indexer_t : public dfs_bt_const_t {
    friend class dfs_rc_indexer_ext_t;
protected:
    dfs_rc_set_base_t * _prc_set;
    uint64_t _index;
    uint64_t _user_def;   //用户定义，例如序号
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
    //复制对象且增加引用计数
    const dfs_rc_indexer_t & operator=(const dfs_rc_indexer_t & src) {
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        if (&src != this) {
            //占有这么一份index..
            if ((err = this->_set_indexer(&src)) != 0) {
                log_level = DF_UL_LOG_FATAL;
                DF_WRITE_LOG_US(log_level, "_set_indexer() returns 0x%x", err);
                DF_WRITE_LOG_US(log_level, "_index=%ld, src._index=%ld", _index, src._index);
            }
            this->_user_def = src._user_def;
        }

        return *this;
    };
    //构造对象且增加相应的引用计数
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
        //如果是UNDEF INDEX的话.
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

        //相当于控制转移...
        //增加psrc的index的索引,然后释放本身的index索引...
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


//recycle set：其中元素循环使用:
//直接进行freee_head的管理...
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
    volatile uint64_t _cur_index;   //当前项
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
    //读写线程都可能调用
    int _push_free(const uint64_t freed_index) {
        PDU pdu = NULL;
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        if ((err = _get_exist_du(freed_index, pdu)) != 0 || NULL == pdu) {
            log_level = DF_UL_LOG_FATAL;
            DF_WRITE_LOG_US(log_level, "_get_exist_du() returns 0x%x", err);
            err = DF_BT_SET_NORMAL_ERR(ERRNO_BT_INVALID_INDEX);
        } else {
            //这是为free..
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
    //只有写线程调用，因此只有一个实例
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
            //获得raw_data以及获得pdu以及pdu->get_next_index()不是一个原子操作，
            //如果有多个_pop_free线程，则在获得raw_data后pdu->get_next_index()可能
            //发生变化(即使raw_data与ret_index没有变化)，因此该算法对多个_pop_free()线程并不安全。
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
    //输入：tobj：新对象的值
    //输出：新对象的indexer
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
    //Return：0 for success, other values for error
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
    //if (ref_counter != 0)则+1，否则什么也不做
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
    //Return：0 for success, other values for error
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
    //要保证：如果new_index不是UNDEF_INDEX，那么其引用计数必须大于0。
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
            //此处不需要额外进行引用计数的加减：前面的old_rc_indexer = new_rc_indexer
            //导致new_cur_index的引用计数+1，以下的set_index(old_cur_index)
            //将最终导致old_cur_index的引用计数-1
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
    //说明：需要保证获得的项没有被回收
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

                //前面调用_inc_t_ref_counter_if_not_zero()时已经增加了引用计数
                ((dfs_rc_indexer_ext_t *)&rc_indexer)->set_indexer(
                    (dfs_rc_set_base_t *)this,
                    t_index);

                break;
            }
            //获得_cur_index的当前值以便重试
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
    //Return：0 for success, other values for error
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



//数组模板类，外部看是一维数组，以uint64_t作为下标访问，数组元素类型是T。
//dfs_bt2d_ary_t<typename T, uint32_t ROW_SIZE>
//  模板参数：T，用户的数据单元类型
//            ROW_SIZE：常数，数组行尺寸，建议为2的方幂
//特别说明：需要使用外部的锁机制来保证对数组的修改是单一的(即同一时间只能有一个线程修改数组)！
//          规则：public接口仅提供“只读”操作，protected接口提供修改操作。
//
//内部是二维数组，每行ROW_SIZE个元素。
//输入参数：
//  T: 对象类(用户数据单元)，需要提供以下接口：
//    T();
//    T(const T & src);
//    const T & operator=(const T & src);
//
//    //  功能：对T进行初始化
//    void init(void);
//
//外部接口(public)：
//  规则：public接口仅提供“只读”操作，protected接口提供修改操作
//  构造函数：缺省构造函数;
//  int acquire_t(const T * ptobjv, uint64_t & t_index, T ** pptobj);
//      功能：申请一个新的对象。成功时用pt所指对象初始化(pt非空时)或调用init()进行初始化(pt为空)。
//            成功时，对象的状态总是为in_use，引用计数为0。
//      输入：ptobjv：其指向的值用来初始化新对象，可以为NULL
//      输出：新对象的index以及指向新对象的指针，UNDEF_INDEX表示没有自由对象且无法申请新对象
//  int get_t_unit(const uint64_t t_index, const T * pct) const
//      功能：获得一个对象的指针，如果输入index非法或者对象不在使用中，则返回NULL
//      输入：对象的INDEX
//      返回：对象的指针，如果输入index非法或者对象不在使用中则返回NULL
//  bool is_valid_t_index(const uint64_t t_index) const
//      功能：判断一个INDEX是否合法(使用中或自由的INDEX都是合法的)
//      输入：对象的INDEX
//      返回：true表示合法INDEX，false表示非法的INDEX
//      说明：新对象是成批申请的
//  uint64_t get_size(void)
//      功能：当前数组的大小(单元个数)
//      返回：返回0表示尚未有任何对象
//外部接口(protected)：
//  int inc_t_ref_counter(const uint64_t t_index, uint64_t & ref_counter)
//      功能：对象的引用计数加1
//      输入：对象的INDEX
//      返回：0 for success, other values for error
//  int dec_t_ref_counter(const uint64_t t_index, uint64_t & ref_counter)
//      功能：对象的引用计数减1
//      输入：对象的INDEX
//      返回：0 for success, other values for error
//  int get_t_ref_counter(const uint64_t t_index, uint64_t & ref_counter) const;
//      功能：获得对象的引用计数
//      输入：对象的INDEX
//      输出：对象的引用计数，UNDEF_REF_COUNTER表示非法的INDEX或者对象的引用计数异常
//      返回：0 for success, other values for error
//  int get_exist_t_unit(const uint64_t t_index, PT & pt)
//  int get_may_not_exist_t_unit(const uint64_t t_index, PT & pt)
//      功能：获得一个对象的指针，如果输入index非法则返回NULL，否则返回该对象指针
//      输入：对象的INDEX
//      返回：0 for success, other values for error
//private:
//  int _release_t(const uint64_t t_index)
//      功能：释放一个对象
//      输入：要释放对象的INDEX
//      返回：0 for success, other values for error

//总是保留第一个元素
template<typename T, uint32_t ROW_SIZE>   //, //建议常数ROW_SIZE是2的方幂
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
    uint64_t _t_index_for_next_alloc; //新申请ROW的分配位置
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
                    //总是保留第一个元素
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
    //扩大row_ary
    //return: 0 for success, other values for error
    int _enlarge_row_ary(void) {
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        if ((_t_index_for_next_row/ROW_SIZE) >= _t_row_ary_size) {
            //当前_t_row_ary数组已满
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
                //把旧数组加入到缓冲区链，暂时不释放，因为其他读线程可能正在访问。
                delete [] _realloc_t_row_ary[_realloc_counter];
                _realloc_t_row_ary[_realloc_counter] = old_t_row_ary;
                _realloc_counter = (_realloc_counter+1)%df_len_of_ary(_realloc_t_row_ary);
            }
        }

        return err;
    };
    //申请一个新列
    //return: 0 for success, other values for error
    int _alloc_new_row(void) {
        int log_level = DF_UL_LOG_NONE;
        int err = 0;

        if ((_t_index_for_next_row/ROW_SIZE) >= _t_row_ary_size) {
            //当前_t_row_ary数组已满
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
    //读写线程都可能调用
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
    //只有写线程调用，因此只有一个实例
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
            //获得raw_data以及获得pdu以及pdu->get_next_index()不是一个原子操作，
            //如果有多个_pop_free线程，则在获得raw_data后pdu->get_next_index()可能
            //发生变化(即使raw_data与ret_index没有变化)，因此该算法对多个_pop_free()线程并不安全。
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
    //      输入：ptobjv：其指向的值用来初始化新对象，可以为NULL
    //      输出：新对象的index以及指向新对象的指针，UNDEF_INDEX表示没有自由对象且无法申请新对象
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
    //Return：0 for success, other values for error
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
    //if (ref_counter != 0)则+1，否则什么也不做
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
    //Return：0 for success, other values for error
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
        //进行引用计数的内存释放..
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
    //当前数组的大小(单元个数)
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


//    //用来以无锁方式对一个结构进行修改内容和复制内容
//    //修改内容时先_mutation_counter增1，修改完成后把_mutation_counter复制到_syn_mutation_counter
//    //复制内容时先复制_syn_mutation_counter，复制完成后再复制_mutation_counter，
//    //如果对象正在被修改，则_mutation_counter与_syn_mutation_counter不相等
//class dfs_mutation_sycnizer_t
//{
//private:
//    //修改_mutation_counter增1，修改完成时设置_syn_mutation_counter为_mutation_counter
//    //复制时按相反顺序：先复制_syn_mutation_counter，再_mutation_counter
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
//    //把赋值操作声明为protected是为了避免用它直接给btree的根节点_root_info赋值，
//    //因为更新_root_info需要特别的步骤(start_mutating()=>更新root_node_index=>end_mutating())
//    inline const dfs_mutation_sycnizer_t & operator=(const dfs_mutation_sycnizer_t & src)
//    {
//        if (&src != this)
//        {
//            //修改顺序：先_mutation_counter，完成时设置_syn_mutation_counter为_mutation_counter
//            //复制顺序：与修改顺序正好相反
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
//    //数据是否完整(vs.正在修改中)
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

