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
 * @brief COM REGEX �Ƕ������ʽ��PCRE���еķ�װ����PCRE�Ļ���������һЩ����,
 * ��������һ���ļ򻯣�����ʹ��
 *  
 **/


#ifndef  __SPREG_H_
#define  __SPREG_H_

#include "pcre.h"

/**
* @brief ������
*
*  
*/
enum {
    SPREG_NOMATCH          = PCRE_ERROR_NOMATCH,		  /**< ��ƥ��           -1               */
    SPREG_NULL             = PCRE_ERROR_NULL,			  /**< ��������Ƿ�     -2               */
    SPREG_BADOPTION        = PCRE_ERROR_BADOPTION,		  /**< ƥ��ѡ�����ô��� -3               */
    SPREG_NOMEMORY         = PCRE_ERROR_NOMEMORY,		  /**< �ڴ治��         -6               */
    SPREG_MATCHLIMIT       = PCRE_ERROR_MATCHLIMIT,		  /**< ƥ����������     -8               */
    SPREG_BADUTF8         = PCRE_ERROR_BADUTF8,			  /**< �����UTF8����   -10              */
    SPREG_INTERNAL        = PCRE_ERROR_INTERNAL,		  /**< �ڲ�����         -14              */
    SPREG_BADNEWLINE      = PCRE_ERROR_BADNEWLINE,		  /**< NEWLINE����      -23              */
	SPREG_BADBUFFER       = -99		                      /**< buffer���㣬��PCRE��û����������*/
		                                                      /**< ����@spreg_replace���滻�����*/
															  /**< ������ռ�̫Сʱ����              */
};

//����ƥ���ѡ��
#define SPREG_DEFAULT       0		                  /**< Ĭ��ѡ��                        */
#define SPREG_CASELESS      PCRE_CASELESS		      /**< ���Դ�Сд                      */
#define SPREG_MULTILINE		PCRE_MULTILINE		      /**< ���ö���ģʽ��^ $����ƥ��\r\n   */
#define SPREG_DOTALL		PCRE_DOTALL			      /**< .�ſ���ƥ��\r\n                 */
#define SPREG_NOTBOL		PCRE_NOTBOL				  /**< ^��ƥ�䴮��ͷ                   */
#define SPREG_NOTEOL		PCRE_NOTEOL				  /**< $��ƥ�䴮ĩβ                   */
#define SPREG_UNGREEDY		PCRE_UNGREEDY		      /**< ��̰��ģʽ                      */
#define SPREG_NOTEMPTY      PCRE_NOTEMPTY			  /**< ƥ��ʱ��ƥ��մ�                */
#define SPREG_UTF8			PCRE_UTF8			      /**< UTF8ģʽ                        */
#define SPREG_NO_UTF8_CHECK PCRE_NO_UTF8_CHECK	      /**< ʹ��UTF8ģʽ�������UTF8�Ƿ�Ϸ�*/



/**
 * @brief �洢ÿ��ƥ��ķ��ؽ��
 *
 *	ÿ��ƥ������ ����������ʽƥ��Ĵ�λ�ã�������ʹ��'('')'��������ƥ����ʽ
 *  
 *  
 *  ʹ����Ҫspreg_match_init�����������������ʹ�ã��±�0��λ��������������ʽƥ��Ĵ�λ�ã�
 *  ��ƥ����ʽ˳���'('���ֵ��Ⱥ�˳����ͬ
 *  
**/
typedef struct _spreg_match_t {
	int match_begin;		  /**< ƥ�䴮��ԭ���еĿ�ʼλ��       */
	int match_end;		      /**< ƥ�䴮��ԭ������ֹλ��       */
} spreg_match_t;

/**
 * @brief ������ƥ��Ľṹ��
 *
 *  ʹ����Ҫspreg_init������ʹ�ú���Ҫ����
 *  һ��spreg_t init�ú��ڶ���߳���ʹ���ǰ�ȫ��
 *  
**/
typedef struct _spreg_t spreg_t;



/**
* @brief �ص���������Ĳ�����������ƥ�����ͽ���ƥ���Ŀ�괮
*
* �ص���������spreg_callback_tʹ�õĲ�����@spreg_search_all�� @spreg_split��Ҫʹ�õ�
*   
* @note ��@spreg_search_all��rmatch���Ǵ������ÿһ�ε�ƥ��������������ƥ�䴮���±�0λ�ñ�ʾ
*       ������ƥ�䴮��λ�á�
*
*       ��@spreg_split�У�rmatch[0]�ı�ʾ�ָ��λ�ã�rmatch[1]��λ�ô���ָ�����һ����������
*       ���ʽ��λ�ã�rmatch[2]�Ժ�����ƥ�����ƥ�䴮
*/
typedef struct _spreg_callback_param_t {
	spreg_match_t *rmatch;		  /**< ÿһ�ε�ƥ��������������ƥ�䴮       */
	const char *src;				  /**< ����ƥ���Ŀ�괮ָ��     */
	void *arg;						  /**< �û������Զ������       */
	int nrmatch;		              /**< (��)ƥ�䴮����       */
	int src_size;		              /**< ����ƥ���Ŀ�괮����       */
} spreg_callback_param_t;

/**
 * @brief �ص���������
 *
 * @param [in] callback_match   : �ص���������Ĳ�����������ƥ�����ͽ���ƥ���Ŀ�괮
 *                                �ص���������ֵС��0����ֹͣƥ�����
 *                                ��@spreg_callback_param_t
 * @author baonh
**/
typedef int spreg_callback_t(spreg_callback_param_t *callback_match);

/**
 * @brief ���䷵�ؽ��ʹ�õĿռ�
 *
 * ���䷵�ؽ��ʹ�õĿռ䣬nΪ�ɴ�ŵ�����ƥ�䴮������
 * ������ʽ����()�����������ַ����ĸ���
 *
 * @param [in] n   : ����(��)ƥ�䴮����
 * @return  �������ƥ��ķ��ؽ�� NULLΪʧ��
 * @author baonh
**/
spreg_match_t *spreg_match_init (int n);

/**
 * @brief ��������ʽ���г�ʼ��
 *
 * @param [in] reg   : ��Ҫ��ʼ����������ʽ
 * @param [in] errinfo   : ��������ؽ���Ĵ������������ڳ�������ͷǳ��࣬���ʺ�ʹ�÷���ֵ
 * @param [in] option   : ���г�ʼ����ѡ��
 *           - SPREG_DEFAULT    Ĭ��ѡ��
 *           - SPREG_CASELESS   ���Դ�Сд
 *           - SPREG_MULTILINE  ʹ�ö���ģʽ ^ $ ����ƥ�� \r\nǰ��
 *           - SPREG_DOTALL  . �ſ���ƥ����� \r\n���ڵĻ��з�
 *           - SPREG_UNGREEDY       ʹ�÷�̰��ģʽ
 *           - SPREG_UTF8            ʹ��UTF8ģʽ�����Լ��UTF8�����Ƿ�Ϸ�
 *           - SPREG_NO_UTF8_CHECK   �����UTF8�����Ƿ�Ϸ�
 *
 * @return  ��ʼ�����spreg_t, �ǽ���������ʽƥ��ʹ�õľ��
 * @author baonh
**/
spreg_t * spreg_init(const char *reg,  
		const char **errinfo = NULL,
		int option = SPREG_DEFAULT);

/**
 * @brief ��src�в��ҷ���re ���ַ���
 *
 * @param [in]   :  ���в��ҵ�������ʽ�������spreg_init��ʼ��
 * @param [in] src   : ��Ҫ���ҵ�Ŀ���ַ���
 * @param [in] src_size   : ��Ҫ���ҵ�Ŀ���ַ�������
 * @param [out] rmatch   : ƥ��Ľ��,���������ΪNULL, n����Ϊ0,����ᱨ��
 * ��������������������������ʱ����ֵΪ0��ʾsrc�д��ڷ���re ���ַ���
 * @param [in] n   : ƥ��Ľ�������(��)ƥ�䴮����
 * @param [in] option   : ����ƥ���ѡ��
 *           - SPREG_DEFAULT   Ĭ��ѡ��
 *           - SPREG_NO_UTF8_CHECK   �����UTF8�����Ƿ�Ϸ�(reʹ��UTF8ģʽ)
 *           - SPREG_NOTBOL     ʹ����SPREG_MULTILINE ��^��ƥ���ַ�����ͷ��
 *                                  ֻƥ�䡯\r����\n��֮������п�ʼ��
 *  								û��ָ��PCRE_MULTILINE�����ѡ����Ч
 *           - SPREG_NOTEOL		ʹ����SPREG_MULTILINE��$��ƥ���ַ���ĩβ��
 *                                  ֻƥ���ԡ�\r����\n����Ϊ��������ĩβ��
 *									û��ָ��PCRE_MULTILINE�����ѡ����Ч
 * @return  >0�ɹ�������ƥ�����ƥ�䴮���� ��0Ϊ���ش��� �� ������˵��
 *          0 �ǵ�n == 0 ʱ, ƥ��ɹ����ص�
 * @author baonh
**/
int spreg_search(const spreg_t *re,
		const char *src,  
		int src_size,
		spreg_match_t *rmatch, 
		int n, 
		int option = SPREG_DEFAULT);
/**
 * @brief �ж�Ŀ�괮 src�Ƿ���ȫƥ�� ������ʽ re
 *
 * 
 * @param [in] re  :  ���в��ҵ�������ʽ�������spreg_init��ʼ��
 * @param [in] src   : ��Ҫƥ���Ŀ���ַ���
 * @param [in] src_size   : ��Ҫ���ҵ�Ŀ���ַ�������
 * @param [out] rmatch   : ƥ��Ľ��, ��Ϊ NULL,����ȡƥ����
 * @param [in] n   : ƥ��Ľ�������(��)ƥ�䴮������ ��0 ����ȡƥ����
 * @param [in] option   : ����ƥ���ѡ��� spreg_search��option˵��
 * @return  >0ƥ��ɹ��� ��0�����ƥ��
 * @author baonh
**/

int spreg_match(const spreg_t *re, 
		const char *src, 
		int src_size,
		spreg_match_t *rmatch = NULL, 
		int n = 0,
		int option = SPREG_DEFAULT);

/**
 * @brief ��src�в������з���re ���ַ���
 *
 * ͨ�� callback ��ÿ��ƥ������в����� callback����ֵС��0����ֹͣƥ��
 * callback�еĲ���˵����@spreg_callback_param_t
 *
 * @param [in] re  :  ���в��ҵ�������ʽ�������spreg_init��ʼ��
 * @param [in] src   : ��Ҫ���ҵ�Ŀ���ַ���
 * @param [in] src_size   : ��Ҫ���ҵ�Ŀ���ַ�������
 * @param [in] callback : ������ƥ���callback,
 * @param [in] arg  : �û�����callbackʹ�õĲ�����Ĭ��ΪNULL
 * @param [in] option   : ����ƥ���ѡ���spreg_search��option˵��
 * @return  >0����ƥ��ɹ��ĸ���(����callback����ֵС�ڵ����һ��ƥ��), <0 ���ش�����
 * @author baonh
**/

int spreg_search_all(const spreg_t *re,
		const char *src,  
		int src_size,
		spreg_callback_t *callback,
		void *arg = NULL,
		int option = SPREG_DEFAULT);

/**
 * @brief ��src�а��� re ƥ����ַ������зָ�
 *
 * ͨ�� callback ��ÿ���ָ������в����� callback����ֵС��0����ֹͣƥ��
 * callback �еĲ���˵����@spreg_callback_param_t
 * ע��:����ķָ�Ĵ������մ�
 *
 * @param [in] re :  ����ƥ���������ʽ�������spreg_init��ʼ��
 * @param [in] src   : ��Ҫ�ָ��Ŀ���ַ���
 * @param [in] src_size   : ��Ҫ�ָ��Ŀ���ַ�������
 * @param [in] callback : �������ָ�����callback 
 * @param [in] arg : �û�����callbackʹ�õĲ�����Ĭ��ΪNULL
 * @param [in] option   : ����ƥ���ѡ��� spreg_search��option˵��
 * @return  >=0���طָ����ĸ���(����callback����ֵС�ڵ����һ��ƥ��), <0 ���ش�����
 * @author baonh
**/

int spreg_split(const spreg_t *re,
		const char *src,  
		int src_size,
		spreg_callback_t *callback,
		void *arg = NULL,
		int option = SPREG_DEFAULT);


/**
 * @brief ��Ŀ�괮������re�Ĵ���replace_string�����滻
 *
 * 
 * replace_string�п���ʹ�������﷨��\\1 \\2 ��ʾƥ�䴮���Ӵ�, ֧��1-9 ��������
 * 
 * @param [in]   :  ���в��ҵ�������ʽ�������spreg_init��ʼ��
 * @param [in] src   : ��Ҫ�滻��Ŀ���ַ���
 * @param [in] src_size   : ��Ҫ���ҵ�Ŀ���ַ�������
 * @param [in] replace_string : �����滻���ַ���
 * @param [in] dst : �滻��Ľ��
 * @param [in] dst_size :  dst ����󳤶�
 * @param [in] limit : ����滻�ĸ�����Ĭ��0Ϊȫ���滻
 * @param [in] option   : ����ƥ���ѡ��� spreg_search��option˵��
 * @return  >=0ƥ��ɹ�,���ؽ���ַ���ʵ�ʳ��ȣ� ��0����
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
 * @brief ���� spreg_t �ṹ��
 *
 * @param [in] re   : ��Ҫ���ٵ�spreg_t �ṹ��
 * @return  0���سɹ� 
 * @author baonh
**/
int spreg_destroy(spreg_t *re);


/**
 * @brief ���� spreg_match_t �ṹ�� 
 *
 * @param [in] rmatch   : ��Ҫ���ٵ�spreg_match_t �ṹ��
 * @return  0 ���سɹ�
 * @author baonh
**/
int spreg_match_destroy(spreg_match_t *rmatch);


/**
 * @brief ��ƥ��Ĵ�����Ϣ���ַ�����ʽ�������Ҫ���ڴ������־
 *
 * @param [in] err   : ƥ��ʱ���صĴ�����Ϣ
 * @author baonh
**/
const char *spreg_error(int err);

#endif  //__SPREG_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
