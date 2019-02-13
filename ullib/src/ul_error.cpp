#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include "ul_error.h"
static const char *g_errstr[] = {
    "CALL SUCCESS",
    "CALL UL_GETERR(INT ERRNO) TO FIND DETAIL",
    "ALLOC MEMORY FAIL",
    "MEMORY SEGMENT",
    "READ ERR",
    "WRITE ERR",
    "INVALID FILE[NET] HANDLE",
    "NET CONNECT FAIL",
    "INVALID INPUT PARAM",
    "OPEN FILE ERR",
};
/**
 * 线程内部使用的统一错误号
 */
static pthread_key_t g_ulerrno_key = PTHREAD_KEYS_MAX;
static pthread_once_t g_ulerrno_ronce = PTHREAD_ONCE_INIT;
static void
ul_errno_destroy(void *ptr)
{
    if (ptr) {
        free(ptr);
    }
}
static void
ul_errno_runonce()
{
    pthread_key_create(&g_ulerrno_key, ul_errno_destroy);
}
static int *
ul_errno_get_ptr()
{
    void *ptr = NULL;
    pthread_once(&g_ulerrno_ronce, ul_errno_runonce);
    ptr = pthread_getspecific(g_ulerrno_key);
    if (ptr == NULL) {
        ptr = calloc(1, sizeof(int));
        pthread_setspecific(g_ulerrno_key, ptr);
    }
    return (int *) ptr;
}
int
ul_geterrno()
{
    int *errno = ul_errno_get_ptr();
    if (errno == NULL) {
        return 0;
    }
    return *errno;
}
int
ul_seterrno(int err)
{
    int *errno = ul_errno_get_ptr();
    if (errno == NULL) {
        return -1;
    }
    *errno = err;
    return 0;
}
const char *
ul_geterrstr(int errno)
{
    if (errno < 0 || errno >= (int) (sizeof(g_errstr) / sizeof(g_errstr[0]))) {
        return "INVALID ERRNO";
    }
    if (errno == UL_DETAIL_ERR) {
        return ul_geterrbuf();
    }
    return g_errstr[errno];
}
/**
 * 线程内部使用的错误缓冲区
 */
static const unsigned int UL_ERRSTR_BUFSIZE = 1024;
static pthread_key_t g_ulerrbuf_key = PTHREAD_KEYS_MAX;
static pthread_once_t g_ulerrbuf_ronce = PTHREAD_ONCE_INIT;
static void
ul_errbuf_destroy(void *ptr)
{
    if (ptr) {
        free(ptr);
    }
}
static void
ul_errbuf_runonce()
{
    pthread_key_create(&g_ulerrbuf_key, ul_errbuf_destroy);
}
static char *
ul_errbuf_get_ptr()
{
    void *ptr = NULL;
    pthread_once(&g_ulerrbuf_ronce, ul_errbuf_runonce);
    ptr = pthread_getspecific(g_ulerrbuf_key);
    if (ptr == NULL) {
        ptr = calloc(UL_ERRSTR_BUFSIZE, sizeof(char));
        pthread_setspecific(g_ulerrbuf_key, ptr);
    }
    return (char *) ptr;
}
const char *
ul_geterrbuf()
{
    char *errbuf = ul_errbuf_get_ptr();
    if (errbuf == NULL) {
        return "";
    }
    return (const char *) errbuf;
}
int
ul_seterrbuf(const char *format, ...)
{
    char *errbuf = ul_errbuf_get_ptr();
    if (errbuf == NULL) {
        return -1;
    }
    va_list args;
    va_start(args, format);
    vsnprintf(errbuf, UL_ERRSTR_BUFSIZE, format, args);
    va_end(args);
    return 0;
}
//############################################################
//modified by zhangyan04@baidu.com
class GlobalDestroy
{
    public:
    ~GlobalDestroy()
    {
        void *ptr;
        ptr=pthread_getspecific(g_ulerrno_key);
        if(ptr){
            free(ptr);
            pthread_setspecific(g_ulerrno_key, NULL);
        }
        ptr=pthread_getspecific(g_ulerrbuf_key);
        if(ptr){
            free(ptr);
            pthread_setspecific(g_ulerrbuf_key, NULL);
        }
    }
};
static GlobalDestroy g_global_destroy;
//############################################################
/* vim: set ts=4 sw=4 tw=100 noet: */
