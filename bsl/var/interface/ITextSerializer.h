/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: ITextSerializer.h,v 1.3 2009/06/15 06:29:05 chenxm Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file ITextSerializer.h
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/10/07 21:16:20
 * @version $Revision: 1.3 $ 
 * @brief 
 *  
 **/
#ifndef  __BSL_VAR_ITEXT_SERIALIZER_H__
#define  __BSL_VAR_ITEXT_SERIALIZER_H__

#include <bsl/var/IVar.h>
#include <bsl/var/Ref.h>

namespace bsl{
namespace var{

    class ITextSerializer{
    public:
        virtual ~ITextSerializer(){}
        virtual void serialize(const IVar& var, bsl::AutoBuffer& buf) = 0;
    };

}}   //namespace bsl::var

#endif  //__BSL_VAR_ITEXT_SERIALIZER_H__

/* vim: set ts=4 sw=4 sts=4 tw=100 */
