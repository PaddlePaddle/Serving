/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: cc_exception.h,v 1.12 2010/04/13 09:59:41 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file exception.h
 * @author yingxiang(com@baidu.com)
 * @date 2008/12/10 17:49:16
 * @version $Revision: 1.12 $ 
 * @brief 
 *  
 **/


#ifndef  __CC_EXCEPTION_H_
#define  __CC_EXCEPTION_H_

#include "cc_struct.h"
#include "bsl/exception/bsl_exception.h"
#include "ul_error.h"


namespace comcfg{
	enum {
		OK = 0,
		ERROR,// other error
		CONSTRAINT_ERROR,
		CONFIG_ERROR,
		UNKNOWN,
		NOSUCHKEY,
		FORMATERROR,
		OUTOFRANGE,//������Χ
		NULLVALUE, //��ֵ��get_stringʱ���᷵�ش˴���
		NULLBUFFER, //�����bufΪ��
		GROUPTOUNIT, //����group��to_xxx��get_xxx����
		DUPLICATED_KEY, /**<  �޸�configureʱ������ظ�key      */
		DUPLICATED_GROUP,
	};
	/*
	 * @brief configure�׳��������쳣����ConfigException������
	 */
	class ConfigException : public bsl::BasicException<ConfigException, bsl::Exception>{
		public:
			ConfigException(){};
			ConfigException(const char * s){
				msg = s;
			}
			ConfigException(const str_t& s){
				msg = s;
			}
			virtual ~ConfigException() throw(){};
			const char *seeError() throw() {
				return msg.c_str();
			}
			void pushMsg(const str_t & s) throw(){
				msg.append(str_t(" | ")).append(s);
			}
		protected:
			str_t msg;
	};
	class NoSuchKeyException : public bsl::BasicException<NoSuchKeyException, ConfigException>{};
	class FormatErrorException : public bsl::BasicException<FormatErrorException, ConfigException>{};
	class UnknownException : public bsl::BasicException<UnknownException, ConfigException>{};
	class OutOfRangeException : public bsl::BasicException<OutOfRangeException, ConfigException>{};
	class NullValueException : public bsl::BasicException<NullValueException, ConfigException>{};
	class NullBufferException : public bsl::BasicException<NullBufferException, ConfigException>{};
	class GroupToUnitException : public bsl::BasicException<GroupToUnitException, NoSuchKeyException>{};
#if 0
	class DebugException : protected ConfigException {};
	class TraceException : protected DebugException {};
	class NoticeException : protected TraceException {};
	class WarningException : protected NoticeException {};
	class FatalException : protected WarningException {};
#endif
	class ConfigThrower {
		public:
			static void throwException(const ErrCode & err){
				switch(err){
					case OK:
						break;
					case NOSUCHKEY:
						throw NoSuchKeyException().pushf("ConfigException.NoSuchKey: %s", ul_geterrbuf());
						break;
					case OUTOFRANGE:
						throw OutOfRangeException().pushf("ConfigException.OutOfRange: %s", ul_geterrbuf());
						break;
					case FORMATERROR:
						throw FormatErrorException().pushf("ConfigException.FormatError: %s", ul_geterrbuf());
						break;
					case NULLVALUE:
						throw NullValueException().pushf("ConfigException.NullValue: %s", ul_geterrbuf());
						break;
					case NULLBUFFER:
						throw NullBufferException().pushf("ConfigException.NullBuffer: %s", ul_geterrbuf());
						break;
					case GROUPTOUNIT:
						throw GroupToUnitException().pushf("ConfigException.GroupToUnit: %s", ul_geterrbuf());
						break;
					case UNKNOWN:
					default:
						throw UnknownException().push("ConfigException.Unknown");
				}
			}
	};

}







#endif  //__CC_EXCEPTION_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
