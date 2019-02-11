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
     * @brief �����л�bufferΪVar����
	 *
	 * Ĭ��ʵ�֣�����������Ҫ�����಻����д�ú�����
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
	
	/* @brief ���Է����л�������֤�ɹ�
	 * 
	 * [NOTICE] �ṩĬ��ʵ�ּ����ϰ汾bslext������Ӧ����д�ýӿڡ�
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
	 * @brief ��ȡ�����л��Ľ�����ú���Ӧ��deserialize()��try_deserialize()�ɹ�ִ�к���á�����Ӧ����Null���������
	 *
	 * [NOTICE] �ṩĬ��ʵ�ּ����ϰ汾bslext������Ӧ����д�ýӿڡ�
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
