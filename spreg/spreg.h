/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: spreg.h,v 1.6 2009/02/06 07:16:04 baonh Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file regexapi.h
 * @author baonh(baonenghui@baidu.com)
 * @version $Revision: 1.6 $ 
 * @brief COM REGEX 是对正则表式库PCRE进行的封装，在PCRE的基础上增加一些功能,
 * 并进行了一定的简化，方便使用
 *  
 **/


#ifndef  __SPREG_H_
#define  __SPREG_H_

#include "pcre.h"

/**
* @brief 错误码
*
*  
*/
enum {
    SPREG_NOMATCH          = PCRE_ERROR_NOMATCH,		  /**< 不匹配           -1               */
    SPREG_NULL             = PCRE_ERROR_NULL,			  /**< 传入参数非法     -2               */
    SPREG_BADOPTION        = PCRE_ERROR_BADOPTION,		  /**< 匹配选项设置错误 -3               */
    SPREG_NOMEMORY         = PCRE_ERROR_NOMEMORY,		  /**< 内存不够         -6               */
    SPREG_MATCHLIMIT       = PCRE_ERROR_MATCHLIMIT,		  /**< 匹配数量过多     -8               */
    SPREG_BADUTF8         = PCRE_ERROR_BADUTF8,			  /**< 错误的UTF8编码   -10              */
    SPREG_INTERNAL        = PCRE_ERROR_INTERNAL,		  /**< 内部错误         -14              */
    SPREG_BADNEWLINE      = PCRE_ERROR_BADNEWLINE,		  /**< NEWLINE错误      -23              */
	SPREG_BADBUFFER       = -99		                      /**< buffer不足，在PCRE中没有这个错误号*/
		                                                      /**< 是在@spreg_replace中替换结果的*/
															  /**< 串缓冲空间太小时返回              */
};

//正则匹配的选项
#define SPREG_DEFAULT       0		                  /**< 默认选项                        */
#define SPREG_CASELESS      PCRE_CASELESS		      /**< 忽略大小写                      */
#define SPREG_MULTILINE		PCRE_MULTILINE		      /**< 采用多行模式，^ $可以匹配\r\n   */
#define SPREG_DOTALL		PCRE_DOTALL			      /**< .号可以匹配\r\n                 */
#define SPREG_NOTBOL		PCRE_NOTBOL				  /**< ^不匹配串开头                   */
#define SPREG_NOTEOL		PCRE_NOTEOL				  /**< $不匹配串末尾                   */
#define SPREG_UNGREEDY		PCRE_UNGREEDY		      /**< 非贪婪模式                      */
#define SPREG_NOTEMPTY      PCRE_NOTEMPTY			  /**< 匹配时不匹配空串                */
#define SPREG_UTF8			PCRE_UTF8			      /**< UTF8模式                        */
#define SPREG_NO_UTF8_CHECK PCRE_NO_UTF8_CHECK	      /**< 使用UTF8模式但不检查UTF8是否合法*/



/**
 * @brief 存储每个匹配的返回结果
 *
 *	每次匹配后除了 整个正则表达式匹配的串位置，还包括使用'('')'包含的子匹配表达式
 *  
 *  
 *  使用需要spreg_match_init出来，当成数组进行使用．下标0的位置是整个正则表达式匹配的串位置，
 *  子匹配表达式顺序和'('出现的先后顺序相同
 *  
**/
typedef struct _spreg_match_t {
	int match_begin;		  /**< 匹配串在原串中的开始位置       */
	int match_end;		      /**< 匹配串在原串中终止位置       */
} spreg_match_t;

/**
 * @brief 用来做匹配的结构体
 *
 *  使用需要spreg_init出来，使用后需要销毁
 *  一个spreg_t init好后在多个线程中使用是安全的
 *  
**/
typedef struct _spreg_t spreg_t;



/**
* @brief 回调函数传入的参数，包括了匹配结果和进行匹配的目标串
*
* 回调函数类型spreg_callback_t使用的参数，@spreg_search_all和 @spreg_split需要使用到
*   
* @note 在@spreg_search_all中rmatch中是代表的是每一次的匹配结果，包括了子匹配串。下标0位置表示
*       完整的匹配串的位置。
*
*       在@spreg_split中，rmatch[0]的表示分割串的位置，rmatch[1]的位置代表分割串后面第一个满足正则
*       表达式的位置，rmatch[2]以后是其匹配的子匹配串
*/
typedef struct _spreg_callback_param_t {
	spreg_match_t *rmatch;		  /**< 每一次的匹配结果，包括了子匹配串       */
	const char *src;				  /**< 进行匹配的目标串指针     */
	void *arg;						  /**< 用户传入自定义参数       */
	int nrmatch;		              /**< (子)匹配串个数       */
	int src_size;		              /**< 进行匹配的目标串长度       */
} spreg_callback_param_t;

/**
 * @brief 回调函数类型
 *
 * @param [in] callback_match   : 回调函数传入的参数，包括了匹配结果和进行匹配的目标串
 *                                回调函数返回值小于0，将停止匹配过程
 *                                见@spreg_callback_param_t
 * @author baonh
**/
typedef int spreg_callback_t(spreg_callback_param_t *callback_match);

/**
 * @brief 分配返回结果使用的空间
 *
 * 分配返回结果使用的空间，n为可存放的最大的匹配串个数，
 * 正则表达式中用()包起来的子字符串的个数
 *
 * @param [in] n   : 最大的(子)匹配串个数
 * @return  存放正则匹配的返回结果 NULL为失败
 * @author baonh
**/
spreg_match_t *spreg_match_init (int n);

/**
 * @brief 用正则表达式进行初始化
 *
 * @param [in] reg   : 需要初始化的正则表达式
 * @param [in] errinfo   : 如果出错返回结果的错误描述，由于出错的类型非常多，不适合使用返回值
 * @param [in] option   : 进行初始化的选项
 *           - SPREG_DEFAULT    默认选项
 *           - SPREG_CASELESS   忽略大小写
 *           - SPREG_MULTILINE  使用多行模式 ^ $ 可以匹配 \r\n前后
 *           - SPREG_DOTALL  . 号可以匹配包括 \r\n在内的换行符
 *           - SPREG_UNGREEDY       使用非贪婪模式
 *           - SPREG_UTF8            使用UTF8模式，可以检查UTF8编码是否合法
 *           - SPREG_NO_UTF8_CHECK   不检查UTF8编码是否合法
 *
 * @return  初始化后的spreg_t, 是进行正则表达式匹配使用的句柄
 * @author baonh
**/
spreg_t * spreg_init(const char *reg,  
		const char **errinfo = NULL,
		int option = SPREG_DEFAULT);

/**
 * @brief 在src中查找符合re 的字符串
 *
 * @param [in]   :  进行查找的正则表达式句柄，由spreg_init初始化
 * @param [in] src   : 需要查找的目标字符串
 * @param [in] src_size   : 需要查找的目标字符串长度
 * @param [out] rmatch   : 匹配的结果,如果这里设为NULL, n必须为0,否则会报错，
 * 　　　　　　　　　　　　此时返回值为0表示src中存在符合re 的字符串
 * @param [in] n   : 匹配的结果的最大(子)匹配串个数
 * @param [in] option   : 进行匹配的选项
 *           - SPREG_DEFAULT   默认选项
 *           - SPREG_NO_UTF8_CHECK   不检查UTF8编码是否合法(re使用UTF8模式)
 *           - SPREG_NOTBOL     使用了SPREG_MULTILINE 后^不匹配字符串开头，
 *                                  只匹配’\r’’\n’之后的新行开始，
 *  								没有指定PCRE_MULTILINE，这个选项无效
 *           - SPREG_NOTEOL		使用了SPREG_MULTILINE后$不匹配字符串末尾，
 *                                  只匹配以’\r’’\n’做为结束符的末尾，
 *									没有指定PCRE_MULTILINE，这个选项无效
 * @return  >0成功，返回匹配的子匹配串数量 非0为返回错误 见 错误码说明
 *          0 是当n == 0 时, 匹配成功返回的
 * @author baonh
**/
int spreg_search(const spreg_t *re,
		const char *src,  
		int src_size,
		spreg_match_t *rmatch, 
		int n, 
		int option = SPREG_DEFAULT);
/**
 * @brief 判断目标串 src是否完全匹配 正则表达式 re
 *
 * 
 * @param [in] re  :  进行查找的正则表达式句柄，由spreg_init初始化
 * @param [in] src   : 需要匹配的目标字符串
 * @param [in] src_size   : 需要查找的目标字符串长度
 * @param [out] rmatch   : 匹配的结果, 设为 NULL,不获取匹配结果
 * @param [in] n   : 匹配的结果的最大(子)匹配串个数， 设0 不获取匹配结果
 * @param [in] option   : 进行匹配的选项，见 spreg_search的option说明
 * @return  >0匹配成功， 非0错误或不匹配
 * @author baonh
**/

int spreg_match(const spreg_t *re, 
		const char *src, 
		int src_size,
		spreg_match_t *rmatch = NULL, 
		int n = 0,
		int option = SPREG_DEFAULT);

/**
 * @brief 在src中查找所有符合re 的字符串
 *
 * 通过 callback 对每个匹配项进行操作， callback返回值小于0，则停止匹配
 * callback中的参数说明见@spreg_callback_param_t
 *
 * @param [in] re  :  进行查找的正则表达式句柄，由spreg_init初始化
 * @param [in] src   : 需要查找的目标字符串
 * @param [in] src_size   : 需要查找的目标字符串长度
 * @param [in] callback : 处理单个匹配的callback,
 * @param [in] arg  : 用户传入callback使用的参数，默认为NULL
 * @param [in] option   : 进行匹配的选项，见spreg_search的option说明
 * @return  >0返回匹配成功的个数(包括callback返回值小于的最后一个匹配), <0 返回错误码
 * @author baonh
**/

int spreg_search_all(const spreg_t *re,
		const char *src,  
		int src_size,
		spreg_callback_t *callback,
		void *arg = NULL,
		int option = SPREG_DEFAULT);

/**
 * @brief 在src中按照 re 匹配的字符串进行分割
 *
 * 通过 callback 对每个分割结果进行操作， callback返回值小于0，则停止匹配
 * callback 中的参数说明见@spreg_callback_param_t
 * 注意:这里的分割的串包括空串
 *
 * @param [in] re :  进行匹配的正则表达式句柄，由spreg_init初始化
 * @param [in] src   : 需要分割的目标字符串
 * @param [in] src_size   : 需要分割的目标字符串长度
 * @param [in] callback : 处理单个分割结果的callback 
 * @param [in] arg : 用户传入callback使用的参数，默认为NULL
 * @param [in] option   : 进行匹配的选项，见 spreg_search的option说明
 * @return  >=0返回分割结果的个数(包括callback返回值小于的最后一个匹配), <0 返回错误码
 * @author baonh
**/

int spreg_split(const spreg_t *re,
		const char *src,  
		int src_size,
		spreg_callback_t *callback,
		void *arg = NULL,
		int option = SPREG_DEFAULT);


/**
 * @brief 对目标串中满足re的串用replace_string进行替换
 *
 * 
 * replace_string中可以使用特殊语法用\\1 \\2 表示匹配串的子串, 支持1-9 反向引用
 * 
 * @param [in]   :  进行查找的正则表达式句柄，由spreg_init初始化
 * @param [in] src   : 需要替换的目标字符串
 * @param [in] src_size   : 需要查找的目标字符串长度
 * @param [in] replace_string : 用来替换的字符串
 * @param [in] dst : 替换后的结果
 * @param [in] dst_size :  dst 的最大长度
 * @param [in] limit : 最多替换的个数，默认0为全部替换
 * @param [in] option   : 进行匹配的选项，见 spreg_search的option说明
 * @return  >=0匹配成功,返回结果字符串实际长度， 非0错误
 * @author baonh
**/
int spreg_replace(const spreg_t *re,
		const char *src,
		int src_size,
		const char *replace_string,
		char *dst,
		int dst_size,
		int limit = 0,
		int option = SPREG_DEFAULT);


/**
 * @brief 销毁 spreg_t 结构体
 *
 * @param [in] re   : 需要销毁的spreg_t 结构体
 * @return  0返回成功 
 * @author baonh
**/
int spreg_destroy(spreg_t *re);


/**
 * @brief 销毁 spreg_match_t 结构体 
 *
 * @param [in] rmatch   : 需要销毁的spreg_match_t 结构体
 * @return  0 返回成功
 * @author baonh
**/
int spreg_match_destroy(spreg_match_t *rmatch);


/**
 * @brief 将匹配的错误信息已字符串形式输出，主要用于打错误日志
 *
 * @param [in] err   : 匹配时返回的错误信息
 * @author baonh
**/
const char *spreg_error(int err);

#endif  //__SPREG_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
