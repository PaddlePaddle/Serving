/***************************************************************************
 * 
 * Copyright (c) 2009 Baidu.com, Inc. All Rights Reserved
 * $Id: IRef.h,v 1.2 2010/04/28 12:45:33 scmpf Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file IRef.h
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2009/12/14 18:26:40
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/
#ifndef  __BSL_VAR__IREF_H_
#define  __BSL_VAR__IREF_H_
#include <bsl/var/IVar.h>
namespace bsl{ namespace var {
    class IRef: public IVar{
    public:
        typedef IVar::string_type   string_type;
        typedef IVar::field_type    field_type;
        virtual IVar& ref() const = 0;
        using IVar::operator=;
    };  //end of class 
}}//end of namespace


#endif  //__BSL_VAR__IREF_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
