/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: stl_hashmap.h,v 1.1 2008/09/03 06:47:49 xiaowei Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file stl_hashmap.h
 * @author xiaowei(com@baidu.com)
 * @date 2008/08/26 15:12:10
 * @version $Revision: 1.1 $ 
 * @brief 
 *  
 **/


#ifndef  __STL_HASHMAP_H_
#define  __STL_HASHMAP_H_


#include <hash_map.h>

template <typename key_t, typename value_t>
class xhashmap
{
public:
	hash_map<key_t, value_t> _map;
	int set(const key_t &key, const value_t &val) {
		_map.insert(std::make_pair(key, val));
		return 0;
	}
	int get(const key_t &key, value_t *val) {
		typename hash_map<key_t, value_t>::iterator iter = _map.find(key);
		if (iter == _map.end()) return -1;
		if (val != 0) *val = iter->second;
		return 0;
	}
	int erase(const key_t &key) {
		_map.erase(key);
		return 0;
	}
};


#endif  //__STL_HASHMAP_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
