/***************************************************************************
 * 
 * Copyright (c) 2013 Baidu.com, Inc. All Rights Reserved
 * $Id: GlobalConsFun.h,v 1.6 2013/01/16 11:14:52 linjieqiong Exp $ 
 * 
 **************************************************************************/



/**
 * @file GlobalConsFun.h
 * @author linjieqiong(sat@baidu.com)
 * @date 2013/01/16 01:04:16
 * @version $Revision: 1.6 $ 
 * @brief global constraint function framework
 *  
 **/


#ifndef __GLOBALCONSFUN_H_
#define __GLOBALCONSFUN_H_

#include <string>
#include <vector>
#include <list> 

namespace comcfg{
	//ÃèÊöÒ»¸öÈ«¾ÖÔ¼Êøº¯ÊýµÄ½á¹¹Ìå£¬°üÀ¨£ºº¯ÊýÃû£¬µØÖ·ºÍ²ÎÊýÁÐ±í
	//½á¹¹ÌåÊÇÄ£°å£¬ÊµÀý»¯ºóÊôÓÚ²»Í¬±äÁ¿ÀàÐÍ
	template<typename T>
	struct GlobalConsFun{	
		typedef int (* GCF_T)(const std::vector<T> &arg_vec, ConfigUnit *);
		GlobalConsFun() : _gcf(NULL){}
		GlobalConsFun(const str_t & name, const GCF_T & gcf) 
			: _name(name), _gcf(gcf){}
		GlobalConsFun(const str_t & name, const GCF_T & gcf, const std::vector<T> & arg)
			: _name(name), _gcf(gcf), _arg(arg){}
		
		str_t _name;
		GCF_T _gcf;
		std::vector<T> _arg;
	};
	
	//½«ÉÏÃæ²»Í¬ÀàÐÍµÄ½á¹¹Ìå°ü×°³Éany¶ÔÏó£¬ÒÔ±ãÓÚ´æ·ÅÔÚlist½á¹¹ÖÐ£¬ÊµÏÖ·½·¨²ÎÕÕboost::any
	class gcf_any{
	public:
		class gcf_placeholder;
		template<typename ValueType>
		gcf_any(const ValueType &value) 
			: content(new gcf_holder<ValueType>(value)){}
		gcf_any(const gcf_any & other) 
			: content(other.content ? other.content->clone() : 0){}
		~gcf_any()
		{
			delete content;
		}
		const std::type_info & type()const
		{
			return content ? content->type() : typeid(void);
		}

		class gcf_placeholder
		{
		public:
			virtual ~gcf_placeholder(){}
			virtual const std::type_info &type()const = 0;
			virtual gcf_placeholder * clone()const = 0;
		};

		template<typename ValueType>
		class gcf_holder : public gcf_placeholder
		{
		public:
			gcf_holder(const ValueType &value) : held(value){}
			virtual const std::type_info &type()const
			{
				return typeid(ValueType);
			}
			virtual gcf_placeholder * clone()const
			{
				return new gcf_holder(held);
			}

			ValueType held;
		};

	private:
		template<typename ValueType>
		friend ValueType * gcf_any_cast(gcf_any *);

		gcf_placeholder *content;
	};

	template<typename ValueType>
	ValueType * gcf_any_cast(gcf_any * operand)
	{
		return operand && operand->type() == typeid(ValueType) 
				? &static_cast<gcf_any::gcf_holder<ValueType> *>(operand->content)->held : 0;
	}

	typedef std::list<gcf_any> GCFLIST_T;
}

#endif
