/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: spreg.cpp,v 1.9 2009/02/06 07:16:04 baonh Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file spreg.cpp
 * @author baonh(baonenghui@baidu.com)
 * @date 2008/05/27 20:03:06
 * @version $Revision: 1.9 $ 
 * @brief 
 *  
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "spreg.h"

struct _spreg_t {
	pcre *_re;
	pcre_extra *_extra;
};

static const char *const spreg_err_list[] = {
	"",
	"no match", 
	"bad argument",
	"bad option",
	"bad magic",
	"unknown opcode",
	"unknown node",
	"no memory",
	"no substring",
	"match limit",
	"callout",
	"bad utf8",
	"bad utf8 offset",
	"partial",
	"bad partial",
	"internal error",
	"bad count",
	"dfa uitem",
	"dfa ucond",
	"dfa match limit",
	"dfa recurse",
	"recurse limit",
	"null wslimit",
	"bad newline"
};



spreg_match_t *spreg_match_init (int n)
{
	spreg_match_t *re_match;
	re_match = (spreg_match_t *)malloc(sizeof(spreg_match_t)*((n/2)+1)*3);
	return re_match;
}

spreg_t * spreg_init(const char *reg,  
		const char **errinfo,
		int option)
{
	static const char *memory_error = "failed to get memory";
	static const char *reg_null = "reg is NULL";
	char const *err = NULL;
	int erroffset = 0;
	spreg_t *re = NULL;

	if (NULL == reg) {
		err = reg_null;
		goto fail;
	}
	re = (spreg_t *)calloc(sizeof(spreg_t), 1);
	if (NULL == re) {
		err = memory_error;
		goto fail;
	}
	re->_re = pcre_compile(reg, option, &err, &erroffset, NULL);
	if (NULL == re->_re) {
		goto fail;
	}
	re->_extra = pcre_study(re->_re, 0, &err);  
	if (err != NULL) {
		goto fail;
	}
	if (errinfo != NULL) {
		*errinfo = err;
	}
	return re;
	
fail:
	if (errinfo != NULL) {
		*errinfo = err;
	}
	if (re != NULL) {
		if (re->_re != NULL) {
			pcre_free(re->_re);
		}
		if (re->_extra != NULL) {
			pcre_free(re->_extra);
		}
		free(re);
	}
	return NULL;
}


int spreg_search(const spreg_t* re,
		const char* src,  
		int src_size,
		spreg_match_t *rmatch, 
		int n, 
		int option)
{
	if (NULL == re || NULL == src || src_size < 0) {
		return SPREG_NULL;
	}

	int ret = pcre_exec(re->_re, 
			re->_extra, 
			src,
			src_size,
			0, 
			option,
			(int*)rmatch,
			3*n);

	if (0 == ret) {
		ret = n;
	}
	return ret;

}



int spreg_match(const spreg_t* re, 
		const char *src, 
		int src_size,
		spreg_match_t *rmatch, 
		int n,
		int option)
{
	spreg_match_t *res = rmatch;
	int res_n = n;
	int ret = 0;
	if (NULL == re || NULL == src || src_size < 0) {
		return SPREG_NULL;
	}
	if (NULL == rmatch || 0 == n) {
		int capturecount = 0;
		pcre_fullinfo(re->_re, NULL, PCRE_INFO_CAPTURECOUNT, &capturecount);
		res_n = capturecount + 1;
		res = spreg_match_init(res_n);
		if (NULL == res) {
			return SPREG_NOMEMORY;
		}
	}
	ret = spreg_search(re, src, src_size, res, res_n, option);
		
	
	if (!(ret > 0 && 0 == res[0].match_begin && src_size == res[0].match_end)) {
		ret = SPREG_NOMATCH;
	}
	if (NULL == rmatch || 0 == n) {
		spreg_match_destroy(res);
	}
	return ret;

}


int spreg_search_all(const spreg_t *re, 
		const char *src,
		int src_size, 
		spreg_callback_t *callback,
		void *arg,
		int option)
{
	if (NULL == re || NULL == callback || src_size < 0) {
		return SPREG_NULL;
	}
	int n = 0;
	spreg_match_t *res = NULL;
	int capturecount = 0;
	int offset = 0;
	int ret = 0;
	int capn = 0;
	spreg_callback_param_t callback_match;

	pcre_fullinfo(re->_re, NULL, PCRE_INFO_CAPTURECOUNT, &capturecount);
	n = capturecount + 1;
	res = spreg_match_init(n);
	if (NULL == res) {
		return SPREG_NOMEMORY;
	}
	callback_match.src = src;
	callback_match.src_size = src_size;
	callback_match.rmatch = res;
	callback_match.arg = arg;
	do {
		capn = spreg_search(re, src + offset,
		src_size - offset, res, n, option);
		
		if (capn > 0) {
			ret ++;
			callback_match.nrmatch = capn;
			for (int i = 0; i < callback_match.nrmatch; ++i) {
				res[i].match_begin += offset;
				res[i].match_end += offset;
			}

			if (res[0].match_end == res[0].match_begin) {
				offset = res[0].match_end + 1;
			} else {
				offset = res[0].match_end;
			}
			if (callback(&callback_match) < 0) {
				break;
			}
			if (offset >= src_size) break;
		}
	} while (capn > 0);

	spreg_match_destroy(res);
	return ret;
}

int spreg_split(const spreg_t* re,
		const char* src,  
		int src_size,
		spreg_callback_t *callback,
		void *arg,
		int option)
{
	if (NULL == re || NULL == src || NULL == callback || 
		src_size < 0) {
		return SPREG_NULL;
	}
	int n = 0;
	spreg_match_t *res = NULL;
	spreg_callback_param_t callback_match;
	int capturecount = 0;
	int offset = 0;
	int ret = 0;
	int capn = 0;
	pcre_fullinfo(re->_re, NULL, PCRE_INFO_CAPTURECOUNT, &capturecount);
	n = capturecount + 1;
	res = spreg_match_init(n + 1);
	if (NULL == res) {
		return SPREG_NOMEMORY;
	}
	callback_match.src = src;
	callback_match.src_size = src_size;
	callback_match.rmatch = res;
	callback_match.arg = arg;	
	do {
		capn = spreg_search(re, src + offset,
				src_size - offset, (res+1), n, option);


		if (capn > 0) {
			ret ++;
			callback_match.nrmatch = capn + 1;
			for (int i = 1; i < callback_match.nrmatch; ++i) {
				res[i].match_begin += offset;
				res[i].match_end += offset;
			}
			res[0].match_begin = offset;
			res[0].match_end = res[1].match_begin;
			
			if (res[1].match_end == res[1].match_begin) {
				offset = res[1].match_end + 1;
				res[0].match_end ++;
			} else {
				offset = res[1].match_end;
			}
		
			if (callback(&callback_match) < 0) {
				goto end;
			}
			if (offset >= src_size) break;

		}
	} while (capn > 0 && offset < src_size);

	if (offset < src_size) {
		res[1].match_begin = -1;
		res[1].match_end = -1;
		res[0].match_begin = offset;
		res[0].match_end = src_size;
		callback_match.nrmatch = 1;
		ret ++;
		if (callback(&callback_match) < 0) {
			goto end;
		}
	}
end:
	spreg_match_destroy(res);
	
	return ret;
}

typedef struct _replace_type_t {
	int error;
	int limit;
	int last_match;
	const char *replace_string;
	char *dst;
	char *dst_end;
} replace_type_t;

static int replace_call_back(spreg_callback_param_t *callback_match)
{
	replace_type_t *replace = (replace_type_t *)(callback_match->arg);
	
	int match_len = 0;
	int n = callback_match->nrmatch;
	const char *walk = replace->replace_string;
	const char *src = callback_match->src;
	char *buffer = replace->dst;
	char *buffer_end = replace->dst_end;
	spreg_match_t *rmatch = callback_match->rmatch;
	int copy_len = rmatch[0].match_begin - replace->last_match;

	int backref = 0;

	if (copy_len != 0) {
		if (buffer + copy_len < buffer_end) {
			memcpy(buffer, src + replace->last_match, copy_len);
			buffer += copy_len;
		} else {
			replace->error = -1;
		}
	}
	replace->last_match = rmatch[0].match_end;
	while (*walk && buffer + 1 < buffer_end) {
		if ('\\' == *walk) {
			if (walk[1] > '9' || walk[1] < '1') {
				if (walk[1] == '\\') {
					*buffer++ = '\\';
					walk += 2;
				} else {
					++ walk;
				}
				continue;
			}
			backref = walk[1] - '0';
			if (backref < n) {
				match_len = rmatch[backref].match_end - rmatch[backref].match_begin;
				if (buffer + match_len >= buffer_end) {
					match_len = buffer_end - buffer;
					replace->error = -1;
				}
				memcpy(buffer, src + rmatch[backref].match_begin, (size_t)match_len);
				buffer += match_len;
				walk += 2;
				continue;
			}
		}
		*buffer++ = *walk++;
	}
	replace->dst = buffer;
	if (1 == replace->limit) {
		//达到替换的数量限制，直接memcpy
		if (replace->dst + callback_match->src_size - rmatch[0].match_end < buffer_end) {
			memcpy(buffer, src + rmatch[0].match_end,
					(size_t)(callback_match->src_size - rmatch[0].match_end));
			replace->dst += callback_match->src_size - rmatch[0].match_end;
		} else {
			//超过buffer的大小
			replace->error = -1;
		}
		return -1;
	} else if (replace->error < 0) {
		return -1;
	} else if (replace->limit > 1) {
		replace->limit --;
	}
	return 0;
}

int spreg_replace(const spreg_t *re,
		const char *src,
		int src_size,
		const char *replace_string,
		char * dst,
		int dst_size,
		int limit,
		int option)
{
	if (NULL == re || NULL == src || NULL == dst ||
		NULL == replace_string || src_size <= 0 ||
		dst_size <= 0 || limit < 0) {
		return SPREG_NULL;
	}
	replace_type_t replace;
	
	replace.replace_string = replace_string;
	replace.dst = dst;
	replace.dst_end = dst + dst_size;
	replace.error = 0;
	replace.limit = limit;
	replace.last_match = 0;
	int ret = 0;
	ret = spreg_search_all(re,  src, src_size, replace_call_back, &replace, option);

	if (ret > 0) {
		ret = replace.dst - dst;
		if (replace.error < 0) {
			return SPREG_BADBUFFER;
		} else {
			if (src_size != replace.last_match && replace.limit != 1) {
				memcpy(replace.dst, src + replace.last_match, src_size - replace.last_match);
				ret += src_size - replace.last_match;
			}
		}
	} else if (0 == ret) {
		//没有找到要替换的字符串，直接memcpy
		if (src_size > dst_size) {
			ret = SPREG_BADBUFFER;
		} else {
			memcpy(dst, src, (size_t)src_size);
			ret = src_size;
		}
	}
	return ret;
}

int spreg_destroy(spreg_t *re)
{
	if (NULL != re) {
		if (NULL != re->_re) {
			pcre_free(re->_re);
			re->_re = NULL;
		}
		
		if (NULL != re->_extra) {
			pcre_free(re->_extra);
			re->_extra = NULL;
		}
		free(re);
	}
	return 0;
}

int spreg_match_destroy(spreg_match_t *rmatch)
{
	if (NULL != rmatch) {
		free(rmatch);
	}
	return 0;
}

const char *spreg_error(int err)
{
	static const char *unknown = "unknown error code";
	static const char *badbuffer = "buffer is too small";
	err = -err;
	if (err < 0) {
		return unknown;
	}
	if (err >= (int)(sizeof(spreg_err_list)/sizeof(spreg_err_list[0]))) {
		return badbuffer;	
	}
	return spreg_err_list[err];
}
/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
