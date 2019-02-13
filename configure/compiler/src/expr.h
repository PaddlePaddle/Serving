/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: expr.h,v 1.1 2008/12/30 03:39:55 yingxiang Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file expr.h
 * @author xiaowei(com@baidu.com)
 * @date 2008/11/21 13:44:15
 * @version $Revision: 1.1 $ 
 * @brief 
 *	直接调用xarthop::calc即可
 *  
 **/


#ifndef  __EXPR_H_
#define  __EXPR_H_

/*
 * <exp> -> <movter> { <move> <movter> }
 * <move> -> << | >>
 * <movter> -> <term> { <addop> <term> }
 * <addop> -> + | -
 * <term> -> <factor> { <mulop> <factor> }
 * <mulop> -> * | \
 * <factor> -> ( <exp> ) | number
 * number -> [0-9]*.[0-9]*
 *
 */

#include <stdio.h>

#define __expr__error(fmt, arg...) \
{ \
	fprintf(stdout, "%s:%d "fmt"\n", __FILE__, __LINE__, ##arg); \
}

namespace xarthop
{

struct int_t {};
struct double_t {};

template <typename T>
struct traits
{
	typedef int_t type;
};

template<>
struct traits<float>
{
	typedef double_t type;
};

template <>
struct traits<double>
{
	typedef double_t type;
};

template <typename type_t>
int exp(const char *, int *, type_t *);

char xget(const char *str, int *pos)
{
	return str[++(*pos)];
}

char xatskip(const char *str, int *pos)
{
	char token = str[*pos];
	while (' ' == token || '\t' == token) {
		token = xget(str, pos);
	}
	return token;
}

template <typename type_t>
inline type_t number(type_t, int_t)
{
	return 0;
}

template <typename type_t>
inline type_t number(type_t t, double_t)
{
	return t;
}


template <typename type_t>
int number(const char *str, int *pos, type_t *val)
{
	char token = xatskip(str, pos);
	type_t temp = 0;
	if (token < '0' || token > '9') {
		__expr__error ("get factor __expr__error from \"%s\" at pos[%d] __expr__error", str, *pos);
		return -1;
	}
	
	while (token >= '0' && token <='9') {
		temp = temp * 10 + token - '0';
		token = xget(str, pos);
	}

	*val = temp;

	if ('.' == token) {
		token = xget(str, pos);
		if (token < '0' || token > '9') {
			__expr__error ("get factor __expr__error from \"%s\" at pos[%d] __expr__error", str, *pos);
			return -1;
		}
		type_t cnt = 1;
		temp = 0;
		while (token >= '0' && token <='9') {
			temp = (token - '0') + temp * 10;
			cnt = cnt * 10;
			token = xget(str, pos);
		}
		typedef typename traits<type_t>::type type;
		*val += number(temp/cnt, type());
	}

	return 0;
}

template <typename type_t>
int factor(const char *str, int *pos, type_t *val)
{
	type_t temp = 0;
	char token = xatskip(str, pos);
	if (token == 0) {
		__expr__error ("get factor err from \"%s\" %d", str, *pos);
		return -1;
	}
	if (token == '(') {
		xget(str, pos);
		if (exp<type_t>(str, pos, &temp) != 0) {
			return -1;
		}
		*val = temp;
		token = xatskip(str, pos);
		if (token != ')') {
			__expr__error ("get exp ) from \"%s\" at pos[%d] __expr__error", str, *pos);
			return -1;
		}
		xget(str, pos);
		return 0;
	}

	if (number(str, pos, &temp) != 0) {
		return -1;
	}

	*val = temp;
	return 0;
}

template <typename type_t>
int term(const char *str, int *pos, type_t *val)
{
	type_t temp = 0;
	if (factor<type_t>(str, pos, &temp) != 0) {
		return -1;
	}
	*val = temp;
	char token = xatskip(str, pos);
	while ('*' == token || '/' == token) {
		xget(str, pos);
		if (factor<type_t>(str, pos, &temp) != 0) {
			return -1;
		}
		switch (token) {
			case '*':
				*val *= temp;
				break;
			case '/':
				*val /= temp;
		}
		token = xatskip(str, pos);
	}
	return 0;
}

template <typename type_t>
int movter(const char *str, int *pos, type_t *val)
{
	type_t temp = 0;
	if (term<type_t>(str, pos, &temp) != 0) {
		return -1;
	}
	*val = temp;
	char token = xatskip(str, pos);
	while ( ('+' == token) || ('-' == token) ) {
		xget(str, pos);
		if (term<type_t>(str, pos, &temp) != 0) {
			return -1;
		}
		switch (token) {
			case '+':
				*val += temp;
				break;
			case '-':
				*val -= temp;
				break;
		}
		token = xatskip(str, pos);
	}
	return 0;
}

template <typename type_t>
int exp_ini(const char *str, int *pos, type_t *val, char token, double_t)
{
	__expr__error("double can't use << | >> operation  from \"%s\" from pos[%d] __expr__error", str, *pos);
	return -1;
}

template <typename type_t>
int exp_ini(const char *str, int *pos, type_t *val, char token, int_t)
{
	type_t temp;
	if (token == '<') {
		token = xget(str, pos);
		if (token != '<') {
			__expr__error("get term from \"%s\" from pos[%d] __expr__error", str, *pos);
			return -1;
		}
		xget(str, pos);
		if (movter<type_t>(str, pos, &temp) != 0) {
			return -1;
		}
		*val = (*val) << temp;
	} else if (token == '>') {
		token = xget(str, pos);
		if (token != '>') {
			__expr__error("get term from \"%s\" from pos[%d] __expr__error", str, *pos);
			return -1;
		}
		xget(str, pos);
		if (movter<type_t>(str, pos, &temp) != 0) {
			return -1;
		}
		*val = (*val) >> temp;
	}
	return 0;
}

template <typename type_t>
int exp(const char *str, int *pos, type_t *val)
{
	typedef typename traits<type_t>::type type;
	type_t temp = 0;
	if (movter<type_t>(str, pos, &temp) != 0) {
		return -1;
	}
	*val = temp;
	char token = xatskip(str, pos);
	while (token == '<' || token == '>') {
		if (exp_ini(str, pos, val, token, type())) {
			return -1;
		}
		token = xatskip(str, pos);
	}
	return 0;
}


/**
 * @brief 
 *
 * @param [in/out] str   : const char* 表达式
 * @param [in/out] val   : type_t*	返回值
 * @return  int 0 表示计算成功, -1 表示失败
 * @retval   
 * @see 
 * @note 
 * @author xiaowei
 * @date 2008/11/21 17:25:38
**/
template <typename type_t>
int calc (const char *str, type_t *val)
{
	if (str == NULL || val == NULL) {
		__expr__error("invalid param");
		return -1;
	}
	if (str[0] == 0) {
		__expr__error("invalid exp in str\n");
		return -1;
	}
	int pos = 0;
	int ret = exp<type_t>(str, &pos, val);
	if (ret != 0) {
		return -1;
	}
	char token = xatskip(str, &pos);
	if (token != 0 &&  token != '\n') {
		__expr__error("invalid exp in \"%s\", at pos[%d] %c", str, pos, token);
		return -1;
	}
	return 0;
}

};


#endif  //__EXPR_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
