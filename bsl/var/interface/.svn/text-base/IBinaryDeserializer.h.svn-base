/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: IBinaryDeserializer.h,v 1.3 2009/06/15 06:29:05 chenxm Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file IBinaryDeserializer.h
 * @author duchuanying(duchuanying@baidu.com)
 * @date 2008/11/01 21:37:08
 * @version $Revision: 1.3 $ 
 * @brief mcpack -> IVar
 *  
 **/


#ifndef  __BSL_VAR_IBINARY_DESERIALIZER_H__
#define  __BSL_VAR_IBINARY_DESERIALIZER_H__

#include <bsl/exception.h>
#include <bsl/var/IVar.h>
#include <bsl/var/Null.h>

namespace bsl{
namespace var{

class IBinaryDeserializer{
public:
    virtual ~IBinaryDeserializer(){}

    /**
     * @brief 反序列化buffer为Var对象
	 *
	 * 默认实现，如无特殊需要，子类不必重写该函数。
     *
     * @param [in] buf   : const void*
     * @param [in] max_size   : size_t
     * @return  IVar& 
     * @retval   
     * @see 
     * @author chenxm
     * @date 2010/12/02 16:58:09
    **/
	virtual IVar& deserialize(const void* buf, size_t max_size){
		size_t res = try_deserialize( buf, max_size );
		if ( res > max_size ){
			throw bsl::OutOfBoundException()<<BSL_EARG<<"buffer too small! max_size["<<max_size<<"] expect[>="<<res<<"]";
		}
		return get_result();
	}
	
	/* @brief 尝试反序列化，不保证成功
	 * 
	 * [NOTICE] 提供默认实现兼容老版本bslext，子类应该重写该接口。
	 * 
	 * @param [in] buf   : const void*
     * @param [in] max_size   : size_t
     * @return  IVar& 
     * @retval   
     * @see 
     * @author zhujianwei
     * @date 2011/03/21 16:58:09
	 */
	virtual size_t try_deserialize( const void* /*buf*/, size_t /*max_size*/){
		throw bsl::NotImplementedException()<<BSL_EARG<<"try_deserialize interface has not been implemented.";
	}

	/**
	 * @brief 获取反串行化的结果，该函数应在deserialize()或try_deserialize()成功执行后调用。否则应返回Null对象的引用
	 *
	 * [NOTICE] 提供默认实现兼容老版本bslext，子类应该重写该接口。
	 * 
	 * @return  IVar& 
	 * @retval   
	 * @see 
	 * @author zhujianwei
	 * @date 2011/03/21 17:22:30
	**/
	virtual IVar& get_result() const{
		throw bsl::NotImplementedException()<<BSL_EARG<<"get_result interface has not been implemented.";
	}
};
}}   //namespace bsl::var


#endif  //__BSL_VAR_IBINARY_DESERIALIZER_H__

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
